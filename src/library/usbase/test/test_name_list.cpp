#include <usbase/exception.h>
#include <usbase/name_list.h>
#include "test_name_list.h"

using namespace UniSim;

void TestNameList::testEmpty() {
    QStringList source, dest;
    NameList nl(source);
    dest = nl.simplified();
    QCOMPARE(dest.size(), 0);
}

void TestNameList::testOne() {
    QStringList source, expected, result;
    source
        << "/abc/def";
    expected
        << "def";
    NameList nl(source);

    try {
        result = nl.simplified();
    }
    catch(Exception &) {
        QFAIL("Unexpected exception");
    }
    QCOMPARE(result, expected);
}

void TestNameList::testMany() {
    QStringList source, expected, result;
    source
        << "/A/B"
        << "/A/B/C"
        << "/A/B/D"
        << "/A/B/D/E"
        << "/C"
        << "/E/B/C"
        << "/E/B/D"
        << "/E/B/F"
        << "/E/B/F/E";
    expected
        << "B"
        << "A/B/C"
        << "A/B/D"
        << "D/E"
        << "/C"
        << "E/B/C"
        << "E/B/D"
        << "F"
        << "F/E";
    NameList nl(source);

    try {
        result = nl.simplified();
    }
    catch(Exception &) {
        QFAIL("Unexpected exception");
    }
    QCOMPARE(result, expected);
}

void TestNameList::testEqual() {
    QStringList source, expected, result;
    source
        << "/A/B/C"
        << "/A/B/C"
        << "/A/B/C";
    expected
        << "/A/B/C"
        << "/A/B/C"
        << "/A/B/C";
    NameList nl(source);
    try {
        result = nl.simplified();
    }
    catch(Exception &) {
        QFAIL("Unexpected exception");
    }
    QCOMPARE(result, expected);
}

void TestNameList::testSomeEqual() {
    QStringList source, expected, result;
    source
        << "/A/B"
        << "/A/B/C"
        << "/A/B/C"
        << "/A/B/C";
    expected
        << "B"
        << "C"
        << "/A/B/C"
        << "/A/B/C";
    NameList nl(source);
    try {
        result = nl.simplified();
    }
    catch(Exception &) {
        QFAIL("Unexpected exception");
    }
    QCOMPARE(result, expected);
}
