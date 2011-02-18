/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/object_pool.h>
#include <usbase/utilities.h>
#include "output_plot.h"
#include "output_table.h"
#include "unisim_output_maker.h"

namespace UniSim{

UniSim::Identifier UniSimOutputMaker::pluginName() const {
    return "UniSim";
}

QString UniSimOutputMaker::pluginDesc() const {
    return
    "The @F UniSim plugin contains a collection of output classes of general utility.";
}

QStringList UniSimOutputMaker::authors() const {
    return QStringList()
        << "Niels Holst, Aarhus University, Denmark";
}

const QMap<Identifier, QString>& UniSimOutputMaker::supportedClasses() {
    if (!desc.isEmpty())
        return desc;

    desc["Plot"] =
    "@I pending";

    desc["Table"] =
    "@I pending";

    desc["SummaryPlot"] =
    "@I pending";

    return desc;
}

void UniSimOutputMaker::useObjectPool(ObjectPool *pool) const {
    objectPool()->deferTo(pool);
}

Output* UniSimOutputMaker::create(Identifier outputType, Identifier objectName, QObject *parent)
{
	// Remember to add integratorType to the list above as well
    UniSim::setSimulationObjectFromDescendent(parent);
    Output *output = 0;
    /*if (outputType.equals("ParameterTable"))
        output = new ParameterTable(objectName, parent);
    else*/ if (outputType.equals("Plot"))
        output = new OutputPlot(objectName, parent);
    else if (outputType.equals("Table"))
        output = new OutputTable(objectName, parent);
    return output;
}

Q_EXPORT_PLUGIN2(unisim_output_maker,UniSimOutputMaker)

} //namespace
