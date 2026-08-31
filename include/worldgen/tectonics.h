#ifndef _worldgen_tectonics_h_
#define _worldgen_tectonics_h_

#include "worldgen/export.h"
#include "worldgen/climate.h"
#include "worldgen/river.h"
#include "worldgen/rock.h"
#include "worldgen/ore.h"
#include "worldgen/current.h"
#include "worldgen/fuel.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <cmath>
#include <limits>
#include <vector>

namespace worldgen
{

enum class BoundaryType
{
    None,       //interior of a plate
    Convergent, //plates closing, subduction or orogeny
    Divergent,  //plates separating, new crust
    Transform   //plates sliding past one another
};

//plate size classes, earth runs roughly 7-8 majors, ~10 minors and a tail of microplates
enum class PlateSize
{
    Major,
    Minor,
    Micro
};

//Mountain building splits four ways by which crust is colliding and how hard. These are
//worldbuilding names rather than formal geological ones, but the split is what decides how wide
//and how broken a range ends up:
//  Andean    - ocean subducting under continent at ordinary speed, a thin belt right on the
//              coast, 80-200km, and a smooth coastline with it
//  Laramide  - the same collision with a spreading ridge going down the trench, or the ocean
//              closing fast. Very wide, 750-1300km, ridge and basin topography behind the front
//  Ural      - a continent collecting an island arc or a microplate rather than another
//              continent, 500-600km
//  Himalayan - a full continent on continent collision, 600-1200km, extremely irregular
enum class OrogenyType
{
    None,
    Andean,
    Laramide,
    Ural,
    Himalayan
};

//A tin belt. The reference boils a long list of settings down to one rule: tin belts sit in
//orogenic regions with a collisional history, and the tin in them goes with the granites. This is
//what makes bronze hard - tin is rare, and it does not turn up where copper does.
inline bool tinBelt(OrogenyType orogeny)
{
    return (orogeny==OrogenyType::Himalayan)||(orogeny==OrogenyType::Ural);
}

struct WORLDGEN_EXPORT InfluenceCell
{
    InfluenceCell():
        point(false),
        heightBase(0.0f),
        heightRange(0.0f),
        tectonicPlate(0),
        borderPlate(0),
        plateValue(0.0f),
        plateDistanceValue(0.0f),
        continentValue(0.0f),
        collision(0.0f),
        shear(0.0f),
        boundaryType(BoundaryType::None),
        orogeny(OrogenyType::None),
        terrainScale(0.0f),
        hotspot(0.0f),
        pressure(0.0f),
        continentality(0.0f),
        onshore(0.0f),
        westernFlank(false),
        climate(ClimateZone::Ocean),
        coast(0),
        flowDirection(-1),
        drainageArea(0.0f),
        riverDischarge(0.0f),
        riverWidth(0.0f),
        riverStage(RiverStage::None),
        water(WaterBody::None),
        deltaShape(DeltaShape::None),
        permanentRiver(false),
        basin(0),
        streamOrder(0),
        rock(RockType::Sediment),
        craton(CratonPart::None),
        shallowSea(false),
        deposit(DepositType::None),
        forelandBasin(false),
        depositExposed(false),
        fuel(FuelSource::None),
        forest(false),
        tinBelt(false),
        habitability(0.0f),
        dominantSpecies(-1),
        ruinCondition(0.0f),
        ruinSpecies(-1),
        ruinCivilization(-1),
        ruinKind(0),
        barrier(0),
        physiographicRegion(-1),
        culturalRegion(-1),
        polity(-1),
        control(0.0f),
        march(false),
        goods(0),
        traffic(0.0f),
        chokepoint(false),
        city(-1),
        remoteness(1.0f),
        onTheBeatenTrack(false),
        ironBearing(false),
        current(0.0f, 0.0f),
        currentWarmth(0.0f),
        currentKind(CurrentKind::None),
        coastCurrent(CurrentKind::None),
        upwelling(false),
        overturning(false),
        airCurrent(0.0f)
    {

    }

    bool point;
    float heightBase;
    float heightRange;

    size_t tectonicPlate;
    size_t borderPlate;
    float plateHeight;
    float plateValue;
    float plateDistanceValue;
    float plateDistanceValueNorm;
    float continentValue;

    float collision;    //motion normal to the boundary, +convergent, -divergent
    float shear;        //motion along the boundary, drives transform faults
    BoundaryType boundaryType;
    OrogenyType orogeny;
    float terrainScale;
    float hotspot;      //relief contributed by a mantle hotspot, independent of any boundary

    size_t weatherCell;
    size_t weatherBand;
    float pressure;   //0 where air rises and leaves a low, 1 where it sinks into a high
    float continentality; //0 on the coast, 1 deep in a continental interior
    float onshore;    //+1 wind blowing in off the sea, -1 blowing out over it
    bool westernFlank; //the sea lies to the west of this ground, so it is a west-facing coast
    ClimateZone climate;
    int coast;   //CoastType, stored loosely so coast.h can include this header

    int flowDirection;      //index into the 8 neighbours water leaves by, -1 if it leaves nowhere
    float drainageArea;     //square kilometres draining through this cell
    float riverDischarge;   //millions of cubic metres a year arriving here
    float riverWidth;       //metres, 0 where there is no river
    RiverStage riverStage;
    WaterBody water;
    DeltaShape deltaShape;  //on delta ground, which of the four shapes it belongs to
    bool permanentRiver;    //runs all year, rather than drying to a wadi for part of it
    size_t basin;           //which drainage basin this cell belongs to
    int streamOrder;        //Strahler order, 1 at a headwater, rising where equal branches meet

    RockType rock;
    CratonPart craton;
    bool shallowSea;        //was under water when sea level last stood 200m higher

    DepositType deposit;    //what can be dug out here, if anything
    bool forelandBasin;     //a trough pressed down by the weight of a neighbouring mountain belt
    bool depositExposed;    //the deposit is at or near the surface, so findable without drilling

    FuelSource fuel;        //the best thing that can be burned here
    bool forest;            //trees will grow, so there is wood
    bool tinBelt;           //collisional orogenic country, where cassiterite is found

    float habitability;     //how well this ground suits whichever people suits it best
    int dominantSpecies;    //index into the generator's species list, -1 if nobody would settle
    float ruinCondition;    //0 nothing left, 1 substantially standing
    int ruinSpecies;        //who built it
    int ruinCivilization;   //which fallen civilization it belonged to
    int ruinKind;           //RuinKind: what the thing standing here actually was

    int barrier;            //BarrierType: ground hard enough to keep peoples apart
    int physiographicRegion;//cluster bounded by the hard divides, -1 if the cell is a barrier
    int culturalRegion;     //that cluster subdivided again wherever the climate changes family

    int polity;             //which state holds this ground, -1 if no rule reaches it
    float control;          //1 in the heartland, falling to 0 where the state's power runs out
    bool march;             //borderland: held loosely, pulled at from both sides

    unsigned int goods;     //TradeGood bits: what this ground has that is worth carrying far
    float traffic;          //0 to 1, how much of the world's trade passes through here
    bool chokepoint;        //heavy traffic through ground with no way round it
    int city;               //index of the city that took this spot, -1 for nearly everywhere
    float remoteness;       //0 on the trade network, 1 as far off it as this world goes
    bool onTheBeatenTrack;  //close enough to the network that people pass through
    bool ironBearing;       //this deposit also carries iron worth smelting

    glm::vec2 current;      //surface current, ocean cells only
    float currentWarmth;    //degrees this water is off the temperature of the latitude it is in
    CurrentKind currentKind;
    CurrentKind coastCurrent; //on a coastal land cell, the current washing it
    bool upwelling;         //cold nutrient rich water rising where offshore wind pulls the surface away
    bool overturning;       //on the poleward limb of the deep overturning circulation

    glm::vec2 direction;
    glm::vec2 airDirection;
    float airCurrent;

    float temperature;
    float moistureCapacity;
    float moisture;
};

struct PlateInfo
{
    PlateInfo():
        index(0),
        value(0.0f),
        eulerPole(0.0f, 0.0f, 1.0f),
        angularVelocity(0.0f),
        height(0.5f),
        area(0.0f),
        oceanic(false),
        size(PlateSize::Micro)
    {
    }

    size_t index;
    float value;

    //plates rotate about an axis through the centre of the planet (an euler pole) rather than
    //sliding in a straight line, so velocity varies across the plate. That is what lets a single
    //boundary run convergent at one end and transform at the other.
    glm::vec3 eulerPole;
    float angularVelocity;

    glm::vec3 direction; //velocity at the plate's own centre, for display

    float height;
    float area;          //fraction of the sphere covered, cosine weighted
    bool oceanic;
    PlateSize size;

    glm::ivec2 point;
    glm::vec3 point3d;
    std::vector<glm::ivec2> points;

    std::vector<size_t> neighbors;
};

//hotspots are anchored in the mantle and are not tied to a plate boundary, the plate drifts over
//them and is left with an age ordered chain of volcanoes
struct Hotspot
{
    glm::vec3 point3d;
    float strength;
    bool oceanic;
    size_t plate;
};

//velocity of the plate at a point on the unit sphere, w x p
inline glm::vec3 plateVelocity(const PlateInfo &plate, const glm::vec3 &point)
{
    return glm::cross(plate.eulerPole*plate.angularVelocity, point);
}

//Splits the relative motion of two plates at a point into the part normal to their shared
//boundary (convergence when positive, divergence when negative) and the part running along it
//(shear). The boundary between two spherical voronoi cells is the perpendicular bisector of their
//seed points, so its normal is the tangent plane part of (b-a).
inline void resolveBoundaryMotion(const PlateInfo &plate1, const PlateInfo &plate2, const glm::vec3 &plate1Point,
    const glm::vec3 &plate2Point, const glm::vec3 &point, float &convergence, float &shear)
{
    glm::vec3 normal=plate2Point-plate1Point;

    normal=normal-(point*glm::dot(normal, point)); //drop onto the tangent plane at point

    float length=glm::length(normal);

    if(length<=std::numeric_limits<float>::epsilon())
    {//seeds are antipodal through this point, no usable boundary normal
        convergence=0.0f;
        shear=0.0f;
        return;
    }
    normal=normal/length;

    glm::vec3 velocity=plateVelocity(plate2, point)-plateVelocity(plate1, point);

    velocity=velocity-(point*glm::dot(velocity, point)); //plate motion is tangent to the sphere

    float normalMotion=glm::dot(velocity, normal);

    convergence=-normalMotion; //plate2 closing on plate1 is convergence
    shear=glm::length(velocity-(normal*normalMotion));
}

inline BoundaryType classifyBoundary(float convergence, float shear, float transformRatio=1.0f)
{
    float normalMotion=std::abs(convergence);

    if((normalMotion<=0.0f)&&(shear<=0.0f))
        return BoundaryType::None;

    if(shear>(normalMotion*transformRatio))
        return BoundaryType::Transform;

    if(convergence>0.0f)
        return BoundaryType::Convergent;

    return BoundaryType::Divergent;
}

inline void calculateCurve(float distance, float &plate1, float &plate2, float cutoff=0.07f)
{
    if(distance>cutoff)
    {
        float d=(distance-cutoff)*(1.0f/(1.0f-cutoff));
        plate2=d*0.5f;
        //        plate2=(d*d)*0.5f;
        plate1=1.0f-plate2;
    }
    else
    {
        plate1=1.0f;
        plate2=0.0f;
    }
}

inline float subductionCurve(float distance)
{
    if(distance>0.9)
    {
        distance=(distance-0.9)*10.0f;
        return (1.0f/(1.0f+pow(((1.0f-distance)/distance), 3.0f)))*0.5f+0.5;
    }
    else if(distance>0.8)
    {
        distance=(distance-0.8)*10.0f;
        return (1.0f/(1.0f+pow(((1.0f-distance)/distance), 3.0f)))*0.3+0.2;
    }
    if(distance>0.6)
    {
        distance=(distance-0.6)*5.0f;
        return (distance*distance)*0.2;
    }
    return 0.0f;
}

inline float orogenicCurve(float distance, float cutoff=0.6f)
{
    if(distance>cutoff)
    {
        distance=(distance-cutoff)/(1.0f-cutoff);
        return 1.0f/(1.0f+pow(3.0f*(1.0f-distance)/distance, 2.0f));
        //        return (distance*distance);
    }
    return 0.0f;
}

inline float divergentCurve(float distance, float cutoff=0.9)
{
    if(distance>cutoff)
    {
        distance=(distance-cutoff)/(1.0f-cutoff);
        return -(1.0f/(1.0f+pow(((1.0f-distance)/distance), 3.0f)));
    }
    return 0.0f;
}

inline float smoothStep(float value)
{
    return value*value*(3.0f-2.0f*value);
}

//Mid ocean ridge. Rising mantle material pushes both plates up into a ridge and only the crest
//itself drops into a narrow rift valley, so the whole profile stays above the abyssal plain. Note
//this is the opposite sign to divergentCurve, which is the continental rifting case.
inline float midOceanRidgeCurve(float distance, float cutoff=0.7f)
{
    if(distance<=cutoff)
        return 0.0f;

    float d=(distance-cutoff)/(1.0f-cutoff);
    float flank=1.0f/(1.0f+pow(((1.0f-d)/d), 3.0f));

    const float riftStart=0.85f;

    if(d>riftStart)
        return flank-(smoothStep((d-riftStart)/(1.0f-riftStart))*0.45f);

    return flank;
}

//Subducting side of a convergent margin, the down going slab drags the sea floor into a deep
//trench right at the boundary.
inline float trenchCurve(float distance, float cutoff=0.75f)
{
    if(distance<=cutoff)
        return 0.0f;

    return -smoothStep((distance-cutoff)/(1.0f-cutoff));
}

//Overriding side of a convergent margin. The volcanic arc (island arc over ocean, cordillera over
//continent) peaks inland of the boundary and falls away again toward the trench, which is what
//makes subduction margins asymmetric.
inline float volcanicArcCurve(float distance, float cutoff=0.55f)
{
    if(distance<=cutoff)
        return 0.0f;

    float d=(distance-cutoff)/(1.0f-cutoff);
    const float peak=0.72f;

    if(d<peak)
        return smoothStep(d/peak);

    return smoothStep((1.0f-d)/(1.0f-peak));
}

//Transform boundaries build no relief of their own, just a narrow fault scarp.
inline float transformCurve(float distance, float cutoff=0.9f)
{
    if(distance<=cutoff)
        return 0.0f;

    return smoothStep((distance-cutoff)/(1.0f-cutoff));
}

//A wide belt carries its interior up onto a plateau behind the mountain front rather than falling
//straight back to the lowlands - tibet behind the himalayas, the altiplano behind the andes. This
//rises out of the plate interior and holds flat, with the front built on top of it.
inline float plateauCurve(float distance, float cutoff, float level)
{
    if(distance<=cutoff)
        return 0.0f;

    float d=(distance-cutoff)/(1.0f-cutoff);

    return level*smoothStep(std::min(d/0.5f, 1.0f));
}

//Which of the four the collision produces. The reference gives two discriminators: how fast the
//ocean is closing (a spreading ridge going down the trench, or simply a fast approach, widens an
//andean belt into a laramide one) and whether the far side is a whole continent or just an island
//arc or microplate riding in on a small plate.
inline OrogenyType classifyOrogeny(const PlateInfo &plate1, const PlateInfo &plate2, float convergence, float fastConvergence=0.6f)
{
    if(plate2.oceanic)
        return (convergence>fastConvergence)?OrogenyType::Laramide:OrogenyType::Andean;

    //a microplate carries an island arc or a fragment of continent, not a whole one
    if((plate1.size==PlateSize::Micro)||(plate2.size==PlateSize::Micro))
        return OrogenyType::Ural;

    return OrogenyType::Himalayan;
}

//Belt width is set by the cutoff, since distance runs from the plate interior at 0 to the boundary
//at 1. The cutoffs keep the four in the proportions the reference gives - andean the narrowest by
//a wide margin, laramide the broadest.
inline float orogenyCurve(OrogenyType type, float distance)
{
    switch(type)
    {
    case OrogenyType::Andean:
        //narrow, but the andes still stand higher than the rockies, and this branch only ever
        //sees the slower convergence, so it needs the amplitude to make that up
        return volcanicArcCurve(distance, 0.86f)*1.35f;
    case OrogenyType::Laramide:
        return (volcanicArcCurve(distance, 0.48f)*0.6f)+plateauCurve(distance, 0.48f, 0.24f);
    case OrogenyType::Ural:
        return volcanicArcCurve(distance, 0.60f)*0.6f;   //wide but worn down, no plateau
    case OrogenyType::Himalayan:
        return (orogenicCurve(distance, 0.55f)*0.85f)+plateauCurve(distance, 0.55f, 0.28f);
    default:
        break;
    }
    return 0.0f;
}

//How broken the belt is. Laramide runs to ridges and basins where the land slumps between them,
//himalayan to extremely irregular deformation, while a thin andean belt stays regular and leaves a
//smooth coastline behind it.
inline float orogenyRoughness(OrogenyType type)
{
    switch(type)
    {
    case OrogenyType::Andean:
        return 0.15f;
    case OrogenyType::Laramide:
        return 0.55f;
    case OrogenyType::Ural:
        return 0.30f;
    case OrogenyType::Himalayan:
        return 0.60f;
    default:
        break;
    }
    return 0.0f;
}

//plate1 is the cell's own plate, plate2 the one across the boundary. Of two oceanic plates the
//denser one subducts, and oceanic crust gets denser as it ages and cools, which is the same thing
//as it sitting deeper - so the lower plate is the one that goes down.
inline float calculateConvergentCurve(float distance, const PlateInfo &plate1, const PlateInfo &plate2,
    float convergence, OrogenyType &orogeny, float &roughness)
{
    orogeny=OrogenyType::None;
    roughness=0.0f;

    if(plate1.oceanic&&plate2.oceanic)
    {//island arc, the philippines
        if(plate1.height<=plate2.height)
            return trenchCurve(distance)*0.5f;
        return volcanicArcCurve(distance)*0.45f;
    }

    if(plate1.oceanic)
        return trenchCurve(distance)*0.6f; //ocean side of an ocean/continent margin

    //this cell sits on the overriding plate, so this is the side that builds mountains
    orogeny=classifyOrogeny(plate1, plate2, convergence);
    roughness=orogenyRoughness(orogeny);

    return orogenyCurve(orogeny, distance);
}

inline float calculateDivergentCurve(float distance, bool plate1Ocean, bool plate2Ocean)
{
    if(!plate1Ocean&&!plate2Ocean)
        return divergentCurve(distance); //continental rift valley, east africa

    //new crust is created at a divergent boundary, so an ocean/continent one turns into an
    //ocean/ocean one almost immediately, all remaining cases spread as a mid ocean ridge
    return midOceanRidgeCurve(distance, 0.7f)*0.5f;
}

//The continental shelf is the drowned rim of continental crust, shallow and close to flat before
//it drops away at the shelf break. Oceanic crust has none, and continentality falls off across the
//plate boundary, so the plateau fades out where the crust stops being continental.
inline float applyContinentalShelf(float height, float continentality, float seaLevel, float shelfLevel)
{
    if((height>=seaLevel)||(height>=shelfLevel))
        return height;

    if(continentality<=0.5f)
        return height;

    float weight=(continentality-0.5f)*2.0f;
    float shelf=shelfLevel-((shelfLevel-height)*0.15f); //keep a little relief on the shelf itself

    return height+((shelf-height)*weight);
}

}//namespace worldgen

#endif //_worldgen_tectonics_h_
