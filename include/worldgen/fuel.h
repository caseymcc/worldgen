#ifndef _worldgen_fuel_h_
#define _worldgen_fuel_h_

#include "worldgen/export.h"
#include "worldgen/climate.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>

namespace worldgen
{

//What can be burned here. This decides far more than comfort: smelting ore, firing pottery and
//boiling brine all need a hot enough fire for long enough, and a people with no fuel within reach
//cannot work metal however much ore is under their feet.
enum class FuelSource
{
    None,   //nothing worth burning - desert, ice, bare rock
    Wood,   //standing forest, the default fuel wherever trees will grow
    Peat,   //waterlogged ground that never rotted down, cut and dried
    Coal    //buried and compressed, the best of the three and the hardest to reach
};

inline const char *fuelSourceName(FuelSource fuel)
{
    switch(fuel)
    {
    case FuelSource::Wood: return "Wood";
    case FuelSource::Peat: return "Peat";
    case FuelSource::Coal: return "Coal";
    default: break;
    }
    return "None";
}

inline glm::ivec3 fuelSourceColor(FuelSource fuel)
{
    switch(fuel)
    {
    case FuelSource::Wood: return glm::ivec3(180, 105, 70);   //terracotta
    case FuelSource::Peat: return glm::ivec3(225, 205, 155);  //beige
    case FuelSource::Coal: return glm::ivec3(45, 45, 50);
    default: break;
    }
    return glm::ivec3(120, 120, 120);
}

//The elevation above which trees will not grow, by latitude. The reference gives this as three
//straight segments: flat through the tropics, then falling steeply, then more gently until it
//reaches the ground and no elevation is low enough.
inline float treelineElevation(float latitude, float tropicalTreeline=4000.0f,
    float steepFalloff=130.0f, float gentleFalloff=75.0f)
{
    float degrees=(float)fabs(latitude)*180.0f/(float)M_PI;

    if(degrees<=30.0f)
        return tropicalTreeline;

    if(degrees<=50.0f)
        return tropicalTreeline-(steepFalloff*(degrees-30.0f));

    //below zero here means no trees at any elevation, which happens around 69 degrees
    return tropicalTreeline-(steepFalloff*20.0f)-(gentleFalloff*(degrees-50.0f));
}

//Trees need somewhere that is neither too high, too dry nor too cold. The arid climates and the
//polar ones are cut out wholesale - there is no forest on a steppe and none on an ice cap.
inline bool treesGrow(ClimateZone climate, float latitude, float elevation)
{
    switch(climate)
    {
    case ClimateZone::BWh:
    case ClimateZone::BWk:
    case ClimateZone::BSh:
    case ClimateZone::BSk:
    case ClimateZone::ET:
    case ClimateZone::EF:
    case ClimateZone::Ocean:
        return false;
    default:
        break;
    }

    return elevation<treelineElevation(latitude);
}

//Peat is plant matter that fell into water too cold and too airless to rot it. That wants a cool
//wet climate, and the reference looks for it specifically where such a climate sits on ground the
//last ice sheet worked over - the bogs of the north are all on land the ice left behind.
inline bool peatClimate(ClimateZone climate)
{
    return (climate==ClimateZone::Dfc)||(climate==ClimateZone::Dfb)||(climate==ClimateZone::Cfb);
}

}//namespace worldgen

#endif //_worldgen_fuel_h_
