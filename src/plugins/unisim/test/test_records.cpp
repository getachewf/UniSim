#include <iostream>
#include <QDir>
#include <QGenericMatrix>
#include <usbase/exception.h>
#include <usbase/file_locations.h>
#include <usbase/model.h>
#include <usbase/parameter.h>
#include <usbase/test_num.h>
#include <usbase/utilities.h>
#include <usengine/simulation.h>
#include <usengine/simulation_maker.h>
#include "test_records.h"

using namespace UniSim;

const int C_totalTimeSteps = 0;
const int C_totalTime = 1;
const int C_dayOfYear = 2;
const int C_day =3;
const int C_month = 4;
const int C_year = 5;
const int C_hour = 6;
const int C_minute = 7;
const int C_second = 8;
const int C_dateAsReal = 9;
const int C_A = 10;
const int C_B = 11;
const int C_C = 12;
const int COLS = 13;
const int ROWS = 10;

void TestRecords::cleanup() {
    delete sim;
    sim = 0;
}

void TestRecords::testCalendarDateIsFirst() {
    createSimulation("test_records.xml");
    Model *records = seekOneDescendant<Model*>("records", sim);
    records->seekOneChild<Parameter<bool>*>("imposeInitialDateTime") -> setValue(false);

    try {
        sim->execute();
    }
    catch (Exception &ex) {
        QString msg = "Could not execute: " + ex.message();
        QFAIL(qPrintable(msg));
    }

    openOutputFile("test_records.prn");
    readLineItems();
    QCOMPARE(lineItems.size(), COLS);
    QCOMPARE(lineItems[C_totalTimeSteps], QString("totalTimeSteps"));
    QCOMPARE(lineItems[C_C], QString("C"));

    QVector<double> data;
    data.reserve(COLS*ROWS);
    readData(&data);
    outputFile.close();
    QCOMPARE(data.size(), COLS*ROWS);
    QGenericMatrix<COLS,ROWS,double> M(data.data());


    // Date = 2 January 2010
    QCOMPARE(M(0, C_totalTimeSteps), 1.);
    QCOMPARE(M(0, C_totalTime), 1.);
    QCOMPARE(M(0, C_dayOfYear), 2.);
    QCOMPARE(M(0, C_day), 2.);
    QCOMPARE(M(0, C_month), 1.);
    QCOMPARE(M(0, C_year), 2010.);
    QVERIFY(TestNum::eq(M(0, C_dateAsReal), 2010. + 1./365.));

    // Date = 11 January 2010
    QCOMPARE(M(9, C_totalTimeSteps), 10.);
    QCOMPARE(M(9, C_totalTime), 10.);
    QCOMPARE(M(9, C_dayOfYear), 11.);
    QCOMPARE(M(9, C_day), 11.);
    QCOMPARE(M(9, C_month), 1.);
    QCOMPARE(M(9, C_year), 2010.);
    QVERIFY(TestNum::eq(M(9, C_dateAsReal), 2010. + 10./365.));

    // Test interpolation
    QCOMPARE(M(0, C_C), 32.);   // 2 Jan
    QCOMPARE(M(1, C_C), 32.);   // 3 Jan
    QCOMPARE(M(2, C_C), 33.);   // 4 Jan
    QCOMPARE(M(3, C_C), 35.5);  // 5 Jan
    QCOMPARE(M(9, C_C), 38.);   // 11 Jan
}

void TestRecords::testCalendarDateIsInside() {
    createSimulation("test_records.xml");
    Model *records = seekOneDescendant<Model*>("records", sim);
    records->seekOneChild<Parameter<bool>*>("imposeInitialDateTime") -> setValue(false);

    Model *calendar = seekOneDescendant<Model*>("calendar", sim);
    calendar->seekOneChild<Parameter<QDate>*>("initialDate") -> setValue(QDate(2010,1,4));

    try {
        sim->execute();
    }
    catch (Exception &ex) {
        QString msg = "Could not execute: " + ex.message();
        QFAIL(qPrintable(msg));
    }

    openOutputFile("test_records.prn");
    readLineItems();
    QCOMPARE(lineItems.size(), COLS);
    QCOMPARE(lineItems[C_totalTimeSteps], QString("totalTimeSteps"));
    QCOMPARE(lineItems[C_C], QString("C"));

    QVector<double> data;
    data.reserve(COLS*ROWS);
    readData(&data);
    outputFile.close();
    QCOMPARE(data.size(), COLS*ROWS);
    QGenericMatrix<COLS,ROWS,double> M(data.data());


    // Date = 5 January 2010
    QCOMPARE(M(0, C_totalTimeSteps), 1.);
    QCOMPARE(M(0, C_totalTime), 1.);
    QCOMPARE(M(0, C_dayOfYear), 5.);
    QCOMPARE(M(0, C_day), 5.);
    QCOMPARE(M(0, C_month), 1.);
    QCOMPARE(M(0, C_year), 2010.);
    QVERIFY(TestNum::eq(M(0, C_dateAsReal), 2010. + 4./365.));

    // Date = 14 January 2010
    QCOMPARE(M(9, C_totalTimeSteps), 10.);
    QCOMPARE(M(9, C_totalTime), 10.);
    QCOMPARE(M(9, C_dayOfYear), 14.);
    QCOMPARE(M(9, C_day), 14.);
    QCOMPARE(M(9, C_month), 1.);
    QCOMPARE(M(9, C_year), 2010.);
    QVERIFY(TestNum::eq(M(9, C_dateAsReal), 2010. + 13./365.));

    // Test interpolation
    QCOMPARE(M(0, C_C), 35.5);   // 5 Jan
    QCOMPARE(M(1, C_C), 38.);   // 6 Jan
    QCOMPARE(M(2, C_C), 38.);   // 7 Jan
    QCOMPARE(M(3, C_C), 38.);  // 8 Jan
    QCOMPARE(M(9, C_C), 38.);   // 14 Jan
}



void TestRecords::testImposeDate() {
    createSimulation("test_records.xml");
    try {
        sim->execute();
    }
    catch (Exception &ex) {
        QString msg = "Could not execute: " + ex.message();
        QFAIL(qPrintable(msg));
    }

    openOutputFile("test_records.prn");
    readLineItems();
    QCOMPARE(lineItems.size(), COLS);
    QCOMPARE(lineItems[C_totalTimeSteps], QString("totalTimeSteps"));
    QCOMPARE(lineItems[C_C], QString("C"));

    QVector<double> data;
    data.reserve(COLS*ROWS);
    readData(&data);
    outputFile.close();
    QCOMPARE(data.size(), COLS*ROWS);
    QGenericMatrix<COLS,ROWS,double> M(data.data());


    // Date = 4 January 2010
    QCOMPARE(M(0, C_totalTimeSteps), 1.);
    QCOMPARE(M(0, C_totalTime), 1.);
    QCOMPARE(M(0, C_dayOfYear), 4.);
    QCOMPARE(M(0, C_day), 4.);
    QCOMPARE(M(0, C_month), 1.);
    QCOMPARE(M(0, C_year), 2010.);
    QVERIFY(TestNum::eq(M(0, C_dateAsReal), 2010. + 3./365.));

    // Date = 13 January 2010
    QCOMPARE(M(9, C_totalTimeSteps), 10.);
    QCOMPARE(M(9, C_totalTime), 10.);
    QCOMPARE(M(9, C_dayOfYear), 13.);
    QCOMPARE(M(9, C_day), 13.);
    QCOMPARE(M(9, C_month), 1.);
    QCOMPARE(M(9, C_year), 2010.);
    QVERIFY(TestNum::eq(M(9, C_dateAsReal), 2010. + 12./365.));

    // Test interpolation
    QCOMPARE(M(0, C_C), 33.);   // 4 Jan
    QCOMPARE(M(1, C_C), 35.5); // 5 Jan
    QCOMPARE(M(2, C_C), 38.);   // 6 Jan
    QCOMPARE(M(3, C_C), 38.);   // 7 Jan
    QCOMPARE(M(9, C_C), 38.);   // 13 Jan
}

//
// Supporting methods
//

void TestRecords::createSimulation(QString fileName) {
    QDir dir = FileLocations::location(FileLocationInfo::Plugins);
    QString filePath = dir.absolutePath() + "/unisim/test/input/" + fileName;
    SimulationMaker maker;
    sim = maker.parse(filePath);
}

void TestRecords::openOutputFile(QString fileName) {
    QString filePath = FileLocations::location(FileLocationInfo::Output).absolutePath() +
                       "/" + fileName;
    outputFile.setFileName(filePath);

    bool fileOk = outputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!fileOk) throw Exception("Cannot open output file: " + filePath);
}

void TestRecords::readLineItems() {
    QString line;
    while (!outputFile.atEnd() && line.isEmpty()) {
        line = QString(outputFile.readLine()).trimmed();
    }
    lineItems = line.split("\t", QString::SkipEmptyParts);
}

void TestRecords::readData(QVector<double> *data) {
    int numItems = -1;
    while (true) {
        readLineItems();
        if (lineItems.isEmpty()) break;
        if (numItems == -1)
            numItems = lineItems.size();
        else
            QCOMPARE(numItems, lineItems.size());
        for (int i = 0; i < numItems; ++i) {
            bool ok;
            data->append(lineItems[i].toDouble(&ok));
            QVERIFY(ok);
        }
    }
}
