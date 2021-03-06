/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UNISIM_INSTANCE_INDEX_FROM_TABLE_H
#define UNISIM_INSTANCE_INDEX_FROM_TABLE_H

#include <QVector>
#include "instance_index.h"

namespace UniSim{

class Model;
	
class InstanceIndexFromTable : public InstanceIndex
{
public:
    InstanceIndexFromTable(QString filePath);
    void reset(QString modelType, Model *parent);
    bool hasNext();
    Record next();

private:
    int nextIndex, columnIndexOfModelType;
    QVector<int> indices;

    void useAllRows();
    void setColumnIndexOfModelType(QString modelType);
};

} //namespace

#endif
