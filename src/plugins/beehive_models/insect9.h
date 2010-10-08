/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef BEEHIVE_INSECT9  // 1
#define BEEHIVE_INSECT9  // 1
#include <QObject>
#include <usbase/model.h>

namespace UniSim {
    class Stage;
}

namespace beehive{

class Insect9 : public UniSim::Model  // 2
{
	Q_OBJECT
public: 
    Insect9(UniSim::Identifier name, QObject *parent=0); // 3
	// standard methods
	void initialize();
	void reset();
	void update();

private:
    // methods
    void applySurvival(Model *model, Model *survival);

	// parameters
    double initEggs;

	// state
    UniSim::Model *egg, *larva, *pupa, *fecundity,
    *eggSurvival, *larvaSurvival, *pupaSurvival;
    QList<UniSim::Stage*> stages;
};

} //namespace
#endif
