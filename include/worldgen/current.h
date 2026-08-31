#ifndef _worldgen_current_h_
#define _worldgen_current_h_

#include "worldgen/export.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <glm/glm.hpp>

namespace worldgen
{

//Surface currents are the wind's doing: it drags the top of the ocean along with it, and because
//the planet spins, that drag closes into big loops - gyres - running clockwise in the northern
//hemisphere and anticlockwise in the southern. Water holds heat far better than air does, so those
//loops carry warmth away from the equator up one side of an ocean basin and bring cold water back
//down the other. Which side a coast sits on decides whether it is a rainforest or a desert.
enum class CurrentKind
{
    None,   //land, or water going nowhere much
    Warm,   //carrying tropical water polewards
    Cold,   //carrying polar water back towards the equator
    Neutral
};

inline const char *currentKindName(CurrentKind kind)
{
    switch(kind)
    {
    case CurrentKind::Warm:    return "Warm current";
    case CurrentKind::Cold:    return "Cold current";
    case CurrentKind::Neutral: return "Neutral current";
    default: break;
    }
    return "None";
}

inline glm::ivec3 currentKindColor(CurrentKind kind)
{
    switch(kind)
    {
    case CurrentKind::Warm:    return glm::ivec3(220, 70, 60);
    case CurrentKind::Cold:    return glm::ivec3(60, 130, 230);
    case CurrentKind::Neutral: return glm::ivec3(80, 100, 130);
    default: break;
    }
    return glm::ivec3(30, 40, 60);
}

struct CurrentThresholds
{
    CurrentThresholds()
    {
        //Ekman transport turns the surface flow off the wind direction - to the right of it in the
        //northern hemisphere, the left in the southern. This is what curls the wind belts into
        //closed gyres rather than leaving them as straight streams.
        m_deflection=25.0f;   //degrees

        //how many times to push the flow back off the shore, so a current runs along a coast
        //rather than into it
        m_coastRelaxation=14;

        //how much of its temperature a parcel of water keeps as it moves on, against how much it
        //takes from the latitude it is passing through. Higher carries warmth further.
        m_retention=0.93f;
        m_advectionPasses=40;

        //how far off the local sea temperature a current has to be before it counts as warm or cold
        m_warmthThreshold=1.2f;   //degrees

        //what a current does to the coast it washes: how much of its temperature anomaly the land
        //takes, and how much drier a cold current leaves that shore
        m_coastTemperature=0.75f;
        m_coastMoisture=0.055f;   //moisture per degree of anomaly

        //Wind blowing off the land drags the surface water away from the shore with it, and cold
        //water rises from below to replace what left. That upwelled water is cold and full of
        //nutrient, which is why the great fisheries sit off desert coasts.
        m_upwellingWind=0.55f;    //how much offshore wind it takes to start one
        m_upwellingChill=3.5f;

        //Overturning. Nine tenths of the ocean moves not because the wind pushes it but because
        //cold salty water at high latitude is dense enough to sink, dragging warm water poleward
        //behind it to replace what went down. That limb delivers an enormous amount of heat a very
        //long way - London and Calgary sit at the same latitude and London is nothing like as cold.
        m_overturningLatitude=42.0f;
        m_overturningStrength=11.0f;  //degrees the limb can add where it runs hardest

        //The limb is a narrow, hard-running boundary current, not a general poleward drift. Any
        //water heading vaguely poleward at high latitude is most of the high-latitude ocean, and
        //treating all of it as overturning spreads the heat so thin the effect vanishes.
        m_overturningFlow=0.25f;    //degrees the upwelled water is below the surface it replaced
    }

    float m_deflection;
    int m_coastRelaxation;
    float m_retention;
    int m_advectionPasses;
    float m_warmthThreshold;
    float m_coastTemperature;
    float m_coastMoisture;
    float m_upwellingWind;
    float m_upwellingChill;
    float m_overturningLatitude;
    float m_overturningStrength;
    float m_overturningFlow;
};

//Turns the wind into the water moving under it: the same direction, swung by the Coriolis
//deflection, which is to the right going north and to the left going south. A retrograde planet
//spins the other way and so does everything downstream of it.
inline glm::vec2 windDrivenCurrent(const glm::vec2 &wind, float latitude, float deflection, float spin=1.0f)
{
    float length=glm::length(wind);

    if(length<=0.0f)
        return glm::vec2(0.0f, 0.0f);

    float angle=deflection*spin;

    if(latitude<0.0f)
        angle=-angle;

    float cosAngle=cos(angle);
    float sinAngle=sin(angle);

    //clockwise in the northern hemisphere, which is what closes a gyre
    return glm::vec2((wind.x*cosAngle)+(wind.y*sinAngle), (wind.y*cosAngle)-(wind.x*sinAngle));
}

//How much extra warmth the overturning limb delivers here. It only runs where water is already
//heading poleward at high latitude - that is the returning surface branch of the deep circulation,
//and it is carrying heat gathered across a whole ocean rather than from one cell upstream.
inline float overturningWarmth(const glm::vec2 &current, float latitude, const CurrentThresholds &thresholds)
{
    float degrees=(float)fabs(latitude)*180.0f/(float)M_PI;

    if(degrees<thresholds.m_overturningLatitude)
        return 0.0f;

    //poleward is +y in the north and -y in the south
    float poleward=(latitude>=0.0f)?current.y:-current.y;

    if(poleward<thresholds.m_overturningFlow)
        return 0.0f;

    //strengthens with latitude, since the further poleward the water gets the further it has
    //carried that heat from
    float reach=(degrees-thresholds.m_overturningLatitude)/(90.0f-thresholds.m_overturningLatitude);

    return thresholds.m_overturningStrength*std::min(poleward, 1.0f)*std::min(reach*2.0f, 1.0f);
}

inline CurrentKind classifyCurrent(float anomaly, float speed, const CurrentThresholds &thresholds)
{
    if(speed<=0.0f)
        return CurrentKind::None;

    if(anomaly>=thresholds.m_warmthThreshold)
        return CurrentKind::Warm;

    if(anomaly<=-thresholds.m_warmthThreshold)
        return CurrentKind::Cold;

    return CurrentKind::Neutral;
}

}//namespace worldgen

#endif //_worldgen_current_h_
