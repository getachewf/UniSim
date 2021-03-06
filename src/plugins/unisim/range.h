/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UNISIM_RANGE_H
#define UNISIM_RANGE_H

#include <usbase/model.h>

namespace UniSim{
	
class Model;

class Range : public Model
{
	//Q_OBJECT
public:
    Range(Identifier name, QObject *parent=0);

    // standard methods
    void initialize();
    void reset();
    void update();
	
private:
    // parameters
    double progress, minValue, maxValue;
	QString scaleAsString;
    enum {Linear, Log10} scale;
    // values
    double value;
    // methods
    void decodeScale();
};

} //namespace

#endif
