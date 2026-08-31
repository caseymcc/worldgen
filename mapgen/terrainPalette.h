#ifndef _mapgen_terrainPalette_h_
#define _mapgen_terrainPalette_h_

#include "worldgen/climate.h"
#include "worldgen/fuel.h"

#include <glm/glm.hpp>

#include <algorithm>

namespace mapgen
{

//Ground colours for the detail view. These are what the land LOOKS like, which is a different job
//to worldgen::climateZoneColor - that is a classification key, picked so eighteen zones can be told
//apart on one map, and it would render a rainforest in flat cyan. A rainforest is dark green.
struct TerrainPalette
{
    TerrainPalette(const glm::ivec3 &low, const glm::ivec3 &high):
        m_low(low), m_high(high)
    {}

    glm::ivec3 m_low;   //valley floor
    glm::ivec3 m_high;  //the tops, before rock and snow take over
};

inline TerrainPalette climateTerrain(worldgen::ClimateZone zone)
{
    switch(zone)
    {
    case worldgen::ClimateZone::Af:
    case worldgen::ClimateZone::Am:
        //rainforest: dark, saturated, and barely varying with height
        return TerrainPalette(glm::ivec3(28, 78, 38), glm::ivec3(66, 112, 58));

    case worldgen::ClimateZone::Aw:
        //savanna, straw for most of the year
        return TerrainPalette(glm::ivec3(120, 128, 58), glm::ivec3(168, 156, 92));

    case worldgen::ClimateZone::BWh:
        //hot desert: sand over rock
        return TerrainPalette(glm::ivec3(196, 158, 96), glm::ivec3(214, 186, 140));

    case worldgen::ClimateZone::BWk:
        //cold desert, more gravel than sand
        return TerrainPalette(glm::ivec3(158, 144, 116), glm::ivec3(184, 176, 158));

    case worldgen::ClimateZone::BSh:
        return TerrainPalette(glm::ivec3(172, 150, 84), glm::ivec3(196, 178, 122));

    case worldgen::ClimateZone::BSk:
        return TerrainPalette(glm::ivec3(142, 140, 100), glm::ivec3(170, 168, 136));

    case worldgen::ClimateZone::Csa:
    case worldgen::ClimateZone::Csb:
        //mediterranean scrub, olive and dry
        return TerrainPalette(glm::ivec3(112, 122, 70), glm::ivec3(150, 148, 106));

    case worldgen::ClimateZone::Cfa:
        return TerrainPalette(glm::ivec3(72, 116, 54), glm::ivec3(126, 144, 88));

    case worldgen::ClimateZone::Cfb:
    case worldgen::ClimateZone::Cfc:
        //oceanic: the greenest ground there is
        return TerrainPalette(glm::ivec3(66, 118, 62), glm::ivec3(118, 142, 100));

    case worldgen::ClimateZone::Dfa:
    case worldgen::ClimateZone::Dfb:
        return TerrainPalette(glm::ivec3(70, 104, 62), glm::ivec3(124, 128, 96));

    case worldgen::ClimateZone::Dfc:
        //taiga, dark and blue-green
        return TerrainPalette(glm::ivec3(56, 84, 66), glm::ivec3(104, 116, 100));

    case worldgen::ClimateZone::ET:
        //tundra: moss and stone
        return TerrainPalette(glm::ivec3(122, 118, 100), glm::ivec3(160, 158, 148));

    case worldgen::ClimateZone::EF:
        return TerrainPalette(glm::ivec3(226, 232, 238), glm::ivec3(244, 248, 252));

    default:
        break;
    }

    return TerrainPalette(glm::ivec3(96, 116, 74), glm::ivec3(150, 150, 112));
}

//Climate is not only a property of the map square: it changes going up a hill, which is why a
//mountain in the tropics still wears snow. Above the treeline the ground is bare rock, and above
//the freezing line it is white, both worked out from the temperature at THIS sample's height
//rather than at the influence cell's.
inline glm::ivec3 terrainColor(worldgen::ClimateZone zone, float lift, float localTemperature,
    float elevation, float latitude, bool floodplain)
{
    TerrainPalette palette=climateTerrain(zone);
    float red=(float)palette.m_low.r+((float)(palette.m_high.r-palette.m_low.r)*lift);
    float green=(float)palette.m_low.g+((float)(palette.m_high.g-palette.m_low.g)*lift);
    float blue=(float)palette.m_low.b+((float)(palette.m_high.b-palette.m_low.b)*lift);

    if(floodplain)
    {//ground the river works over is greener than what surrounds it, in any climate
        red*=0.88f;
        green*=1.10f;
        blue*=0.86f;
    }

    //bare rock above the treeline
    float treeline=worldgen::treelineElevation(latitude);

    if((treeline>0.0f)&&(elevation>treeline))
    {
        float bare=std::min((elevation-treeline)/900.0f, 1.0f);

        red=red+((132.0f-red)*bare);
        green=green+((126.0f-green)*bare);
        blue=blue+((118.0f-blue)*bare);
    }

    //and snow where the year does not get above freezing
    if(localTemperature<0.0f)
    {
        float snow=std::min(-localTemperature/6.0f, 1.0f);

        red=red+((248.0f-red)*snow);
        green=green+((250.0f-green)*snow);
        blue=blue+((255.0f-blue)*snow);
    }

    return glm::ivec3((int)red, (int)green, (int)blue);
}

}//namespace mapgen

#endif//_mapgen_terrainPalette_h_
