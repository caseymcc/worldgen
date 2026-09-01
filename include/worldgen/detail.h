#ifndef _worldgen_detail_h_
#define _worldgen_detail_h_

#include "worldgen/export.h"
#include "worldgen/tectonics.h"
#include "worldgen/maths/math_helpers.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace worldgen
{

//One sample of a detail map. An influence cell covers thousands of blocks; this is what the ground
//inside it actually looks like once somebody stands on it.
struct DetailCell
{
    DetailCell():
        base(0.0f),
        height(0.0f),
        elevation(0.0f),
        water(0.0f),
        channelWidth(0.0f),
        discharge(0.0f),
        channel(false),
        floodplain(false),
        sea(false),
        saltFlat(false)
    {}

    float base;         //the coarse value this sample was amplified from, kept for verification
    float height;       //normalized, the same 0 to 1 convention as InfluenceCell::heightBase
    float elevation;    //metres
    float water;        //water depth in metres, 0 on dry ground
    float channelWidth; //metres of open water across, which is not the same as samples across
    float discharge;    //cubic metres a second passing this sample
    bool channel;       //on the routed river course
    bool floodplain;    //ground the channel has worked over
    bool sea;           //below sea level, so the water here is the sea rather than a river
    bool saltFlat;      //a closed basin in dry country: what is left after the water evaporated
};

//Everything the detail pass is allowed to decide for itself. Everything it is NOT allowed to decide
//- where the river enters, where it leaves, how much water it carries, what the ground averages -
//comes from the overview map and is a boundary condition, not a parameter.
struct DetailThresholds
{
    DetailThresholds()
    {
        m_resolution=256;

        //Amplification. The residual is sampled in WORLD space, so two neighbouring regions read
        //the same noise along their shared edge and agree there exactly, and a region regenerates
        //identically from the seed without being stored.
        //How much relief to inject, as a share of how much the neighbouring cells differ by. This
        //is a share of an ELEVATION difference: expressed as a share of normalized height instead,
        //the same number would be worth a few metres beside the sea and several kilometres at the
        //top of the range, because the height curve is anything but linear.
        m_residualScale=0.30f;
        m_minimumRelief=45.0f;  //metres of relief even where the overview says the ground is level
        m_residualFrequency=0.012f;
        m_octaves=5;
        m_lacunarity=2.0f;
        m_gain=0.45f;

        //depression filling: the surface is left this much proud so water has somewhere to go.
        //In metres, because the whole pass works in metres.
        m_epsilon=0.001f;

        //how much water a filled hollow has to hold before it counts as a lake rather than damp ground
        m_lakeDepth=12.0f;

        //A basin only holds water if what runs into it beats what evaporates out of it. In dry
        //country it does not, and a closed basin there is a salt pan rather than a lake - which is
        //where salt flats come from in the first place. A big enough river can still sustain a
        //terminal lake in a desert, the way the Nile and the Jordan do.
        m_lakeMoisture=0.38f;
        m_lakeRiver=25.0f;      //cubic metres a second passing through

        //Galin's coprime segment mask. k=5 is the author's recommendation: 11.3 degrees of angle
        //resolution against 45 for plain 8-connectivity.
        m_segmentMask=5;

        //water does not climb, so gaining height along the channel is what the routing pays for
        m_climbPenalty=48.0f;

        //A least-cost path is the straightest line the ground allows, and a real river is not that.
        //How far it wanders is a fact about the ground rather than a setting: a torrent in a gorge
        //runs straight because it has nowhere else to go, and a big river on a flood plain wanders
        //hard because nothing stops it. m_meanderSlope is the gradient above which a river stops
        //bothering.
        m_meanderAmplitude=0.16f;   //share of the region it may wander across at its loosest
        m_meanderWavelength=2.4f;   //loops per traverse of the region
        m_meanderSlope=0.045f;

        //the road paper's re-use weight, which is what makes tributaries join the stem instead of
        //running alongside it
        m_reuseWeight=0.08f;

        //Velocity model. Riverscapes gives the cross-section area as discharge over velocity but
        //never defines the velocity, so it is chosen here: holding u constant would make wetted
        //area scale as Q, where Leopold and Maddock's downstream hydraulic geometry has width as
        //Q^0.5 and depth as Q^0.4, so area goes as Q^0.9 and velocity itself as Q^0.1.
        m_velocity=0.6f;            //metres a second at the reference discharge
        m_velocityDischarge=10.0f;  //that reference, in cubic metres a second
        m_velocityExponent=0.1f;

        //width over depth at the reference discharge, and how it opens out downstream
        //(w ~ Q^0.5 against d ~ Q^0.4 leaves the ratio going as Q^0.1)
        m_widthDepthRatio=14.0f;
        m_widthDepthExponent=0.1f;

        m_bankSlope=2.5f;       //metres out for every metre down
        m_floodplainWidth=3.0f; //multiples of channel width

        //How deep the water gets. heightToElevation flattens everything at or below sea level to
        //zero, which is right for a land elevation but leaves the sea floor with no shape at all,
        //so the sea is measured downwards on its own curve.
        m_maxDepth=6000.0f;
        m_depthCurve=1.5f;
    }

    int m_resolution;

    float m_residualScale;
    float m_minimumRelief;
    float m_residualFrequency;
    int m_octaves;
    float m_lacunarity;
    float m_gain;

    float m_epsilon;
    float m_lakeDepth;
    float m_lakeMoisture;
    float m_lakeRiver;

    int m_segmentMask;
    float m_climbPenalty;
    float m_meanderAmplitude;
    float m_meanderWavelength;
    float m_meanderSlope;
    float m_reuseWeight;

    float m_velocity;
    float m_velocityDischarge;
    float m_velocityExponent;
    float m_widthDepthRatio;
    float m_widthDepthExponent;

    float m_bankSlope;
    float m_floodplainWidth;
    float m_maxDepth;
    float m_depthCurve;
};

//Where water crosses the edge of a region. These are read off the overview and are not negotiable:
//the neighbouring region reads the same numbers from the same place, which is what makes two
//regions generated at different times agree without either knowing about the other.
struct DetailFlow
{
    glm::ivec2 m_at;      //the sample on the region boundary
    float m_discharge;    //cubic metres a second
    int m_direction;      //which of the eight neighbours it comes from or goes to
};

struct DetailMap
{
    DetailMap():
        m_size(0),
        m_metresPerSample(0.0f),
        m_hasRiver(false),
        m_inflow(0.0f),
        m_outflow(0.0f),
        m_baseMean(0.0f),
        m_amplifiedMean(0.0f),
        m_lowest(0.0f),
        m_highest(0.0f),
        m_channelSamples(0),
        m_seaSamples(0),
        m_lakeSamples(0),
        m_saltFlatSamples(0),
        m_widest(0.0f),
        m_filledSamples(0)
    {}

    const DetailCell &at(int x, int y) const { return m_cells[((size_t)y*m_size)+x]; }
    DetailCell &at(int x, int y) { return m_cells[((size_t)y*m_size)+x]; }

    glm::ivec2 m_cell;          //which influence cell this is the inside of
    int m_size;                 //samples per side
    float m_metresPerSample;
    std::vector<DetailCell> m_cells;

    std::vector<DetailFlow> m_entries;
    DetailFlow m_exit;
    bool m_hasRiver;
    float m_inflow;             //what the overview says arrives, cubic metres a second
    float m_outflow;            //what the overview says leaves

    //verification, carried on the map rather than printed: the amplification is meant to add
    //detail without moving the ground, so these two should agree closely
    float m_baseMean;
    float m_amplifiedMean;
    float m_lowest;             //metres
    float m_highest;
    size_t m_channelSamples;
    size_t m_seaSamples;    //how much of the region is under the sea
    size_t m_lakeSamples;
    size_t m_saltFlatSamples;
    float m_widest;             //the widest the channel gets anywhere in the region, in metres
    size_t m_filledSamples;     //how many samples depression filling had to raise
};

//NOTE: every pow here is qualified std::pow deliberately. maths/math_helpers.h declares
//worldgen::pow(_Type, unsigned) - an integer power - and an unqualified call from inside this
//namespace finds THAT first, truncating a fractional exponent to an integer. It fails silently:
//an exponent of 1.5 becomes 1 and the curve goes linear, and 0.667 becomes 0 so the function
//returns 1.0 for every input.
//
//How far below the surface the sea floor sits, in metres. The mirror of heightToElevation, which
//returns zero for everything at or below sea level.
inline float heightToDepth(float height, float seaLevel, float maxDepth, float curve=1.5f)
{
    float below=(seaLevel-height)/seaLevel;

    if(below<=0.0f)
        return 0.0f;

    return maxDepth*std::pow(below, curve);
}

//How much a river wanders, from the ground rather than from a setting. Steep runs straight; flat
//and full wanders. Returns 0 to 1.
inline float sinuosity(float gradient, float discharge, const DetailThresholds &rules)
{
    float flat=1.0f-clamp(gradient/rules.m_meanderSlope, 0.0f, 1.0f);
    float full=clamp(discharge/rules.m_velocityDischarge, 0.0f, 1.0f);

    //a trickle on a plain still wanders, but not as far as a river does
    return flat*(0.35f+(0.65f*full));
}

//The exact inverses of the two height curves. Carving works in metres and the rest of the library
//works in normalized height, so the two have to be converted between - and doing that from the
//local gradient falls apart at the shoreline, where the elevation curve is flat and a metre of
//depth maps to an unbounded change in height.
inline float elevationToHeight(float elevation, float seaLevel, float maxElevation, float curve=1.5f)
{
    if(elevation<=0.0f)
        return seaLevel;

    return seaLevel+((1.0f-seaLevel)*std::pow(elevation/maxElevation, 1.0f/curve));
}

inline float depthToHeight(float depth, float seaLevel, float maxDepth, float curve=1.5f)
{
    if(depth<=0.0f)
        return seaLevel;

    return seaLevel-(seaLevel*std::pow(depth/maxDepth, 1.0f/curve));
}

//The signed height of a cell in metres, and its inverse. Working in metres rather than in
//normalized height is what keeps injected detail the same physical size everywhere: a unit of
//height is worth a few metres beside the sea and tens of thousands at the top of the curve.
inline float heightToSigned(float height, float seaLevel, float maxElevation, float elevationCurve,
    float maxDepth, float depthCurve)
{
    if(height>=seaLevel)
        return heightToElevation(height, seaLevel, maxElevation, elevationCurve);

    return -heightToDepth(height, seaLevel, maxDepth, depthCurve);
}

inline float signedToHeight(float signed_, float seaLevel, float maxElevation, float elevationCurve,
    float maxDepth, float depthCurve)
{
    if(signed_>=0.0f)
        return elevationToHeight(signed_, seaLevel, maxElevation, elevationCurve);

    return depthToHeight(-signed_, seaLevel, maxDepth, depthCurve);
}

//Catmull-Rom through four samples. Unlike a bilinear blend of averaged corners it passes THROUGH
//the values it is given, so a cell's own height survives into the middle of its own region - and it
//is still continuous across a boundary, because both sides evaluate the same function of the same
//world position.
inline float catmullRom(float p0, float p1, float p2, float p3, float t)
{
    float result=0.5f*((2.0f*p1)
        +((p2-p0)*t)
        +(((2.0f*p0)-(5.0f*p1)+(4.0f*p2)-p3)*t*t)
        +(((3.0f*p1)-(3.0f*p2)+p3-p0)*t*t*t));

    //A cubic through a step overshoots, and at a coastline an overshoot puts dry land under water
    //or lifts the sea floor into the air. Holding the result between the two samples it sits
    //between costs nothing anywhere else.
    float low=std::min(p1, p2);
    float high=std::max(p1, p2);

    return (result<low)?low:((result>high)?high:result);
}

//Bilinear over the four corner values of a cell. The corners are shared with the neighbouring
//cells, so a field built this way is continuous across every cell boundary by construction - which
//is the whole reason the overview keeps corner values rather than cell values.
inline float bilinearCorners(float nw, float ne, float sw, float se, float x, float y)
{
    return ((nw*(1.0f-x))+(ne*x))*(1.0f-y)+(((sw*(1.0f-x))+(se*x))*y);
}

//--- channel geometry from discharge ---
//This is the mechanism by which one overview river is a stream in one region and a torrent in the
//next: the same normalized cross section, scaled by how much water has to fit through it.

inline float channelVelocity(float discharge, const DetailThresholds &rules)
{
    if(discharge<=0.0f)
        return rules.m_velocity;

    return rules.m_velocity*powf(discharge/rules.m_velocityDischarge, rules.m_velocityExponent);
}

//continuity: Q = A * v, so the wetted area is whatever the discharge needs at that speed
inline float channelArea(float discharge, const DetailThresholds &rules)
{
    float velocity=channelVelocity(discharge, rules);

    return (velocity>0.0f)?(discharge/velocity):0.0f;
}

//Split that area into a width and a depth. Keeping the width over depth ratio itself growing with
//discharge is what puts width on Q^0.5 and depth on Q^0.4 rather than both on the square root.
inline void channelShape(float discharge, const DetailThresholds &rules, float &width, float &depth)
{
    float area=channelArea(discharge, rules);

    if(area<=0.0f)
    {
        width=0.0f;
        depth=0.0f;
        return;
    }

    float ratio=rules.m_widthDepthRatio;

    if(discharge>0.0f)
        ratio*=powf(discharge/rules.m_velocityDischarge, rules.m_widthDepthExponent);

    ratio=std::max(ratio, 1.0f);

    width=sqrtf(area*ratio);
    depth=sqrtf(area/ratio);
}

//The generator stores discharge as millions of cubic metres a year, which is the unit the overview
//pass works in. Channel geometry wants cubic metres a second.
inline float dischargePerSecond(float millionsPerYear)
{
    return (millionsPerYear*1.0e6f)/31557600.0f;
}

//--- Galin's coprime segment mask ---
//Four and eight connectivity cap the achievable angle at 45 degrees, which shows up as a visibly
//faceted channel. Taking every segment to (i, j) in [-k, k] squared with gcd(i, j) = 1 gets that
//down to arctan(1/k) without checking any segment twice - a longer segment through the same
//direction is a multiple of a shorter one and adds nothing.
inline std::vector<glm::ivec2> coprimeSegmentMask(int k)
{
    std::vector<glm::ivec2> mask;

    if(k<1)
        k=1;

    for(int j=-k; j<=k; ++j)
    {
        for(int i=-k; i<=k; ++i)
        {
            if((i==0)&&(j==0))
                continue;

            if(std::gcd(abs(i), abs(j))!=1)
                continue;

            mask.push_back(glm::ivec2(i, j));
        }
    }

    return mask;
}

}//namespace worldgen

#endif //_worldgen_detail_h_
