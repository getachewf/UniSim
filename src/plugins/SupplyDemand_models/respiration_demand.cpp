/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/pull_variable.h>
#include "respiration_demand.h"
#include "life_stage.h"

using namespace UniSim;


namespace SupplyDemand {

RespirationDemand::RespirationDemand(UniSim::Identifier name, QObject *parent)
	: Model(name, parent)
{
    new Parameter<double>("resp20", &respRate, 0.1, this, "Respiration rate (g/g per time step) at 20 {@Ring}C");
    new PullVariable<double>("value", &value, this, "Respiration during this time step (g)");
}

void RespirationDemand::initialize() {
    Model *stage = seekNearestAscendant<LifeStage*>("*");
    Model *massModel = stage->seekOneDescendant<Model*>("lifetable/mass");
    mass = massModel->pullVariablePtr<double>("value");

    Model *weather = seekOne<Model*>("weather");
    temp = weather->pullVariablePtr<double>("Tavg");
}

void RespirationDemand::reset() {
    update();
}

void RespirationDemand::update() {
    value = *mass*respRate*pow(2., (*temp - 20.)/10.);
}

} //namespace

