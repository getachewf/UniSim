/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/variable.h>
#include <usbase/test_num.h>
#include "phenology.h"

using namespace UniSim;

namespace intercom{

Phenology::Phenology(UniSim::Identifier name, QObject *parent)
	: Model(name, parent)
{
    setRecursionPolicy(Component::Update, Component::ChildrenNot);
    new Variable<double>("alive", &alive, this,
                             "Proportion of plant population still alive [0;1], "
                             "i.e. the proportion that has not yet developed past the last stage "
                             "defined as child of @F {Phenology}.");
}

void Phenology::amend() {
    stages = seekChildren<Model*>("*");
    int n = stages.size();
    if (n == 0)
        throw Exception("There must be at least one UniSim::Stage child model");

    proportions.fill(0., n);
    for (int i = 0; i < n; ++i) {
        Identifier id = stages[i]->id();
        double *valuePtr = &proportions[i];
        new Variable<double>(id, valuePtr, this,
        "Proportion of plant population in " + id.label() + " stage. "
        "Note: The number and names of these pull variables are defined by the stages "
        "included as children of @F Phenology in the XML model file.");
    }
}

void Phenology::reset() {
    alive = proportions[0] = 1.;
}


void Phenology::update() {
    // Development through stages
    stages[0]->deepUpdate();
    alive = proportions[0] = stages[0]->pullValue<double>("number");
    int n = stages.size();
    for (int i = 1; i < n; ++i) {
        Model *stage = stages[i];
        stage->pushValue("inflow", stages[i-1]->pullValue<double>("outflow"));
        stage->deepUpdate();
        alive += proportions[i] = stage->pullValue<double>("number");
    }

    TestNum::assureLe(alive, 1., "Fraction alive (sum of stages)", this);
    Q_ASSERT(alive >= 0.);
    int checkSum = 0.;
    if (alive > 0.) {
        for (int i = 0; i < n; ++i)
            checkSum += proportions[i] /= alive;
    }
    TestNum::assureLe(alive, 1., "Fraction alive (sum of stages)", this);
    if (checkSum > 0. && !TestNum::eq(checkSum,1.))
        throw Exception("Phenology proportions do not add up to 1: " +
                        QString::number(checkSum), this);
}

} //namespace

