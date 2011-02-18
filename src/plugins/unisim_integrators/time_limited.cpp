/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <iostream>
#include <QString>
#include <usbase/model.h>
#include <usbase/parameter.h>
#include <usbase/pull_variable.h>
#include <usbase/utilities.h>
#include "time_limited.h"

namespace UniSim{
	
TimeLimited::TimeLimited(Identifier name, QObject *parent)
    : Integrator(name, parent)
{
    new Parameter<double>("maxTime", &maxTime, 30., this, "description");
}

void TimeLimited::initialize() {
    Integrator::initialize();
    time = UniSim::seekOneChild<Model*>("time", this);
}

bool TimeLimited::nextStep()
{
    return time->pullVariable<double>("total") >= maxTime;
}

} //namespace

