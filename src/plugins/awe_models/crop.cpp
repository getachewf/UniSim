/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <QMessageBox>
#include <usbase/exception.h>
#include <usbase/parameter.h>
#include <usbase/pull_variable.h>
#include <usbase/utilities.h>
#include "crop.h"
#include "weather.h"

using namespace UniSim;

namespace awe {

Crop::Crop(UniSim::Identifier name, QObject *parent)
    : Model(name,parent)
{
    new Parameter<int>("sowingDay", &sowingDay, 1, this, "Day in month of sowing");
    new Parameter<int>("sowingMonth", &sowingMonth, 4, this, "Month of sowing");
    new Parameter<int>("harvestDay", &harvestDay, 1, this, "Day in month of harvest");
    new Parameter<int>("harvestMonth", &harvestMonth, 8, this, "Month of harvest");
    new Parameter<double>("weedExchangeRate", &weedExchangeRate, 0.6, this, "Final biomass of weeds as a proportion of yield loss (g\"/\"g)");
    new Parameter<double>("maxYield", &maxYield, 800., this, "Weed-free yield (g\"/\"m @Sup {2})");
    new Parameter<double>("maxYieldLossPct", &maxYieldLossPct, 60., this, "Maximum yield loss (%) caused by weeds (Cousens's @I {a})");
    new Parameter<double>("slopeYieldLossPct", &slopeYieldLossPct, 0.5, this, "Initial yield loss per weed (% per weed density equivalent) "
                                                                "caused by weeds (Cousens's @I {i})");
    new Parameter<QString>("laiCalendar", &laiCalendar,
                 QString("((0 0)(110 0)(210 0.3)(310 0.8)(410 1.6)"
                         "(510 2.9)(610 5)(1000 5)(1200 5)(1650 2))"), this,
                 "Calendar for leaf area index (LAI) running on temperature sum since sowing (day-degrees above @Char {ring}C). "
                 "Defined as a list of (@I {temperature-sum LAI}) pairs");

    new PullVariable<double>("lai", &lai, this, "Leaf area index (m @Sup {2}\"/\"m @Sup {2}).");
    new PullVariable<double>("Tsum", &Tsum, this, "Temperature sum since sowing (day-degrees above @Char {ring}C).");
}



void Crop::initialize() {
    sowingDayOfYear = UniSim::toDayOfYear(sowingDay, sowingMonth);
    harvestDayOfYear = UniSim::toDayOfYear(harvestDay, harvestMonth);

    decodeLai();

    weather = seekOne<Model*>("weather");
    calendar = seekOne<Model*>("calendar");
}

void Crop::reset() {
    isGrowing = false;
    lai = Tsum = 0.;
}

void Crop::update() {
    int dayOfYear = int(calendar->pullVariable<double>("dayOfYear"));

    if (isGrowing) {
        Tsum += weather->pullVariable<double>("T");
        lai = lookupLai();
        if (dayOfYear == harvestDayOfYear) {
            isGrowing = false;
            emit event(this, "harvest");
        }
    }

    if (!isGrowing) {
        Tsum = lai = 0;
        if (dayOfYear == sowingDayOfYear) {
            isGrowing = true;
            emit event(this, "sowing");
        }
    }

}

void Crop::decodeLai() {
    QString s = laiCalendar.simplified();
    if (s.size() == 0) throw UniSim::Exception("LAI calendar is empty: " + s);
    if (s.left(1) != "(") throw UniSim::Exception("LAI calendar must begin with '(': " + s);

    QStringList parts = s.split("(");
    for (int i = 0; i < 2; ++i) {
        if (parts[i].size() > 0 && parts[i].left(1) != " ")
            throw UniSim::Exception("LAI calendar must begin with two left parentheses: " + s);
    }

    if (parts.size() < 3) throw UniSim::Exception("LAI calendar is incomplete: " + s);

    for (int i = 2; i< parts.size(); ++i) {
        QString part = parts[i];
        chopRightParenthesis(part);
        if (i == parts.size() - 1)
            chopRightParenthesis(part);

        QStringList pair = part.split(" ");
        bool ok = pair.size() == 2;
        if (ok) {
            bool ok0, ok1;
            int tSum = pair[0].toInt(&ok0);
            double lai = pair[1].toDouble(&ok1);
            ok = ok0 && ok1;
            if (ok) laiByTSum[tSum] = lai;
        }

        if (!ok)
            throw UniSim::Exception("LAI calendar must contain pairs of numbers: (integer decimal) "
                                    + parts[i]);
    }
}

void Crop::chopRightParenthesis(QString &s) const {
    if (s.right(1) != ")")
        throw UniSim::Exception("LAI calendar miss right parenthesis: (" + s);
    s.chop(1);
    s = s.simplified();
}

double Crop::lookupLai() const {
    return interpolate(laiByTSum, int(Tsum));
}

}
