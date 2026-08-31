#ifndef _worldgen_city_h_
#define _worldgen_city_h_

#include "worldgen/export.h"
#include "worldgen/trade.h"

#include <string>
#include <vector>

namespace worldgen
{

//"The most powerful cities in history don't produce anything. They take." The reference's argument
//is that a coastal city is not a settlement that happens to be on the sea - it is a DRAIN built at
//the one place where two costs meet, because "moving a ton of grain by wagon costs 40 times more
//than moving it by ship". Everything below follows from that single ratio.
enum class NodeType
{
    None,
    Mouth,      //a river meets the sea, so two supply chains meet and one man weighs both
    Strait,     //a gap ships have no choice but to pay to pass
    Anchorage,  //shelter on a coast that otherwise kills ships
    Extraction  //the world sails here because of what comes out of the ground
};

inline const char *nodeTypeName(NodeType node)
{
    switch(node)
    {
    case NodeType::Mouth:      return "Mouth";
    case NodeType::Strait:     return "Strait";
    case NodeType::Anchorage:  return "Anchorage";
    case NodeType::Extraction: return "Extraction";
    default: break;
    }
    return "None";
}

//Which coasts shelter a hull and which ones break it. "Most coastline is hostile. Cliffs, rocks,
//shallow water, exposed headlands. Ships die on bad coastline. But sometimes the coast is
//merciful."
inline bool shelteredCoast(CoastType coast)
{
    return (coast==CoastType::Fjord)||(coast==CoastType::Estuarine)||(coast==CoastType::Dalmatian);
}

inline bool hostileCoast(CoastType coast)
{
    return (coast==CoastType::Rocky)||(coast==CoastType::CoralReef)||(coast==CoastType::BarrierIsland);
}

struct CityThresholds
{
    CityThresholds()
    {
        //"You can haul calories across an ocean, but shipping water for a metropolis is
        //impossible." Below this a place is a fort or a way station, never a city.
        m_thirstMoisture=0.18f;

        //What a river has to carry before it is a road rather than a stream. Set off this world's
        //own distribution rather than off earth's: mouth discharge runs to about 11000 here, with
        //the median near 250, so this takes the top quarter or so of rivers.
        m_navigableDischarge=600.0f;

        //"No river means a town. One river means a city. A delta means something larger."
        //Likewise measured: mouth drainage areas run p50 200, p75 510, p90 1100, max 7400.
        m_townDrainage=500.0f;
        m_cityDrainage=2500.0f;

        //how far a city can reach for its own bread before the wagon stops being worth it
        m_hinterland=3;

        //"The parasite city refuses the limit. It stops farming its own land and starts eating the
        //world." It can only do that if it sits on a lane that can carry the grain.
        m_parasiteTraffic=0.3f;
        m_parasiteShortfall=0.35f;

        m_minimumSize=0.18f;
        m_spacing=4;
        m_count=24;
    }

    float m_thirstMoisture;
    float m_navigableDischarge;
    float m_townDrainage;
    float m_cityDrainage;
    int m_hinterland;
    float m_parasiteTraffic;
    float m_parasiteShortfall;
    float m_minimumSize;
    int m_spacing;
    int m_count;
};

//"Water first. Fresh water comes before anything else... Where there is no fresh water, there is no
//city." A river beside the walls answers it outright; otherwise the rain has to.
inline float waterCeiling(const InfluenceCell &cell, const CityThresholds &thresholds)
{
    if(cell.water==WaterBody::River)
        return (cell.riverDischarge>=thresholds.m_navigableDischarge)?1.0f:0.8f;

    if(cell.moisture<thresholds.m_thirstMoisture)
        return 0.05f;   //a way station at best

    return clamp(0.25f+(cell.moisture*0.9f), 0.0f, 1.0f);
}

//"How much river drainage feeds this point? No river means a town. One river means a city. A delta
//means something larger. The river system is the ceiling."
inline float riverCeiling(const InfluenceCell &cell, const CityThresholds &thresholds)
{
    float area=cell.drainageArea;

    if(cell.deltaShape!=DeltaShape::None)
        area*=1.5f;   //a delta gathers a continent rather than a valley

    if(area<thresholds.m_townDrainage)
        return 0.35f;   //a town, and no more

    if(area<thresholds.m_cityDrainage)
        return 0.7f;

    return 1.0f;
}

//What sort of drain the ground has built here, if any. The four are checked strongest first,
//because a place can be more than one and the reference is clear which wins: "the mouth is the
//strongest node on your map. Its power is purely geographic."
inline NodeType classifyNode(const InfluenceCell &cell, bool navigable, bool onNarrows,
    bool shelteredAmongHostile, bool monopoly)
{
    if(cell.coast==(int)CoastType::None)
        return NodeType::None;

    //A creek reaching the sea is not a mouth node. "Everything produced inland, grain, timber,
    //arrives by river" only where the river is a road, so the river has to be one.
    if((cell.coast==(int)CoastType::Delta)||(cell.water==WaterBody::Delta)
        ||((cell.water==WaterBody::River)&&navigable))
        return NodeType::Mouth;

    //the toll is charged from the shore beside the narrows, not from the water in it
    if(onNarrows)
        return NodeType::Strait;

    if(shelteredAmongHostile)
        return NodeType::Anchorage;

    if(monopoly)
        return NodeType::Extraction;

    return NodeType::None;
}

//A place on the coast that takes. Every field here answers one of the reference's own questions.
struct City
{
    size_t m_cell;
    NodeType m_node;
    int m_polity;

    float m_water;      //the freshwater ceiling
    float m_food;       //what its own backyard can feed
    float m_river;      //what the river system behind it can deliver
    float m_size;       //the smallest ceiling wins, unless it is fed from the sea

    bool m_parasite;    //it outgrew its fields and eats the world instead
    unsigned int m_monopoly; //goods it has that almost nowhere else does
    float m_traffic;

    std::string m_reason; //"the sentence is the city"
};

//Step 4 of the reference's workflow: "Every node gets one sentence... The sentence is the city.
//Everything else follows."
inline std::string cityReason(const City &city)
{
    std::string reason;

    switch(city.m_node)
    {
    case NodeType::Mouth:
        reason="This city exists because everything the valley produces must pass its docks to reach the sea.";
        break;
    case NodeType::Strait:
        reason="This city exists because every hull passing between two seas pays it to go through.";
        break;
    case NodeType::Anchorage:
        reason="This city exists because ships needed somewhere to stop on a coast that would have killed them.";
        break;
    case NodeType::Extraction:
        reason="This city exists because the world sails here for what comes out of the ground.";
        break;
    default:
        reason="This is a landing, not a city.";
        break;
    }

    if(city.m_parasite)
        reason+=" It outgrew its own fields long ago and eats imported grain; cut the lanes and it starves.";
    else if(city.m_water<=0.25f)
        reason+=" It will never be large, because there is not enough fresh water to drink.";

    return reason;
}

}//namespace worldgen

#endif //_worldgen_city_h_
