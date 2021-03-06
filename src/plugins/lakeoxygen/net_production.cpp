/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/output.h>
#include "usbase/parameter.h"
#include "lake_environment.h"
#include "net_production.h"
#include "oxygen_concentration.h"
#include "math.h"

using namespace UniSim;


namespace lakeoxygen {

NetProduction::NetProduction(UniSim::Identifier name, QObject *parent)
	: Model(name, parent)
{
    new Parameter<double>("K", &K, 1., this,
    "Reaeration Constant estimated from stochastic sampling (m/day)");

    new Parameter<double>("Ksys", &Ksys, 0.22, this,
    "Reaeration Constant for lemming system (m/d)");

    new Parameter<double>("unit", &unit, 0.375, this,
    "Converts between Oxygen and Carbon as Net production unit");

    new Parameter<int>("layers", &layers, 3, this,
    "Number of layers to subdivide the water column vertically");

    new Parameter<double>("maxDepth", &maxDepth, 15., this,
    "Maximum depth of lake (m)");


    new Variable<double>("DOObsSatPct", &doObsSatPct, this,
    "Observed dissolved oxygen concentration as percentage of saturation");

    new Variable<double>("Kwind", &Kwind, this,
    "Reaeration coefficent estimated from wind speed (m/day)");

    new Variable<double>("netProd", &netProd, this,
    "Net community production from stochastic sampling (O{@Sub 2} (or C)/m @Sup 2 /d)");

    new Variable<double>("netProdWind", &netProdWind, this,
    "Net community production from wind speed (O{@Sub 2} (or C)/m @Sup 2 /d) ");

    new Variable<double>("layerHeight", &layerHeight, this,
    "Height of water layer (m)");

    new Variable<double>("sumStochastic", &sumStochastic, this,
    "Calculated light at a certain depth ({@Sym mu}mol/m @Sup 2/d)");

    new Variable<double>("sumWind", &sumWind, this,
    "Calculated light at a certain depth ({@Sym mu}mol/m @Sup 2/d)");

    new Variable<double>("light", &light, this,
    "Calculated light at a certain depth ({@Sym mu}mol/m @Sup 2/d)");

}

void NetProduction::amend() {
    CreateLayers(); //Was moved from the constructor to ensure that the xml file has been read.
}

void NetProduction::initialize() {

    weather = seekOne<Model*>("weather");
    calendar = seekOne<Model*>("calendar");
    lakeEnvironments = seekMany<LakeEnvironment*>("*");
    oxygenConcentration = seekOne<Model*>("OxygenConcentration");
}


void NetProduction::reset() {
    sumStochastic = 0;
    sumWind = 0;
}

void NetProduction::update() {
    // Pull the variables from the models then do calculations
    double dayLength = calendar->pullValue<double>("Daylength");
    double obsDO = weather->pullValue<double>("obsDO");
    double DLI = weather->pullValue<double>("obsDLI");
    double wind = weather->pullValue<double>("wind");
    double doSatConc = oxygenConcentration->pullValue<double>("DOsatConc");


    // Ensure that division is not by zero
    if (doSatConc == 0)
        doObsSatPct = 100;
    else
        doObsSatPct = (obsDO/doSatConc)*100;
    Kwind = 0.728*sqrt(wind) - 0.317*wind + 0.0372*wind*wind;

    /* Define the Reaeration coefficient if it should be calculated from wind speed,
    ** or if it is measured directly for the lake and held constant.
    ** Ku10 (Reaeration constant based on wind speed in 10 m height) final unit [m/d]*/
    if  (K == 0)
        Ku10 = 0.2*wind/Ksys;
    else
        Ku10 = K/Ksys;

    // Change the unit calculation between Carbon units and Oxygen units
    if  (unit == 0)
        unit = 1;
    else
        unit = 0.375;

    /* Calculate the Net primary production from the Oxygen saturation, DLI and Daylength
    ** The Net Primary Production is corrected for units and for average depth of the lake.
    ** The unit of the Net Primary production is [g Unit/m3/day]*/
    netProd = (-0.787 + 0.014*doObsSatPct + 0.022*DLI - 0.081*dayLength)*Ku10*unit;
    netProdWind = (-0.787 + 0.014*doObsSatPct + 0.022*DLI -0.081*dayLength)*Kwind/Ksys*unit;

    QDate today = calendar->pullValue<QDate>("date");
    if (today.day() == 1 && today.month() == 1) {
        sumStochastic = 0;
        sumWind = 0;
    }
    sumStochastic += netProd;
    sumWind += netProdWind;
}

void NetProduction::CreateLayers() {
    for (int i = 0; i < layers; ++i){
        QString Name = "Layer" + QString::number(i+1);
        Model *layer= new LakeEnvironment(Name,this);  //new creates it in the Heap
        layer->deepInitialize();

        Parameter<double> *avgDepth = layer->seekOneChild<Parameter<double>*>("AvgDepth");

        double layerheight = maxDepth/layers - i;
        avgDepth->setValue(layerheight);
    }
}


} //namespace

