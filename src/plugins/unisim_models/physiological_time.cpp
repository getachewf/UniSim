/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/exception.h>
#include <usbase/pull_variable.h>
#include <usbase/time.h>
#include "physiological_time.h"

namespace UniSim{

PhysiologicalTime::PhysiologicalTime(UniSim::Identifier name, QObject *parent)
    : Model(name, parent)
{
    new PullVariable<double>("step", &step, this,
        "Duration of latest time step (physiological time units)");
    new PullVariable<double>("total", &total, this,
        "Total duration since beginning of simulation (in physiological time units) "
        "or since most recent trigger event (see e.g. @F TriggerByDate)");
}

void PhysiologicalTime::initialize() {
    calendar = seekOne<Model*>("calendar");
    calendarTimeStep = calendar->parameter<int>("timeStep");
    Time::Unit unit = Time::charToUnit( calendar->parameter<char>("timeUnit") );
    calendarTimeStep /= Time::conversionFactor(unit, Time::Days);

    static bool always = true;
    Model *trigger = peekOneChild<Model*>("trigger");
    triggered = trigger ? trigger->pullVariablePtr<bool>("value") : &always;
}

void PhysiologicalTime::reset() {
    step = total = 0.;
}

void PhysiologicalTime::update() {
    if (*triggered) {
        step = calcDailyTimeStep()*calendarTimeStep;
        total += step;
    }
    else {
        step = total = 0.;
    }
}

} //namespace

