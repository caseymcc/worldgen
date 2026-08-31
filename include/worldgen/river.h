#ifndef _worldgen_river_h_
#define _worldgen_river_h_

#include "worldgen/export.h"

#include <glm/glm.hpp>

namespace worldgen
{

//A river reads differently along its length, and the reference splits it three ways by the
//gradient it is running down.
enum class RiverStage
{
    None,
    Youthful,   //mountain stage, steep and fast, a deep narrow V cut. Canyons, rapids, waterfalls
    Mature,     //hill stage, moderate. Braided channels, mild meanders, narrow flood plains
    OldAge      //lowland stage, slow and wide. Broad flood plains, hard meanders, oxbows, deltas
};

//What shape a delta takes is decided by a contest between the river pushing sediment out and the
//sea sweeping it away. The reference gives four outcomes, and they run in order of how much
//stronger the sea is than the river.
enum class DeltaShape
{
    None,       //the sea wins outright and carries everything off - no delta forms at all
    Cuspate,    //the current meets the mouth head on, cutting the fan back to a curved V
    Fan,        //the sea only slightly the stronger: the classic triangular delta
    BirdsFoot   //the river wins, and pushes long narrow fingers out into open water
};

inline const char *deltaShapeName(DeltaShape shape)
{
    switch(shape)
    {
    case DeltaShape::Cuspate:   return "Cuspate";
    case DeltaShape::Fan:       return "Fan";
    case DeltaShape::BirdsFoot: return "Bird's foot";
    default: break;
    }
    return "None";
}

//Compares what the river is delivering against what the sea can carry away.
//The reference separates the cuspate case by the current striking the mouth HEAD ON rather than
//running along the shore. That distinction cannot be drawn here: the current model deliberately
//removes the into-shore component of the flow so that water runs along a coast instead of into
//it, which is right for a coastline in general and leaves nothing pointing at a river mouth. So
//cuspate is taken on strength alone - a current strong enough to rework the fan back against the
//shore - which is the same mechanism, just without being able to tell the angle it arrives at.
inline DeltaShape deltaShapeFor(float riverPower, float currentPower)
{
    if(riverPower<=0.0f)
        return DeltaShape::None;

    float ratio=currentPower/riverPower;

    if(ratio>2.5f)
        return DeltaShape::None;    //swept away as fast as it arrives

    if(ratio>0.9f)
        return DeltaShape::Cuspate; //reworked back against the shore into a V

    if(ratio>0.45f)
        return DeltaShape::Fan;

    return DeltaShape::BirdsFoot;   //nothing much to stop it
}

enum class WaterBody
{
    None,
    River,
    Delta,       //the fan a river builds out into standing water
    Lake,        //fresh, drains onward
    GlacialLake, //a hollow an ice sheet gouged out, filled by its meltwater when it went
    SaltLake,    //endorheic and wet enough to stand as open water
    InlandDelta  //endorheic and dry, the river discharges into swamp and evaporates
};

inline const char *riverStageName(RiverStage stage)
{
    switch(stage)
    {
    case RiverStage::Youthful: return "Youthful";
    case RiverStage::Mature:   return "Mature";
    case RiverStage::OldAge:   return "Old age";
    default: break;
    }
    return "None";
}

inline const char *waterBodyName(WaterBody water)
{
    switch(water)
    {
    case WaterBody::River:       return "River";
    case WaterBody::Delta:       return "Delta";
    case WaterBody::Lake:        return "Lake";
    case WaterBody::GlacialLake: return "Glacial lake";
    case WaterBody::SaltLake:    return "Salt lake";
    case WaterBody::InlandDelta: return "Inland delta";
    default: break;
    }
    return "None";
}

struct RiverThresholds
{
    RiverThresholds()
    {
        //a channel has to be draining this much ground before it is worth calling a river
        m_minimumDrainage=60.0f;   //square kilometres

        //gradient, in height field units per cell, splitting the three stages
        m_youthfulGradient=0.010f;
        m_matureGradient=0.003f;

        //rainfall depth at full moisture, millimetres per year
        m_maximumRainfall=2600.0f;

        //a basin whose water evaporates faster than it arrives never reaches the sea
        m_endorheicMoisture=0.32f;
        m_saltLakeMoisture=0.20f;

        //a river only runs all year where there is enough rain to keep it going
        m_permanentMoisture=0.40f;

        //what fraction of its water an ephemeral river carries on to the next cell, the rest
        //going to the ground and the air
        m_ephemeralRetention=0.80f;

        //A delta is sediment dropped where the current slows against standing water, so it needs a
        //slow river carrying enough of it. The land builds outward as more arrives, which is why a
        //coastline is almost never straight where one forms.
        m_deltaMinDischarge=400.0f;   //millions of cubic metres a year
        m_deltaDischargePerCell=350.0f;
        m_deltaMaxCells=14;

        //Glacial lakes sit in hollows an ice sheet cut and left behind. The reference looks for
        //them inside the last glacial maximum's ice, in the wetter ground, near river headwaters -
        //and never below 40 degrees, because ice reaching that far equatorward means the planet has
        //tipped into a runaway freeze.
        m_glacialDrop=-6.0f;        //degrees the planet ran colder at the glacial maximum
        m_glacialLatitude=40.0f;
        m_glacialMoisture=0.55f;
        m_glacialOrder=2;           //at or below this stream order counts as headwaters
    }

    float m_minimumDrainage;
    float m_youthfulGradient;
    float m_matureGradient;
    float m_maximumRainfall;
    float m_endorheicMoisture;
    float m_saltLakeMoisture;
    float m_permanentMoisture;
    float m_ephemeralRetention;
    float m_deltaMinDischarge;
    float m_deltaDischargePerCell;
    int m_deltaMaxCells;
    float m_glacialDrop;
    float m_glacialLatitude;
    float m_glacialMoisture;
    int m_glacialOrder;
};

//A drainage basin: everything that drains out through one point. Exoreic if that point is the sea,
//endoreic if the water has nowhere to go but the air. The reference wants no more than about a
//fifth of basins to be endoreic.
struct DrainageBasin
{
    size_t outlet;      //the cell the whole basin drains through
    float area;         //square kilometres
    float discharge;    //millions of cubic metres a year
    int riverCells;
    bool endoreic;
};

//The reference's depth factor: a wider provisional channel runs proportionally deeper, so the
//water surface ends up narrower than the provisional figure by this much.
inline float riverDepthFactor(float provisionalWidth)
{
    if(provisionalWidth>=500.0f)
        return 5.0f;
    if(provisionalWidth>=200.0f)
        return 4.0f;
    if(provisionalWidth>=100.0f)
        return 3.0f;
    if(provisionalWidth>=20.0f)
        return 2.0f;

    return 1.0f;
}

//Width of a river from what it drains, straight out of the reference:
//    B = (R/4) x A x 0.1     W = B / F
//with R the rainfall over the drained ground in millions of cubic metres a year, A the drained
//area in thousands of square kilometres, and B and W in metres. Their worked example - 2000 square
//kilometres taking 500 million cubic metres a year - gives B of 25 and a final width of 12.5.
inline float riverWidth(float dischargeMillionM3, float areaThousandKm2)
{
    float provisional=(dischargeMillionM3/4.0f)*areaThousandKm2*0.1f;

    if(provisional<=0.0f)
        return 0.0f;

    return provisional/riverDepthFactor(provisional);
}

inline RiverStage riverStageForGradient(float gradient, const RiverThresholds &thresholds)
{
    if(gradient>=thresholds.m_youthfulGradient)
        return RiverStage::Youthful;
    if(gradient>=thresholds.m_matureGradient)
        return RiverStage::Mature;

    return RiverStage::OldAge;
}

}//namespace worldgen

#endif //_worldgen_river_h_
