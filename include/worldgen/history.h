#ifndef _worldgen_history_h_
#define _worldgen_history_h_

#include "worldgen/export.h"
#include "worldgen/city.h"

#include <string>
#include <vector>

namespace worldgen
{

//How a civilization ended. The reference's technique is an INVERSION of the obvious order: "we
//actually have a sort of inversion of historical logic where we decide how an event ends and then
//create a rationalization for why it ended that way." So the fate is rolled first, and only then
//is the map searched for what would account for it. That is backwards from how history happened
//and exactly right for how history is written.
enum class CivilizationFate
{
    Unknown,
    Drought,     //the climate turned under them and the fields stopped feeding the cities
    Ice,         //the same, the other way
    Conquest,    //somebody else wanted the ground
    Exhaustion,  //the thing they were built on ran out
    Sundered,    //the pieces were cut off from each other and stopped being one people
    Drowned,     //the sea took the ports, and the ports were the whole of it
    Plague       //no cause the map can show, which is honest: not every collapse leaves a mark
};

inline const char *civilizationFateName(CivilizationFate fate)
{
    switch(fate)
    {
    case CivilizationFate::Drought:    return "Drought";
    case CivilizationFate::Ice:        return "Ice";
    case CivilizationFate::Conquest:   return "Conquest";
    case CivilizationFate::Exhaustion: return "Exhaustion";
    case CivilizationFate::Sundered:   return "Sundered";
    case CivilizationFate::Drowned:    return "Drowned";
    case CivilizationFate::Plague:     return "Plague";
    default: break;
    }
    return "Unknown";
}

//What a ruin actually is on the ground. The reference picks these off weighted tables with a
//priority fallthrough - "if you've got a fish village it rolls off the factional table, if it
//doesn't it falls down to a regional table, and if that doesn't exist it gets the default" - so
//that only the cases worth specialising have to be written out.
enum class RuinKind
{
    None,
    Monument,   //the default. Something raised to be looked at
    Fortress,   //defensible ground keeps its walls longest
    MineHead,   //workings, spoil heaps, and the village that served them
    Harbour,    //moles and quays, usually half under water now
    Terraces,   //field walls on a slope, which outlast everything else people build
    Tomb,       //dry country preserves what was meant to be sealed anyway
    Bridge      //a river crossing, because the crossing outlives whoever built it
};

inline const char *ruinKindName(RuinKind kind)
{
    switch(kind)
    {
    case RuinKind::Monument: return "Monument";
    case RuinKind::Fortress: return "Fortress";
    case RuinKind::MineHead: return "Mine head";
    case RuinKind::Harbour:  return "Harbour works";
    case RuinKind::Terraces: return "Terraces";
    case RuinKind::Tomb:     return "Tomb";
    case RuinKind::Bridge:   return "Bridge";
    default: break;
    }
    return "None";
}

//The reference's population table, reduced to what this generator needs: weighted entries, rolled
//with a seed, and a caller that tries the specialised table first and falls through to the general
//one when nothing specialised applies.
struct WeightedEntry
{
    int m_value;
    int m_weight;
};

inline int rollTable(const std::vector<WeightedEntry> &table, unsigned int roll)
{
    int total=0;

    for(size_t i=0; i<table.size(); ++i)
        total+=table[i].m_weight;

    if(total<=0)
        return 0;

    int pick=(int)(roll%(unsigned int)total);

    for(size_t i=0; i<table.size(); ++i)
    {
        pick-=table[i].m_weight;

        if(pick<0)
            return table[i].m_value;
    }

    return table.back().m_value;
}

//The site-specific table. Ground that says something definite about what stood on it takes
//priority; everything else falls through to the default below.
inline std::vector<WeightedEntry> ruinKindsForSite(const InfluenceCell &cell, float relief,
    bool mountainous)
{
    std::vector<WeightedEntry> table;

    if((cell.coast!=(int)CoastType::None)&&(cell.water!=WaterBody::River))
    {
        table.push_back(WeightedEntry{(int)RuinKind::Harbour, 55});
        table.push_back(WeightedEntry{(int)RuinKind::Fortress, 25});
        table.push_back(WeightedEntry{(int)RuinKind::Monument, 20});
        return table;
    }

    if((cell.deposit!=DepositType::None)&&cell.depositExposed)
    {
        table.push_back(WeightedEntry{(int)RuinKind::MineHead, 60});
        table.push_back(WeightedEntry{(int)RuinKind::Monument, 25});
        table.push_back(WeightedEntry{(int)RuinKind::Fortress, 15});
        return table;
    }

    if(cell.water==WaterBody::River)
    {
        table.push_back(WeightedEntry{(int)RuinKind::Bridge, 45});
        table.push_back(WeightedEntry{(int)RuinKind::Monument, 35});
        table.push_back(WeightedEntry{(int)RuinKind::Terraces, 20});
        return table;
    }

    //dry inland ground is where a sealed chamber survives, and it is the only thing that
    //reliably does
    if(climateGroup(cell.climate)==ClimateGroup::Arid)
    {
        table.push_back(WeightedEntry{(int)RuinKind::Tomb, 45});
        table.push_back(WeightedEntry{(int)RuinKind::Monument, 30});
        table.push_back(WeightedEntry{(int)RuinKind::Fortress, 25});
        return table;
    }

    if(mountainous||(relief>=0.15f))
    {
        table.push_back(WeightedEntry{(int)RuinKind::Fortress, 40});
        table.push_back(WeightedEntry{(int)RuinKind::Terraces, 35});
        table.push_back(WeightedEntry{(int)RuinKind::Monument, 25});
        return table;
    }

    return table;   //empty, so the caller falls through
}

//The default table, reached whenever nothing more specific applied.
inline std::vector<WeightedEntry> defaultRuinKinds(const InfluenceCell &cell)
{
    std::vector<WeightedEntry> table;

    table.push_back(WeightedEntry{(int)RuinKind::Monument, 50});
    table.push_back(WeightedEntry{(int)RuinKind::Fortress, 25});

    //dry ground keeps a sealed chamber the way nowhere else does
    if(climateGroup(cell.climate)==ClimateGroup::Arid)
        table.push_back(WeightedEntry{(int)RuinKind::Tomb, 45});
    else
        table.push_back(WeightedEntry{(int)RuinKind::Tomb, 10});

    table.push_back(WeightedEntry{(int)RuinKind::Terraces, 15});

    return table;
}

//The history written after the fact. The fate was already decided; every clause below is a real
//measurement over the civilization's own sites, so the rationalization is FOUND rather than
//invented. Where the map has nothing to offer, the history says so plainly, which the reference is
//comfortable with: "we let them be sort of spotty."
inline std::string fateStory(const FallenCivilization &fallen)
{
    std::string story;

    switch((CivilizationFate)fallen.m_fate)
    {
    case CivilizationFate::Drought:
        //the fate is a given; the WETTER WORLD is a claim, and it is only made where the map
        //actually shows one
        if(fallen.m_moistureSwing>0.02f)
            story="Their world was wetter than this one, and the ground they farmed has lost the rain that fed it.";
        else
            story="The rains failed them, though nothing in this world's weather still shows it.";
        break;

    case CivilizationFate::Ice:
        if(fallen.m_temperatureSwing<-0.5f)
            story="They built in a warmth this world no longer has, and the cold came down over them.";
        else
            story="The cold came down over them.";
        break;

    case CivilizationFate::Conquest:
        if(fallen.m_nearestRival>=0)
            story="A power seated some way off wanted the ground, and took it.";
        else
            story="They were taken by somebody, though nothing that still stands says who.";
        break;

    case CivilizationFate::Exhaustion:
        if(fallen.m_workedOut>0.25f)
            story="They were built on what was under them, and it ran out.";
        else
            story="Whatever they lived on stopped being there, and nothing on the ground now says what it was.";
        break;

    case CivilizationFate::Sundered:
        if(fallen.m_barrierSplit>1)
            story="Their country was cut into pieces that could not reach each other, and they stopped being one people.";
        else
            story="They came apart, though the ground gives no reason why.";
        break;

    case CivilizationFate::Drowned:
        if(fallen.m_drownedSites>0)
            story="They lived off their harbours, and the sea came up over them.";
        else
            story="The water took them, though nothing along this coast still shows it.";
        break;

    case CivilizationFate::Plague:
        story="Nothing on the map accounts for it. They were simply gone.";
        break;

    default:
        story="How they ended is not recorded.";
        break;
    }

    if(fallen.m_refuge>=0)
        story+=" Whoever walked out went over the divide, and their descendants are there still.";

    return story;
}

}//namespace worldgen

#endif //_worldgen_history_h_
