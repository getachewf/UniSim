/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <QMessageBox>
#include <usbase/exception.h>
#include <usbase/pull_variable.h>
#include <usbase/parameter.h>
#include <usbase/test_num.h>
#include "area.h"
#include "organ.h"
#include "plant.h"

using namespace UniSim;

namespace intercom{

Organ::Organ(UniSim::Identifier name, QObject *parent)
	: Model(name, parent)
{
    new Parameter<double>("maintenanceCoeff", &maintenanceCoeff, 0.03, this,
                          "Maintenance coefficient (g/g/day) at 20 {@Degree}C");
    new Parameter<double>("CH2ORequirement", &CH2ORequirement, 1.5, this,
                          "Carbohydrate requirement g CH @Sub 2 O per g biomass");

    new PullVariable<double>("lightAbsorption", &lightAbsorption, this,
                             "Absorbed light (W per m @Sup 2 ground per day)");
    new PullVariable<double>("CO2Assimilation", &CO2Assimilation, this,
                             "Assimilated kg CO @Sub 2 per ha ground per day");
    new PullVariable<double>("maintenanceResp", &maintenanceResp, this,
                             "Maintenance respiration (kg CH @Sub 2 O per ha ground per day)");
    new PullVariable<double>("growthResp", &growthResp, this,
                             "Growth respiration (kg CH @Sub 2 O per ha ground per day)");
    new PullVariable<double>("allocatedPerPlant", &allocatedPerPlant, this,
                             "Net biomass allocated to this organ per plant (g per plant per day)");
    new PullVariable<double>("propAllocatedPerPlant", &propAllocatedPerPlant, this,
                             "Proportion biomass allocated to this organ among all the plant's organs [0;1]");
}

void Organ::initialize() {
    if (TestNum::lt(CH2ORequirement, 1.))
        throw Exception("Parameter CH2ORequirement must be >= 1");
    weather = seekOne<Model*>("weather");
    plant = seekOneAscendant<Plant*>("*");
    mass = seekOneChild<Model*>("mass");
    area = peekOneChild<Model*>("area");
}

void Organ::reset() {
    lightAbsorption =
    CO2Assimilation =
    maintenanceResp =
    growthResp = 0.;
}

void Organ::accumulate() {
    updateMaintenanceRespiration();

    if (area) {
        lightAbsorption = area->pullVariable<double>("lightAbsorption");
        CO2Assimilation = area->pullVariable<double>("CO2Assimilation");;
    }
}

void Organ::updateMaintenanceRespiration() {
    double temp = weather->pullVariable<double>("Tavg");
    double resp20 = maintenanceCoeff*mass->pullVariable<double>("value");
    resp20 = plant->gPerPlant_to_kgPerHa(resp20);
    maintenanceResp = resp20*pow(2., (temp - 20.)/10.);
}

double Organ::allocate(double proportion, double totalCarbohydrates) {
    propAllocatedPerPlant = proportion;
    double myShare = proportion*totalCarbohydrates;
    double netCarbohydrates = myShare/CH2ORequirement;
    return doAllocate(myShare, netCarbohydrates);
}

double  Organ::allocateNet(double proportion, double netCarbohydrates) {
    propAllocatedPerPlant = proportion;
    double myShare = proportion*netCarbohydrates;
    double totalCarbohydrates = myShare*CH2ORequirement;
    return doAllocate(totalCarbohydrates, myShare);
}

double Organ::doAllocate(double totalCarbohydrates, double netCarbohydrates) {
    TestNum::assureGeZero(totalCarbohydrates, "totalCarbohydrates", this);
    TestNum::assureGeZero(netCarbohydrates, "netCarbohydrates", this);

    growthResp = totalCarbohydrates - netCarbohydrates;
    allocatedPerPlant = plant->kgPerHa_to_gPerPlant(netCarbohydrates);

    mass->pushVariable<double>("allocation", allocatedPerPlant);
    if (area)
        area->pushVariable<double>("allocation", allocatedPerPlant);
    return allocatedPerPlant;
}


} //namespacee

