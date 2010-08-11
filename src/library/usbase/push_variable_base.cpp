/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/

#include "push_variable_base.h"

namespace UniSim{


PushVariableBase::PushVariableBase(Identifier id, QObject *parent, QString description)
    : QObject(parent), _id(id), _description(description)
{
    setObjectName(id.key());
}

Identifier PushVariableBase::id() const {
    return _id;
}

QString PushVariableBase::description() const {
    return _description;
}


} //namespace


