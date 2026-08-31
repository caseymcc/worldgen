#ifndef _worldgen_polity_h_
#define _worldgen_polity_h_

#include "worldgen/export.h"
#include "worldgen/civilization.h"

#include <algorithm>
#include <string>
#include <vector>

namespace worldgen
{

//A state on the map. The reference is emphatic that a pre-modern border is not a line: "the border
//was as far as the Lord could project his power", and "if you ever see a map of pre-modern
//countries, just know that the borders are more of an approximation of how far the rulers could
//project their power than they are hard lines in the sand." So a polity here is not an outline. It
//is a CORE plus a power budget, and the territory is whatever that budget can reach.
struct Polity
{
    std::string m_name;
    size_t m_core;              //influence index of the seat of power
    int m_species;              //who rules it
    ClimateGroup m_homeClimate; //the climate family the core sits in
    bool m_mountainCore;        //raised in high country, which changes the shape it grows into

    size_t m_cells;
    float m_area;               //square kilometres
    size_t m_marchCells;        //frontier ground, held loosely
    size_t m_minorityCells;     //ground that suits a different people to the one that rules it
    float m_meanControl;
    bool m_enclave;             //never absorbed, and surrounded by the power that failed to take it
};

//What it costs a state to hold one cell of ground, in units of its power budget.
//
//The reference's list of what a state reaches for and what stops it: "you got rivers, trade, unity
//and irrigation. Then along coasts for fish, trade, unity and transport. Fertile areas for food...
//Then go up to the natural borders, because it's hard to bring an army and trade over mountains,
//deserts, jungles and rivers." Rivers and coasts are read here as the highways they were - a state
//grows ALONG them - and the mountain, desert and jungle costs are what stop it.
struct PolityThresholds
{
    PolityThresholds()
    {
        m_count=14;
        m_reach=26.0f;
        m_coreSeparation=14;

        //ordinary walkable, farmable country is the unit
        m_plainCost=1.0f;
        m_mountainCost=4.5f;
        m_desertCost=3.4f;
        m_jungleCost=2.6f;
        m_iceCost=5.0f;

        //a state can put an army across a strait, but not cheaply, and it never holds the water
        m_seaCost=6.0f;

        m_riverDiscount=0.55f;
        m_coastDiscount=0.7f;

        //"it gives a sort of home territory advantage for whatever biome they exist in, so
        //expanding out would be a bit easier"
        m_homeDiscount=0.65f;

        //ground nobody wants is still ground somebody has to garrison
        m_barrenPenalty=1.8f;

        //"by the time you reached the frontier, the ability of the rulers to enforce their laws
        //and collect taxes was less effective than in the heartland"
        m_marchControl=0.3f;

        m_mountainElevation=2300.0f;
        m_mountainRelief=0.24f;

        //A holdout needs ground worth defending and a wall to defend it from. Tlaxcala was ONE
        //hole in the Aztec map, not a rash of them - the point of an inconsistency is that it is
        //inconsistent - so the bar is high and the count is capped.
        m_enclaveRelief=0.3f;
        m_enclaveControl=0.3f;
        m_enclaveMaxCells=7;
        m_enclaveLimit=3;
    }

    int m_count;
    float m_reach;
    int m_coreSeparation;

    float m_plainCost;
    float m_mountainCost;
    float m_desertCost;
    float m_jungleCost;
    float m_iceCost;
    float m_seaCost;

    float m_riverDiscount;
    float m_coastDiscount;
    float m_homeDiscount;
    float m_barrenPenalty;

    float m_marchControl;

    float m_mountainElevation;
    float m_mountainRelief;

    float m_enclaveRelief;
    float m_enclaveControl;
    int m_enclaveMaxCells;
    int m_enclaveLimit;
};

//How well a cell would serve as the seat of a power. A core is not simply the best single acre on
//the map: "this should be well in control of the state", which wants a HEARTLAND around it. The
//two great examples given are both crossroads rather than beauty spots - Mali sat "between the gold
//mines of the south and the salt mines of the north", Venice on the trade coming in from the east -
//so reach and resources count for as much as the ground underfoot.
inline float coreScore(const InfluenceCell &cell, float ownHabitability, float heartland,
    int nearbyDeposits)
{
    if(cell.heightBase<0.5f)
        return 0.0f;

    //the heartland matters more than the single cell: a rich valley beats a rich hill
    float score=(ownHabitability*0.35f)+(heartland*0.65f);

    //"since they were mostly connected by two river systems and the Grand Canal, communications
    //and trade were fairly easy"
    if(cell.water==WaterBody::River)
        score*=(cell.streamOrder>=3)?1.35f:1.2f;

    if(cell.coast!=0)
        score*=1.15f;

    //the Mali case: sitting where more than one thing worth carrying comes together
    score*=1.0f+(0.08f*(float)std::min(nearbyDeposits, 4));

    return score;
}

//The cost of pushing rule one cell further out.
inline float expansionCost(const InfluenceCell &cell, float elevation, float relief,
    float habitability, ClimateGroup home, const PolityThresholds &thresholds)
{
    if(cell.heightBase<0.5f)
        return thresholds.m_seaCost;

    float cost=thresholds.m_plainCost;
    ClimateGroup group=climateGroup(cell.climate);

    if((elevation>=thresholds.m_mountainElevation)||(relief>=thresholds.m_mountainRelief))
        cost=thresholds.m_mountainCost;
    else if(group==ClimateGroup::Arid)
        cost=thresholds.m_desertCost;
    else if((cell.climate==ClimateZone::Af)||(cell.climate==ClimateZone::Am))
        cost=thresholds.m_jungleCost;
    else if(group==ClimateGroup::Polar)
        cost=thresholds.m_iceCost;

    //rivers and coasts move grain, troops and messages, so rule runs down them
    if(cell.water==WaterBody::River)
        cost*=thresholds.m_riverDiscount;
    else if(cell.coast!=0)
        cost*=thresholds.m_coastDiscount;

    //home field advantage: ground that farms the way home farms
    if(group==home)
        cost*=thresholds.m_homeDiscount;

    //"fertile areas for food, because nobody likes a starving proletariat class" - poor ground
    //costs more to hold than it returns
    cost*=1.0f+((thresholds.m_barrenPenalty-1.0f)*(1.0f-clamp(habitability*2.0f, 0.0f, 1.0f)));

    return cost;
}

//Power spent to reach a cell, against the budget, read as how firmly the state holds it. 1 in the
//heartland, falling away to nothing at the frontier - the "borderlands and marches" the reference
//describes, with "a bit more autonomy, cultural and political influence from both sides, and a lot
//of fighting".
inline float projectedControl(float spent, float reach)
{
    if(reach<=0.0f)
        return 0.0f;

    return clamp(1.0f-(spent/reach), 0.0f, 1.0f);
}

}//namespace worldgen

#endif //_worldgen_polity_h_
