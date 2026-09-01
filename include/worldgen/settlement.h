#ifndef _worldgen_settlement_h_
#define _worldgen_settlement_h_

#include "worldgen/export.h"
#include "worldgen/civilization.h"

#include <string>
#include <vector>

namespace worldgen
{

//The tier below a realm. A state is a thousand miles of ground and a city is a drain built where
//flow concentrates; neither of them is what somebody standing in a valley has for a neighbour.
//These are the places ordinary people actually live, a few miles apart, and the reason there is a
//separate tier at all is that a realm spans hundreds of cells while a walk to the next village
//spans one or two.
enum class SettlementKind
{
    None,
    Steading,   //one family and their fields
    Hamlet,     //a few households, no market and no church
    Village,    //a market some of the time, and somewhere to be buried
    Town        //a market most of the time, and a reason for the roads to meet
};

inline const char *settlementKindName(SettlementKind kind)
{
    switch(kind)
    {
    case SettlementKind::Steading: return "Steading";
    case SettlementKind::Hamlet:   return "Hamlet";
    case SettlementKind::Village:  return "Village";
    case SettlementKind::Town:     return "Town";
    default: break;
    }
    return "None";
}

//What each criterion said about a place, kept on the settlement so it can explain itself rather
//than just asserting a score.
struct SettlementCriteria
{
    SettlementCriteria():
        m_water(0.0f), m_arable(0.0f), m_grazing(0.0f), m_timber(0.0f), m_stone(0.0f),
        m_fuel(0.0f), m_ore(0.0f), m_slope(0.0f), m_sociability(0.0f), m_accessibility(0.0f),
        m_domination(0.0f)
    {}

    float m_water;
    float m_arable;
    float m_grazing;
    float m_timber;
    float m_stone;
    float m_fuel;
    float m_ore;
    float m_slope;
    float m_sociability;   //other people already nearby
    float m_accessibility; //how near the roads and routes pass
    float m_domination;    //how far this ground stands over what surrounds it
};

//What a settlement of a given SIZE wants, as distinct from what a PEOPLE wants. The species
//already carries its own weights for water, soil, timber and the rest (SpeciesHabitat), so the
//two compose: the people decide what ground suits them, the size decides whether it wants a road,
//neighbours, or a hill to stand on.
struct SettlementPreference
{
    SettlementPreference():
        m_sociability(0.0f), m_accessibility(0.0f), m_domination(0.0f), m_land(1.0f),
        m_spacing(1), m_threshold(0.0f), m_population(0)
    {}

    float m_sociability;   //a steading does not care who else is about; a town is made of it
    float m_accessibility; //a town without a road is not a town
    float m_domination;    //standing over the country, for pride or for defence
    float m_land;          //how much the site factors themselves count

    int m_spacing;         //cells this kind keeps from another of its own kind
    float m_threshold;     //interest below which this kind will not be founded at all
    size_t m_population;   //households at a middling site, scaled by how good the site is
};

struct SettlementThresholds
{
    SettlementThresholds()
    {
        //Bigger places are rarer, further apart, and harder to please. Spacing is in overview
        //cells, so at the generator's own default a village keeps about eight kilometres from the
        //next village and a town twenty from the next town.
        m_steading=SettlementPreference();
        m_steading.m_sociability=0.10f;
        m_steading.m_accessibility=0.05f;
        m_steading.m_domination=0.0f;
        m_steading.m_land=1.0f;
        m_steading.m_spacing=1;
        m_steading.m_threshold=0.14f;
        m_steading.m_population=6;

        m_hamlet=SettlementPreference();
        m_hamlet.m_sociability=0.25f;
        m_hamlet.m_accessibility=0.15f;
        m_hamlet.m_domination=0.05f;
        m_hamlet.m_land=0.95f;
        m_hamlet.m_spacing=1;
        m_hamlet.m_threshold=0.20f;
        m_hamlet.m_population=40;

        m_village=SettlementPreference();
        m_village.m_sociability=0.35f;
        m_village.m_accessibility=0.40f;
        m_village.m_domination=0.10f;
        m_village.m_land=0.85f;
        m_village.m_spacing=2;
        m_village.m_threshold=0.27f;
        m_village.m_population=220;

        m_town=SettlementPreference();
        m_town.m_sociability=0.45f;
        m_town.m_accessibility=0.70f;
        m_town.m_domination=0.20f;
        m_town.m_land=0.75f;
        m_town.m_spacing=5;
        m_town.m_threshold=0.36f;
        m_town.m_population=1400;

        //Nobody builds where there is no water to drink, on ground too steep to stand a barn on,
        //or where nothing will grow at any time of year. These are vetoes rather than penalties -
        //no amount of ore makes up for having nothing to drink.
        m_thirstVeto=0.02f;
        m_slopeVeto=0.30f;

        //"Either an indicator of social superiority or as necessity for defense" - how far out to
        //look when asking whether this ground stands over what surrounds it.
        m_dominationRadius=3;

        //how far the pull of existing neighbours reaches
        m_sociabilityRadius=3;

        //A site is accepted with a probability that follows its interest rather than by taking the
        //best cell, so a good region fills in unevenly the way a real one does.
        m_acceptance=0.55f;
    }

    const SettlementPreference &preference(SettlementKind kind) const
    {
        switch(kind)
        {
        case SettlementKind::Town:    return m_town;
        case SettlementKind::Village: return m_village;
        case SettlementKind::Hamlet:  return m_hamlet;
        default: break;
        }
        return m_steading;
    }

    SettlementPreference m_steading;
    SettlementPreference m_hamlet;
    SettlementPreference m_village;
    SettlementPreference m_town;

    float m_thirstVeto;
    float m_slopeVeto;
    int m_dominationRadius;
    int m_sociabilityRadius;
    float m_acceptance;
};

struct Settlement
{
    Settlement():
        m_cell(0), m_kind(SettlementKind::None), m_species(-1), m_polity(-1),
        m_interest(0.0f), m_population(0)
    {}

    size_t m_cell;
    SettlementKind m_kind;
    int m_species;
    int m_polity;       //whose ground it stands on, -1 if nobody's
    float m_interest;   //I(B), what the place was worth
    size_t m_population;

    SettlementCriteria m_criteria;
    std::string m_name;
    std::string m_reason;   //the one thing that most explains why here
};

//A place offered to somebody about to found a settlement of their own. Not a settlement: the whole
//point is that it is EMPTY and somebody else will build it. What it carries is why it was offered
//and what the choice costs - a site with good soil and no defence is a different bet to a poor one
//behind a mountain wall, and the player is choosing between bets rather than between names.
struct FoundingSite
{
    FoundingSite():
        m_cell(0), m_interest(0.0f), m_polity(-1), m_nearestSettlement(-1.0f),
        m_neighbours(0), m_neighbourPopulation(0)
    {}

    size_t m_cell;
    float m_interest;
    SettlementCriteria m_criteria;

    int m_polity;              //whose ground it stands on, -1 if nobody claims it
    float m_nearestSettlement; //cells to the nearest neighbour, -1 if there are none
    size_t m_neighbours;       //how many settlements are within reach
    size_t m_neighbourPopulation;

    std::string m_reason;      //the one thing that most recommends it
    std::string m_character;   //the bet it represents, in a phrase
};

//One way between two places. Roads are not planned here any more than borders were: each settlement
//is joined to a larger one near it, every road is the cheapest line the ground allows, and following
//a road somebody else already cut is nearly free - which is the whole mechanism. Nothing decides
//that four lanes should meet at a town; they meet there because each of them was cheapest.
struct Road
{
    Road(): m_from(0), m_to(0), m_cost(0.0f), m_length(0) {}

    size_t m_from;   //settlement index
    size_t m_to;
    float m_cost;
    size_t m_length; //cells
};

struct RoadThresholds
{
    RoadThresholds()
    {
        //"the less costly connection to the existing network should be looked for (road re-use
        //coming for free)" - the one number that decides whether a network branches or sprawls
        m_reuseWeight=0.15f;

        //A lane climbs where a cart will not. Roads pay for slope more than trade routes do,
        //because a trade route can be a mule and a road cannot.
        m_climbPenalty=26.0f;

        //How far a place will reach for a bigger neighbour before deciding it has none. Without a
        //cutoff every steading searches the whole map for a town it will never walk to.
        m_reach=34.0f;
        m_searchLimit=42.0f;
    }

    float m_reuseWeight;
    float m_climbPenalty;
    float m_reach;
    float m_searchLimit;
};

//--- the interest model ---
//Emilien et al: a weighted sum of independent criteria, each in -1 to 1, where any single -1 makes
//the site impossible however good the rest of it is. Interest is not a decision - the caller then
//accepts the site with a probability that follows it, so a good region fills in unevenly.

const float SettlementImpossible=-1.0f;

//site factors come back in 0 to 1; the interest model works in -1 to 1
inline float toInterest(float value)
{
    return (value*2.0f)-1.0f;
}

//Nothing stands on water, on ground too steep to build on, on ice, or anywhere with nothing to
//drink. A veto, not a penalty.
inline bool settlementImpossible(const InfluenceCell &cell, float relief,
    const SettlementThresholds &thresholds)
{
    if(cell.heightBase<0.5f)
        return true;

    if(relief>=thresholds.m_slopeVeto)
        return true;

    if(siteWater(cell)<=thresholds.m_thirstVeto)
        return true;

    if((cell.climate==ClimateZone::EF)||(cell.climate==ClimateZone::Ocean))
        return true;

    return false;
}

//Weighted sum, with the veto applied first. The species supplies the weights on the land itself,
//the settlement size supplies the weights on wanting neighbours, a road and a hill.
inline float settlementInterest(const SettlementCriteria &criteria, const SpeciesHabitat &habitat,
    const SettlementPreference &preference)
{
    if((criteria.m_water<=SettlementImpossible)||(criteria.m_slope<=SettlementImpossible))
        return 0.0f;

    float weighted=(criteria.m_water*habitat.m_water)
        +(criteria.m_arable*habitat.m_arable)
        +(criteria.m_grazing*habitat.m_grazing)
        +(criteria.m_timber*habitat.m_timber)
        +(criteria.m_stone*habitat.m_stone)
        +(criteria.m_fuel*habitat.m_fuel)
        +(criteria.m_ore*habitat.m_ore);

    float total=habitat.m_water+habitat.m_arable+habitat.m_grazing+habitat.m_timber
        +habitat.m_stone+habitat.m_fuel+habitat.m_ore;

    float land=(total>0.0f)?(weighted/total):0.0f;

    float interest=(land*preference.m_land)
        +(criteria.m_slope*0.25f)
        +(criteria.m_sociability*preference.m_sociability)
        +(criteria.m_accessibility*preference.m_accessibility)
        +(criteria.m_domination*preference.m_domination);

    float scale=preference.m_land+0.25f+preference.m_sociability
        +preference.m_accessibility+preference.m_domination;

    if(scale>0.0f)
        interest/=scale;

    return (interest>0.0f)?interest:0.0f;
}

//The one thing that most explains why somebody stopped here.
inline std::string settlementReason(const Settlement &settlement)
{
    const SettlementCriteria &c=settlement.m_criteria;
    const char *best="the ground was as good as any";
    float most=0.15f;

    if(c.m_arable>most)      { most=c.m_arable;      best="the soil here takes a crop"; }
    if(c.m_water>most)       { most=c.m_water;       best="there is water enough here for anyone"; }
    if(c.m_grazing>most)     { most=c.m_grazing;     best="there is open grass here for herds"; }
    if(c.m_timber>most)      { most=c.m_timber;      best="there is standing wood here to build with"; }
    if(c.m_ore>most)         { most=c.m_ore;         best="there is metal in the ground here"; }
    if(c.m_fuel>most)        { most=c.m_fuel;        best="there is fuel here, so metal can be worked"; }
    if(c.m_stone>most)       { most=c.m_stone;       best="there is workable rock here"; }
    if(c.m_accessibility>most){ most=c.m_accessibility; best="the roads pass this way"; }
    if(c.m_domination>most)  { most=c.m_domination;  best="this ground stands over the country around it"; }
    if(c.m_sociability>most) { most=c.m_sociability; best="there were already people here"; }

    return std::string(settlementKindName(settlement.m_kind))+": "+best+".";
}

//What is unusual about this place, measured against what an ordinary offered site looks like.
//Naming the highest criterion instead gives every site on good ground the same sentence about
//water, because all good ground has water - what a chooser needs is what this one has that the
//others do not.
inline std::string foundingReason(const SettlementCriteria &site, const SettlementCriteria &average)
{
    const char *best="nothing here stands out; it is simply good enough";
    float most=0.10f;

    if((site.m_arable-average.m_arable)>most)
        { most=site.m_arable-average.m_arable; best="the soil here is better than most"; }
    if((site.m_water-average.m_water)>most)
        { most=site.m_water-average.m_water; best="there is more water here than most places have"; }
    if((site.m_grazing-average.m_grazing)>most)
        { most=site.m_grazing-average.m_grazing; best="there is open grass here for more herds than most"; }
    if((site.m_timber-average.m_timber)>most)
        { most=site.m_timber-average.m_timber; best="the wood here is heavier than most"; }
    if((site.m_ore-average.m_ore)>most)
        { most=site.m_ore-average.m_ore; best="there is metal under this ground and not under much else"; }
    if((site.m_fuel-average.m_fuel)>most)
        { most=site.m_fuel-average.m_fuel; best="there is fuel here, and a forge needs it"; }
    if((site.m_stone-average.m_stone)>most)
        { most=site.m_stone-average.m_stone; best="the rock here is worth quarrying"; }
    if((site.m_domination-average.m_domination)>most)
        { most=site.m_domination-average.m_domination; best="this ground stands higher than what surrounds it"; }
    if((site.m_accessibility-average.m_accessibility)>most)
        { most=site.m_accessibility-average.m_accessibility; best="the roads come nearer here than elsewhere"; }

    return best;
}

//The bet a site represents. Two numbers decide it: how good the ground is, and how exposed the
//place is to whoever else is out there. A rich valley with neighbours on every side is not the same
//offer as a poor one nobody has ever wanted, and saying which is which is the difference between
//choosing a place and choosing a name.
inline std::string siteCharacter(const FoundingSite &site)
{
    bool rich=(site.m_interest>=0.42f);
    bool poor=(site.m_interest<0.30f);
    bool crowded=(site.m_neighbours>=4);
    bool lonely=(site.m_neighbours==0);
    bool defensible=(site.m_criteria.m_domination>0.25f);

    if(lonely&&rich)
        return "rich country, and nobody near enough to want it yet";
    if(lonely)
        return "far from anybody, for better and for worse";
    if(rich&&crowded)
        return "good ground, and you will not be the only one who thinks so";
    if(rich)
        return "good ground, with neighbours a walk away";
    if(poor&&defensible)
        return "hard country, but hard to take";
    if(poor)
        return "thin ground; what it has it keeps";
    if(defensible)
        return "middling ground on a height, which counts for something";
    if(crowded)
        return "ordinary country, well settled";

    return "ordinary country, and quiet";
}

}//namespace worldgen

#endif //_worldgen_settlement_h_
