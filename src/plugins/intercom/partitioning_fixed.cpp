/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/parameter.h>
#include "partitioning_fixed.h"

using namespace UniSim;

namespace intercom{

PartitioningFixed::PartitioningFixed(UniSim::Identifier name, QObject *parent)
    : Partitioning(name, parent)
{
}

} //namespace

