/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include "mussel_bed_factory.h"
#include "mussel.h"
#include "mussel_growth_rate.h"
#include "temperature_scale.h"
#include "salinity_scale.h"
#include "starfish.h"
#include "starfish_feeding_rate.h"
#include "starfish_growth_rate.h"
#include "temperature_stscale.h"
#include "mussel_loss_rate.h"

using namespace UniSim;

namespace mussel_bed{

void Mussel_bedFactory::defineProducts() {

    addProduct<Mussel>("Mussel", this,
    "Mussel population model");

    addProduct<MusselGrowthRate>("MusselGrowthRate", this,
    "Mussel population growth rate model");

    addProduct<TemperatureScale>("TemperatureScale", this,
    "Scaling of growth rate by temperature. Yields a number [0;1]");

    addProduct<SalinityScale>("SalinityScale", this,
    "Scaling of growth rate by Salinity. Yields a number [0;1]");

    addProduct<Starfish>("Starfish", this,
    "starfish population model");

    addProduct<StarfishFeedingRate>("StarfishFeedingRate", this,
    "Scaled Feeding rate of starfish");

    addProduct<StarfishGrowthRate>("StarfishGrowthRate", this,
    "transform feeding rate into new starfish biomass");

    addProduct<TemperatureStScale>("TemperatureStScale", this,
    "Scaling of feeding rate of starfish by Temperature. Yields a number [0;1]");

    addProduct<MusselLossRate>("MusselLossRate", this,
    "Mussel biomass reduction rate");
}

UniSim::Identifier Mussel_bedFactory::id() const {
    return "mussel_bed";
}

QString Mussel_bedFactory::description() const {
    return
    "The mussel_bed plugin includes models for creatures on a mussel bed";
}

QStringList Mussel_bedFactory::authors() const {
    return QStringList()
        << "Niels & Antonio";

}

QObject* Mussel_bedFactory::asQObject() {
   return this;
}

#if QT_VERSION < 0x50000
Q_EXPORT_PLUGIN2(mussel_bed_factory, Mussel_bedFactory)
#endif

}
 //namespace

