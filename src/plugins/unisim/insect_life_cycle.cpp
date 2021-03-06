/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <iostream>
#include <QString>
#include <usbase/parameter.h>
#include <usbase/variable.h>
#include "insect_life_cycle.h"
#include "stage.h"

namespace UniSim{
	
InsectLifeCycle::InsectLifeCycle(Identifier name, QObject *parent)
	: Model(name, parent)
{
    new Variable<double>("eclosedReproductiveAdults", &eclosedReproductiveAdults, this,
                             "Number of individuals just eclosed and entering the reproductive adult stage");
    new Variable<double>("adultsToHibernation", &adultsToHibernation, this,
                             "Number of individuals entering the hibernating adult stage");
    new Variable<double>("deadAdults", &deadAdults, this,
                             "Number of individuals dead from senescence");
}

void InsectLifeCycle::initialize() {
    _stages = seekChildren<Stage*>("*");
    if (_stages.size() < 3)
        throw Exception("There must be at least two Stage models in lifeCycle", this);
}

void InsectLifeCycle::reset() {
    eclosedReproductiveAdults =
    adultsToHibernation =
    deadAdults = 0.;
}

void InsectLifeCycle::update() {
    double transfer = updateStage(_stages[0]);
    int n = _stages.size();
    for (int i = 1;  i < n; ++i) {
        Stage *stage = _stages[i];
        bool isLastStage = (i == n-1);
        bool isLastInstance = !nextInstance();
        if (isLastStage) {
            if (isLastInstance) {
                adultsToHibernation = transfer;
            }
            else {
                eclosedReproductiveAdults = transfer;
                double emergedReproductiveAdults = stage->pullValue<double>("inflow");
                stage->pushValue<double>("inflow", transfer + emergedReproductiveAdults);
            }
        }
        else {
            stage->pushValue<double>("inflow", transfer);
        }
        transfer = updateStage(stage);
    }
    deadAdults = transfer;
}

double InsectLifeCycle::updateStage(Stage *stage) {
    Model *survival = stage->peekOneChild<Model*>("survival");
    if (survival) {
        survival->deepUpdate();
        stage->pushValue<double>("growthRate", survival->pullValue<double>("value"));
    }
    stage->deepUpdate();
    return stage->pullValue<double>("outflow");
}


QList<Stage*>& InsectLifeCycle::stages() {
    return _stages;
}

} //namespace



