#ifndef _worldgen_climate_h_
#define _worldgen_climate_h_

#include "worldgen/export.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace worldgen
{

//The Koppen zones, in the groups the classification uses: A tropical, B arid, C subtropical,
//D continental, E polar.
enum class ClimateZone
{
    Ocean,
    Af,     //tropical rainforest, hot and wet year round
    Am,     //tropical monsoon, a long very wet season and a short dry one
    Aw,     //tropical savanna, wet summers and long dry winters
    BWh,    //hot desert
    BSh,    //hot steppe
    BWk,    //cold desert
    BSk,    //cold steppe
    Csa,    //mediterranean, hot dry summer
    Csb,    //mediterranean, warm dry summer. Always sits poleward of its Csa siblings
    Cfa,    //humid subtropical, hot summer
    Cfb,    //oceanic, no dry season and a narrow temperature range
    Cfc,    //subpolar oceanic - mild winters still, but cool summers
    Dfa,    //humid continental, hot summer
    Dfb,    //humid continental, warm summer
    Dfc,    //subarctic, taiga
    ET,     //polar tundra
    EF      //polar ice cap
};

constexpr size_t ClimateZoneCount=18;

inline const char *climateZoneName(ClimateZone zone)
{
    switch(zone)
    {
    case ClimateZone::Af:  return "Tropical rainforest";
    case ClimateZone::Am:  return "Tropical monsoon";
    case ClimateZone::Aw:  return "Tropical savanna";
    case ClimateZone::BWh: return "Hot desert";
    case ClimateZone::BSh: return "Hot steppe";
    case ClimateZone::BWk: return "Cold desert";
    case ClimateZone::BSk: return "Cold steppe";
    case ClimateZone::Csa: return "Mediterranean (hot summer)";
    case ClimateZone::Csb: return "Mediterranean (warm summer)";
    case ClimateZone::Cfa: return "Humid subtropical";
    case ClimateZone::Cfc: return "Subpolar oceanic";
    case ClimateZone::Cfb: return "Oceanic";
    case ClimateZone::Dfa: return "Humid continental (hot)";
    case ClimateZone::Dfb: return "Humid continental (warm)";
    case ClimateZone::Dfc: return "Subarctic";
    case ClimateZone::ET:  return "Tundra";
    case ClimateZone::EF:  return "Ice cap";
    default: break;
    }
    return "Ocean";
}

//the colours the Koppen maps conventionally use, so the output reads like one
inline glm::ivec3 climateZoneColor(ClimateZone zone)
{
    switch(zone)
    {
    case ClimateZone::Af:  return glm::ivec3(0, 0, 254);
    case ClimateZone::Am:  return glm::ivec3(0, 119, 255);
    case ClimateZone::Aw:  return glm::ivec3(70, 169, 250);
    case ClimateZone::BWh: return glm::ivec3(254, 0, 0);
    case ClimateZone::BSh: return glm::ivec3(245, 163, 1);
    case ClimateZone::BWk: return glm::ivec3(255, 150, 149);
    case ClimateZone::BSk: return glm::ivec3(255, 219, 99);
    case ClimateZone::Csa: return glm::ivec3(255, 255, 0);
    case ClimateZone::Csb: return glm::ivec3(198, 199, 0);
    case ClimateZone::Cfa: return glm::ivec3(200, 255, 80);
    case ClimateZone::Cfb: return glm::ivec3(100, 255, 80);
    case ClimateZone::Cfc: return glm::ivec3(50, 199, 0);
    case ClimateZone::Dfa: return glm::ivec3(0, 255, 255);
    case ClimateZone::Dfb: return glm::ivec3(55, 200, 255);
    case ClimateZone::Dfc: return glm::ivec3(0, 125, 125);
    case ClimateZone::ET:  return glm::ivec3(178, 178, 178);
    case ClimateZone::EF:  return glm::ivec3(110, 110, 110);
    default: break;
    }
    return glm::ivec3(12, 36, 92);
}

struct ClimateThresholds
{
    ClimateThresholds()
    {
        //temperature groups, in celsius. Real Koppen works off monthly means; this generator only
        //carries an annual figure, so the cuts are set against annual means instead.
        m_tropicalTemperature=18.0f;
        m_subtropicalTemperature=8.0f;
        m_continentalTemperature=-2.0f;
        m_iceCapTemperature=-12.0f;
        m_iceCapMoistureBonus=6.0f;
        m_hotAridTemperature=12.0f;

        //Moisture, on the influence map's 0 to 1 scale. These are set against where the generator's
        //moisture field actually falls rather than against absolute rainfall, so that each zone
        //ends up with roughly the share of land it holds on earth.
        m_aridMoisture=0.22f;
        m_semiAridMoisture=0.42f;
        m_rainforestMoisture=0.88f;
        m_monsoonMoisture=0.84f;
        m_mediterraneanMoisture=0.62f;

        //latitudes the reference gives for the tropical subtypes
        m_rainforestLatitude=12.0f;
        m_tropicalLatitude=22.0f;
        m_mediterraneanLatitudeLow=28.0f;
        m_mediterraneanLatitudeHigh=48.0f;

        //a coast where the wind blows in off the sea against one where it blows out over it
        m_onshore=0.15f;
        m_continentalInterior=0.20f;

        //The third letter of a Koppen code is how hot the summer gets: a hot, b warm, c cool.
        //Real Koppen reads that off monthly means, which this generator does not carry, so the
        //split is taken on the annual mean instead - which still separates the zones in the right
        //order, since a place with hot summers has a warmer year.
        m_hotSummerSubtropical=14.0f;
        m_hotSummerContinental=6.0f;
        m_coolSummerOceanic=7.0f;
    }

    float m_tropicalTemperature;
    float m_subtropicalTemperature;
    float m_continentalTemperature;
    float m_iceCapTemperature;
    float m_iceCapMoistureBonus;
    float m_hotAridTemperature;

    float m_aridMoisture;
    float m_semiAridMoisture;
    float m_rainforestMoisture;
    float m_monsoonMoisture;
    float m_mediterraneanMoisture;

    float m_rainforestLatitude;
    float m_tropicalLatitude;
    float m_mediterraneanLatitudeLow;
    float m_mediterraneanLatitudeHigh;

    float m_onshore;
    float m_continentalInterior;
    float m_hotSummerSubtropical;
    float m_hotSummerContinental;
    float m_coolSummerOceanic;
};

//Annual mean surface temperature by latitude, fitted to earth: 26C at the equator, 20 at 30
//degrees, 1 at 60, -25 at the pole. globalTemperature shifts the whole planet off that baseline -
//the reference works from earth about 5 to 6 degrees colder (the last glacial maximum) and about 2
//to 3 warmer (the last interglacial), but says to read the guidelines across a wider range than
//the two they were derived from.
inline float latitudeTemperature(float latitude, float globalTemperature=0.0f, float polarAmplification=1.2f)
{
    float x=(float)fabs(latitude)/(float)M_PI_2;
    float x2=x*x;

    //A change in a planet's heat budget does not land evenly. The poles swing far harder than the
    //tropics - at the last glacial maximum the tropics gave up a couple of degrees while the high
    //latitudes gave up ten and grew ice sheets. Weighted so the area average still comes to the
    //requested figure: sin^2(latitude) averages 1/3 over a sphere, so a base of (1 - amp/3) keeps
    //the mean honest while the poles take amp times more than the equator.
    float sinLatitude=sin(latitude);
    float weight=(1.0f-(polarAmplification/3.0f))+(polarAmplification*sinLatitude*sinLatitude);

    return 26.0f-(60.0f*x2)+(9.0f*x2*x2)+(globalTemperature*weight);
}

//A colder planet is a drier one - air near freezing cannot carry what tropical air carries - which
//is why the reference tells you not to be stingy with deserts on a cold earth even though the poles
//are icing over. Applied as a gamma rather than a plain multiplier, because the drying does not
//fall evenly: the equatorial rain belt keeps raining on a cold earth while the margins give way to
//desert, and a flat multiply would take the most off the wettest ground and wipe the tropics out.
inline float applyGlobalMoisture(float moisture, float globalTemperature, float strength=0.05f)
{
    if(moisture<=0.0f)
        return 0.0f;

    float gamma=1.0f-(strength*globalTemperature);

    return std::pow(moisture, gamma);
}

//Height on the influence map is not proportional to elevation. Real land is overwhelmingly low -
//earth averages about 840m - with the high ground confined to narrow belts, so mapping the height
//field straight onto metres turns every ordinary lowland into a plateau and freezes it. Curving it
//keeps the mountains tall while letting the bulk of the land sit near sea level.
inline float heightToElevation(float heightBase, float seaLevel, float maxElevation, float curve=2.0f)
{
    float above=(heightBase-seaLevel)/(1.0f-seaLevel);

    if(above<=0.0f)
        return 0.0f;

    return maxElevation*std::pow(above, curve);
}

//Air cools as it rises, about 6.5C per kilometre, which is why a mountain carries polar climates
//at tropical latitudes and why cold deserts sit higher than hot ones.
inline float elevationTemperature(float elevationMetres, float lapseRate=6.5f)
{
    if(elevationMetres<=0.0f)
        return 0.0f;

    return -(lapseRate*(elevationMetres/1000.0f));
}

//Land heats and cools faster and further than water does, so a continental interior runs hotter
//than a coast at the same latitude while the ocean holds the coast near its own temperature.
inline float continentalTemperature(float continentality, float latitude, float strength=8.0f)
{
    //the effect reverses with latitude: interiors bake in the tropics and freeze near the poles,
    //because what the ocean really does is damp the swing either way
    float x=(float)fabs(latitude)/(float)M_PI_2;

    return continentality*strength*(1.0f-(2.2f*x*x));
}

//Locate the rough zone by temperature and latitude, then settle it with moisture, how far inland
//it sits and whether the prevailing wind comes off the sea or off the land. Aridity is checked
//first because deserts and steppes cut straight across the other bands.
//coldCurrent says whether the sea washing this coast is bringing polar water down past it. The
//reference puts mediterranean climates on the cold current flank of a continent and humid
//subtropical on the warm one, which is why they end up on opposite sides of the same landmass.
//westernFlank says the sea lies to the west of this ground. Three separate references put
//mediterranean climates on the WESTERN side of a continent and humid subtropical on the EASTERN,
//and that is a geographic fact about which flank a coast is on rather than something that can be
//inferred reliably from wind or current on a world whose oceans form no proper gyres.
inline ClimateZone classifyClimate(float latitude, float temperature, float moisture, float continentality,
    float onshore, const ClimateThresholds &thresholds, int coldCurrent=0, bool westernFlank=false)
{
    float latitudeDegrees=(float)fabs(latitude)*180.0f/(float)M_PI;

    //Polar comes first, it outranks dryness - an ice cap is a desert but it is an ice cap. How far
    //the ice reaches is set by precipitation as much as by cold, though: a glacier has to be fed
    //to grow, so a cold wet coast ices over while a cold dry interior stays bare tundra.
    float iceCapLimit=thresholds.m_iceCapTemperature+(thresholds.m_iceCapMoistureBonus*(moisture-0.5f));

    if(temperature<iceCapLimit)
        return ClimateZone::EF;

    if(temperature<thresholds.m_continentalTemperature)
        return ClimateZone::ET;

    //deserts and steppes are defined by how dry they are, not by where they sit
    if(moisture<thresholds.m_aridMoisture)
        return (temperature>=thresholds.m_hotAridTemperature)?ClimateZone::BWh:ClimateZone::BWk;

    if(moisture<thresholds.m_semiAridMoisture)
        return (temperature>=thresholds.m_hotAridTemperature)?ClimateZone::BSh:ClimateZone::BSk;

    //tropical group
    if((temperature>=thresholds.m_tropicalTemperature)&&(latitudeDegrees<thresholds.m_tropicalLatitude))
    {
        if((latitudeDegrees<thresholds.m_rainforestLatitude)&&(moisture>=thresholds.m_rainforestMoisture))
            return ClimateZone::Af;

        if(moisture>=thresholds.m_monsoonMoisture)
            return ClimateZone::Am;

        return ClimateZone::Aw;
    }

    //subtropical group. The reference's headline result is mediterranean down one flank of a
    //continent and humid subtropical down the other, which follows the wind coming off the land
    //against coming in off the sea.
    if(temperature>=thresholds.m_subtropicalTemperature)
    {
        //Warm enough for the C group but well inland, so the sea never gets to damp the swing:
        //hot summers and hard winters, which is the hot-summer continental climate. This is the
        //only branch Dfa can come from - the D group proper is defined here by annual mean, and
        //a hot summer with a freezing winter averages out warmer than that cut allows.
        if(continentality>thresholds.m_continentalInterior)
            return ClimateZone::Dfa;

        //Mediterranean wants the drier flank: the one a cold current runs past, or failing that
        //the one the wind leaves over rather than arrives from. It only runs over a fairly narrow
        //band of latitude either way.
        //It has to be the western flank, and dry with it - a cold current running past or the
        //wind leaving over the land rather than arriving off the sea.
        bool dryFlank=westernFlank&&((coldCurrent>0)||(onshore<-thresholds.m_onshore)
            ||(moisture<thresholds.m_mediterraneanMoisture*0.75f));

        if(dryFlank&&(moisture<thresholds.m_mediterraneanMoisture)
            &&(latitudeDegrees>=thresholds.m_mediterraneanLatitudeLow)
            &&(latitudeDegrees<=thresholds.m_mediterraneanLatitudeHigh))
        {//Csb always sits poleward of its Csa siblings on the same western coast
            return (temperature>=thresholds.m_hotSummerSubtropical)?ClimateZone::Csa:ClimateZone::Csb;
        }

        return ClimateZone::Cfa;
    }

    //continental group. Dfa has hot summers, Dfb warm ones, Dfc is the subarctic beyond both.
    if(continentality>thresholds.m_continentalInterior)
    {
        if(temperature<(thresholds.m_subtropicalTemperature*0.5f))
            return ClimateZone::Dfc;
        return ClimateZone::Dfb;
    }

    //a maritime flank this cool with the wind off the sea is the oceanic climate, and Cfc is the
    //subpolar end of it - mild winters still, but summers that never warm up
    if(onshore>thresholds.m_onshore)
        return (temperature>=thresholds.m_coolSummerOceanic)?ClimateZone::Cfb:ClimateZone::Cfc;

    if(temperature<(thresholds.m_subtropicalTemperature*0.5f))
        return ClimateZone::Dfc;

    return ClimateZone::Dfb;
}

}//namespace worldgen

#endif //_worldgen_climate_h_
