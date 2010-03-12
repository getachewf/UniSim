/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include "weather.h"

using namespace UniSim;


namespace conductance {

Weather::Weather(UniSim::Identifier name, QObject *parent)
	: Model(name, parent) {
    setState("irradiation", &irradiation);
    setState("Tavg", &Tavg);
}

void Weather::initialize() {
    setParameter("irradiation", &irradiation, 10.);
    setParameter("Tavg", &Tavg, 15.);
}

void Weather::reset() {
}

void Weather::update() {
}

} //namespace
