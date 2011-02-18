/* Copyright (C) 2009-2011 by Niels Holst [niels.holst@agrsci.dk] and co-authors.
** Copyrights reserved.
** Released under the terms of the GNU General Public License version 3.0 or later.
** See www.gnu.org/copyleft/gpl.html.
*/
#include <usbase/object_pool.h>
#include <usbase/utilities.h>
#include "area.h"
#include "area_density_even.h"
#include "area_density_symmetric.h"
#include "area_density_tapering.h"
#include "area_density_topheavy.h"
#include "assimilation_max_given_temp.h"
#include "community.h"
#include "early_growth.h"
#include "organ.h"
#include "height.h"
#include "intercom_model_maker.h"
#include "leaf_stem_ratio.h"
#include "light_use_efficiency_given_temp.h"
#include "partitioning_fixed.h"
#include "phenology.h"
#include "plant.h"
#include "specific_leaf_area.h"
#include "weather.h"

using namespace UniSim;

namespace intercom{

bool initialized = false;

UniSim::Identifier IntercomModelMaker::pluginName() const {
    return "intercom";
}

QString IntercomModelMaker::pluginDesc() const {
    return
    "The @F intercom plugin is an implementation of the INTERCOM model for plant growth and competition, "
    "as described by @Cite{$label{Kropff \"&\" Laar (1993)}kropff_laar}.";
}

QStringList IntercomModelMaker::authors() const {
    return QStringList()
        << "Niels Holst, Aarhus University, Denmark"
        << "Marleen Riemens, Plant Research International, Netherlands"
        << "Jonathan Storkey, Rothamstead Research International, UK";
}

const QMap<Identifier, QString>& IntercomModelMaker::supportedClasses() {
    if (!desc.isEmpty())
        return desc;

    desc["Area"] =
    "An @F Area object keeps track of the area (cm @Sup 2 per plant) which carries out photosynthesis according to its parameters and sub-models. "
    "It must hold three sub-models as children, named @F {density}, @F amax and @F {lightUseEfficiency}. The latter two specify the shape "
    "of the light response curve and the first the shape of the canopy.";

    desc["AreaDensityEven"] =
    "This model can serve as a @F density sub-model for an @F Area object. The density is the same at all heights from zero to canopy height.";

    desc["AreaDensitySymmetric"] =
    "This model can serve as a @F density sub-model for an @F Area object. The density is parabolic from zero to canopy height.";

    desc["AreaDensityTapering"] =
    "This model can serve as a @F density sub-model for an @F Area object. The density increases linearly from zero at canopy height to its maximum at zero height.";

    desc["AreaDensityTopheavy"] =
    "This model can serve as a @F density sub-model for an @F Area object. The density increases non-linearly from zero to canopy height, "
    "resulting in a topheavy shape typical of flowers";

    desc["AssimilationMaxGivenTemp"] =
    "This model can serve as the @F amax sub-model of an @F Area object. Based on the @F Tday variable of @F weather model, it calculates the maximum assimilation rate as "
    "@Math{ maxAmax (1 - radix sup Tday)}";

    desc["Community"] =
    "A @F Community model holds one or more @F Plant child models. It resolves the competition for light in daily time steps, calculating total light absorption "
    "and CO @Sub 2 assimilation of the whole canopy (i.e. of all its @F Plant objects) and dividing the resulting total among its @F Plant objects."
    "@PP If more than one @F Community object is present they will work in parallel without any interference (e.g. competition for light). "
    "Thus different scenarios can be compared in one simulation run.";

    desc["EarlyGrowth"] =
    "The @F EarlyGrowth model is a simple expontial curve based on photothermal time. "
    "A @F photothermal time model must be available as a sibling.";

    desc["Height"] =
    "A @F Plant model must have one child model named @F {height} with a @F height pull variable returning the height. "
    "The @F Height class provides the standard height model of INTERCOM which is a logistic growth curve working on photothermal time."
    "@PP @Math{height(t) = h0 + (hmax - h0) over {1 + exp(-slope(t - tm))}} @LP "
    "The @F height model looks for the nearest model named @F time to provide the daily time increment. "
    "A @F UniSim::PhotoThermalTime model can be used for this purpose (see UniSim::PhotoThermalTime @CrossLink {page @PageOf UniSim::PhotoThermalTime}).";

    desc["LeafStemRatio"] =
    "@I pending";

    desc["LightUseEfficiencyGivenTemp"] =
    "This model can serve as the @F lightUseEfficiency sub-model of an @F Area object. It calculates light use (or light conversion) effiency as a linear relation on "
    "@F Tday of the @F weather model";

    desc["Organ"] =
    "An @F Organ model is a child of a @F Plant model. It contains one child model called @F mass and optionally an additional model of the @F Area class";

    desc["PartitioningFixed"] =
    "This model supplies a partitioning coefficient with a fixed value";

    desc["Phenology"] =
    "The model of plant phenology must be a child of a @F Plant model. It may consist of any number of @F UniSim::Stage models";

    desc["Plant"] =
    "A @F Plant model is a child of the @F Community model. It holds one or more @F Organ child models and "
    "additional child models called @F {time}, @F {height} and @F {earlyGrowth}. ";

    desc["SpecificLeafArea"] =
    "This can serve as the @F specificLeafArea model of an @F Area model. It models specific leaf area (m @Sup {2}\"/\"g) as a linear relation on photothermal time, "
    "supplied by the nearest model called @F {time}, for example a @F UniSim::PhotoThermalTime model.";

    desc["Weather"] =
    "@I pending";

    return desc;
}

void IntercomModelMaker::useObjectPool(ObjectPool *pool) const {
    objectPool()->deferTo(pool);
}

Model* IntercomModelMaker::create(Identifier modelType, Identifier objectName, QObject *parent)
{
	setSimulationObjectFromDescendent(parent);
	Model *model = 0;
    if (modelType.equals("Area"))
        model = new Area(objectName, parent);
    else if (modelType.equals("AreaDensityEven"))
        model = new AreaDensityEven(objectName, parent);
    else if (modelType.equals("AreaDensitySymmetric"))
        model = new AreaDensitySymmetric(objectName, parent);
    else if (modelType.equals("AreaDensityTapering"))
        model = new AreaDensityTapering(objectName, parent);
    else if (modelType.equals("AreaDensityTopheavy"))
        model = new AreaDensityTopheavy(objectName, parent);
    else if (modelType.equals("AssimilationMaxGivenTemp"))
        model = new AssimilationMaxGivenTemp(objectName, parent);
    else if (modelType.equals("Community"))
        model = new Community(objectName, parent);
    else if (modelType.equals("EarlyGrowth"))
        model = new EarlyGrowth(objectName, parent);
    else if (modelType.equals("Height"))
        model = new Height(objectName, parent);
    else if (modelType.equals("LeafStemRatio"))
        model = new LeafStemRatio(objectName, parent);
    else if (modelType.equals("LightUseEfficiencyGivenTemp"))
        model = new LightUseEfficiencyGivenTemp(objectName, parent);
    else if (modelType.equals("Organ"))
        model = new Organ(objectName, parent);
    else if (modelType.equals("PartitioningFixed"))
        model = new PartitioningFixed(objectName, parent);
    else if (modelType.equals("Phenology"))
        model = new Phenology(objectName, parent);
    else if (modelType.equals("Plant"))
		model = new Plant(objectName, parent);
    else if (modelType.equals("SpecificLeafArea"))
        model = new SpecificLeafArea(objectName, parent);
    else if (modelType.equals("Weather"))
		model = new Weather(objectName, parent);
	return model;
}

Q_EXPORT_PLUGIN2(Intercom_model_maker, IntercomModelMaker)

} //namespace
