/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UNISIM_TRIGGER_BY_DATE
#define UNISIM_TRIGGER_BY_DATE
#include <QDate>
#include <QObject>
#include <usbase/date_interval.h>
#include <usbase/model.h>

namespace UniSim{

class TriggerByDate : public Model
{
	//Q_OBJECT
public: 
    TriggerByDate(Identifier name, QObject *parent=0);
    // standard methods
    void initialize();
    void reset();
    void update();
private:
    // methods
    void decodeFrequency();
    // parameters
    DateInterval::Frequency frequency;
    QString frequencyAsString;
    QDate fromDate, toDate;
    double dateShift;
    bool triggerAtReset;
    // pull variables
    bool value;
    // links
    Model *calendar;
};

} //namespace
#endif
