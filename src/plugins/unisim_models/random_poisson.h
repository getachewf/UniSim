/* Copyright (C) 2009-2010 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef UNISIM_RANDOM_POISSON
#define UNISIM_RANDOM_POISSON
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <QObject>
#include "random_base.h"
#include "random_generator.h"

namespace UniSim{

class RandomPoisson : public RandomBase
{
	Q_OBJECT
public: 
    RandomPoisson(UniSim::Identifier name, QObject *parent=0);
    ~RandomPoisson();
    // standard methods
    void initialize();
private:
    // parameters
    double mean;
    // methods
    double drawValue();
    // random number generation
    typedef boost::poisson_distribution<int, double> Distribution;
    typedef boost::variate_generator<RandomGenerator::Generator&, Distribution> Variate;
    Distribution *distribution;
    Variate *variate;
};

} //namespace
#endif
