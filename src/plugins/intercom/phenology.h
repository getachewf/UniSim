/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef INTERCOM_PHENOLOGY
#define INTERCOM_PHENOLOGY
#include <QObject>
#include <QVector>
#include <usbase/model.h>

namespace intercom{

class Phenology : public UniSim::Model
{
	//Q_OBJECT
public: 
    Phenology(UniSim::Identifier name, QObject *parent=0);
	//standard methods
    void amend();
    void reset();
	void update();

private:
    // pull variables
    double alive;
    QVector<double> proportions;

    // links
    QList<UniSim::Model*> stages;
};

} //namespace
#endif
