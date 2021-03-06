/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <cmath>
#include <iostream>
#include <QMessageBox>
#include <QTextStream>
#include <usbase/clock.h>
#include <usbase/file_locations.h>
#include <usbase/parameter.h>
#include <usbase/variable.h>
#include <usbase/utilities.h>
#include "../unisim/calendar.h"
#include "area.h"
#include "area_density.h"
#include "constants.h"
#include "plant.h"
#include "weather.h"

using namespace UniSim;

namespace intercom{

Area::Area(UniSim::Identifier name, QObject *parent)
	: Model(name, parent)
{
    setRecursionPolicy(Update, ChildrenLast);
    new Parameter<double>("scatteringCoeff", &scatteringCoeff, 0.2, this,
                          "Light scattering coefficient. Usually the default value is used.");
    new Parameter<double>("kDiffuse", &kDiffuse, 0.7, this,
                          "Light extinction coefficient of diffuse light");
    new Parameter<bool>("writeTestOutput", &writeTestOutput, false, this,
                          "Write detailed output? The resulting file has a name that begins with"
                          "\"area_test\" followed by the full name of the @F Area object");
    new Parameter<double>("allocation", &allocation, 0., this,
                             "Allocated dry matter (g per plant per day) to be converted into area");

    new Variable<double>("value", &value, this,
                             "The area of this organ per plant (cm @Sup {2} per plant)");
    new Variable<double>("LAI", &lai, this,
                             "Leaf area index of this organ");
    new Variable<double>("lightAbsorption", &photosynthesisPerDay[Absorption], this,
                             "Light absorbed by this area (W per m @Sup 2 ground per day)");
    new Variable<double>("CO2Assimilation", &photosynthesisPerDay[Assimilation], this,
                             "CO @Sub 2 assimilated by this area (kg CO @Sub 2 per ha ground per day)");
    new Variable<double>("grossProduction", &grossProduction, this,
                             "Carbohydrates produced by this area (kg CH @Sub {2}O per ha ground per day)");
}

void Area::initialize() {
    calendar = seekOne<Model*>("calendar");
    weather = seekOne<Model*>("weather");
    plant = seekOneAscendant<Plant*>("*");
    plantHeightModel = plant->seekOneDescendant<Model*>("height");
    area = seekOneChild<Model*>("area");
    density = seekOneChild<AreaDensity*>("*");
    specificLeafArea = seekOneChild<Model*>("specificLeafArea");
    lightUseEfficiency = seekOneChild<Model*>("lightUseEfficiency");
    assimilationMax = seekOneChild<Model*>("amax");
}

void Area::reset() {
    value = lai =
    photosynthesisPerDay[Absorption] =
    photosynthesisPerDay[Assimilation] =
    grossProduction =
    allocation = 0.;

    if (writeTestOutput) {
        QString path = FileLocations::location(FileLocationInfo::Output).absolutePath();
        QString fileName = "area_test_" + plant->fullName() + ".prn";
        fileName = fileName.replace(QRegExp("[/:\\[\\]]"),"_");
        QString filePath = path + "/" + fileName;
        test.setFileName(filePath);
        bool ok = test.open(QIODevice::Text | QIODevice::WriteOnly);
        if (!ok)
            throw Exception("Could not open file for test output:\""+filePath+"\"");
    }
}

void Area::updateLai() {
    double density = plant->pullValue<double>("density");
    value = area->pullValue<double>("number");
    lai = density*value/10000.;
}

void Area::update() {
    // Fetch current values
    dayLength = calendar->pullValue<double>("dayLength");
    plantHeight = plantHeightModel->pullValue<double>("height");
    double specificLA = specificLeafArea->pullValue<double>("value");

    // Add allocated carbohydrates as area
    double newArea = allocation*specificLA;
	Q_ASSERT(newArea >= 0.);
    area->pushValue<double>("inflow", newArea);
    updateLai();
    allocation = 0.;
}

void Area::cleanup() {
    test.close();
}

void Area::setPoint(int hourPoint_, int heightPoint_) {
    hourPoint = hourPoint_;
    heightPoint = heightPoint_;
    if (hourPoint == 0 && heightPoint == 0) {
        for (int i = 0; i < 2; ++i)
            photosynthesisPerDay[i] = 0.;
    }

    double hour = 12. + 0.5*dayLength*XGAUSS3[hourPoint];
    UniSim::clock()->doTick(hour);
    sinb = calendar->pullValue<double>("sinb");

    height = plantHeight*XGAUSS5[heightPoint];
}

const double * Area::calcELAI() {
    static double elai[3];
    updateExtensionCoeff();
    double densityAbove = density->above(height);
    for (int i = 0; i < 3; ++i)
        elai[i] = k[i]*lai*densityAbove;
    return elai;
}

void Area::updateExtensionCoeff() {
    double scat = sqrt(1 - scatteringCoeff);
    if (sinb > 0.) {
        k[Diffuse] = kDiffuse;
        k[DirectDirect] = 0.5/sinb*kDiffuse/0.8/scat;
        k[DirectTotal] = k[DirectDirect]*scat;
    }
    else {
        k[0] = k[1] = k[2] = 0.;
    }
}

void Area::updatePhotosynthesis(const double *sumELAI) {
    updateReflection();

    double par[3];
    par[Diffuse] = weather->pullValue<double>("parDiffuse");
    par[DirectDirect] =
    par[DirectTotal] = weather->pullValue<double>("parDirect");

    double absorbed[3];
    for (int i = 0; i < 3; ++i) {
        double available = (1. - reflection[i])*par[i]*exp(-sumELAI[i]);
        absorbed[i] = k[i]*available;
    }
    double shaded[2];
    shaded[Absorption] = absorbed[Diffuse] + std::max(absorbed[DirectTotal] - absorbed[DirectDirect], 0.);
    shaded[Assimilation] = assimilation(shaded[Absorption]);

    double sunlit[2] = {0.,0.};
    double perpLight = (sinb == 0.) ? 0. : (1. - reflection[DirectDirect])*par[DirectTotal]/sinb;
    for (int v = 0; v < 3; ++v) {
        double absorbed = shaded[Absorption] + XGAUSS3[v]*perpLight;
        sunlit[Absorption] += WGAUSS3[v]*absorbed;
        sunlit[Assimilation] += WGAUSS3[v]*assimilation(absorbed);
    }

    double fsl = exp(-sumELAI[DirectDirect]);
    double perLA[2];
    for (int i = 0; i < 2; ++i)
        perLA[i] = (1. - fsl)*shaded[i] + fsl*sunlit[i];
    perLA[Absorption] *= 3600*1e-6;


    double densityAt = density->at(height);
    double increment[2];
    for (int i = 0; i < 2; ++i) {
        increment[i] = perLA[i]*lai*densityAt*plantHeight*dayLength*WGAUSS3[hourPoint]*WGAUSS5[heightPoint];
        photosynthesisPerDay[i] += increment[i];
    }
    grossProduction = 30./44.*photosynthesisPerDay[Assimilation];
    if (writeTestOutput) {
        QString s;
        QTextStream str(&s);
        str << calendar->pullValue<int>("dayOfYear") << '\t' << calendar->pullValue<double>("daylength") << '\t'
            << hourPoint << '\t' << heightPoint << '\t'
            << par[Diffuse] << '\t' << par[DirectDirect] << '\t'
            << shaded[Absorption] << '\t' << shaded[Assimilation] << '\t'
            << sunlit[Absorption] << '\t' << sunlit[Assimilation] << '\t'
            << fsl << '\t' << height << '\t' << densityAt << '\t'
            << perLA[0] << '\t' << perLA[1] << '\t'
            << increment[0] << '\t' << increment[1] << '\t'
            << photosynthesisPerDay[0] << '\t' << photosynthesisPerDay[1];
        for (int i = 0; i < 3; ++i) {
            double available = (1. - reflection[i])*par[i]*exp(-sumELAI[i]);
            str << '\t' << available;
        }
        str << '\n';
        test.write(str.string()->toLatin1());
    }
}

void Area::updateReflection() {
    double refHorz = (1 - sqrt(0.8))/(1 + sqrt(0.8));
    double refSphec = refHorz*2./(1. + 1.6*sinb);
    reflection[Diffuse] = refHorz;
    reflection[DirectDirect] = scatteringCoeff;
    reflection[DirectTotal] = refSphec;
}

double Area::assimilation(double absorption) const {
    double efficiency = lightUseEfficiency->pullValue<double>("value");
    double amax = assimilationMax->pullValue<double>("value");
    return amax == 0. ? 0. : amax*(1. - exp(-absorption*efficiency/amax));
}

} //namespace

