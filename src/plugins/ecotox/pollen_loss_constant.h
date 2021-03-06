/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef ECOTOX_POLLEN_LOSS_CONSTANT
#define ECOTOX_POLLEN_LOSS_CONSTANT
#include <QObject>
#include <usbase/model.h>

namespace ecotox {

class PollenLossConstant : public UniSim::Model
{
	//Q_OBJECT
public: 
    PollenLossConstant(UniSim::Identifier name, QObject *parent=0);
	// standard methods
	void reset();
    void update();

private:
    // parameters
    double rate;

    // derived

    // pull variables
    double value;

    // links

    // methods
};

} //namespace
#endif
