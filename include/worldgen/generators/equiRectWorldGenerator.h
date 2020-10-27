#ifndef _worldgen_equiRectWorldGenerator_h_
#define _worldgen_equiRectWorldGenerator_h_

#include "worldgen/worldDescriptors.h"
#include "worldgen/maths/coords.h"
#include "worldgen/tectonics.h"
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

#include <FastNoise/FastNoise.h>

#undef None

#include <cassert>
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

        m_plateCount=16;
        m_plateCountMin=8;
        m_plateCountMax=24;

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

        m_plateFrequency=2.56f/m_influenceSize.x;
        m_continentFrequency=10*m_plateFrequency;
    }

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

    int m_plateCount;
    int m_plateCountMin;
    int m_plateCountMax;

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

template<typename _Region, typename _Chunk>
class EquiRectWorldGenerator
{
public:
//    typedef typename _Grid::RegionType Region;
//
//    typedef typename _Grid::ChunkType ChunkType;
//    typedef std::unique_ptr<ChunkType> UniqueChunkType;
//
    typedef _Region Region;
    typedef _Chunk Chunk;

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

    //for debugging
    int getPlateCount() { return m_plateCount; }
    const InfluenceMap &getInfluenceMap() { return m_influenceMap; }
    const glm::ivec2 &getInfluenceMapSize() { return m_descriptorValues.m_influenceSize; }

    EquiRectDescriptors &getDecriptors() { return m_descriptorValues; }

    int m_plateSeed;
    int m_plateCount;
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

    FastNoise::SmartNode<FastNoise::OpenSimplex2> m_os2Noise;

    FastNoise::SmartNode<FastNoise::DomainScale> m_domainScale;
    FastNoise::SmartNode<FastNoise::DomainWarpGradient> m_domainWarp;
    FastNoise::SmartNode<FastNoise::DomainWarpFractalIndependant> m_domainWarpFractal;
    FastNoise::SmartNode<FastNoise::FractalFBm> m_fractalFbmNoise;

    FastNoise::SmartNode<FastNoise::CellularValue> m_cellularNoise;
    FastNoise::SmartNode<FastNoise::CellularDistance> m_cellularDistanceNoise;

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

#include "worldgen/generators/equiRectWorldGenerator.inl"

#endif //_worldgen_equiRectEquiRectWorldGenerator_h_