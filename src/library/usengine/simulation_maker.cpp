/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <iostream>
#include <typeinfo>
#include <QFile>
#include <QHashIterator>
#include <QMessageBox>
#include <QTextStream>
#include <QXmlStreamReader>
#include <usbase/data_grid.h>
#include <usbase/model.h>
#include <usbase/integrator.h>
#include <usbase/exception.h>
#include <usbase/output.h>
#include <usbase/trace.h>
#include <usbase/parameter.h>
#include <usbase/parameter_base.h>
#include <usbase/parameter_vector.h>
#include <usbase/variable.h>
#include <usbase/variable_base.h>
#include <usbase/variable_vector.h>
#include <usbase/utilities.h>
#include "instance_index_from_condensed_table.h"
#include "instance_index_from_table.h"
#include "instance_index_numbered.h"
#include "instance_index_one.h"
#include "mega_factory.h"
#include "parameter_index_from_condensed_table.h"
#include "parameter_index_from_table.h"
#include "simulation.h"
#include "simulation_maker.h"
#include "xml_node.h"

namespace UniSim{

namespace {
    inline QList<QObject*> asList(QObject * object) {
        return (QList<QObject*>() << object);
    }

    template <class T> void amend() {
        QList<T> components = seekChildren<T>("*", simulation());
        QListIterator<T>  i(components);
        while (i.hasNext()) {
            i.next()->deepAmend();
        }
    }
}

SimulationMaker::SimulationMaker()
	: QObject()
{
    reader = new QXmlStreamReader;
    instanceTables.clear();
    parameterTables.clear();
}

SimulationMaker::~SimulationMaker()
{
    delete reader;
}

bool SimulationMaker::nextElementDelim()
{
    QString myTest = elementName();
    bool unusedElement;
    if (moreToRead()) {
		do {
			reader->readNext();
            myTest = elementName();
            bool commonElement = reader->isStartElement() && elementNameEquals("common");

            unusedElement = commonElement || !(reader->isStartElement() || reader->isEndElement());
            if (commonElement)
                ignoreElement();
        } while (unusedElement && moreToRead());
	}
	return reader->isStartElement() || reader->isEndElement();
}

bool SimulationMaker::moreToRead() {
    return !reader->hasError() && !reader->isEndDocument();
}

void SimulationMaker::ignoreElement() {
    int levels = 1;
    while (levels>0  && moreToRead()) {
        reader->readNext();
        if (reader->isStartElement())
            ++levels;
        else if (reader->isEndElement())
            --levels;
    }
}

Simulation* SimulationMaker::parse(FileLocationInfo::FileType fileType, QString subFolder, QString fileName) {
    QDir dir = FileLocations::location(fileType);
    QString filePath = dir.absolutePath() + "/" + subFolder + "/" + fileName;
    return parse(filePath);
}

Simulation* SimulationMaker::parse(QString filePath)
{
    QString simName;

    redirectedParameters.clear();
    traceVariableParam.clear();
    clearTables();

    emit beginExpansion();
    fileName = compileToFile(filePath);
    emit endExpansion();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QString msg("Cannot open file '%1' for reading.");
        throw Exception(message(msg.arg(fileName)));
    }
	reader->setDevice(&file);

    if (!nextElementDelim())
        throw Exception(message("File is not in valid XML format"));
    if (elementNameNotEquals("simulation"))
        throw Exception(message("Root element must be 'simulation'"));

    simName = attributeValue("name", "anonymous");
	
    Simulation *sim = new Simulation(simName);
    UniSim::setSimulationObject(sim);
    sim->setFilePath(filePath);

    nextElementDelim();
    int numIntegrators(0);
    bool noModels(true), noOutputs(true);
    while (!reader->hasError() && reader->isStartElement()) {
        if (elementNameEquals("integrator")) {
            readIntegratorElement(sim);
            ++numIntegrators;
        } else if (elementNameEquals("model")) {
            readModelElement(asList(sim));
            noModels = false;
        } else if (elementNameEquals("output")) {
            readOutputElement(sim);
            noOutputs = false;
		} else {
            QString msg("Unexpected element: '%1'");
            throw Exception(message(msg.arg(elementName())), sim);
		}
	}
	Q_ASSERT(reader->isEndElement());
    if (reader->hasError())
        throw Exception(message(""));
    if (numIntegrators == 0)
        throw Exception(message("Missing 'integrator' element in 'simulation'"));
    if (numIntegrators > 1)
        throw Exception(message("Only one 'integrator' element allowed in 'simulation'"));
    if (noModels)
        throw Exception(message("Missing 'model' element in 'simulation'"));
    if (noOutputs)
        throw Exception(message("Missing 'output' element in 'simulation'"));

    amend<Integrator*>();
    amend<Model*>();
    redirectParameters();
    createTraces();
    amend<Output*>();

    reader->clear();
    clearTables();
    return sim;
}

void SimulationMaker::clearTables() {
    qDeleteAll(instanceTables);
    qDeleteAll(parameterTables);
    instanceTables.clear();
    parameterTables.clear();
}

QString SimulationMaker::compileToFile(QString filePath) {
    QDir dir = FileLocations::location(FileLocationInfo::Temporary);
    QString fileName = QFileInfo(filePath).baseName();
    fileName += "_compiled.xml";
    QString compiledFilePath = dir.absolutePath() + "/" + fileName;

    XmlNode *original = XmlNode::createFromFile(filePath);
    original->compile(filePath);
    original->writeToFile(compiledFilePath);
    delete original;
    return compiledFilePath;
}

void SimulationMaker::readIntegratorElement(QObject* parent) {
	Q_ASSERT(reader->isStartElement() && parent);
	
    QString type = attributeValue("type", parent);
    QString name = attributeValue("name", "anonymous");

    Integrator *integrator = MegaFactory::create<Integrator>(type, name, parent);

	nextElementDelim();
	while (!reader->hasError() && reader->isStartElement()) {
        if (elementNameEquals("sequence")) {
			readSequenceElement(integrator);
        }
        else if (elementNameEquals("parameter")){
            readParameterElement(asList(integrator));
        }
        else if (elementNameEquals("model")){
            readModelElement(asList(integrator));
        }
        else {
            QString msg("Unexpected element: '%1'");
            throw Exception(message(msg.arg(elementName())), parent);
		}
	}	
	Q_ASSERT(reader->isEndElement());
	nextElementDelim();
}	

void SimulationMaker::readSequenceElement(QObject* parent)
{
	Q_ASSERT(reader->isStartElement() && parent);

	nextElementDelim();
	while (!reader->hasError() && reader->isStartElement()) {
        if (elementNameEquals("model")) {
            QString model = attributeValue("name", parent);
            // deprecated: sequence.append(model);
        }
        else {
            QString msg("Unknown child element of 'sequence' element: '%1'");
            throw Exception(message(msg.arg(elementName())), parent);
		}
		nextElementDelim();
		Q_ASSERT(reader->isEndElement());
		nextElementDelim();
	}	
	Q_ASSERT(reader->isEndElement());

	nextElementDelim();
}

void SimulationMaker::readModelElement(QList<QObject*> parents) {
    Q_ASSERT(!parents.isEmpty());
    Q_ASSERT(reader->isStartElement());

    QString modelType = attributeValue("type", "Anonymous");
    QString parameterName = attributeValue("parameter", "");
    QString context = !parameterName.isEmpty() ? parameterName : modelType;

    InstanceIndex *table = createIndexTable();
    QList<QObject*> models;
    for (int i = 0; i < parents.size(); ++i) {;
        QObject *parent = parents[i];
        table->reset(context, dynamic_cast<Model*>(parent));
        models << createModelElement(table, parent);
    }

	nextElementDelim();
    if (!models.isEmpty()) {
        while (!reader->hasError() && reader->isStartElement()) {
            if (elementNameEquals("model"))
                readModelElement(models);
            else if (elementNameEquals("parameter"))
                readParameterElement(models);
            else {
                QString msg("Unexpected element: '%1'");
                throw Exception(message(msg.arg(elementName())));
            }
        }
    }
	Q_ASSERT(reader->isEndElement());
	nextElementDelim();
}

InstanceIndex* SimulationMaker::createIndexTable() {
    QString objectName = attributeValue("name", "");
    QString table = attributeValue("table", "");
    QString crosstab = attributeValue("crosstab", "");
    QString instancesStr = attributeValue("instances", "");
    QString parameterName = attributeValue("parameter", "");
    bool hasTable = !table.isEmpty();
    bool hasCrosstab = !crosstab.isEmpty();
    bool hasParameterName = !parameterName.isEmpty();
    bool hasInstances = !instancesStr.isEmpty();

    if (int(hasTable) + int(hasCrosstab) + int(hasInstances) > 1)
        throw Exception("Set only one of these: 'instances', 'table' and 'crosstab'");
    if (hasParameterName && !hasCrosstab) {
        QString msg("If you set the 'parameter' attribute (to '%1') you must also set the 'crosstab' attribute");
        throw Exception(message(msg.arg(parameterName)));
    }
    if (hasCrosstab && !hasParameterName) {
        QString msg("If you set the 'crosstab' attribute (to '%1') you must also set the 'parameter' attribute");
        throw Exception(message(msg.arg(crosstab)));
    }

    int numInstances(0);
    if (hasInstances) {
        bool ok(true);
        numInstances = instancesStr.toInt(&ok);
        if (!ok || numInstances <= 0)
            throw Exception(message("Attribute 'instances'' must a number larger than zero"));
    }

    if (hasInstances)
        return new InstanceIndexNumbered(objectName, numInstances);
    else if (!(hasTable || hasCrosstab))
        return new InstanceIndexOne(objectName);

    QString fileName = hasTable ? table : crosstab;
    QString filePath = simulation()->inputFilePath(fileName);
    if (instanceTables.contains(filePath))
        return instanceTables.value(filePath);

    if (hasParameterName)
        return instanceTables[filePath] = new InstanceIndexFromCondensedTable(filePath);
    else
        return instanceTables[filePath] = new InstanceIndexFromTable(filePath);
}

QList<QObject*> SimulationMaker::createModelElement(InstanceIndex *table, QObject *parent) {
    QString modelType = attributeValue("type", "Anonymous");
    QString hide = attributeValue("hide", "");
    QList<QObject*> models;
    Models instances;

    try {
        while (table->hasNext()) {
            InstanceIndex::Record rec = table->next();
            Model *model = MegaFactory::create<Model>(modelType, rec.modelName, parent);

            models << model;
            instances << model;
            int numParam = rec.paramNameValue.size();
            for (int i = 0;i < numParam; ++i) {
                QString name = rec.paramNameValue.keys().at(i);
                QString value = rec.paramNameValue.values().at(i);
                ParameterBase *parameter = seekOneChild<ParameterBase*>(name, model);
                parameter->setValueFromString(value);
            }
            if (!hide.isEmpty()) {
                bool isHidden = UniSim::stringToValue<bool>(hide);
                model->setHide(isHidden);
            }
        }
    }
    catch (Exception &ex) {
        throw Exception(message(ex.message()), parent);
    }
    int numInstances = instances.size();
    for (int i = 0; i < numInstances; ++i) {
        Model *prev = (i==0) ? 0 : instances[i-1];
        Model *next = (i<numInstances-1) ? instances[i+1] : 0;
        instances[i]->setInstances(prev, next);
    }
    return models;
}

void SimulationMaker::readParameterElement(QList<QObject*> parents)
{
    Q_ASSERT(!parents.isEmpty());
    Q_ASSERT(reader->isStartElement());
    for (int i = 0; i < parents.size(); ++i) {
        setParameterElement(parents[i]);
    }
	nextElementDelim();
	Q_ASSERT(reader->isEndElement());
	nextElementDelim();
}

void SimulationMaker::setParameterElement(QObject *parent) {
    QString name = attributeValue("name", "");
    QString value = attributeValue("value", "");
    QString reference = attributeValue(QStringList() << "ref" << "variable", "");
    QString table = attributeValue("table", "");
    QString crosstab = attributeValue("crosstab", "");

    bool hasName = !name.isEmpty();
    bool hasValue = !value.isEmpty();
    bool hasReference = !reference.isEmpty();
    bool hasTable = !table.isEmpty();
    bool hasCrosstab = !crosstab.isEmpty();

    if (hasCrosstab && !hasName) {
        QString msg("If you set the 'crosstab' attribute (to '%1') you must also set the 'name' attribute");
        throw Exception(message(msg.arg(crosstab)), parent);
    }
    if (hasTable && hasName) {
        QString msg("If you set the 'table' attribute (to '%1') you cannnot also set the 'name' (set to %2)attribute. Maybe you meant 'crosstab'?");
        throw Exception(message(msg.arg(crosstab).arg(name)), parent);
    }

    if (hasTable || hasCrosstab) {
        if (hasValue) {
            QString msg("Parameter in table '%1' cannot also gave a 'value'attribute");
            throw Exception(message(msg.arg(fileName)), parent);
        }
        if (hasReference) {
            QString msg("Parameter in table '%1' cannot also gave a 'variable' attribute");
            throw Exception(message(msg.arg(fileName)), parent);
        }
        QString fileName = hasTable ? table : crosstab;
        ParameterIndex *table = createParameterTable(fileName);
        table->reset(dynamic_cast<Model*>(parent));
        while (table->hasNext()) {
            ParameterIndex::Result theNext = table->next();
            ParameterBase *parameter = seekOneChild<ParameterBase*>(theNext.first, parent);
            parameter->setValueFromString(theNext.second);
        }
    }
    else {
        if (!hasName) {
            QString msg("Parameter must have a name");
            throw Exception(message(msg), parent);
        }
        ParameterBase *parameter = seekOneChild<ParameterBase*>(name, parent);
        if (hasValue)
            parameter->setValueFromString(value.trimmed());
        else if (hasReference)
            redirectedParameters.append(RedirectedParameter(parameter, reference));
        else {
            QString msg("Parameter '%1' must have either a 'value' or a 'reference' attribute");
            throw Exception(message(msg.arg(name)), parent);
        }
    }
}

ParameterIndex *SimulationMaker::createParameterTable(QString fileName) {
    QString name = attributeValue("name", "");
    bool hasName = !name.isEmpty();
    QString filePath = simulation()->inputFilePath(fileName);
    if (parameterTables.contains(filePath))
        return parameterTables.value(filePath);
    if (hasName)
        return parameterTables[filePath] = new ParameterIndexFromCondensedTable(filePath, name);
    else
        return parameterTables[filePath] = new ParameterIndexFromTable(filePath);
}

void SimulationMaker::readOutputElement(QObject* parent)
{
	Q_ASSERT(reader->isStartElement() && parent);
	
    QString objectName = attributeValue("name", "anonymous");
    QString type = attributeValue("type", parent);

    Output *output = MegaFactory::create<Output>(type, objectName, parent);

	nextElementDelim();
	while (!reader->hasError() && reader->isStartElement()) {
        if (elementNameEquals("parameter")) {
            readParameterElement(asList(output));
        }
        else if (elementNameEquals("read-parameter")
                 || elementNameEquals("variable")
                 || elementNameEquals("trace")) {
            readOutputSubElement(output);
        }
        else if (elementNameEquals("table")) {
            readOutputTableElement(output);
        }
        else {
            throw Exception(message("Unexpected element: '" + elementName() + "'"), parent);
        }
    }
	Q_ASSERT(reader->isEndElement());
	nextElementDelim();
}	

void SimulationMaker::readOutputSubElement(QObject* parent)
{
    Q_ASSERT(reader->isStartElement() && parent);

    TraceParam param;
    param.setAttribute( "label", attributeValue("label", parent) );
    param.setAttribute( "ref", attributeValue(QStringList() << "ref" << "value", parent) );
    param.setAttribute( "summary", attributeValue("summary", "") );
    param.setAttribute( "type", attributeValue("type", "") );
    param.setAttribute( "columns", attributeValue("columns", "") );
    param.setAttribute( "rows", attributeValue("rows", "") );
    param.parent = parent;
    traceVariableParam.append(param);

    nextElementDelim();
    Q_ASSERT(reader->isEndElement());
    nextElementDelim();
}

void SimulationMaker::readOutputTableElement(QObject* parent) {
    Q_ASSERT(reader->isStartElement() && parent);
    QString fileName = attributeValue("table", parent);
    QString filePath = simulation()->inputFilePath(fileName);
    new DataGrid(filePath, parent);
    nextElementDelim();
 }

void SimulationMaker::createTraces() {
    for (int i = 0; i < traceVariableParam.size(); ++i) {
        const TraceParam &param( traceVariableParam.value(i) );
        QString label = param.attribute("label").toString();
        QString ref = param.attribute("ref").toString();
        QList<VariableBase*> bases = seekMany<QObject*, VariableBase*>(ref);
        // If several bases are found they will all get the same label.
        // This must be fixed, as needed, by Output
        for (int i = 0; i < bases.size(); ++i) {
            VariableBase *base = bases[i];
            QString useLabel = (label == "*") ? base->id().label() : label;
            Trace *trace = new Trace(useLabel, base, param.parent);
            trace->appendAttributes(param);
        }
    }

}

bool SimulationMaker::elementNameEquals(QString s) const {
    return reader->name().compare(s, Qt::CaseInsensitive) == 0;
}

bool SimulationMaker::elementNameNotEquals(QString s) const {
    return reader->name().compare(s, Qt::CaseInsensitive) != 0;
}

QString SimulationMaker::elementName() const {
    return reader->name().toString();
}

QString SimulationMaker::attributeValue(QString name, QString defaultValue) const {
    QString result = reader->attributes().value(name).toString().trimmed();
    return result.isEmpty() ? defaultValue : result;
}

QString SimulationMaker::attributeValue(QString name, QObject *parent) const {
    QString result = attributeValue(name, "");
    if (result.isEmpty()) {
        QString msg("Missing attribute: '%1'");
        throw Exception(message(msg.arg(name)), parent);
    }
    return result;
}

QString SimulationMaker::attributeValue(QStringList synonyms, QString defaultValue) const {
    QString result;
    for (int i = 0; i < synonyms.size(); ++i) {
        QString value = attributeValue(synonyms[i], "");
        if (!value.isEmpty()) {
            if (result.isEmpty())
                result = value;
            else {
                QString msg("Only one of these synonymous attributes can be specified: %1");
                throw Exception(message(msg.arg(synonyms.join(", "))));
            }
        }
    }
    return result.isEmpty() ? defaultValue : result;
}

QString SimulationMaker::attributeValue(QStringList synonyms, QObject *parent) const {
    QString result = attributeValue(synonyms, "");
    if (result.isEmpty()) {
        QString msg("Missing attribute: '%1'");
        throw Exception(message(msg.arg(synonyms[0])), parent);
    }
    return result;
}

QString SimulationMaker::message(QString text) const {
    QString s;
    QTextStream str(&s);
    str   << "Error reading UniSim file: "  <<  fileName << "\n"
          << "In line: " << reader->lineNumber() << "\n"
          << "In position: " << reader->columnNumber() << "\n"
          << (text.isEmpty() ? QString() : "Error message: " + text) << "\n"
          << "XML-reader reported: " << reader->errorString() << "\n"
          << "Last token read: " << reader->tokenString();
    return s;
}

namespace {


    template <class ParameterT, class VariableT, class ValuePtrT>
    bool baseCouple(ParameterBase *parameter, const VariableBase *variable)
    {
        ParameterT* parameterTyped = dynamic_cast<ParameterT*>(parameter);
        const VariableT *variableTyped = dynamic_cast<const VariableT*>(variable);
        bool matchedT = parameterTyped && variableTyped;
            if (matchedT) {
                const ValuePtrT *redirectTo = variableTyped->valuePtr();
                parameterTyped->redirectValuePtr(redirectTo);
            }
        return matchedT;
    }


    template <class T>
    bool couple(ParameterBase *parameter, const VariableBase *variable)
    {
        return baseCouple<Parameter<T>, Variable<T>, T >(parameter, variable) ||
               baseCouple<ParameterVector<T>, VariableVector<T>, QVector<T> >(parameter, variable);
    }

//    template <class T>
//    bool couple(ParameterBase *parameter, const VariableBase *variable) {
//        Parameter<T>* parameterT = dynamic_cast<Parameter<T>*>(parameter);
//        const Variable<T> *variableT = dynamic_cast<const Variable<T>*>(variable);
//        bool matchedT = parameterT && variableT;
//        if (matchedT) {
//            const T *redirectTo = variableT->valuePtr();
//            parameterT->redirectValuePtr(redirectTo);
//        }
//        return matchedT;
//    }
}

void SimulationMaker::redirectParameters() {
    for (int i = 0; i < redirectedParameters.size(); ++i) {
        ParameterBase *parameter = redirectedParameters[i].first;
        QString variableFullName = redirectedParameters[i].second;
        const VariableBase *variable = seekOne<QObject*, VariableBase*>(variableFullName);
        bool coupled =
            couple<bool>(parameter, variable) ||
            couple<int>(parameter, variable) ||
            couple<long>(parameter, variable) ||
            couple<unsigned>(parameter, variable) ||
            couple<float>(parameter, variable) ||
            couple<double>(parameter, variable) ||
            couple<QDate>(parameter, variable) ||
            couple<QString>(parameter, variable);
        if (!coupled) {
            QString msg("The type of variable '%1' does not match that of the parameter");
            throw Exception(message(msg.arg(variableFullName)), parameter);
        }
    }
}

} //namespace
