#include <iostream>
#include <QSettings>
#include <usbase/exception.h>
#include <usbase/identifier.h>
#include <usbase/file_locations.h>
#include <usbase/output.h>
#include <usbase/utilities.h>
#include <usengine/simulation_maker.h>
#include <unisim/anonymous_model.h>
#include <unisim/steps.h>
#include "../simulation.h"
#include "trickle_box.h"
#include "trickle_sequence.h"
#include "test_simulation.h"


using namespace UniSim;
	
void TestSimulation::initTestCase()
{	
    QDir dir = FileLocations::location(FileLocationInfo::Temporary);
    QString filePath = dir.absolutePath() + "/test.xml";
    writeStandardTestFile(filePath);

	_simulation = 0;
	SimulationMaker maker;
	try {
        _simulation = maker.parse(filePath);
	}
	catch (const UniSim::Exception &ex) {
		delete _simulation;
		_simulation = 0;
		QWARN(qPrintable(ex.message()));
		QVERIFY(false);
	}
}

void TestSimulation::init() {
    setSimulationObject(_simulation);
}

void TestSimulation::cleanupTestCase()
{
	delete _simulation;
	_simulation = 0;
}

void TestSimulation::testInitialize()
{
	QVERIFY(_simulation);
}

void TestSimulation::testFindModels() 
{
    Simulation *sim = new Simulation("apple-tree");
    setSimulationObject(sim);
    new Steps("integrator", sim);
	
	AnonymousModel *butterfly, *mite;
	
	butterfly = new AnonymousModel("butterfly", sim);
	new AnonymousModel("egg", butterfly);
	new AnonymousModel("larva", butterfly);
	new AnonymousModel("pupa", butterfly);
	new AnonymousModel("adult", butterfly);
	
	mite = new AnonymousModel("mite", sim);
	new AnonymousModel("egg", mite);
	new AnonymousModel("nymph", mite);
	new AnonymousModel("adult", mite);
	
    QList<Model*> models;
    models = seekDescendants<Model*>("adult", 0);
	QCOMPARE(models.size(), 2);
	QVERIFY(models[0]->parent());
	QCOMPARE(models[0]->parent()->objectName(), QString("butterfly"));
	QVERIFY(models[1]->parent());
	QCOMPARE(models[1]->parent()->objectName(), QString("mite"));
	
    models = seekDescendants<Model*>("mite/egg", 0);
	QCOMPARE(models.size(), 1);
	QCOMPARE(models[0]->objectName(), QString("egg"));
	QVERIFY(models[0]->parent());
	QCOMPARE(models[0]->parent()->objectName(), QString("mite"));
	
    QList<Model*> same;
    same = seekDescendants<Model*>("/mite/egg", 0);
	QCOMPARE(same.size(), 1);
	QCOMPARE(models[0], same[0]);
	
    models = seekDescendants<Model*>("/mite", 0);
	QCOMPARE(same.size(), 1);
	QCOMPARE(models[0]->objectName(), QString("mite"));

    models = seekDescendants<Model*>("/", 0);
	QCOMPARE(models.size(), 0);
	
    models = seekDescendants<Model*>("", 0);
	QCOMPARE(models.size(), 0);

    models = seekDescendants<Model*>("/ghost/egg", 0);
	QCOMPARE(models.size(), 0);

    models = seekDescendants<Model*>("ghost/egg", 0);
	QCOMPARE(models.size(), 0);

	// Jokers
    models = seekDescendants<Model*>("butterfly/*", 0);
	QCOMPARE(models.size(), 4);
	QCOMPARE(models[0]->objectName(), QString("egg"));
	QCOMPARE(models[3]->objectName(), QString("adult"));
	
    models = seekDescendants<Model*>("/butterfly/*", 0);
	QCOMPARE(models.size(), 4);
	QCOMPARE(models[0]->objectName(), QString("egg"));
	QCOMPARE(models[3]->objectName(), QString("adult"));
		
    models = UniSim::seekChildren<Model*>("*", 0);
    QCOMPARE(models.size(), 3);
    QCOMPARE(models[0]->objectName(), QString("integrator"));
    QCOMPARE(models[2]->objectName(), QString("mite"));
		
	delete sim;
}

void TestSimulation::testFindOutputs()
{
    QList<Output*> outputs;
    outputs = seekDescendants<Output*>("butterflyTable", 0);
    QCOMPARE(outputs.size(), 1);
    QVERIFY(Identifier("butterflyTable").equals(outputs[0]->objectName()));
}

void TestSimulation::testExecute()
{
    QList<Model*> search;
    search = seekDescendants<Model*>("/butterfly/egg", 0);
    QCOMPARE(search.size(), 1);
    search[0]->pushValue("inflow", 15.);

    search = seekDescendants<Model*>("/wasp/egg", 0);
    QCOMPARE(search.size(), 1);
    search[0]->pushValue("inflow", 80.);

	try {
		_simulation->execute();
	}
	catch (const UniSim::Exception &ex) {
		QWARN(qPrintable(ex.message()));
		QVERIFY(false);
	}
}
