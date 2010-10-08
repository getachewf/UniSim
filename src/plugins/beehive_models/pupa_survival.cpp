/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/parameter.h>
#include <usbase/pull_variable.h>
#include "pupa_survival.h"

using namespace UniSim;


namespace beehive{

PupaSurvival::PupaSurvival(UniSim::Identifier name, QObject *parent)
    : Model(name, parent)
{
    new Parameter<double>("T0", &T0, 0., this,
    "Zero survival below this temperature");

    new Parameter<double>("Topt", &Topt, 25, this,
    "Optimal survival (Sopt) at this temperature");

    new Parameter<double>("Tmax", &Tmax, 40., this,
    "Zero survival above this temperature");

    new Parameter<double>("Sopt", &Sopt, 1., this,
    "Optimal survival [0;1]");

    new PullVariable<double>("survival", &survival, this,
    "Survival [0;1] depending on current temperature");
}

void PupaSurvival::initialize() {
    weather = seekOne<Model*>("weather");
}

void PupaSurvival::reset() {
    survival = 1;
}

void PupaSurvival::update() {
    double temp = weather->pullVariable<double>("Tavg");
    survival = calcSurvival(temp);
}

double PupaSurvival::calcSurvival(double temp) const {
    double prop;
    if (temp < T0)
        prop = 0.;
    else if (temp < Topt && Topt != T0)
        prop = (temp - T0)/(Topt - T0);
    else if (temp < Tmax && Tmax != Topt)
        prop = (Tmax - temp)/(Tmax - Topt);
    else
        prop = 0.;
    return prop*Sopt;

}
} //namespace

