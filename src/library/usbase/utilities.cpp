/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <iomanip>
#include <iostream>
#include <cmath>
#include <QDate>
#include <QFile>
#include <QMap>
#include <QObject>
#include <QStringList>
#include <QXmlStreamWriter>
#include "exception.h"
#include <usengine/simulation.h>
#include "model.h"
#include "utilities.h"

/*! \namespace UniSim
    \brief Contains all classes, global functions and definitions.
*/

using namespace std;

namespace UniSim {

//
// Navigation
//

//! \cond
QObject *SimulationObject::simulation = 0;
//! \endcond

//! Sets simulation object directly
void setSimulationObject(QObject *simulation) {
    SimulationObject::simulation = simulation;
}

//! Sets simulation object from descendent
/*! The parent line of the decendent is followed until an object of Simulation class is found.
    Or else the simulation object is set to null.
*/
void setSimulationObjectFromDescendent(QObject *descendent) {
    QObject *p = descendent;
    SimulationObject::simulation = 0;
    while (p && !SimulationObject::simulation) {
        QString className = p->metaObject()->className();
        if (className.contains("Simulation"))
            SimulationObject::simulation = p;
        p = p->parent();
    }
}

//! Returns Simulation object
/*! Throws Exception if the Simulation object has not been set.
*/
QObject* simulationObject() {
    if (!SimulationObject::simulation)
        throw Exception("Simulation object has not been set");
    return SimulationObject::simulation;
}

//
// Mathematics
//

//! Interpolate from (x,y) table
/*! Interpolates a y-value from the given x-value. For x-values less than the first x-value in the table,
the first y-value is returned. For x-values greater then the last x-value in the table, the last y-value
is returned.
*/
double interpolate(const QMap<int, double> xy, int x) {
    if (xy.size() == 0) return 0;
    double lastY = xy[0];

    QMap<int, double>::const_iterator low = xy.begin();
    for (QMap<int, double>::const_iterator high = xy.begin();
                                           high != xy.end();
                                           ++high)
    {
        if (x < high.key()) {
            if (high.key() == low.key()) return low.value();
            return double(x - low.key())/(high.key() - low.key())*(high.value() - low.value()) + low.value();
        }
        lastY = high.value();
        low = high;
    }
    return lastY;
}

//! Power function that tolerates x equal to zero
double pow0(double x, double c) {
    return (x == 0) ? 0. : std::pow(x, c);
}

//! Returns exp(-x) where x>=0
/*! Also works for extreme values
*/
double negExp(double x)
{
    return (x > MAX_EXP) ? 0. : (x < MIN_EXP) ? 1. : exp(-x);
}

//! Gutierrez-Baumgärtner functional response
double GBFuncResp(double demand, double supply) {
    if (demand < 0 ||  supply < 0)
        throw Exception("Illegal arguments to GBFuncResp(d,s), "
                        "d = " + QString::number(demand) +
                        "s = " + QString::number(supply));

    if (demand <= DBL_EPSILON)
        return 0.;

    double res = demand*(1. - negExp(supply/demand));
    return (res > supply) ? supply : res;
}

//! Returns the Julian day (1..365)
int toDayOfYear(int day, int month) {
    QDate date(2001,month, day);
    return date.dayOfYear();
}

//
// String handling
//

//! \cond
namespace local {
    QString stringDefault(QString s, QString defaultValue) {
        return s.isEmpty() ? defaultValue : s;
    }

    void chopParentheses(QString &s, QString errorContext) {
        if (s.left(1) != "(")
            throw UniSim::Exception(stringDefault(errorContext, "Value list") +
                                    " miss left parenthesis: " + s);
        s.remove(0, 1);

        if (s.right(1) != ")")
            throw UniSim::Exception(stringDefault(errorContext, "Value list") +
                                    " miss right parenthesis: " + s);
        s.chop(1);
        s = s.simplified();
    }

    void chopRightParenthesis(QString &s, QObject *concerning) {
        if (s.right(1) != ")")
            throw Exception("Missing right parenthesis: (" + s, concerning);
        s.chop(1);
        s = s.simplified();
    }


} //namespace
//! \endcond

QString fullName(const QObject *object) {
    bool noObject = !object;
    bool isSimulation = dynamic_cast<const Simulation*>(object) != 0;
    if (noObject || isSimulation) return QString();
    QString name = object->objectName();
    if (name.isEmpty() || name.toLower() == "anonymous")
        name = QString("unnamed[") +
               object->metaObject()->className() + "]";
    return fullName(object->parent()) + "/" + name;
}


//! Parses a simple list "(A B C)"
/*! Throws an Exception if the list is ill-formated or is empty. The optional errorContext is used
    as explanatory test in the Exception
*/
QStringList decodeSimpleList(QString parenthesizedList, QString errorContext) {
    QString s = parenthesizedList.simplified();
    if (s.size() == 0)
        return QStringList();
    if (s[0] != '(') {
        if (s.endsWith(')'))
            throw UniSim::Exception(local::stringDefault(errorContext,
                                    "Value list misses left parenthesis"));
        else
            return QStringList() << s;
    }
    local::chopParentheses(s, errorContext);
    return s.split(" ", QString::SkipEmptyParts);
}


QList< QPair<QString, int> > decodeNameValueList(QString nameValueList, QObject *concerning) {
    QList< QPair<QString, int> > result;
    QString s = nameValueList.simplified();
    s = s.trimmed();
    if (s.size() == 0) throw Exception("Name-value list is empty", concerning);
    if (s.left(1) != "(") throw Exception("Name-value list must begin with '(': " + s, concerning);

    QStringList parts = s.split("(");
    for (int i = 0; i < 2; ++i) {
        if (parts[i].size() > 0 && parts[i].left(1) != " ")
            throw UniSim::Exception("Name-value list must begin with two left parentheses: " + s, concerning);
    }

    if (parts.size() < 3) throw UniSim::Exception("Name-value list is incomplete: " + s, concerning);

    for (int i = 2; i< parts.size(); ++i) {
        QString part = parts[i].trimmed();
        local::chopRightParenthesis(part, concerning);
        if (i == parts.size() - 1)
            local::chopRightParenthesis(part, concerning);

        QStringList strPair = part.split(" ");
        bool ok = strPair.size() == 2;
        if (ok) {
            QPair<QString, int> pair;
            pair.first = strPair[0];
            pair.second = strPair[1].toInt(&ok);
            if (ok) result.append(pair);
        }

        if (!ok)
            throw UniSim::Exception("Name-value list must contain (name value) pairs: " + s, concerning);
    }
    return result;
}

//
void splitAtNamespace(QString s, QString *namespacePart, QString *ownNamePart) {
    Q_ASSERT(namespacePart && ownNamePart);
    int i = s.lastIndexOf("::");
    if (i == -1) {
        *namespacePart = QString();
        *ownNamePart = s;
    }
    else {
        *namespacePart = s.left(i);
        *ownNamePart = s.right(s.size() - i - 2);
    }
}

QStringList splitParentChildExpression(QString expression) {
    int begin = expression.indexOf('[');
    int end = expression.indexOf(']');
    QString msg;
    if (begin == -1)
        msg = "Missing '[' in expression";
    else if (end == -1)
        msg = "Missing ']' in expression";
    else if (end < expression.size() - 1)
        msg = "Expression must end with ']'";
    else if (begin == 0)
        msg = "Expression misses parent name before '['";
    else if (end - begin == 1)
        msg = "Expression misses child name inside the brackets";
    if (!msg.isEmpty()) {
        msg += ": '" + expression + "'";
        throw (Exception(msg));
    }
    QString parent = expression.left(begin);
    QString child = expression.mid(begin + 1, end - begin -1);

    QStringList result;
    result.append(parent);
    result.append(child);
    return result;
}

template<> bool stringToValue<bool>(QString s_) {
    QString s = s_.trimmed().toLower();
    bool value;
    if (s=="y" || s=="yes" || s=="t" || s=="true")
        value = true;
    else if (s=="n" || s=="no" || s=="f" || s=="false")
        value = false;
    else {
        QString msg = "Cannot convert '" + s + "' to bool";
        throw Exception(msg);
    }
    return value;
}

template<> QDate stringToValue<QDate>(QString s) {
    QDate date = QDate::fromString(s.trimmed(), "d/M/yyyy");
    if (!date.isValid()) {
        QString msg = "Cannot convert '" + s + "' to a date";
        throw Exception(msg);
    }
    return date;
}


//! Write object tree to std::cout
/*!
    Write the name of an object and all its descendants on std::cout (intended for debugging).
    \param root		the root of the tree to be written
    \param level	indentation (spaces) written before the object's name
*/
void writeObjectTree(QObject *obj, int level) {
	if (level > 0) cout << setw(2*level) << " ";
	
	cout
		<< obj->metaObject()->className() 
		<< " '" << qPrintable(obj->objectName()) << "'\n";
		
	for (QList<QObject*>::const_iterator ch = obj->children().begin(); ch != obj->children().end(); ++ch) {
		writeObjectTree(*ch, level+1);
	}
}	

//! \cond
namespace local {
	
	void writeIntegrator(QXmlStreamWriter *xw)
	{
		xw->writeStartElement("integrator");
        xw->writeAttribute("name", "integrator");
        xw->writeAttribute("type", "TimeStepLimited");
		xw->writeEmptyElement("parameter");
        xw->writeAttribute("name", "maxSteps");
		xw->writeAttribute("value", "365");

		xw->writeStartElement("sequence");
		xw->writeEmptyElement("model");
		xw->writeAttribute("name", "world");
		xw->writeEmptyElement("model");
		xw->writeAttribute("name", "wasp");
		xw->writeEmptyElement("model");
		xw->writeAttribute("name", "butterfly");
		xw->writeEndElement();

		xw->writeEndElement();
	}

	void writeWorld(QXmlStreamWriter *xw)
	{
		xw->writeStartElement("model");
		xw->writeAttribute("name", "world");
		xw->writeAttribute("type", "ConstantWorld");
		xw->writeEmptyElement("parameter");
		xw->writeAttribute("name", "temperature");
		xw->writeAttribute("value", "20");
		xw->writeEndElement();
	}
	
	void writeStage(QXmlStreamWriter *xw, QString name, double duration, unsigned k)
	{
		xw->writeStartElement("model");
		xw->writeAttribute("type", "LifeStage");
		xw->writeAttribute("name", name);
		
		xw->writeEmptyElement("parameter");
		xw->writeAttribute("name", "duration");
		xw->writeAttribute("value", QString::number(duration));
	
		xw->writeEmptyElement("parameter");
		xw->writeAttribute("name", "k");
		xw->writeAttribute("value", QString::number(k));
		
		xw->writeEndElement();
	}
	
	void writeModel(QXmlStreamWriter *xw)
	{
				
		// Butterfly
		xw->writeStartElement("model");
		xw->writeAttribute("type", "LifeCycle");
		xw->writeAttribute("name", "butterfly");
		
		writeStage(xw, "egg", 1.1, 5);
		writeStage(xw, "larva", 2.3, 10);
		writeStage(xw, "pupa", 5.8, 20);
		writeStage(xw, "adult", 13.21, 40);
		xw->writeEndElement();
		
		// Wasp
		xw->writeStartElement("model");
		xw->writeAttribute("type", "LifeCycle");
		xw->writeAttribute("name", "wasp");
		
		writeStage(xw, "egg", 5, 15);
		writeStage(xw, "larva", 14, 30);
		writeStage(xw, "pupa", 3, 20);
		writeStage(xw, "adult", 90, 10);
		xw->writeEndElement();

	} 

    void writeVariable(QXmlStreamWriter *xw, QString label, QString axis, QString value) {
        xw->writeEmptyElement("variable");
        xw->writeAttribute("label", label);
        xw->writeAttribute("axis", axis);
        xw->writeAttribute("value", value);
	}

    void writePlot(QXmlStreamWriter *xw, QString insect) {
		xw->writeStartElement("output");
		xw->writeAttribute("name", insect+"PhasePlot");
		xw->writeAttribute("type", "plot");

        xw->writeEmptyElement("parameter");
        xw->writeAttribute("name", "title");
        xw->writeAttribute("value", insect+" phase plot");

        writeVariable(xw, "larva", "x", insect + "/egg[contents]");
        writeVariable(xw, "larva", "y", insect + "/adult[contents]");

        xw->writeEndElement();
    }
	
	void writeTable(QXmlStreamWriter *xw, QString insect) {
		xw->writeStartElement("output");
		xw->writeAttribute("name", insect+"Table");
		xw->writeAttribute("type", "table");

        xw->writeEmptyElement("parameter");
        xw->writeAttribute("name", "filename");
        xw->writeAttribute("value", "unit_test_output.prn");

        writeVariable(xw, "larva", "x", insect + "/egg[contents]");
        writeVariable(xw, "larva", "y", insect + "/adult[contents]");

        xw->writeEndElement();
	}
	
	void writeOutput(QXmlStreamWriter *xw, QString insect)
	{
        // writePlot(xw, insect);
        // A plot needs mainWindow (with plotWidget) to run
        writeTable(xw, insect);
	}

	void writeSimulation(QXmlStreamWriter *xw)
	{
		xw->setAutoFormatting(true);
		xw->setCodec("ISO-8859-1");
		xw->writeStartDocument();
	
		xw->writeStartElement("simulation");
		xw->writeAttribute("version", "1.0");
	
		writeIntegrator(xw);
		writeWorld(xw);
		writeModel(xw);
		writeOutput(xw, "butterfly");
		writeOutput(xw, "wasp");
	
		xw->writeEndElement();
		xw->writeEndDocument();
	}	
	
} //namespace
//! \endcond

//! Write standard UniSim file for test purposes

void writeStandardTestFile(QString filePath)  {
	QFile file(filePath);
	bool ok = file.open(QFile::WriteOnly | QFile::Text);
    Q_ASSERT_X(ok, "In utilities.h (writeStandardTestFile)",
               qPrintable("Could not open file for writing: " + filePath));
	
	QXmlStreamWriter *xw = new QXmlStreamWriter(&file);
	local::writeSimulation(xw);
	file.close();
	delete xw;
}



} // namespace
