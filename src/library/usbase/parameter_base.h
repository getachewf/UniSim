/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UNISIM_PARAMETER_BASE_H
#define UNISIM_PARAMETER_BASE_H

#include <QObject>
#include "identifier.h"

namespace UniSim{

class ParameterBase : public QObject
{
    // no Q_OBJECT
public:
    ParameterBase(Identifier id, QObject *parent, QString desc);
    virtual QVariant toVariant() const = 0;
    virtual QString typeId() const = 0;
    virtual void setValueFromString(QString newValue) = 0;
    virtual void followRedirection() = 0;
    Identifier id() const;
    QString description() const;
protected:
    void assertUniqueness(Identifier id, QObject *parent);

private:
    Identifier _id;
    QString _description;
};

} //namespace

#endif

