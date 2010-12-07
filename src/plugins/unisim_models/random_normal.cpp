/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include "random_normal.h"

namespace UniSim{

RandomNormal::RandomNormal(Identifier name, QObject *parent)
    : RandomBase(name, parent), distribution(0), variate(0)
{
    new Parameter<double>("mean", &mean, 10., this, "Mean of normal distribution");
    new Parameter<double>("sd", &sd, 2., this, "Standard deviation of normal distribution");
}

RandomNormal::~RandomNormal() {
    delete distribution;
    delete variate;
}

void RandomNormal::initialize() {
    RandomBase::initialize();
    distribution = new Distribution(mean, sd);
    variate = new Variate(*(generator->generator()), *distribution);
}

double RandomNormal::drawValue() {
    return (*variate)();
}

} //namespace
