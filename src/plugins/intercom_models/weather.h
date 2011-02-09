/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef INTERCOM_WEATHER
#define INTERCOM_WEATHER
#include "weather_interface.h"

namespace UniSim {
    class Calendar;
}

namespace intercom {

class Weather : public WeatherInterface
{
	Q_OBJECT
public: 
    Weather(UniSim::Identifier name, QObject *parent=0);
    // standard methods
    void initialize();
    void update();

private slots:
    void handleClockTick(double hour);

private:
    // methods
    void verifySequence();
    void updatePar();

    // links
    UniSim::Calendar *calendar;

    int testInt;

};

} //namespace
#endif
