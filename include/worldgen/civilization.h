#ifndef _worldgen_civilization_h_
#define _worldgen_civilization_h_

#include "worldgen/export.h"
#include "worldgen/tectonics.h"
#include "worldgen/coast.h"
#include "worldgen/maths/math_helpers.h"

#include <cmath>

#include <string>
#include <vector>

namespace worldgen
{

//What a people needs from the ground under them.
//
//Settlement geography splits this into SITE - the exact ground: water, soil to farm, grass to
//graze, material to build with, something to burn - and SITUATION, what surrounds it. The site
//factors are what a generator can answer, and every one of them is already on the map.
//
//The weights are what make this configurable. A profile is not a species; it is a statement about
//what that species wants, and nothing here assumes the people asking are human. Elves weight
//timber and forest and want nothing to do with open steppe; dwarves weight stone and ore, live
//above the treeline and do not care whether anything grows. Anyone can write a profile without
//touching the generator.
struct SpeciesHabitat
{
    SpeciesHabitat():
        m_name("unnamed"),
        m_water(1.0f),
        m_arable(1.0f),
        m_grazing(1.0f),
        m_timber(1.0f),
        m_stone(1.0f),
        m_fuel(1.0f),
        m_ore(1.0f),
        m_forest(0.0f),
        m_coast(0.0f),
        m_river(0.0f),
        m_idealElevation(200.0f),
        m_elevationTolerance(900.0f),
        m_reliefPreference(0.0f),
        m_idealTemperature(14.0f),
        m_temperatureTolerance(18.0f),
        m_idealMoisture(0.6f),
        m_moistureTolerance(0.45f),
        m_subterranean(false)
    {
    }

    std::string m_name;

    //the site factors. 0 means it does not matter to them, 1 is ordinary need, above 1 is a
    //people who will not settle without it
    float m_water;
    float m_arable;    //soil that will take a crop
    float m_grazing;   //open grass for herds
    float m_timber;    //standing wood to build with
    float m_stone;     //workable rock to build with
    float m_fuel;      //something to burn, which is what lets them smelt
    float m_ore;       //metal worth digging for

    //affinities that are not site factors so much as inclinations
    float m_forest;    //wants to be IN forest, not merely near wood
    float m_coast;
    float m_river;

    //the shape of the ground
    float m_idealElevation;      //metres
    float m_elevationTolerance;  //metres either side before it stops being tolerable
    float m_reliefPreference;    //negative wants flat ground, positive wants broken ground

    //what they can stand
    float m_idealTemperature;
    float m_temperatureTolerance;
    float m_idealMoisture;
    float m_moistureTolerance;

    //a people who live in the rock rather than on it care about what is under them and very
    //little about what grows over it
    bool m_subterranean;
};

//how well a value sits against a preferred one, 1 at the ideal and falling to 0 at the tolerance
inline float preferenceScore(float value, float ideal, float tolerance)
{
    if(tolerance<=0.0f)
        return 1.0f;

    float offset=(float)fabs(value-ideal)/tolerance;

    if(offset>=1.0f)
        return 0.0f;

    return 1.0f-(offset*offset);
}

//--- the site factors, each read off the map ---

//Water first, because the reference is blunt about it: you cannot live where there is none.
inline float siteWater(const InfluenceCell &cell)
{
    if((cell.water==WaterBody::River)&&cell.permanentRiver)
        return 1.0f;
    if((cell.water==WaterBody::Lake)||(cell.water==WaterBody::GlacialLake))
        return 1.0f;
    if(cell.water==WaterBody::River)
        return 0.55f;   //runs dry part of the year
    if((cell.water==WaterBody::SaltLake)||(cell.water==WaterBody::InlandDelta))
        return 0.2f;    //water, but not water you can drink

    //no standing water, so it comes down to rain
    return clamp(cell.moisture*0.8f, 0.0f, 0.8f);
}

//Soil that will take a crop: warm enough, wet enough, flat enough, and sitting on something that
//weathers to soil rather than bare rock.
inline float siteArable(const InfluenceCell &cell, float relief)
{
    switch(cell.climate)
    {
    case ClimateZone::Ocean:
    case ClimateZone::EF:
    case ClimateZone::ET:
    case ClimateZone::BWh:
    case ClimateZone::BWk:
        return 0.0f;
    default:
        break;
    }

    float soil=sedimentaryRock(cell.rock)?1.0f:0.45f;

    if(cell.rock==RockType::Sediment)
        soil=1.0f;      //flood plain and basin fill, the best of it

    float wet=preferenceScore(cell.moisture, 0.65f, 0.55f);
    float warm=preferenceScore(cell.temperature, 16.0f, 22.0f);
    float flat=clamp(1.0f-(relief*8.0f), 0.0f, 1.0f);

    return soil*wet*warm*flat;
}

//Open grass. The steppe and savanna climates are exactly this, and it is the land that is no good
//for crops but will still carry a herd.
inline float siteGrazing(const InfluenceCell &cell)
{
    switch(cell.climate)
    {
    case ClimateZone::Aw:
    case ClimateZone::BSh:
    case ClimateZone::BSk:
        return 1.0f;
    case ClimateZone::Cfb:
    case ClimateZone::Cfc:
    case ClimateZone::Csb:
    case ClimateZone::Dfb:
        return 0.75f;
    case ClimateZone::Csa:
    case ClimateZone::Dfa:
    case ClimateZone::Dfc:
        return 0.55f;
    case ClimateZone::ET:
        return 0.25f;   //thin, but reindeer manage
    default:
        break;
    }
    return 0.25f;
}

inline float siteTimber(const InfluenceCell &cell)
{
    if(cell.forest)
        return 1.0f;

    return (cell.fuel==FuelSource::Wood)?0.6f:0.0f;
}

//Stone worth building with, and the clay the reference asks for as an alternative.
inline float siteStone(const InfluenceCell &cell, float relief)
{
    float exposure=clamp(relief*6.0f, 0.0f, 1.0f); //bare rock where the ground is broken

    switch(cell.rock)
    {
    case RockType::Granite:
    case RockType::Metamorphic:
        return 0.7f+(0.3f*exposure);
    case RockType::Basalt:
    case RockType::Andesite:
        return 0.6f+(0.3f*exposure);
    case RockType::Sandstone:
    case RockType::Carbonate:
        return 0.75f;   //cut stone, and limestone for mortar
    case RockType::Sediment:
        return 0.5f;    //clay, which the reference names outright
    default:
        break;
    }
    return 0.4f;
}

inline float siteFuel(const InfluenceCell &cell)
{
    switch(cell.fuel)
    {
    case FuelSource::Coal: return 1.0f;
    case FuelSource::Wood: return 0.85f;
    case FuelSource::Peat: return 0.6f;
    default: break;
    }
    return 0.0f;
}

//Ore is only worth anything if these people can actually reach it and it is rich enough to repay
//the digging - which is exactly what the grade and exposure work answers.
inline float siteOre(const InfluenceCell &cell)
{
    if(cell.deposit==DepositType::None)
        return 0.0f;

    if(!workableByHand(cell.deposit, cell.depositExposed))
        return cell.depositExposed?0.25f:0.0f;

    return (depositGrade(cell.deposit)==OreGrade::Rich)?1.0f:0.7f;
}

//How well this ground suits a given people. Ocean is uninhabitable to everyone here; beyond that
//it is the weighted site factors against how tolerable they find the place at all.
inline float habitability(const InfluenceCell &cell, const SpeciesHabitat &habitat, float elevation, float relief)
{
    if(cell.heightBase<0.5f)
        return 0.0f;

    float total=0.0f;
    float weight=0.0f;

    struct Factor { float score; float weight; };
    const Factor factors[]=
    {
        {siteWater(cell),            habitat.m_water},
        {siteArable(cell, relief),   habitat.m_arable},
        {siteGrazing(cell),          habitat.m_grazing},
        {siteTimber(cell),           habitat.m_timber},
        {siteStone(cell, relief),    habitat.m_stone},
        {siteFuel(cell),             habitat.m_fuel},
        {siteOre(cell),              habitat.m_ore},
        {cell.forest?1.0f:0.0f,      habitat.m_forest},
        {(cell.coast!=(int)CoastType::None)?1.0f:0.0f, habitat.m_coast},
        {(cell.water==WaterBody::River)?1.0f:0.0f,     habitat.m_river}
    };

    for(size_t i=0; i<sizeof(factors)/sizeof(factors[0]); ++i)
    {
        if(factors[i].weight<=0.0f)
            continue;

        total+=factors[i].score*factors[i].weight;
        weight+=factors[i].weight;
    }

    float site=(weight>0.0f)?(total/weight):0.0f;

    //A people who live inside the rock are sheltered from the weather above it, so the climate
    //matters far less to them than what the mountain is made of.
    float exposure=habitat.m_subterranean?0.35f:1.0f;

    float climate=1.0f;

    climate*=1.0f-(exposure*(1.0f-preferenceScore(cell.temperature, habitat.m_idealTemperature, habitat.m_temperatureTolerance)));
    climate*=1.0f-(exposure*(1.0f-preferenceScore(cell.moisture, habitat.m_idealMoisture, habitat.m_moistureTolerance)));

    float ground=preferenceScore(elevation, habitat.m_idealElevation, habitat.m_elevationTolerance);

    //some peoples want broken country and some want a plain to plough
    float reliefFit=clamp(0.5f+(habitat.m_reliefPreference*((relief*10.0f)-0.5f)), 0.0f, 1.0f);

    return clamp(site*climate*ground*(0.5f+(0.5f*reliefFit)), 0.0f, 1.0f);
}

//--- profiles ---
//These are starting points, not a fixed roster. The weights above are the interface; anything can
//be described by writing another one of these.

inline SpeciesHabitat humanHabitat()
{
    SpeciesHabitat h;

    h.m_name="Human";
    //generalists, and the only ones who want every site factor at once
    h.m_water=2.0f;
    h.m_arable=1.8f;
    h.m_grazing=1.0f;
    h.m_timber=0.8f;
    h.m_stone=0.6f;
    h.m_fuel=1.0f;
    h.m_ore=0.5f;
    h.m_river=1.2f;
    h.m_coast=0.7f;
    h.m_idealElevation=150.0f;
    h.m_elevationTolerance=1200.0f;
    h.m_reliefPreference=-0.4f;   //river valleys and plains
    h.m_idealTemperature=14.0f;
    h.m_temperatureTolerance=20.0f;
    h.m_idealMoisture=0.6f;
    h.m_moistureTolerance=0.5f;
    return h;
}

inline SpeciesHabitat elfHabitat()
{
    SpeciesHabitat h;

    h.m_name="Elf";
    //in the forest rather than merely near it, and no interest in ploughing it under
    h.m_water=1.4f;
    h.m_arable=0.4f;
    h.m_grazing=0.0f;
    h.m_timber=1.6f;
    h.m_stone=0.2f;
    h.m_fuel=0.5f;
    h.m_ore=0.1f;
    h.m_forest=3.0f;
    h.m_river=0.9f;
    h.m_coast=0.2f;
    h.m_idealElevation=350.0f;
    h.m_elevationTolerance=1100.0f;
    h.m_reliefPreference=0.1f;
    h.m_idealTemperature=13.0f;
    h.m_temperatureTolerance=15.0f;
    h.m_idealMoisture=0.75f;      //wants it wet enough to hold forest
    h.m_moistureTolerance=0.35f;
    return h;
}

inline SpeciesHabitat dwarfHabitat()
{
    SpeciesHabitat h;

    h.m_name="Dwarf";
    //hills and mountains, and what is inside them. Living in the rock, the weather above hardly
    //reaches them, so they will hold ground nobody else would take.
    h.m_water=1.0f;
    h.m_arable=0.15f;
    h.m_grazing=0.1f;
    h.m_timber=0.4f;
    h.m_stone=2.2f;
    h.m_fuel=1.3f;                //no fuel, no forge
    h.m_ore=3.0f;
    h.m_forest=0.0f;
    h.m_river=0.3f;
    h.m_coast=0.0f;
    h.m_idealElevation=1800.0f;
    h.m_elevationTolerance=2400.0f;
    h.m_reliefPreference=0.9f;    //wants broken country
    h.m_idealTemperature=6.0f;
    h.m_temperatureTolerance=26.0f;
    h.m_idealMoisture=0.45f;
    h.m_moistureTolerance=0.6f;
    h.m_subterranean=true;
    return h;
}

inline SpeciesHabitat orcHabitat()
{
    SpeciesHabitat h;

    h.m_name="Orc";
    //hard open country nobody is competing for: steppe, badland, upland scrub
    h.m_water=1.2f;
    h.m_arable=0.3f;
    h.m_grazing=1.8f;
    h.m_timber=0.4f;
    h.m_stone=0.8f;
    h.m_fuel=0.8f;
    h.m_ore=1.2f;
    h.m_reliefPreference=0.5f;
    h.m_idealElevation=700.0f;
    h.m_elevationTolerance=1800.0f;
    h.m_idealTemperature=12.0f;
    h.m_temperatureTolerance=26.0f;
    h.m_idealMoisture=0.35f;
    h.m_moistureTolerance=0.5f;
    return h;
}

inline SpeciesHabitat halflingHabitat()
{
    SpeciesHabitat h;

    h.m_name="Halfling";
    //mild, low, green and farmed - the narrowest tolerance of any of them
    h.m_water=2.0f;
    h.m_arable=2.5f;
    h.m_grazing=1.2f;
    h.m_timber=0.9f;
    h.m_stone=0.4f;
    h.m_fuel=1.0f;
    h.m_ore=0.1f;
    h.m_river=1.3f;
    h.m_idealElevation=120.0f;
    h.m_elevationTolerance=500.0f;
    h.m_reliefPreference=-0.9f;
    h.m_idealTemperature=13.0f;
    h.m_temperatureTolerance=12.0f;
    h.m_idealMoisture=0.7f;
    h.m_moistureTolerance=0.3f;
    return h;
}

inline std::vector<SpeciesHabitat> defaultSpecies()
{
    std::vector<SpeciesHabitat> species;

    species.push_back(humanHabitat());
    species.push_back(elfHabitat());
    species.push_back(dwarfHabitat());
    species.push_back(orcHabitat());
    species.push_back(halflingHabitat());
    return species;
}

//--- what divides one people from the next ---

//The reference draws two sorts of divide and is careful that they do NOT mean the same thing.
//A GEOGRAPHIC barrier - open sea, a mountain wall, a desert - stops both the spread of a people
//and the exchange between them. A CLIMATIC barrier stops the spread but not the exchange: trade
//and diplomacy cross it happily, but a civilization is unlikely to expand into ground that
//farms nothing like home.
enum class BarrierType
{
    None,
    Ocean,      //the hardest divide there is
    Mountain,   //steep ground, and the high country inside it can be a cluster of its own
    Desert      //desert and the rougher steppe
};

inline const char *barrierTypeName(BarrierType barrier)
{
    switch(barrier)
    {
    case BarrierType::Ocean:    return "Ocean";
    case BarrierType::Mountain: return "Mountain";
    case BarrierType::Desert:   return "Desert";
    default: break;
    }
    return "None";
}

//The broad climate families the reference groups by when drawing the softer divides: arid,
//tropical, temperate, subarctic, polar. Agriculture and the way of life built on it travel
//within one of these far more readily than across into another.
enum class ClimateGroup
{
    Tropical,
    Arid,
    Temperate,
    Subarctic,
    Polar,
    Water
};

inline ClimateGroup climateGroup(ClimateZone zone)
{
    switch(zone)
    {
    case ClimateZone::Af: case ClimateZone::Am: case ClimateZone::Aw:
        return ClimateGroup::Tropical;
    case ClimateZone::BWh: case ClimateZone::BWk: case ClimateZone::BSh: case ClimateZone::BSk:
        return ClimateGroup::Arid;
    case ClimateZone::Csa: case ClimateZone::Csb: case ClimateZone::Cfa:
    case ClimateZone::Cfb: case ClimateZone::Cfc:
    case ClimateZone::Dfa: case ClimateZone::Dfb:
        return ClimateGroup::Temperate;
    case ClimateZone::Dfc:
        return ClimateGroup::Subarctic;
    case ClimateZone::ET: case ClimateZone::EF:
        return ClimateGroup::Polar;
    default:
        break;
    }
    return ClimateGroup::Water;
}

struct BarrierThresholds
{
    BarrierThresholds()
    {
        //"where you start to see steep elevation change". It is the STEEPNESS that divides more
        //than the height: a high plateau you can walk onto is far less of a wall than a lower but
        //broken range, so the elevation cut is set high enough that plateau country does not read
        //as impassable on its own.
        m_mountainElevation=2300.0f;
        m_mountainRelief=0.24f;

        //desert proper divides; ordinary steppe does not, unless it is also broken ground
        m_steppeRelief=0.05f;

        //the reference's own note on scale: this is drawn for an iron age to late medieval world.
        //An earlier one wants more divides, a later one fewer.
        m_minimumRegion=12;   //cells, so tiny slivers are not cut off as regions of their own
    }

    float m_mountainElevation;
    float m_mountainRelief;
    float m_steppeRelief;
    int m_minimumRegion;
};

//Whether this ground is hard enough to cross that it keeps the people either side of it apart.
inline BarrierType geographicBarrier(const InfluenceCell &cell, float elevation, float relief,
    const BarrierThresholds &thresholds)
{
    if(cell.heightBase<0.5f)
        return BarrierType::Ocean;

    if((elevation>=thresholds.m_mountainElevation)||(relief>=thresholds.m_mountainRelief))
        return BarrierType::Mountain;

    if((cell.climate==ClimateZone::BWh)||(cell.climate==ClimateZone::BWk))
        return BarrierType::Desert;

    //steppe only divides where it is also rough going
    if(((cell.climate==ClimateZone::BSh)||(cell.climate==ClimateZone::BSk))
        &&(relief>=thresholds.m_steppeRelief))
        return BarrierType::Desert;

    return BarrierType::None;
}

//A cluster of ground people can move around within. Physiographic regions are bounded by the hard
//divides; cultural regions subdivide those again wherever the climate changes family, because a
//people rarely expands into country that will not grow what it knows how to grow.
struct CultureRegion
{
    size_t m_cells;
    float m_area;             //square kilometres
    int m_dominantSpecies;
    ClimateGroup m_climate;
    size_t m_physiographic;   //which hard-bounded cluster it sits inside
    float m_meanHabitability;
    bool m_riverValley;       //an arid region carried by a river, the way Egypt is
};

//--- what is left of the people who were here before ---

//A civilization that rose, held ground and fell. The world it lived in was not this one: it may
//have been warmer or colder, and the ground that suited it then may be desert or forest now.
struct FallenCivilization
{
    std::string m_name;
    size_t m_species;
    float m_age;                //0 recent, 1 as ancient as the world's memory goes
    float m_globalTemperature;  //the climate it actually lived in
    int m_sites;

    //--- how it ended, and what the map can be made to say about why ---
    //The fate is decided BEFORE any of the evidence below is gathered, and the evidence is then
    //looked for rather than invented. See history.h. Stored loosely as an int, the same way coast
    //is, so that history.h can include this header rather than the other way about.
    int m_fate;
    size_t m_seat;              //the influence cell of its greatest site

    float m_temperatureSwing;   //degrees between their world and this one, at their own sites
    float m_moistureSwing;
    float m_workedOut;          //share of their sites that sat on a deposit
    int m_drownedSites;         //sites now on ground the sea has come up over
    int m_barrierSplit;         //how many separate regions their sites ended up divided among
    int m_nearestRival;         //the polity whose seat of power sits closest to theirs
    float m_rivalDistance;      //in cells
    int m_refuge;               //the cultural region their survivors could have reached
};

//How much of a ruin is still standing. Age takes them down, and so does anything that grows over
//them: a desert keeps its ruins for millennia while a rainforest swallows them in centuries.
inline float ruinSurvival(float age, const InfluenceCell &cell)
{
    float survival=1.0f-(age*0.75f);

    switch(cell.climate)
    {
    case ClimateZone::BWh:
    case ClimateZone::BWk:
        survival*=1.35f;   //dry, and nothing grows over it
        break;
    case ClimateZone::Af:
    case ClimateZone::Am:
        survival*=0.45f;   //swallowed
        break;
    case ClimateZone::ET:
    case ClimateZone::EF:
        survival*=1.15f;   //frozen in
        break;
    default:
        break;
    }

    //rivers and ice take a site apart, and people rebuild over the good sites anyway
    if(cell.water==WaterBody::River)
        survival*=0.8f;

    return clamp(survival, 0.0f, 1.0f);
}

}//namespace worldgen

#endif //_worldgen_civilization_h_
