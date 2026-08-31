#ifndef _worldgen_coast_h_
#define _worldgen_coast_h_

#include "worldgen/export.h"
#include "worldgen/climate.h"
#include "worldgen/tectonics.h"

#include <glm/glm.hpp>

namespace worldgen
{

//Coastlines are bewilderingly varied and most of what distinguishes them - sea stacks, tombolos,
//blowholes, coves - happens well below the size of an influence cell. What does survive at this
//scale is the split the reference opens with: every coast is either a rocky coast or a sandy
//coastal plain, with the organic and reef types laid over the top of them.
enum class CoastType
{
    None,           //not a coastal cell
    Rocky,          //cliffs and shore platforms, on active margins and under coastal mountains
    Fjord,          //drowned glacial valleys, mid to high latitude coastal mountains
    Dalmatian,      //folds running parallel to the shore, drowned into shore-parallel islands
    SandyPlain,     //low relief depositional coast
    BarrierIsland,  //offshore bars built up above the waterline, mostly on passive margins
    Estuarine,      //drowned river valleys and other submergent coastal plain
    Delta,          //land the river built out into the sea, so never a straight coastline
    Mangrove,       //organic, salt tolerant trees, tropics and subtropics only
    SaltMarsh,      //organic, salt tolerant grasses, temperate and ice free
    CoralReef       //shallow warm clear water off a tropical shore
};

constexpr size_t CoastTypeCount=11;

inline const char *coastTypeName(CoastType coast)
{
    switch(coast)
    {
    case CoastType::Rocky:         return "Rocky coast";
    case CoastType::Fjord:         return "Fjord";
    case CoastType::Dalmatian:     return "Dalmatian coast";
    case CoastType::SandyPlain:    return "Sandy coastal plain";
    case CoastType::BarrierIsland: return "Barrier island";
    case CoastType::Estuarine:     return "Estuarine";
    case CoastType::Delta:         return "Delta";
    case CoastType::Mangrove:      return "Mangrove swamp";
    case CoastType::SaltMarsh:     return "Salt marsh";
    case CoastType::CoralReef:     return "Coral reef";
    default: break;
    }
    return "None";
}

inline glm::ivec3 coastTypeColor(CoastType coast)
{
    switch(coast)
    {
    case CoastType::Rocky:         return glm::ivec3(120, 120, 130);
    case CoastType::Fjord:         return glm::ivec3(190, 235, 255);
    case CoastType::Dalmatian:     return glm::ivec3(150, 120, 190);
    case CoastType::SandyPlain:    return glm::ivec3(240, 220, 150);
    case CoastType::BarrierIsland: return glm::ivec3(255, 170, 90);
    case CoastType::Estuarine:     return glm::ivec3(120, 190, 190);
    case CoastType::Delta:         return glm::ivec3(210, 240, 120);
    case CoastType::Mangrove:      return glm::ivec3(30, 120, 60);
    case CoastType::SaltMarsh:     return glm::ivec3(140, 190, 110);
    case CoastType::CoralReef:     return glm::ivec3(255, 120, 190);
    default: break;
    }
    return glm::ivec3(0, 0, 0);
}

struct CoastThresholds
{
    CoastThresholds()
    {
        //how much the ground rises within a cell or two of the shore before the coast reads rocky
        m_rockyRelief=0.075f;
        m_fjordRelief=0.085f;
        m_fjordLatitude=45.0f;

        //coral and mangroves are both held to warm water
        m_reefTemperature=20.0f;
        m_mangroveTemperature=18.0f;

        //salt marshes want a quiet low lying shore that does not freeze
        m_marshTemperature=0.0f;
        m_marshRelief=0.02f;

        //an organic coast needs somewhere sheltered, so it wants the wind off the sea rather than
        //a shore taking the full weather
        m_shelteredOnshore=0.0f;
    }

    float m_rockyRelief;
    float m_fjordRelief;
    float m_fjordLatitude;
    float m_reefTemperature;
    float m_mangroveTemperature;
    float m_marshTemperature;
    float m_marshRelief;
    float m_shelteredOnshore;
};

//the climates that carry, or recently carried, enough ice to grind out a valley
inline bool glaciatedClimate(ClimateZone climate)
{
    return (climate==ClimateZone::EF)||(climate==ClimateZone::ET)||(climate==ClimateZone::Dfc);
}

//A margin is active where the coast sits on a plate boundary that is doing something - closing or
//shearing. Divergent coasts are rifting open and behave like the passive case at this scale.
inline bool activeMargin(BoundaryType boundary)
{
    return (boundary==BoundaryType::Convergent)||(boundary==BoundaryType::Transform);
}

//Classifies the land side of a shoreline. Rocky where the margin is active or the ground climbs
//away from the sea, sandy where it is passive and flat, then the organic types laid over the sandy
//ones where the climate allows.
inline CoastType classifyCoast(const InfluenceCell &cell, BoundaryType boundary, float relief,
    bool glaciated, const CoastThresholds &thresholds)
{
    bool rocky=activeMargin(boundary)||(relief>=thresholds.m_rockyRelief);

    if(rocky)
    {
        //Glaciers cut valleys into coastal mountains and the sea floods them once the ice goes.
        //That needs the mountains and the ice both.
        if(glaciated&&(relief>=thresholds.m_fjordRelief))
            return CoastType::Fjord;

        //folds running parallel to the shore, drowned - a concordant coast rather than a
        //discordant one, so the boundary has to be running along the coast rather than into it
        if((boundary==BoundaryType::Convergent)&&(relief<thresholds.m_fjordRelief)&&(cell.continentality<=0.06f))
            return CoastType::Dalmatian;

        return CoastType::Rocky;
    }

    //low relief, passive margin: a coastal plain, and then whatever can grow on it
    if((cell.temperature>=thresholds.m_mangroveTemperature)&&(cell.moisture>=0.5f))
        return CoastType::Mangrove;

    if((cell.temperature>=thresholds.m_marshTemperature)&&(relief<thresholds.m_marshRelief)
        &&(cell.onshore>=thresholds.m_shelteredOnshore))
        return CoastType::SaltMarsh;

    return CoastType::SandyPlain;
}

}//namespace worldgen

#endif //_worldgen_coast_h_
