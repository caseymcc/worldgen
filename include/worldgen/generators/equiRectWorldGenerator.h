#ifndef _worldgen_equiRectWorldGenerator_h_
#define _worldgen_equiRectWorldGenerator_h_

#include "worldgen/worldDescriptors.h"
#include "worldgen/maths/coords.h"
#include "worldgen/tectonics.h"
#include "worldgen/climate.h"
#include "worldgen/coast.h"
#include "worldgen/river.h"
#include "worldgen/rock.h"
#include "worldgen/ore.h"
#include "worldgen/current.h"
#include "worldgen/fuel.h"
#include "worldgen/civilization.h"
#include "worldgen/polity.h"
#include "worldgen/trade.h"
#include "worldgen/city.h"
#include "worldgen/history.h"
#include "worldgen/detail.h"
#include "worldgen/weather.h"
#include "worldgen/perturbedWeather.h"
#include "worldgen/wrap.h"
#include "worldgen/fill.h"
#include "worldgen/maths/math_helpers.h"
#include "worldgen/sortedVector.h"
#include "worldgen/maths/glm_point.h"
#include "worldgen/progress.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/integer.hpp>

#undef None

#include <cassert>
#include <memory>
#include <random>
#include <chrono>
namespace chrono=std::chrono;

namespace worldgen
{

constexpr int NeighborCount=4;

//this is expecting cylindrical wrap
WORLDGEN_EXPORT std::vector<size_t> get2DCellNeighbors_eq(const glm::ivec2 &index, const glm::ivec2 &size);

struct WORLDGEN_EXPORT EquiRectDescriptors
{
    EquiRectDescriptors()
    {
        seed=1;
        m_noiseScale=0.001f;

        //    contientFrequency=1.0;
        m_continentFrequency=0.005f;
        m_continentOctaves=2;
        m_continentLacunarity=2.2f;

        m_seaLevel=0.0f;
        m_continentalShelf=0.46f;
        m_oceanFraction=0.70f;

        m_plateCount=16;
        m_plateCountMin=8;
        m_plateCountMax=24;

        //earth carries about 7-8 major plates, ~10 minor plates and a tail of microplates
        m_plateCountMajor=8;
        m_plateCountMinor=10;
        m_plateCountMicro=6;
        m_plateDetailFactor=2.4f;

        //A plate is a rigid body rotating about its euler pole, so it has to be one piece. The
        //domain warp is what keeps the boundaries from looking like flat voronoi edges, but turned
        //up far enough it folds space and splits cells into islands.
        m_plateWarpAmplitude=0.2f;

        m_plateDriftRate=1.2f;
        m_transformRatio=1.0f;

        //relative to earth. Rotation rate sets how many circulation cells each hemisphere runs,
        //and the direction sets which way the coriolis effect throws the winds.
        m_rotationRate=1.0f;
        m_rotationDirection=1.0f;

        //Pressure organises into discrete anticyclones rather than a smooth band. Weight is how
        //far the world is pulled from the plain zonal model toward those cells; 0 restores it.
        m_pressureCentreWeight=0.6f;
        m_subtropicalHighs=3;
        m_subtropicalLatitude=30.0f;
        m_pressureCentreRadius=18.0f;
        m_anticycloneOutflow=45.0f;
        m_continentalHighArea=0.012f;
        m_continentalHighLatitude=35.0f;
        m_pressureDryness=0.45f;

        //metres of elevation at the top of the height range, used for the temperature lapse rate
        //degrees celsius off the earth baseline. -5.5 is roughly the last glacial maximum,
        //+2.5 the last interglacial
        m_globalTemperature=0.0f;
        m_polarAmplification=1.2f;

        //The curve is set against earth's HYPSOMETRY - the share of land at each elevation - not
        //against its mean. Calibrating on the mean alone (iteration 6) hid a bimodal distribution:
        //everything crushed near sea level with a thin spike of peaks, and almost no upland
        //between. 1.5 gives <500m 55% against earth's 50 and >3km 6% against its 5.
        m_maxElevation=9000.0f;
        m_elevationCurve=1.5f;

        //Tectonics gives the big shapes and nothing else - which leaves the middle of a plate
        //almost featureless, because nothing there is being pushed on. Real terrain is fractal:
        //the events that carve an ocean basin are not the events that carve a valley. So the
        //tectonic result stands in for the low frequency layers and this adds the high ones.
        m_terrainDetail=0.030f;
        m_terrainDetailScale=0.06f;

        //Who might live here, and who used to. The species list is the configuration point: it
        //defaults to a few profiles but a caller can replace it wholesale before create().
        m_species=defaultSpecies();
        //Below this nobody would bother with the ground. 0.25 leaves about 60% of land claimed
        //by someone, which is close to the share of earth's land that carries people at all -
        //what is left out is desert, ice and bare peak rather than merely poor country.
        m_settleThreshold=0.25f;
        m_ruinCivilizations=4;
        m_ruinSitesPerCivilization=14;
        m_ruinSpacing=6;
        m_ruinClimateSwing=5.0f;
        m_barriers=BarrierThresholds();
        m_polityRules=PolityThresholds();
        m_tradeRules=TradeThresholds();
        m_cityRules=CityThresholds();
        m_detailRules=DetailThresholds();

        //metres of real ground per world block, which sets how big a drainage basin actually is
        m_metresPerBlock=1.0f;
        m_lapseRate=6.5f;
        m_continentalityRange=0.18f;
        m_continentalityStrength=8.0f;

        m_hotspotCount=12;
        m_hotspotTrackSteps=10;
        m_hotspotTrackLength=0.9f;
        m_hotspotStrength=0.22f;

        m_plateFrequency=0.00025f;
        m_plateOctaves=3;
        m_plateLacunarity=2.0f;

        m_influenceSize={4096, 4096};
        m_influenceGridSize={4096, 4096};
    }

    void calculateInfluenceSize(const WorldDescriptors &worldDescriptors)
    {
        glm::ivec3 size=worldDescriptors.getSize();

        m_influenceSize=glm::ivec2(size.x, size.y)/m_influenceGridSize;

        while(m_influenceSize.x*m_influenceGridSize.x<size.x)
            m_influenceSize.x++;
        while(m_influenceSize.y*m_influenceGridSize.y<size.y)
            m_influenceSize.y++;

        float baseFrequency=2.56f/m_influenceSize.x;

        //the cellular noise is run finer than the final plate count so the plates can be grown out
        //of several cells each and end up with a range of sizes rather than all matching
        m_plateFrequency=baseFrequency*m_plateDetailFactor;
        m_continentFrequency=10*baseFrequency;
    }

    size_t plateCountTarget() const { return (size_t)(m_plateCountMajor+m_plateCountMinor+m_plateCountMicro); }

    bool load(const char *json);
    bool save(char *json, size_t &size);

    void init(const WorldDescriptors &worldDescriptors);

    int seed;

    float m_noiseScale;
    float m_continentFrequency;
    int m_continentOctaves;
    float m_continentLacunarity;

    float m_seaLevel;
    float m_continentalShelf;
    float m_oceanFraction;

    int m_plateCount;
    int m_plateCountMin;
    int m_plateCountMax;

    int m_plateCountMajor;
    int m_plateCountMinor;
    int m_plateCountMicro;
    float m_plateDetailFactor;

    float m_plateWarpAmplitude;
    float m_plateDriftRate;
    float m_transformRatio;

    float m_rotationRate;
    float m_rotationDirection;

    float m_pressureCentreWeight;
    int m_subtropicalHighs;
    float m_subtropicalLatitude;
    float m_pressureCentreRadius;
    float m_anticycloneOutflow;
    float m_continentalHighArea;
    float m_continentalHighLatitude;
    float m_pressureDryness;

    float m_globalTemperature;
    float m_polarAmplification;
    float m_maxElevation;
    float m_elevationCurve;
    float m_terrainDetail;
    float m_terrainDetailScale;

    std::vector<SpeciesHabitat> m_species;
    float m_settleThreshold;
    int m_ruinCivilizations;
    int m_ruinSitesPerCivilization;
    int m_ruinSpacing;
    float m_ruinClimateSwing;
    BarrierThresholds m_barriers;
    PolityThresholds m_polityRules;
    TradeThresholds m_tradeRules;
    CityThresholds m_cityRules;
    DetailThresholds m_detailRules;
    float m_metresPerBlock;
    float m_lapseRate;
    float m_continentalityRange;
    float m_continentalityStrength;

    ClimateThresholds m_climate;
    CoastThresholds m_coast;
    RiverThresholds m_river;
    RockThresholds m_rock;
    OreThresholds m_ore;
    CurrentThresholds m_current;

    int m_hotspotCount;
    int m_hotspotTrackSteps;
    float m_hotspotTrackLength;
    float m_hotspotStrength;

    float m_plateFrequency;
    int m_plateOctaves;
    float m_plateLacunarity;

    glm::ivec2 m_influenceSize;
    glm::ivec2 m_influenceGridSize;
};

constexpr unsigned int EquiRectWorldGeneratorHeader_Marker=0x0f0f0f0f;
struct EquiRectWorldGeneratorHeader
{
    unsigned int marker;
    unsigned int version;
    unsigned int x;
    unsigned int y;
    unsigned int cellSize;
    unsigned int size;
};

struct NormalizeHeader
{
    unsigned int marker;
    unsigned int version;
    unsigned int size;
};

struct ThreadStorage
{
    std::vector<float> heightMap;
    std::vector<float> blockHeightMap;
    std::vector<float> blockScaleMap;
    std::vector<float> xMap;
    std::vector<float> yMap;
    std::vector<float> zMap;
    std::vector<float> layerMap;
//    std::unique_ptr<HastyNoise::VectorSet> vectorSet;

    std::vector<float> regionHeightMap;
//    std::unique_ptr<HastyNoise::VectorSet> regionVectorSet;
};

//template<typename _Region, typename _Chunk>
class EquiRectWorldGenerator
{
public:
    class Hidden;
//    typedef typename _Grid::RegionType Region;
//
//    typedef typename _Grid::ChunkType ChunkType;
//    typedef std::unique_ptr<ChunkType> UniqueChunkType;
//
//    typedef _Region Region;
//    typedef _Chunk Chunk;

    typedef std::vector<InfluenceCell> InfluenceMap;

    EquiRectWorldGenerator();
    ~EquiRectWorldGenerator();

    static const char *typeName() { return "EquiRectWorldGenerator"; }

    void create(WorldDescriptors *descriptors, Progress &progress);
    template<typename _FileIO>
    bool load(WorldDescriptors *descriptors, const std::string &directory, Progress &progress);
    template<typename _FileIO>
    void save(const std::string &directory);

    void loadDescriptors(WorldDescriptors *descriptors);
    void saveDescriptors(WorldDescriptors *descriptors);
    void saveDescriptors(std::string &descriptors);
    //    void initialize(WorldDescriptors *descriptors);
    //    void setWorld(WorldDescriptors descriptors);
    //    void setWorldDiscriptors(WorldDescriptors descriptors);

    void generateWorldOverview(Progress &progress);

    //    UniqueChunkType generateChunk(unsigned int hash, void *buffer, size_t bufferSize);
    //    UniqueChunkType generateChunk(glm::ivec3 chunkIndex, void *buffer, size_t bufferSize);
    //    UniqueChunkType generateChunk(unsigned int hash, glm::ivec3 &chunkIndex, void *buffer, size_t bufferSize);
    unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod);
    unsigned int generateRegion(const glm::vec3 &startPos, const glm::ivec3 &regionSize, void *buffer, size_t bufferSize, size_t lod);

    int getBaseHeight(const glm::vec2 &pos);

    //Expand one influence cell into the ground it actually covers. The overview says what averages
    //where, which river crosses this cell and how much water it carries; this fills in what that
    //looks like at a scale somebody can stand on. Deterministic from the world seed and the cell
    //coordinates, so the same cell always comes back the same and nothing has to be stored.
    void generateDetail(const glm::ivec2 &influenceCell, DetailMap &detail);

    //for debugging
    int getPlateCount() { return m_plateCount; }
    const std::vector<Hotspot> &getHotspots() { return m_hotspots; }
    const std::vector<PressureCentre> &getPressureCentres() { return m_pressureCentres; }
    const std::vector<DrainageBasin> &getDrainageBasins() { return m_basins; }
    const std::vector<FallenCivilization> &getFallenCivilizations() { return m_fallen; }
    const std::vector<CultureRegion> &getCultureRegions() { return m_regions; }
    const std::vector<Polity> &getPolities() { return m_polities; }
    const std::vector<TradeRoute> &getTradeRoutes() { return m_tradeRoutes; }
    const std::vector<City> &getCities() { return m_cities; }
    const std::vector<SpeciesHabitat> &getSpecies() { return m_descriptorValues.m_species; }
    const InfluenceMap &getInfluenceMap() { return m_influenceMap; }
    const glm::ivec2 &getInfluenceMapSize() { return m_descriptorValues.m_influenceSize; }

    EquiRectDescriptors &getDecriptors() { return m_descriptorValues; }

    int m_plateSeed;
    int m_plateCount;
    std::vector<Hotspot> m_hotspots;
    std::vector<PressureCentre> m_pressureCentres;
    std::vector<DrainageBasin> m_basins;
    std::vector<FallenCivilization> m_fallen;
    std::vector<CultureRegion> m_regions;
    std::vector<Polity> m_polities;
    std::vector<TradeRoute> m_tradeRoutes;
    std::vector<City> m_cities;
    std::vector<glm::vec2> m_influencePoints;
    std::vector<std::vector<glm::vec2>> m_influenceLines;

    std::vector<float> heightMap_noise;
    std::vector<float> plateMap_noise;
    std::vector<float> plate2Map_noise;
    std::vector<float> plateScaleMap;
    std::vector<float> plate2ScaleMap;
    std::vector<float> plateDistance_noise;
    std::vector<float> continentMap_noise;

private:
    void initialize(WorldDescriptors *descriptors);

    template<typename _FileIO>
    bool loadWorldOverview(const std::string &directory);
    template<typename _FileIO>
    void saveWorldOverview(const std::string &directory);
    template<typename _FileIO>
    bool loadNormalize(const std::string &fileName);
    template<typename _FileIO>
    void saveNormalize(const std::string &fileName);

    void buildHeightMap(const glm::vec3 &startPos, const glm::ivec3 &lodSize, size_t stride);

    void generatePlates(Progress &progress);
    void generateContinents(Progress &progress);

    void updateInfluenceNeighbors();

//    WorldDescriptors<_Grid> *m_descriptors;
    WorldDescriptors m_descriptors;
    EquiRectDescriptors m_descriptorValues;

//    size_t m_simdLevel;
//    std::unique_ptr<HastyNoise::NoiseSIMD> m_continentPerlin;
//    std::unique_ptr<HastyNoise::NoiseSIMD> m_layersPerlin;
//    std::unique_ptr<HastyNoise::NoiseSIMD> m_cellularNoise;
//    std::unique_ptr<HastyNoise::NoiseSIMD> m_continentCellular;

    std::unique_ptr<Hidden> m_hidden;

//    FastNoise::SmartNode<FastNoise::OpenSimplex2> m_os2Noise;
//
//    FastNoise::SmartNode<FastNoise::DomainScale> m_domainScale;
//    FastNoise::SmartNode<FastNoise::DomainWarpGradient> m_domainWarp;
//    FastNoise::SmartNode<FastNoise::DomainWarpFractalIndependant> m_domainWarpFractal;
//    FastNoise::SmartNode<FastNoise::FractalFBm> m_fractalFbmNoise;
//
//    FastNoise::SmartNode<FastNoise::CellularValue> m_cellularNoise;
//    FastNoise::SmartNode<FastNoise::CellularDistance> m_cellularDistanceNoise;

    int m_continentSeed;

    InfluenceMap m_influenceMap;
    std::vector<float> m_influenceNeighborMap;

    std::vector<float> m_xPositions;
    std::vector<float> m_yPositions;
    std::vector<float> m_zPositions;
//    std::unique_ptr<HastyNoise::VectorSet> m_influenceVectorSet;

    //    Regular2DGrid<InfluenceCell> m_influence;
    //    noise::module::Perlin m_perlin;
    //    noise::module::Perlin m_continentPerlin;
    //    noise::module::Curve m_continentCurve;
    //
    //    noise::module::Perlin m_layersPerlin;

        //convert these to scratch buffer request as this is not thread safe
//    static std::vector<float> heightMap;
//    static std::vector<float> blockHeightMap;
//    static std::vector<float> blockScaleMap;
//    static std::vector<float> xMap;
//    static std::vector<float> yMap;
//    static std::vector<float> zMap;
//    static std::vector<float> layerMap;
//    static std::unique_ptr<HastyNoise::VectorSet> vectorSet;
//
//    static std::vector<float> regionHeightMap;
//    static std::unique_ptr<HastyNoise::VectorSet> regionVectorSet;
    static thread_local ThreadStorage m_threadStorage;

    //timing
    double allocationTime=0.0;
    double coordsTime=0.0;
    double cellularPlateTime=0.0;
    double cellularPlate2Time=0.0;
    double cellularDistanceTime=0.0;
    double processingTime=0.0;
};

}//namespace worldgen

//#include "worldgen/generators/equiRectWorldGenerator.inl"

#endif //_worldgen_equiRectEquiRectWorldGenerator_h_