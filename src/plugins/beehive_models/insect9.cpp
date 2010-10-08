/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/parameter.h>
#include <usbase/utilities.h>
#include "../unisim_models/stage.h"
#include "insect9.h"  // 1

using namespace UniSim;


namespace beehive{

Insect9::Insect9(UniSim::Identifier name, QObject *parent)  // 2
    : Model(name, parent)
{
    new Parameter<double>("initEggs", &initEggs, 30., this, "Initial number of insect eggs");
    setRecursionPolicy(Component::Update, Component::ChildrenNot);
}

void Insect9::initialize() {
    // Find stages
    egg = seekOneChild<Model*>("egg");
    larva = seekOneChild<Model*>("larva");
    pupa = seekOneChild<Model*>("pupa");
    stages = seekChildren<Stage*>("*");
    Model *adult = seekOneChild<Model*>("adult");
    // Find submodels
    fecundity = adult->seekOneChild<Model*>("fecundity");
    eggSurvival = egg->seekOneChild<Model*>("survival");
    larvaSurvival = larva->seekOneChild<Model*>("survival");
    pupaSurvival = pupa->seekOneChild<Model*>("survival");
}

void Insect9::reset() { // 2
    egg->pushVariable("inflow", initEggs);
}

void Insect9::update() { // 2
    // Development through stages
    stages[0]->deepUpdate();
    for (int i = 1; i < stages.size(); ++i) {
        stages[i]->pushVariable("inflow", stages[i-1]->pullVariable<double>("outflow"));
        stages[i]->deepUpdate();
    }

    // Reproduction
    egg->pushVariable("inflow", fecundity->pullVariable<double>("eggsLaid"));

    // Survival
    applySurvival(egg, eggSurvival);
    applySurvival(larva, larvaSurvival);
    applySurvival(pupa, pupaSurvival);
}

void Insect9::applySurvival(Model *model, Model *survival) { // 2
    // Apply survival as finite growth rate which must be > 0
    double fgr = survival->pullVariable<double>("survival");
    if (fgr == 0.)
        fgr = 1e-6;
    model->pushVariable("growthRate", fgr);
}

} //namespace

