/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/pull_variable.h>
#include "pt_emigration.h"

using namespace UniSim;

namespace grainstore{

PtEmigration::PtEmigration(UniSim::Identifier name, QObject *parent)
	: Model(name, parent)
{
    new Parameter<double>("Ninit", &Ninit, 1., this, "desc");
    new Parameter<double>("K", &K, 1000., this, "desc");
    new Parameter<double>("r", &r, 1.2, this, "desc");
    new PullVariable<double>("N", &density, this, "desc");
}

void PtEmigration::initialize()
{
}

void PtEmigration::reset()
{
	density = Ninit;
}

void PtEmigration::update()
{
	density += (K<=0) ? 0 : density*r*(K-density)/K;
}

} //namespace
