/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UNISIM_RANDOM_BASE
#define UNISIM_RANDOM_BASE
#include <QObject>
#include <boost/random/mersenne_twister.hpp>
#include <usbase/identifier.h>
#include <usbase/model.h>

namespace UniSim{

class RandomGenerator ;

class RandomBase : public Model
{
	Q_OBJECT
public: 
    RandomBase(Identifier name, QObject *parent=0);
    virtual ~RandomBase();
    // standard methods
    virtual void initialize();
    virtual void reset();
    virtual void update();
    // special methods
    static Identifier id();
protected:
    // Common random number generator
    RandomGenerator *generator;
private:
    // methods
    virtual double drawValue() = 0;
    bool triggered();
    void nextValue();
    // parameters
    double minValue, maxValue;
    // pull variables
    double value;
    // links
    Models triggers;
};

} //namespace
#endif
