/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UNISIM_PARAMETER_BASE_H
#define UNISIM_PARAMETER_BASE_H

#include <QObject>
#include <QVariant>
#include "identifier.h"
#include "variable_base.h"

namespace UniSim{

class ParameterBase : public virtual VariableBase
{
    //Q_OBJECT
public:
    ParameterBase(Identifier id, QObject *parent, QString desc) :
        VariableBase(id, parent, desc) { }
    virtual void setValueFromString(QString newValue) = 0;
    virtual void followRedirection() = 0;
};

} //namespace

#endif

