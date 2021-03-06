/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <limits>
#include <usbase/exception.h>
#include <usbase/object_pool.h>
#include <usbase/parameter.h>
#include <usbase/variable.h>
#include "random_base.h"

namespace UniSim{

RandomBase::RandomBase(Identifier name, QObject *parent)
	: Model(name, parent)
{
    new Parameter<double>("minValue", &minValue, -std::numeric_limits<double>::max(), this,
                          "Minimum random value");
    new Parameter<double>("maxValue", &maxValue, std::numeric_limits<double>::max(), this,
                          "Maximum random value");
    new Parameter<int>("maxTries", &maxTries, 100, this,
                       "Maximum number of tries to find a number inside the interval "
                       "@F minValue to @F {maxValue}. An exception is cast if exceeded");
    new Variable<double>("value", &value, this,
                             "Random value");
}

void RandomBase::initialize() {
    trigger = peekOneNearest<Model*>("trigger");
}

void RandomBase::reset() {
    nextValue();
    emit event(this, "reset");
}

void RandomBase::update() {
    if (triggered()) {
        nextValue();
        emit event(this, "update");
    }
}

bool RandomBase::triggered() {
    return trigger && trigger->pullValue<bool>("value");
}

void RandomBase::nextValue() {
    int i = 0;
    do {
        value = drawValue();
        if (++i == maxTries) {
            QString msg = "Max number of tries exceeded to find random value inside interval: %1 to %2";
            throw Exception(msg.arg(minValue).arg(maxValue), this);
        }
    } while (value < minValue || value > maxValue);
}

} //namespace
