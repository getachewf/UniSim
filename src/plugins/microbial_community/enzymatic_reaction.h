/* Copyright (C) 2009-2012 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#ifndef ENZYMATIC_REACTION
#define ENZYMATIC_REACTION
#include <QObject>
#include <usbase/model.h>
#include "population.h"
#include "reaction.h"

namespace MicrobialCommunity {

    class EnzymaticReaction : public Reaction {
            Q_OBJECT

    public:
            EnzymaticReaction(UniSim::Identifier name, QObject *parent=0);

            // standard methods
            void update();

    };

    } //namespace

#endif //ENZYMATIC_REACTION
