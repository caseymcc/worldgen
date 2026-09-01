#include "worldgen/generators/equiRectWorldGenerator.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <glm/gtx/projection.hpp>

#include <FastNoise/FastNoise.h>

#include <queue>

#include "generic/io/fs.h"

namespace chrono=std::chrono;

namespace worldgen
{

bool EquiRectDescriptors::load(const char *json)
{
    if(strlen(json)<=0)
        return false;

    rapidjson::Document document;

    document.Parse(json);
    bool retValue=true;
    
    if(document.HasMember("noiseScale"))
        m_noiseScale=document["noiseScale"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentFrequency"))
        m_continentFrequency=document["continentFrequency"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentOctaves"))
        m_continentOctaves=document["continentOctaves"].GetInt();
    else
        retValue=false;
    if(document.HasMember("continentLacunarity"))
        m_continentLacunarity=document["continentLacunarity"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("seaLevel"))
        m_seaLevel=document["seaLevel"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentalShelf"))
        m_continentalShelf=document["continentalShelf"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("oceanFraction"))
        m_oceanFraction=document["oceanFraction"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("plateCountMajor"))
        m_plateCountMajor=document["plateCountMajor"].GetInt();
    else
        retValue=false;
    if(document.HasMember("plateCountMinor"))
        m_plateCountMinor=document["plateCountMinor"].GetInt();
    else
        retValue=false;
    if(document.HasMember("plateCountMicro"))
        m_plateCountMicro=document["plateCountMicro"].GetInt();
    else
        retValue=false;
    if(document.HasMember("plateDetailFactor"))
        m_plateDetailFactor=document["plateDetailFactor"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("plateWarpAmplitude"))
        m_plateWarpAmplitude=document["plateWarpAmplitude"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("plateDriftRate"))
        m_plateDriftRate=document["plateDriftRate"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("transformRatio"))
        m_transformRatio=document["transformRatio"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("pressureCentreWeight"))
        m_pressureCentreWeight=document["pressureCentreWeight"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("subtropicalHighs"))
        m_subtropicalHighs=document["subtropicalHighs"].GetInt();
    else
        retValue=false;
    if(document.HasMember("subtropicalLatitude"))
        m_subtropicalLatitude=document["subtropicalLatitude"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("pressureCentreRadius"))
        m_pressureCentreRadius=document["pressureCentreRadius"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("anticycloneOutflow"))
        m_anticycloneOutflow=document["anticycloneOutflow"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentalHighArea"))
        m_continentalHighArea=document["continentalHighArea"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentalHighLatitude"))
        m_continentalHighLatitude=document["continentalHighLatitude"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("pressureDryness"))
        m_pressureDryness=document["pressureDryness"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("maxElevation"))
        m_maxElevation=document["maxElevation"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("lapseRate"))
        m_lapseRate=document["lapseRate"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentalityRange"))
        m_continentalityRange=document["continentalityRange"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("continentalityStrength"))
        m_continentalityStrength=document["continentalityStrength"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("globalTemperature"))
        m_globalTemperature=document["globalTemperature"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("elevationCurve"))
        m_elevationCurve=document["elevationCurve"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("polarAmplification"))
        m_polarAmplification=document["polarAmplification"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("metresPerBlock"))
        m_metresPerBlock=document["metresPerBlock"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("deltaMinDischarge"))
        m_river.m_deltaMinDischarge=document["deltaMinDischarge"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("deltaDischargePerCell"))
        m_river.m_deltaDischargePerCell=document["deltaDischargePerCell"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("deltaMaxCells"))
        m_river.m_deltaMaxCells=document["deltaMaxCells"].GetInt();
    else
        retValue=false;
    if(document.HasMember("terrainDetail"))
        m_terrainDetail=document["terrainDetail"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("terrainDetailScale"))
        m_terrainDetailScale=document["terrainDetailScale"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("rotationRate"))
        m_rotationRate=document["rotationRate"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("rotationDirection"))
        m_rotationDirection=document["rotationDirection"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("hotspotCount"))
        m_hotspotCount=document["hotspotCount"].GetInt();
    else
        retValue=false;
    if(document.HasMember("hotspotTrackSteps"))
        m_hotspotTrackSteps=document["hotspotTrackSteps"].GetInt();
    else
        retValue=false;
    if(document.HasMember("hotspotTrackLength"))
        m_hotspotTrackLength=document["hotspotTrackLength"].GetFloat();
    else
        retValue=false;
    if(document.HasMember("hotspotStrength"))
        m_hotspotStrength=document["hotspotStrength"].GetFloat();
    else
        retValue=false;

    return retValue;
}

bool EquiRectDescriptors::save(char *json, size_t &size)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    rapidjson::Document document;

    document.SetObject();

    document.AddMember("noiseScale", rapidjson::Value(m_noiseScale).Move(), document.GetAllocator());
    document.AddMember("continentFrequency", rapidjson::Value(m_continentFrequency).Move(), document.GetAllocator());
    document.AddMember("continentOctaves", rapidjson::Value(m_continentOctaves).Move(), document.GetAllocator());
    document.AddMember("continentLacunarity", rapidjson::Value(m_continentLacunarity).Move(), document.GetAllocator());
    document.AddMember("seaLevel", rapidjson::Value(m_seaLevel).Move(), document.GetAllocator());
    document.AddMember("continentalShelf", rapidjson::Value(m_continentalShelf).Move(), document.GetAllocator());
    document.AddMember("oceanFraction", rapidjson::Value(m_oceanFraction).Move(), document.GetAllocator());
    document.AddMember("plateCountMajor", rapidjson::Value(m_plateCountMajor).Move(), document.GetAllocator());
    document.AddMember("plateCountMinor", rapidjson::Value(m_plateCountMinor).Move(), document.GetAllocator());
    document.AddMember("plateCountMicro", rapidjson::Value(m_plateCountMicro).Move(), document.GetAllocator());
    document.AddMember("plateDetailFactor", rapidjson::Value(m_plateDetailFactor).Move(), document.GetAllocator());
    document.AddMember("plateWarpAmplitude", rapidjson::Value(m_plateWarpAmplitude).Move(), document.GetAllocator());
    document.AddMember("plateDriftRate", rapidjson::Value(m_plateDriftRate).Move(), document.GetAllocator());
    document.AddMember("transformRatio", rapidjson::Value(m_transformRatio).Move(), document.GetAllocator());
    document.AddMember("pressureCentreWeight", rapidjson::Value(m_pressureCentreWeight).Move(), document.GetAllocator());
    document.AddMember("subtropicalHighs", rapidjson::Value(m_subtropicalHighs).Move(), document.GetAllocator());
    document.AddMember("subtropicalLatitude", rapidjson::Value(m_subtropicalLatitude).Move(), document.GetAllocator());
    document.AddMember("pressureCentreRadius", rapidjson::Value(m_pressureCentreRadius).Move(), document.GetAllocator());
    document.AddMember("anticycloneOutflow", rapidjson::Value(m_anticycloneOutflow).Move(), document.GetAllocator());
    document.AddMember("continentalHighArea", rapidjson::Value(m_continentalHighArea).Move(), document.GetAllocator());
    document.AddMember("continentalHighLatitude", rapidjson::Value(m_continentalHighLatitude).Move(), document.GetAllocator());
    document.AddMember("pressureDryness", rapidjson::Value(m_pressureDryness).Move(), document.GetAllocator());
    document.AddMember("maxElevation", rapidjson::Value(m_maxElevation).Move(), document.GetAllocator());
    document.AddMember("lapseRate", rapidjson::Value(m_lapseRate).Move(), document.GetAllocator());
    document.AddMember("continentalityRange", rapidjson::Value(m_continentalityRange).Move(), document.GetAllocator());
    document.AddMember("continentalityStrength", rapidjson::Value(m_continentalityStrength).Move(), document.GetAllocator());
    document.AddMember("globalTemperature", rapidjson::Value(m_globalTemperature).Move(), document.GetAllocator());
    document.AddMember("elevationCurve", rapidjson::Value(m_elevationCurve).Move(), document.GetAllocator());
    document.AddMember("polarAmplification", rapidjson::Value(m_polarAmplification).Move(), document.GetAllocator());
    document.AddMember("metresPerBlock", rapidjson::Value(m_metresPerBlock).Move(), document.GetAllocator());
    document.AddMember("deltaMinDischarge", rapidjson::Value(m_river.m_deltaMinDischarge).Move(), document.GetAllocator());
    document.AddMember("deltaDischargePerCell", rapidjson::Value(m_river.m_deltaDischargePerCell).Move(), document.GetAllocator());
    document.AddMember("deltaMaxCells", rapidjson::Value(m_river.m_deltaMaxCells).Move(), document.GetAllocator());
    document.AddMember("terrainDetail", rapidjson::Value(m_terrainDetail).Move(), document.GetAllocator());
    document.AddMember("terrainDetailScale", rapidjson::Value(m_terrainDetailScale).Move(), document.GetAllocator());
    document.AddMember("rotationRate", rapidjson::Value(m_rotationRate).Move(), document.GetAllocator());
    document.AddMember("rotationDirection", rapidjson::Value(m_rotationDirection).Move(), document.GetAllocator());
    document.AddMember("hotspotCount", rapidjson::Value(m_hotspotCount).Move(), document.GetAllocator());
    document.AddMember("hotspotTrackSteps", rapidjson::Value(m_hotspotTrackSteps).Move(), document.GetAllocator());
    document.AddMember("hotspotTrackLength", rapidjson::Value(m_hotspotTrackLength).Move(), document.GetAllocator());
    document.AddMember("hotspotStrength", rapidjson::Value(m_hotspotStrength).Move(), document.GetAllocator());

    document.Accept(writer);

    if(sb.GetSize()<size-1)
    {
        strncpy(json, sb.GetString(), sb.GetSize());
        size=sb.GetSize();
        return true;
    }
    
    size=sb.GetSize();
    return false;
}

void EquiRectDescriptors::init(const WorldDescriptors &worldDescriptors)
{
    calculateInfluenceSize(worldDescriptors);
}

//this is expecting cylindrical wrap
std::vector<size_t> get2DCellNeighbors_eq(const glm::ivec2 &index, const glm::ivec2 &size)
{
    glm::ivec2 neighborIndex(index.x-1, index.y-1);
    std::vector<size_t> neighbors(9);

    size_t i=0;
    size_t nIndex;

    for(size_t y=0; y<3; ++y)
    {
        if(neighborIndex.y<0)
            neighborIndex.y=size.y+neighborIndex.y;
        else if(neighborIndex.y>=size.y)
            neighborIndex.y=neighborIndex.y-size.y;

        nIndex=size.x*(size_t)neighborIndex.y;

        neighborIndex.x=index.x-1;
        for(size_t x=0; x<3; ++x)
        {
            if(neighborIndex.x<0)
                neighborIndex.x=size.x+neighborIndex.x;
            else if(neighborIndex.x>=size.x)
                neighborIndex.x=neighborIndex.x-size.x;

            neighbors[i]=nIndex+(size_t)neighborIndex.x;
//            assert(neighbors[i]<size.x*size.y);
            i++;
            neighborIndex.x++;
        }

        neighborIndex.y++;
    }

    return neighbors;
}

inline float lerp(float v0, float v1, float t)
{
    return (1-t)*v0+t*v1;
}

inline float bi_lerp(float v00, float v10, float v01, float v11, float t0, float t1)
{
    return lerp(lerp(v00, v10, t0), lerp(v01, v11, t0), t1);
}

typedef std::chrono::high_resolution_clock clock;
typedef std::chrono::high_resolution_clock::time_point time_point;
typedef std::chrono::milliseconds ms;

typedef FastNoise::CellularDistance::ReturnType CellularReturnType;


thread_local ThreadStorage EquiRectWorldGenerator::m_threadStorage;

//
//std::vector<float> EquiRectWorldGenerator::heightMap;
//
//std::vector<float> EquiRectWorldGenerator::blockHeightMap;
//
//std::vector<float> EquiRectWorldGenerator::blockScaleMap;
//
//std::vector<float> EquiRectWorldGenerator::xMap;
//
//std::vector<float> EquiRectWorldGenerator::yMap;
//
//std::vector<float> EquiRectWorldGenerator::zMap;
//
//std::vector<float> EquiRectWorldGenerator::layerMap;
//
//std::unique_ptr<HastyNoise::VectorSet> EquiRectWorldGenerator::vectorSet;
//
//
//std::vector<float> EquiRectWorldGenerator::regionHeightMap;
//
//std::unique_ptr<HastyNoise::VectorSet> EquiRectWorldGenerator::regionVectorSet;

class EquiRectWorldGenerator::Hidden
{
public:
    Hidden()
    {}

    FastNoise::SmartNode<FastNoise::OpenSimplex2> m_os2Noise;

    FastNoise::SmartNode<FastNoise::DomainScale> m_domainScale;
    FastNoise::SmartNode<FastNoise::DomainWarpGradient> m_domainWarp;
    FastNoise::SmartNode<FastNoise::DomainWarpFractalIndependant> m_domainWarpFractal;
    FastNoise::SmartNode<FastNoise::FractalFBm> m_fractalFbmNoise;

    FastNoise::SmartNode<FastNoise::CellularValue> m_cellularNoise;
    FastNoise::SmartNode<FastNoise::CellularDistance> m_cellularDistanceNoise;
};

EquiRectWorldGenerator::EquiRectWorldGenerator()
{
    m_hidden.reset(new Hidden());

    m_plateCount=16;
//    initNoise();//make sure noise dlls are loaded
}


EquiRectWorldGenerator::~EquiRectWorldGenerator()
{}


void EquiRectWorldGenerator::create(WorldDescriptors *descriptors, Progress &progress)
{
    initialize(descriptors);
    generateWorldOverview(progress);
}


void EquiRectWorldGenerator::initialize(WorldDescriptors *descriptors)
{
    m_descriptors=*descriptors;

    bool loaded=m_descriptorValues.load(m_descriptors.getGeneratorArgs().c_str());

    if(!loaded)
    {
        m_descriptorValues.calculateInfluenceSize(m_descriptors);
        saveDescriptors(m_descriptors.getGeneratorArgs());
    }

    m_descriptorValues.init(m_descriptors);

    //    m_descriptors=descriptors;
    //    assert(m_descriptors!=nullptr);
//    assert(m_descriptors.getChunkSize() == glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
    int seed=m_descriptorValues.seed;

    //these were only ever assigned inside the HastyNoise block below, so every world was drawing
    //its plates from the same uninitialised seed
    m_continentSeed=seed;
    m_plateSeed=seed+2;
//    m_simdLevel=HastyNoise::GetFastestSIMD();

//    m_continentPerlin=HastyNoise::CreateNoise(seed, m_simdLevel);
//
//    m_continentSeed=seed;
//    m_continentPerlin->SetNoiseType(HastyNoise::NoiseType::PerlinFractal);
//    m_continentPerlin->SetFrequency(m_descriptorValues.m_continentFrequency);
//    m_continentPerlin->SetFractalLacunarity(m_descriptorValues.m_continentLacunarity);
//    m_continentPerlin->SetFractalOctaves(m_descriptorValues.m_continentOctaves);
//    
//
//    m_layersPerlin=HastyNoise::CreateNoise(seed+1, m_simdLevel);
//
//    m_layersPerlin->SetNoiseType(HastyNoise::NoiseType::PerlinFractal);
//    m_layersPerlin->SetFrequency(m_descriptorValues.m_continentFrequency);
//    m_layersPerlin->SetFractalLacunarity(m_descriptorValues.m_continentLacunarity);
//    m_layersPerlin->SetFractalOctaves(m_descriptorValues.m_continentOctaves);
//
//    m_plateSeed=seed+2;
//    m_cellularNoise=HastyNoise::CreateNoise(seed+2, m_simdLevel);
//
//    m_cellularNoise->SetNoiseType(HastyNoise::NoiseType::Cellular);

    m_hidden->m_os2Noise=FastNoise::New<FastNoise::OpenSimplex2>();

    m_hidden->m_domainScale=FastNoise::New<FastNoise::DomainScale>();
    m_hidden->m_domainWarp=FastNoise::New<FastNoise::DomainWarpGradient>();
    m_hidden->m_domainWarpFractal=FastNoise::New<FastNoise::DomainWarpFractalIndependant>();
    m_hidden->m_fractalFbmNoise=FastNoise::New<FastNoise::FractalFBm>();

    m_hidden->m_cellularNoise=FastNoise::New<FastNoise::CellularValue>();
    m_hidden->m_cellularDistanceNoise=FastNoise::New<FastNoise::CellularDistance>();

    FastSIMD::eLevel simdLevel=m_hidden->m_cellularNoise->GetSIMDLevel();

    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> plateDistribution(m_descriptorValues.m_plateCountMin, m_descriptorValues.m_plateCountMax);

    //    m_plateCount=plateDistribution(generator);

    std::uniform_real_distribution<float> jitterDistribution(m_descriptorValues.m_plateCountMin, m_descriptorValues.m_plateCountMax);
//    generateWorldOverview();
}


void EquiRectWorldGenerator::loadDescriptors(WorldDescriptors *descriptors)
{
    initialize(descriptors);
}


void EquiRectWorldGenerator::saveDescriptors(WorldDescriptors *descriptors)
{
    std::string descriptorsString;

    saveDescriptors(descriptorsString);
//    descriptors->setGeneratorDescriptors(descriptorsString.c_str());
}


void EquiRectWorldGenerator::saveDescriptors(std::string &descriptors)
{
    size_t size=1024;

    descriptors.resize(size);

    //removing const as we plan to stay inside the memory range from the resize above
    if(!m_descriptorValues.save((char *)descriptors.data(), size))
    {
        descriptors.resize(size);
        m_descriptorValues.save((char *)descriptors.data(), size);
    }
    descriptors.resize(size);
}


template<typename _FileIO>
bool EquiRectWorldGenerator::load(WorldDescriptors *descriptors, const std::string &directory, Progress &progress)
{
    typedef generic::io::fs<_FileIO> fs;
    std::string overviewFileName=directory+"/overview.bin";

    bool loaded=false;

    initialize(descriptors);
    if(fs::exists(overviewFileName))
        loaded=loadWorldOverview<_FileIO>(overviewFileName);

    if(!loaded)
    {
        generateWorldOverview(progress);
        return false;
    }
    
    progress.update("Normalize neighbors", 80, false);

    std::string normalizeFileName=directory+"/normalize.bin";

    bool normlizeLoaded=false;

    if(fs::exists(normalizeFileName))
        normlizeLoaded=loadNormalize<_FileIO>(normalizeFileName);

    if(!normlizeLoaded)
    {
        updateInfluenceNeighbors();
        saveNormalize<_FileIO>(normalizeFileName);
        return false;
    }

    progress.update("Generating complete", 90, false);

    return true;
}


template<typename _FileIO>
void EquiRectWorldGenerator::save(const std::string &directory)
{
    typedef generic::io::fs<_FileIO> fs;

    if(!fs::exists(directory))
        fs::create_directory(directory);

    std::string overviewFileName=directory+"/overview.bin";

    saveWorldOverview<_FileIO>(overviewFileName);
}


void EquiRectWorldGenerator::generateWorldOverview(Progress &progress)
{

    generatePlates(progress);
    //    generateContinents();
}


template<typename _FileIO>
bool EquiRectWorldGenerator::loadWorldOverview(const std::string &fileName)
{
    typedef generic::io::fs<_FileIO> fs;

    typename fs::Type *file=fs::open(fileName, "rb");

    if(!file)
        return false;

    EquiRectWorldGeneratorHeader header;
    
    size_t readSize=fs::read(&header, 1, sizeof(EquiRectWorldGeneratorHeader), file);

    if(readSize != sizeof(EquiRectWorldGeneratorHeader))
        return false;

    if(header.marker != EquiRectWorldGeneratorHeader_Marker)
        return false;

    if(header.cellSize!=sizeof(InfluenceCell))
        return false;

    size_t influenceMapSize=header.x*header.y;

    if(header.size!=influenceMapSize*sizeof(InfluenceCell))
        return false;

    m_influenceMap.resize(influenceMapSize);

    readSize=fs::read(m_influenceMap.data(), sizeof(InfluenceCell), influenceMapSize, file);
    fs::close(file);

    return (readSize == influenceMapSize);
}


template<typename _FileIO>
void EquiRectWorldGenerator::saveWorldOverview(const std::string &fileName)
{
    typedef generic::io::fs<_FileIO> fs;

    typename fs::Type *file=fs::open(fileName, "wb");

    if(!file)
        return;

    EquiRectWorldGeneratorHeader header;

    header.marker=EquiRectWorldGeneratorHeader_Marker;
    header.version=1;
    header.x=m_descriptorValues.m_influenceSize.x;
    header.y=m_descriptorValues.m_influenceSize.y;
    header.cellSize=sizeof(InfluenceCell);
    header.size=header.x*header.y*sizeof(InfluenceCell);

    assert(m_influenceMap.size()==(header.x*header.y));

    fs::write(&header, sizeof(EquiRectWorldGeneratorHeader), 1, file);
    fs::write(m_influenceMap.data(), sizeof(InfluenceCell), m_influenceMap.size(), file);
    fs::close(file);
}


template<typename _FileIO>
bool EquiRectWorldGenerator::loadNormalize(const std::string &fileName)
{
    typedef generic::io::fs<_FileIO> fs;

    typename fs::Type *file=fs::open(fileName, "rb");

    if(!file)
        return false;

    NormalizeHeader header;

    size_t readSize=fs::read(&header, 1, sizeof(NormalizeHeader), file);

    if(readSize != sizeof(NormalizeHeader))
        return false;

    if(header.marker != EquiRectWorldGeneratorHeader_Marker)
        return false;

    m_influenceNeighborMap.resize(header.size);

    readSize=fs::read(m_influenceNeighborMap.data(), sizeof(float), header.size, file);
    fs::close(file);

    return (readSize == header.size);
}


template<typename _FileIO>
void EquiRectWorldGenerator::saveNormalize(const std::string &fileName)
{
    typedef generic::io::fs<_FileIO> fs;

    typename fs::Type *file=fs::open(fileName, "wb");

    if(!file)
        return;

    NormalizeHeader header;

    header.marker=EquiRectWorldGeneratorHeader_Marker;
    header.version=1;
    header.size=m_influenceNeighborMap.size();

    fs::write(&header, sizeof(NormalizeHeader), 1, file);
    fs::write(m_influenceNeighborMap.data(), sizeof(float), m_influenceNeighborMap.size(), file);
    fs::close(file);
}


//Cellular noise hands back plates of roughly equal area, but a believable world runs a size
//hierarchy instead - a handful of majors, a set of minors and a tail of microplates. The noise is
//run finer than the plate count we actually want and the final plates are grown out of those raw
//cells, majors absorbing the most. Merging only ever erases a boundary, so the distance field
//stays valid: an erased boundary leaves index==borderIndex, which contributes no relief.
static void buildPlateHierarchy(size_t rawCount, const std::vector<int> &rawNeighbors, const std::vector<glm::vec3> &rawCenters,
    size_t majorCount, size_t minorCount, size_t microCount, std::default_random_engine &generator,
    std::vector<size_t> &plateRemap, std::vector<PlateSize> &plateSizes)
{
    size_t targetCount=majorCount+minorCount+microCount;
    const size_t unassigned=std::numeric_limits<size_t>::max();

    plateRemap.assign(rawCount, unassigned);

    if((targetCount<=0)||(rawCount<=targetCount))
    {//not enough raw cells to grow a hierarchy out of, take them as they come
        plateSizes.assign(rawCount, PlateSize::Micro);
        for(size_t i=0; i<rawCount; ++i)
            plateRemap[i]=i;
        return;
    }

    plateSizes.resize(targetCount);

    //Plate areas on earth follow roughly a power law rather than three flat tiers - one pacific,
    //then a run of smaller majors, down to a tail of microplates. Ramp the growth weights the same
    //way so the largest plate ends up about largestPlateRatio times the smallest.
    const float largestPlateRatio=12.0f;

    std::vector<size_t> weights(targetCount);
    float decay=(targetCount>1)?pow(1.0f/largestPlateRatio, 1.0f/(float)(targetCount-1)):1.0f;
    float weight=largestPlateRatio;

    for(size_t i=0; i<targetCount; ++i)
    {
        if(i<majorCount)
            plateSizes[i]=PlateSize::Major;
        else if(i<majorCount+minorCount)
            plateSizes[i]=PlateSize::Minor;
        else
            plateSizes[i]=PlateSize::Micro;

        weights[i]=std::max((size_t)1, (size_t)(weight+0.5f));
        weight=weight*decay;
    }

    size_t totalWeight=0;

    for(size_t i=0; i<targetCount; ++i)
        totalWeight+=weights[i];

    std::vector<size_t> quota(targetCount);

    for(size_t i=0; i<targetCount; ++i)
        quota[i]=std::max((size_t)1, (rawCount*weights[i])/totalWeight);

    //seed the final plates with raw cells spread over the sphere, farthest point first so the
    //majors do not all start on top of one another
    std::vector<size_t> seeds;
    std::vector<float> seedDistance(rawCount, std::numeric_limits<float>::max());
    std::uniform_int_distribution<size_t> seedDistribution(0, rawCount-1);

    seeds.push_back(seedDistribution(generator));

    while(seeds.size()<targetCount)
    {
        const glm::vec3 &last=rawCenters[seeds.back()];
        size_t best=0;
        float bestDistance=-1.0f;

        for(size_t i=0; i<rawCount; ++i)
        {
            seedDistance[i]=std::min(seedDistance[i], glm::distance2(rawCenters[i], last));

            if(seedDistance[i]>bestDistance)
            {
                bestDistance=seedDistance[i];
                best=i;
            }
        }

        seedDistance[best]=-1.0f; //taken
        seeds.push_back(best);
    }

    std::vector<std::vector<size_t>> frontier(targetCount);
    std::vector<size_t> assigned(targetCount, 1);

    for(size_t i=0; i<targetCount; ++i)
    {
        plateRemap[seeds[i]]=i;

        for(size_t j=0; j<rawCount; ++j)
        {
            if(rawNeighbors[(seeds[i]*rawCount)+j])
                frontier[i].push_back(j);
        }
    }

    //grow all the final plates at once, each taking as many cells per round as its weight, so the
    //majors pull ahead while the shapes stay interlocked rather than concentric. The second pass
    //drops the quotas so nothing is left stranded.
    for(size_t pass=0; pass<2; ++pass)
    {
        bool growing=true;

        while(growing)
        {
            growing=false;

            for(size_t i=0; i<targetCount; ++i)
            {
                for(size_t step=0; step<weights[i]; ++step)
                {
                    if((pass==0)&&(assigned[i]>=quota[i]))
                        break;

                    size_t next=unassigned;

                    while(!frontier[i].empty())
                    {
                        size_t candidate=frontier[i].back();

                        frontier[i].pop_back();

                        if(plateRemap[candidate]==unassigned)
                        {
                            next=candidate;
                            break;
                        }
                    }

                    if(next==unassigned)
                        break;

                    plateRemap[next]=i;
                    assigned[i]++;
                    growing=true;

                    for(size_t j=0; j<rawCount; ++j)
                    {
                        if(rawNeighbors[(next*rawCount)+j]&&(plateRemap[j]==unassigned))
                            frontier[i].push_back(j);
                    }
                }
            }
        }
    }

    //anything with no path to a seed (an island cell in the neighbor graph) joins the first plate
    for(size_t i=0; i<rawCount; ++i)
    {
        if(plateRemap[i]==unassigned)
            plateRemap[i]=0;
    }
}


//the map is a cylinder, x wraps
static inline int wrapIndex(int value, int size)
{
    value=value%size;
    if(value<0)
        value+=size;
    return value;
}


//Hotspots sit in the mantle and are not tied to a plate boundary, so the plate slides over one and
//is left with an age ordered chain behind it - live volcanoes at one end, cooled and eroded
//seamounts and atolls at the other. Rotating the hotspot backwards about its plate's euler pole
//traces exactly that chain. Under a continent the same plume makes a broad caldera instead.
static void rasterizeHotspots(const EquiRectDescriptors &descriptors, const glm::ivec2 &influenceSize,
    const std::vector<PlateInfo> &plateDetails, const std::vector<size_t> &cellPlate,
    const std::vector<float> &xPositions, const std::vector<float> &yPositions, const std::vector<float> &zPositions,
    std::default_random_engine &generator, std::vector<Hotspot> &hotspots, std::vector<float> &hotspotMap)
{
    hotspots.clear();
    hotspotMap.assign((size_t)influenceSize.x*influenceSize.y, 0.0f);

    if((descriptors.m_hotspotCount<=0)||(descriptors.m_hotspotTrackSteps<=0)||plateDetails.empty())
        return;

    size_t count=(size_t)descriptors.m_hotspotCount;
    size_t steps=(size_t)descriptors.m_hotspotTrackSteps;
    float sphereRadius=(float)influenceSize.x/2.0f;

    std::uniform_real_distribution<float> jitterDistribution(-0.5f, 0.5f);
    std::uniform_real_distribution<float> strengthDistribution(0.6f, 1.0f);
    std::vector<float> jitter(count);

    for(size_t i=0; i<count; ++i)
        jitter[i]=jitterDistribution(generator);

    //hotspots are independent of the plates, so spread them evenly over the whole sphere
    std::vector<glm::vec3> sphericalPoints=generateFibonacciSphere(count, jitter);

    //one volcano is about this wide. It has to stay well under the spacing between track steps or
    //the chain fuses into a ridge instead of reading as separate islands.
    float baseRadius=2.0f*glm::pi<float>()/(float)influenceSize.y;
    float stepAngle=descriptors.m_hotspotTrackLength/(float)steps;

    for(size_t i=0; i<count; ++i)
    {
        glm::vec3 origin;

        projectPoint<Projections::Spherical, Projections::Cartesian>(sphericalPoints[i], origin);
        origin=glm::normalize(origin);

        glm::vec2 originMap;

        projectPoint<Projections::Cartesian, Projections::Equirectangular>(origin, originMap);

        int originX=wrapIndex((int)(originMap.x*influenceSize.x), influenceSize.x);
        int originY=clamp((int)(originMap.y*influenceSize.y), 0, influenceSize.y-1);
        size_t originIndex=((size_t)originY*influenceSize.x)+originX;

        //the plume takes on the character of whatever crust is currently sitting over it
        const PlateInfo &plate=plateDetails[cellPlate[originIndex]];

        Hotspot hotspot;

        hotspot.point3d=origin;
        hotspot.strength=descriptors.m_hotspotStrength*strengthDistribution(generator);
        hotspot.oceanic=plate.oceanic;
        hotspot.plate=cellPlate[originIndex];
        hotspots.push_back(hotspot);

        for(size_t step=0; step<steps; ++step)
        {
            //crust that sat over the plume `step` ago has since been carried this far around the
            //plate's euler pole, so that is where its volcano is now
            glm::vec3 track=glm::vec3(glm::rotate(stepAngle*(float)step, plate.eulerPole)*glm::vec4(origin, 1.0f));

            track=glm::normalize(track);

            //islands cool, subside and erode as they are carried off the plume
            float age=1.0f/(1.0f+((float)step*0.22f));
            float amplitude=hotspot.strength*age;
            float angularRadius=baseRadius*(0.35f+(0.65f*age));

            glm::vec2 trackMap;

            projectPoint<Projections::Cartesian, Projections::Equirectangular>(track, trackMap);

            int centerX=(int)(trackMap.x*influenceSize.x);
            int centerY=(int)(trackMap.y*influenceSize.y);

            //equirectangular squeezes longitude toward the poles, so the stamp has to widen in x
            float cosLatitude=std::max(sqrt(std::max(1.0f-(track.z*track.z), 0.0f)), 1e-4f);
            int windowY=(int)ceil(angularRadius*influenceSize.y/glm::pi<float>())+1;
            int windowX=(int)ceil(angularRadius*influenceSize.x/(glm::two_pi<float>()*cosLatitude))+1;

            windowX=std::min(windowX, influenceSize.x/2);

            float cosRadius=cos(angularRadius);

            for(int y=centerY-windowY; y<=centerY+windowY; ++y)
            {
                if((y<0)||(y>=influenceSize.y))
                    continue;

                size_t row=(size_t)y*influenceSize.x;

                for(int x=centerX-windowX; x<=centerX+windowX; ++x)
                {
                    size_t index=row+wrapIndex(x, influenceSize.x);

                    //positions were stored with x and y swapped for the noise generator
                    glm::vec3 cellPoint(yPositions[index], xPositions[index], zPositions[index]);

                    cellPoint=cellPoint/sphereRadius;

                    float cosAngle=glm::dot(track, cellPoint);

                    if(cosAngle<=cosRadius)
                        continue;

                    float falloff=smoothStep(clamp((acos(clamp(cosAngle, -1.0f, 1.0f))/angularRadius), 0.0f, 1.0f));

                    falloff=1.0f-falloff;

                    float value;

                    if(hotspot.oceanic)
                        value=amplitude*falloff; //seamount, island, then atoll as it subsides
                    else
                    {//a plume under continental crust lifts a broad dome and drops a caldera in it
                        float dip=falloff*falloff;

                        dip=dip*dip*dip;
                        value=amplitude*0.5f*(falloff-(1.6f*dip));
                    }

                    //tracks rarely overlap, keep the stronger of the two rather than summing
                    if(std::abs(value)>std::abs(hotspotMap[index]))
                        hotspotMap[index]=value;
                }
            }
        }
    }
}


//Places the highs the winds spiral out of. Three sorts, following the reference: subtropical highs
//sitting over the oceans at around 30 degrees, continental highs over big cold landmasses (land
//loses heat faster than water, and cold air is heavy), and one at each pole. Everything between
//them ends up as the low pressure the winds converge into, which is where the fronts fall.
static void placePressureCentres(const EquiRectDescriptors &descriptors, const glm::ivec2 &influenceSize,
    const std::vector<InfluenceCell> &influenceMap, std::vector<PressureCentre> &centres)
{
    centres.clear();

    size_t mapSize=(size_t)influenceSize.x*influenceSize.y;
    float radius=rads(descriptors.m_pressureCentreRadius);

    auto latitudeOf=[&influenceSize](int y)
    {
        return (float)M_PI_2-((float)M_PI*(((float)y+0.5f)/(float)influenceSize.y));
    };
    auto longitudeOf=[&influenceSize](int x)
    {
        return (float)(2.0*M_PI)*(((float)x+0.5f)/(float)influenceSize.x);
    };

    //subtropical highs, one band per hemisphere, sat over open water
    for(int hemisphere=0; hemisphere<2; ++hemisphere)
    {
        float targetLatitude=rads(descriptors.m_subtropicalLatitude)*((hemisphere==0)?1.0f:-1.0f);
        int row=(int)((((float)M_PI_2-targetLatitude)/(float)M_PI)*influenceSize.y);

        row=clamp(row, 0, influenceSize.y-1);

        //how much open water sits around each longitude on this band
        std::vector<float> ocean(influenceSize.x, 0.0f);
        int window=std::max(1, influenceSize.y/16);

        for(int x=0; x<influenceSize.x; ++x)
        {
            float water=0.0f;
            float total=0.0f;

            for(int dy=-window; dy<=window; ++dy)
            {
                int y=clamp(row+dy, 0, influenceSize.y-1);

                for(int dx=-window; dx<=window; ++dx)
                {
                    size_t index=((size_t)y*influenceSize.x)+wrapIndex(x+dx, influenceSize.x);

                    if(influenceMap[index].heightBase<0.5f)
                        water+=1.0f;
                    total+=1.0f;
                }
            }
            ocean[x]=(total>0.0f)?(water/total):0.0f;
        }

        //take the wettest longitudes, keeping them apart so the cells do not stack up
        int separation=influenceSize.x/std::max(1, descriptors.m_subtropicalHighs+1);
        std::vector<bool> taken(influenceSize.x, false);

        for(int i=0; i<descriptors.m_subtropicalHighs; ++i)
        {
            int best=-1;
            float bestScore=-1.0f;

            for(int x=0; x<influenceSize.x; ++x)
            {
                if(taken[x]||(ocean[x]<=bestScore))
                    continue;

                bestScore=ocean[x];
                best=x;
            }

            if((best<0)||(bestScore<=0.35f)) //no real ocean left on this band
                break;

            for(int dx=-separation; dx<=separation; ++dx)
                taken[wrapIndex(best+dx, influenceSize.x)]=true;

            PressureCentre centre;

            centre.position=glm::vec2(longitudeOf(best), latitudeOf(row));
            centre.strength=1.0f;
            centre.radius=radius;
            projectPoint<Projections::Spherical, Projections::Cartesian>(glm::vec3(1.0f, centre.position.x, centre.position.y), centre.point3d);
            centres.push_back(centre);
        }
    }

    //continental highs. Flood fill the landmasses and give the big ones that sit far enough from
    //the equator a cell of their own, sat on the middle of the landmass.
    {
        const size_t noLand=std::numeric_limits<size_t>::max();
        std::vector<size_t> landmass(mapSize, noLand);
        std::vector<size_t> stack;
        size_t landmassCount=0;

        for(size_t seed=0; seed<mapSize; ++seed)
        {
            if((landmass[seed]!=noLand)||(influenceMap[seed].heightBase<0.5f))
                continue;

            float area=0.0f;
            float weightedLatitude=0.0f;
            glm::vec2 sumDirection(0.0f, 0.0f); //longitude averaged as an angle, the map wraps
            float sumLatitude=0.0f;

            landmass[seed]=landmassCount;
            stack.push_back(seed);

            while(!stack.empty())
            {
                size_t index=stack.back();

                stack.pop_back();

                int x=(int)(index%influenceSize.x);
                int y=(int)(index/influenceSize.x);
                float latitude=latitudeOf(y);
                float weight=cos(latitude);

                area+=weight;
                sumLatitude+=latitude*weight;
                sumDirection+=glm::vec2(cos(longitudeOf(x)), sin(longitudeOf(x)))*weight;
                weightedLatitude+=weight;

                const int offsets[4][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

                for(size_t n=0; n<4; ++n)
                {
                    int neighborY=y+offsets[n][1];

                    if((neighborY<0)||(neighborY>=influenceSize.y))
                        continue;

                    size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                    if((landmass[neighborIndex]==noLand)&&(influenceMap[neighborIndex].heightBase>=0.5f))
                    {
                        landmass[neighborIndex]=landmassCount;
                        stack.push_back(neighborIndex);
                    }
                }
            }
            landmassCount++;

            float sphereArea=2.0f*(float)influenceSize.x*influenceSize.y/(float)M_PI;

            if((weightedLatitude<=0.0f)||((area/sphereArea)<descriptors.m_continentalHighArea))
                continue;

            float centreLatitude=sumLatitude/weightedLatitude;

            if(fabs(centreLatitude)<rads(descriptors.m_continentalHighLatitude))
                continue; //too close to the equator to run cold

            if(glm::length2(sumDirection)<=std::numeric_limits<float>::epsilon())
                continue;

            PressureCentre centre;

            centre.position=glm::vec2(atan2(sumDirection.y, sumDirection.x), centreLatitude);
            if(centre.position.x<0.0f)
                centre.position.x+=(float)(2.0*M_PI);
            centre.strength=1.0f;
            centre.radius=radius;
            projectPoint<Projections::Spherical, Projections::Cartesian>(glm::vec3(1.0f, centre.position.x, centre.position.y), centre.point3d);
            centres.push_back(centre);
        }
    }

    //and the poles, the coldest ground of all
    for(int hemisphere=0; hemisphere<2; ++hemisphere)
    {
        PressureCentre centre;

        centre.position=glm::vec2(0.0f, (float)M_PI_2*((hemisphere==0)?1.0f:-1.0f));
        centre.strength=1.0f;
        centre.radius=radius*1.4f;
        projectPoint<Projections::Spherical, Projections::Cartesian>(glm::vec3(1.0f, centre.position.x, centre.position.y), centre.point3d);
        centres.push_back(centre);
    }
}


void EquiRectWorldGenerator::generatePlates(Progress &progress)
{

    //Cell count comes from how fast the planet spins rather than being fixed at earth's three per
    //hemisphere. At a rotation rate of 1.0 this reproduces the old hardcoded table exactly, down to
    //the 0/30/60/90 boundaries and the trade/westerly/easterly wind directions.
    int cellsPerHemisphere=weatherCellsForRotation(m_descriptorValues.m_rotationRate);
    std::vector<WeatherCell> weatherCells=generateWeatherCells(cellsPerHemisphere, m_descriptorValues.m_rotationDirection);

	glm::ivec2 influenceSize=m_descriptorValues.m_influenceSize;
    int influenceMapSize=influenceSize.x*influenceSize.y;
    int alignedMapSize=influenceSize.x*influenceSize.y;// HastyNoise::AlignedSize(influenceSize.x*influenceSize.y, m_simdLevel);
	glm::ivec2 size=m_descriptors.getSize();

    progress.update("Generating weather bands", 0, false);
//    WeatherBands weather(weatherCells);
    PerturbedWeatherBands weather(m_plateSeed+10, influenceSize, weatherCells);

    chrono::high_resolution_clock::time_point time1;
    chrono::high_resolution_clock::time_point time2;

    std::vector<float> &plateMap=plateMap_noise;
    std::vector<float> &plate2Map=plate2Map_noise;
    std::vector<float> &plateDistanceMap=plateDistance_noise;
    std::vector<float> &continentMap=continentMap_noise;
    std::vector<float> &heightMap=heightMap_noise;
    std::vector<float> terrainScaleMap;
    std::vector<float> detailMap;
    std::vector<float> i;
    std::vector<float> nsAirCurrent;
    std::vector<float> ewAirCurrent;
    std::vector<PlateInfo> plateDetails;
    std::vector<float> moistureMap;
    std::vector<float> moistureMap2;
    std::vector<float> moistureDeltaMap;

    time1=chrono::high_resolution_clock::now();

    progress.update("Allocating resources", 5, false);

    plateMap.resize(alignedMapSize);
    plate2Map.resize(alignedMapSize);
    plateScaleMap.resize(alignedMapSize);
    plate2ScaleMap.resize(alignedMapSize);
    plateDistanceMap.resize(alignedMapSize);
    continentMap.resize(alignedMapSize);
    heightMap.resize(alignedMapSize);
    terrainScaleMap.resize(alignedMapSize);
    detailMap.resize(alignedMapSize);
    moistureMap.resize(alignedMapSize);
    moistureMap2.resize(alignedMapSize);
    moistureDeltaMap.resize(alignedMapSize);

    nsAirCurrent.resize(alignedMapSize);
    ewAirCurrent.resize(alignedMapSize);

    m_influenceMap.resize(influenceMapSize);

//    m_influenceVectorSet=std::make_unique<HastyNoise::VectorSet>(m_simdLevel);
//    m_influenceVectorSet->SetSize(alignedMapSize);
    m_xPositions.resize(alignedMapSize);
    m_yPositions.resize(alignedMapSize);
    m_zPositions.resize(alignedMapSize);


    time2=chrono::high_resolution_clock::now();
    allocationTime=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

    progress.update("Generating coordinates", 10, false);
    glm::vec3 mapPos;
    size_t index=0;
    mapPos.z=(float)(influenceSize.x/2.0f);
    for(int y=0; y<influenceSize.y; y++)
    {
        mapPos.y=y;
        for(int x=0; x<influenceSize.x; x++)
        {
            mapPos.x=x;
            glm::vec3 pos=getSphericalCoords(influenceSize.x, influenceSize.y, mapPos);

            //fastnoise treats x and y in reverse, need to change
            m_xPositions[index]=pos.y;//*m_descriptorValues.m_plateFrequency;
            m_yPositions[index]=pos.x;//*m_descriptorValues.m_plateFrequency;
            m_zPositions[index]=pos.z;//*m_descriptorValues.m_plateFrequency;
            index++;
        }
    }

    time1=chrono::high_resolution_clock::now();
    coordsTime=chrono::duration_cast<chrono::milliseconds>(time1-time2).count();

    progress.update("Generating heightmap", 15, false);
//height map
    m_hidden->m_domainScale->SetSource(m_hidden->m_os2Noise);
    m_hidden->m_fractalFbmNoise->SetSource(m_hidden->m_domainScale);

    m_hidden->m_domainScale->SetScale(0.01f);
    m_hidden->m_fractalFbmNoise->SetGain(0.5f);
    m_hidden->m_fractalFbmNoise->SetOctaveCount(4);
    m_hidden->m_fractalFbmNoise->SetLacunarity(2.0f);
    m_hidden->m_fractalFbmNoise->GenPositionArray3D(heightMap.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_plateSeed);

    //High frequency detail, finer than the base height field and much weaker. This is what keeps
    //the middle of a plate from reading as a featureless plain.
    m_hidden->m_domainScale->SetScale(m_descriptorValues.m_terrainDetailScale);
    m_hidden->m_fractalFbmNoise->SetGain(0.5f);
    m_hidden->m_fractalFbmNoise->SetOctaveCount(5);
    m_hidden->m_fractalFbmNoise->SetLacunarity(2.0f);
    m_hidden->m_fractalFbmNoise->GenPositionArray3D(detailMap.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_plateSeed+7);

    progress.update("Generating terrain", 20, false);

    m_hidden->m_domainScale->SetScale(0.05f);
    m_hidden->m_fractalFbmNoise->SetGain(0.5f);
    m_hidden->m_fractalFbmNoise->SetOctaveCount(4);
    m_hidden->m_fractalFbmNoise->SetLacunarity(2.0f);
    m_hidden->m_fractalFbmNoise->GenPositionArray3D(terrainScaleMap.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_plateSeed);

    progress.update("Generating air currents", 25, false);

    m_hidden->m_domainWarp->SetSource(m_hidden->m_domainScale);
    m_hidden->m_fractalFbmNoise->SetSource(m_hidden->m_domainWarp);

    m_hidden->m_domainScale->SetScale(0.01f);
    m_hidden->m_domainWarp->SetWarpAmplitude(0.5f);
    m_hidden->m_domainWarp->SetWarpFrequency(1.0f);
    m_hidden->m_fractalFbmNoise->SetGain(0.5f);
    m_hidden->m_fractalFbmNoise->SetOctaveCount(4);
    m_hidden->m_fractalFbmNoise->SetLacunarity(2.0f);
    m_hidden->m_fractalFbmNoise->GenPositionArray3D(nsAirCurrent.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_plateSeed+3);


//    m_continentPerlin->SetSeed(m_plateSeed+4);
//    m_continentPerlin->FillSet(ewAirCurrent.data(), m_influenceVectorSet.get());
    m_hidden->m_fractalFbmNoise->GenPositionArray3D(ewAirCurrent.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_plateSeed+4);

    progress.update("Generating tectonic plates", 30, false);

//plate map
    m_hidden->m_cellularNoise->SetDistanceFunction(FastNoise::DistanceFunction::Hybrid);
    m_hidden->m_cellularNoise->SetValueIndex(0);
    m_hidden->m_domainWarp->SetSource(m_hidden->m_cellularNoise);
    m_hidden->m_domainWarp->SetWarpAmplitude(m_descriptorValues.m_plateWarpAmplitude);
    m_hidden->m_domainWarp->SetWarpFrequency(1.0f);
    m_hidden->m_domainWarpFractal->SetSource(m_hidden->m_domainWarp);
    m_hidden->m_domainWarpFractal->SetGain(0.5f);
    m_hidden->m_domainWarpFractal->SetOctaveCount(5);
    m_hidden->m_domainWarpFractal->SetLacunarity(2.0f);
    m_hidden->m_domainScale->SetSource(m_hidden->m_domainWarpFractal);
    m_hidden->m_domainScale->SetScale(m_descriptorValues.m_plateFrequency);

    m_hidden->m_domainScale->GenPositionArray3D(plateMap.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_plateSeed);
//    m_hidden->m_cellularNoise->GenPositionArray3D(plateMap.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
//        0.0f, 0.0f, 0.0f, m_plateSeed);
//    m_hidden->m_cellularNoise->FillSet(plateMap.data(), m_influenceVectorSet.get());

//Border plate Map
    m_hidden->m_cellularNoise->SetValueIndex(1);
    m_hidden->m_domainScale->GenPositionArray3D(plate2Map.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_plateSeed);

    time2=chrono::high_resolution_clock::now();
    cellularPlateTime=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

    progress.update("Generating tectonic plates", 35, false);

//Distance Map
    //going to generate twice as I want the distance value as well, will mod HastyNoise later to produce both (as it has already done the work)
    m_hidden->m_cellularDistanceNoise->SetDistanceFunction(FastNoise::DistanceFunction::Hybrid);
    m_hidden->m_cellularDistanceNoise->SetReturnType(CellularReturnType::Index0Div1);//gives distance to border
    m_hidden->m_cellularDistanceNoise->SetDistanceIndex0(0);
    m_hidden->m_cellularDistanceNoise->SetDistanceIndex1(1);
//    m_hidden->m_cellularDistanceNoise->FillSet(plateDistanceMap.data(), m_influenceVectorSet.get());
//    m_hidden->m_domainScale->SetSource(m_hidden->m_cellularDistanceNoise);
    m_hidden->m_domainWarp->SetSource(m_hidden->m_cellularDistanceNoise);

    m_hidden->m_domainScale->GenPositionArray3D(plateDistanceMap.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_plateSeed);

    time1=chrono::high_resolution_clock::now();
    cellularDistanceTime=chrono::duration_cast<chrono::milliseconds>(time1-time2).count();


    time2=chrono::high_resolution_clock::now();
    cellularPlate2Time=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

    progress.update("Generating continents", 45, false);
//continent map
    m_hidden->m_cellularNoise->SetValueIndex(0);
    m_hidden->m_domainWarp->SetSource(m_hidden->m_cellularNoise);
    m_hidden->m_domainScale->SetScale(m_descriptorValues.m_continentFrequency);
//    m_hidden->m_cellularDistanceNoise->SetSeed(m_continentSeed);
//    m_hidden->m_cellularDistanceNoise->SetFrequency(m_descriptorValues.m_continentFrequency);
//    m_hidden->m_cellularDistanceNoise->FillSet(continentMap.data(), m_influenceVectorSet.get());
    m_hidden->m_domainScale->GenPositionArray3D(continentMap.data(), influenceMapSize, m_xPositions.data(), m_yPositions.data(), m_zPositions.data(),
        0.0f, 0.0f, 0.0f, m_continentSeed);

    time1=chrono::high_resolution_clock::now();

    glm::ivec2 point={0, 0};

    //setup plates
    std::default_random_engine generator(m_descriptorValues.seed);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distributionNorm(0.0f, 1.0f);
    std::vector<float> plateMinDistance;
    std::vector<float> plateMaxDistance;
    std::vector<glm::ivec2> plateMinPoint;

    //Raw cells are found by flood filling equal plate values rather than by taking each distinct
    //value as a plate. CellularValue hands back a hash of the cell, and separate cells on opposite
    //sides of the world collide on the same value often enough that identifying plates by value
    //alone leaves them scattered in pieces. A plate has to be one rigid body for its euler pole to
    //mean anything, so the components are the raw cells.
    const size_t noPlate=std::numeric_limits<size_t>::max();

    std::vector<size_t> cellRawPlate(influenceMapSize, noPlate);
    std::vector<size_t> cellRawBorderPlate(influenceMapSize, noPlate);
    std::vector<glm::vec3> rawCenters;
    std::vector<size_t> fillStack;

    size_t rawCount=0;

    progress.update("Generating height map", 50, false);

    for(size_t seedIndex=0; seedIndex<influenceMapSize; seedIndex++)
    {
        if(cellRawPlate[seedIndex]!=noPlate)
            continue;

        float value=plateMap[seedIndex];
        glm::vec3 center(0.0f, 0.0f, 0.0f);

        cellRawPlate[seedIndex]=rawCount;
        fillStack.push_back(seedIndex);

        while(!fillStack.empty())
        {
            size_t index=fillStack.back();

            fillStack.pop_back();

            int x=(int)(index%influenceSize.x);
            int y=(int)(index/influenceSize.x);

            //averaging the cell positions gives the plate centre, which is what the boundary
            //normal is measured against
            center+=glm::vec3(m_yPositions[index], m_xPositions[index], m_zPositions[index]);

            const int offsets[4][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

            for(size_t n=0; n<4; ++n)
            {
                int neighborY=y+offsets[n][1];

                if((neighborY<0)||(neighborY>=influenceSize.y))
                    continue;

                size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                if((cellRawPlate[neighborIndex]==noPlate)&&(plateMap[neighborIndex]==value))
                {
                    cellRawPlate[neighborIndex]=rawCount;
                    fillStack.push_back(neighborIndex);
                }
            }
        }

        if(glm::length2(center)<=std::numeric_limits<float>::epsilon())
            center=glm::vec3(0.0f, 0.0f, 1.0f);

        rawCenters.push_back(glm::normalize(center));
        rawCount++;
    }

    std::vector<int> rawNeighbors(rawCount*rawCount, 0);

    //Which plate lies across a cell's nearest border. Seed from the cells that actually touch a
    //border and flood inward through their own plate, so this stays consistent with the distance
    //field rather than depending on the cellular value of the second nearest seed.
    for(size_t i=0; i<influenceMapSize; i++)
    {
        int x=(int)(i%influenceSize.x);
        int y=(int)(i/influenceSize.x);
        const int offsets[4][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

        for(size_t n=0; n<4; ++n)
        {
            int neighborY=y+offsets[n][1];

            if((neighborY<0)||(neighborY>=influenceSize.y))
                continue;

            size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

            if(cellRawPlate[neighborIndex]==cellRawPlate[i])
                continue;

            rawNeighbors[(cellRawPlate[i]*rawCount)+cellRawPlate[neighborIndex]]=1;
            rawNeighbors[(cellRawPlate[neighborIndex]*rawCount)+cellRawPlate[i]]=1;

            if(cellRawBorderPlate[i]==noPlate)
            {
                cellRawBorderPlate[i]=cellRawPlate[neighborIndex];
                fillStack.push_back(i);
            }
        }
    }

    for(size_t head=0; head<fillStack.size(); ++head)
    {
        size_t index=fillStack[head];
        int x=(int)(index%influenceSize.x);
        int y=(int)(index/influenceSize.x);
        const int offsets[4][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

        for(size_t n=0; n<4; ++n)
        {
            int neighborY=y+offsets[n][1];

            if((neighborY<0)||(neighborY>=influenceSize.y))
                continue;

            size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

            if((cellRawBorderPlate[neighborIndex]!=noPlate)||(cellRawPlate[neighborIndex]!=cellRawPlate[index]))
                continue;

            cellRawBorderPlate[neighborIndex]=cellRawBorderPlate[index];
            fillStack.push_back(neighborIndex);
        }
    }

    fillStack.clear();
    fillStack.shrink_to_fit();

    point={0, 0};
    for(size_t i=0; i<influenceMapSize; i++)
    {
        if(cellRawBorderPlate[i]==noPlate) //a plate with no neighbours, all interior
            cellRawBorderPlate[i]=cellRawPlate[i];

        m_influenceMap[i].plateValue=plateMap[i];
        m_influenceMap[i].plateDistanceValue=plateDistanceMap[i];
        m_influenceMap[i].continentValue=continentMap[i];

//        glm::vec2 airDirection(ewAirCurrent[i], nsAirCurrent[i]);
//        
//        m_influenceMap[i].airDirection=glm::normalize(airDirection);
        m_influenceMap[i].airDirection.x=ewAirCurrent[i];
        m_influenceMap[i].airDirection.y=nsAirCurrent[i];

        point.x++;
        if(point.x>=influenceSize.x)
        {
            point.x=0;
            point.y++;
        }
    }

    progress.update("Generating tectonic zones", 55, false);

    std::vector<size_t> plateRemap;
    std::vector<PlateSize> plateSizes;

    buildPlateHierarchy(rawCount, rawNeighbors, rawCenters, (size_t)m_descriptorValues.m_plateCountMajor,
        (size_t)m_descriptorValues.m_plateCountMinor, (size_t)m_descriptorValues.m_plateCountMicro,
        generator, plateRemap, plateSizes);

    size_t plateCount=plateSizes.size();

    plateDetails.resize(plateCount);
    plateMinDistance.resize(plateCount, 2.0f);
    plateMaxDistance.resize(plateCount, -2.0f);
    plateMinPoint.resize(plateCount);

    std::vector<int> plateNeighbors(plateCount*plateCount, 0);

    //second pass over the map, now that the raw cells have been merged into their final plates.
    //Areas are cosine weighted because an equirectangular cell covers less of the sphere near the
    //poles, and the ocean budget below is only meaningful in real surface area.
    point={0, 0};
    float totalArea=0.0f;

    for(size_t i=0; i<influenceMapSize; i++)
    {
        size_t index=plateRemap[cellRawPlate[i]];
        size_t borderIndex=plateRemap[cellRawBorderPlate[i]];

        m_influenceMap[i].tectonicPlate=index;
        m_influenceMap[i].borderPlate=borderIndex;

        if(index!=borderIndex)
        {
            plateNeighbors[(index*plateCount)+borderIndex]=1;
            plateNeighbors[(borderIndex*plateCount)+index]=1;
        }

        if(plateDistanceMap[i]<plateMinDistance[index])
        {
            plateMinDistance[index]=plateDistanceMap[i];
            plateMinPoint[index]=point;
        }

        if(plateDistanceMap[i]>plateMaxDistance[index])
            plateMaxDistance[index]=plateDistanceMap[i];

        float latitude=glm::half_pi<float>()-(glm::pi<float>()*(((float)point.y+0.5f)/(float)influenceSize.y));
        float area=cos(latitude);

        plateDetails[index].area+=area;
        totalArea+=area;

        plateDetails[index].points.emplace_back(point.x, point.y);

        point.x++;
        if(point.x>=influenceSize.x)
        {
            point.x=0;
            point.y++;
        }
    }

    //An earth like planet needs most of its surface under water, and the trick is to make the
    //biggest plates the oceanic ones - the pacific being the obvious example. Take plates largest
    //first until the ocean budget is covered.
    std::vector<size_t> plateByArea(plateCount);

    for(size_t i=0; i<plateCount; ++i)
        plateByArea[i]=i;

    std::sort(plateByArea.begin(), plateByArea.end(), [&plateDetails](size_t a, size_t b)
    {
        return plateDetails[a].area>plateDetails[b].area;
    });

    float oceanTarget=totalArea*m_descriptorValues.m_oceanFraction;
    float oceanArea=0.0f;

    for(size_t i=0; i<plateByArea.size(); ++i)
    {
        if(oceanArea>=oceanTarget)
            break;

        plateDetails[plateByArea[i]].oceanic=true;
        oceanArea+=plateDetails[plateByArea[i]].area;
    }

    for(size_t i=0; i<plateCount; i++)
    {
        PlateInfo &details=plateDetails[i];

        details.index=i;
        details.size=plateSizes[i];
        details.point=plateMinPoint[i];
        details.value=(float)i/(float)plateCount;

        //Oceanic crust is thin and dense so it rides low, continental crust is thick and buoyant so
        //it rides high. 0.5 is sea level for the influence map. The two ranges deliberately sit
        //close enough to sea level for the terrain noise to reach across, because a continental
        //plate is only predominantly land, not land right out to its boundary - otherwise every
        //coastline would trace a plate edge.
        if(details.oceanic)
            details.height=0.42f-(distributionNorm(generator)*0.08f);
        else
            details.height=0.52f+(distributionNorm(generator)*0.08f);

        glm::vec2 mapPoint((float)details.point.x/(float)influenceSize.x, (float)details.point.y/(float)influenceSize.y);

        projectPoint<Projections::Equirectangular, Projections::Cartesian>(mapPoint, details.point3d);

        //Plate motion is a rotation about an euler pole through the centre of the planet, not a
        //straight line drift. Velocity is w x p, so it varies over the plate, which is what lets
        //one boundary run convergent at one end and transform at the other.
        glm::vec3 pole(distribution(generator), distribution(generator), distribution(generator));

        if(glm::length2(pole)<=std::numeric_limits<float>::epsilon())
            pole=glm::vec3(0.0f, 0.0f, 1.0f);

        details.eulerPole=glm::normalize(pole);

        //the small plates get shoved around faster than the big ones
        float sizeRate=1.5f;

        if(details.size==PlateSize::Major)
            sizeRate=0.6f;
        else if(details.size==PlateSize::Minor)
            sizeRate=1.0f;

        details.angularVelocity=m_descriptorValues.m_plateDriftRate*sizeRate*(0.6f+(distributionNorm(generator)*0.8f));
        details.direction=plateVelocity(details, details.point3d);

        for(size_t j=0; j<plateCount; j++)
        {
            if(plateNeighbors[(i*plateCount)+j])
                details.neighbors.push_back(j);
        }
    }

    progress.update("Generating hotspots", 58, false);

    std::vector<float> hotspotMap;
    std::vector<size_t> cellPlate(influenceMapSize);

    for(size_t i=0; i<influenceMapSize; i++)
        cellPlate[i]=m_influenceMap[i].tectonicPlate;

    rasterizeHotspots(m_descriptorValues, influenceSize, plateDetails, cellPlate, m_xPositions, m_yPositions, m_zPositions,
        generator, m_hotspots, hotspotMap);


    progress.update("Generating influence map", 60, false);

    //below this the cell is inside a plate as far as every boundary curve is concerned
    const float boundaryCutoff=0.55f;

    point={0, 0};
    for(size_t i=0; i<influenceMapSize; i++)
    {
        size_t &index=m_influenceMap[i].tectonicPlate;
        size_t &borderIndex=m_influenceMap[i].borderPlate;

        PlateInfo &details=plateDetails[index];
        PlateInfo &details2=plateDetails[borderIndex];

        //normalize distance
        m_influenceMap[i].plateDistanceValue=(plateDistanceMap[i]-plateMinDistance[index])/(plateMaxDistance[index]-plateMinDistance[index]);
//        m_influenceMap[i].heightBase=0.0f;

//build per pixel direction
        glm::vec3 sphericalPoint;
        glm::vec3 cartPoint;
        glm::vec3 direction;
        glm::vec3 sphericalDirection;

        glm::vec2 normPoint((float)point.x/influenceSize.x, (float)point.y/influenceSize.y);
        projectPoint<Projections::Equirectangular, Projections::Spherical>(normPoint, sphericalPoint);
        projectPoint<Projections::Spherical, Projections::Cartesian>(sphericalPoint, cartPoint);
        
        direction=plateVelocity(details, cartPoint);

        projectVector(direction, sphericalPoint, sphericalDirection);
        m_influenceMap[i].direction.x=sphericalDirection.y;
        m_influenceMap[i].direction.y=sphericalDirection.z;

//Boundary motion is resolved here, per cell, rather than once per pair of plates. Both plates are
//rotating, so the same boundary can close at one end and shear at the other. The normal comes from
//the raw cellular seeds either side of this cell, which is the exact bisector direction, while the
//velocities come from the merged plates those cells belong to.
        float convergence=0.0f;
        float shear=0.0f;

        if(index!=borderIndex)
        {
            resolveBoundaryMotion(details, details2, rawCenters[cellRawPlate[i]], rawCenters[cellRawBorderPlate[i]],
                cartPoint, convergence, shear);

            convergence=clamp(convergence, -1.0f, 1.0f);
            shear=std::min(shear, 1.0f);
        }

        m_influenceMap[i].collision=convergence;
        m_influenceMap[i].shear=shear;

        //every cell knows which plate is across its nearest border, but only the ones close enough
        //to that border to pick up any relief are worth calling a boundary
        if(m_influenceMap[i].plateDistanceValue>boundaryCutoff)
            m_influenceMap[i].boundaryType=classifyBoundary(convergence, shear, m_descriptorValues.m_transformRatio);
        else
            m_influenceMap[i].boundaryType=BoundaryType::None;

//air currents determined by banding and random vectors from before
        glm::vec2 latLong(sphericalPoint.y, sphericalPoint.z);
        float latitude=sphericalPoint.z;
        float dir=1.0f;
        glm::vec2 bandDirection;

        if(sphericalPoint.z<0.0f)
            dir=-1.0f;

        m_influenceMap[i].weatherCell=weather.getCellIndex(latLong);
        m_influenceMap[i].weatherBand=weather.getBandIndex(latLong);
        m_influenceMap[i].pressure=weather.getPressure(latLong);

        bandDirection=weather.getWindDirection(latLong);
        m_influenceMap[i].airDirection=bandDirection;
//        m_influenceMap[i].airDirection=(bandDirection+m_influenceMap[i].airDirection)/2.0f;

//build terrain
        bool oceanPlate=details.oceanic;
        bool oceanPlate2=details2.oceanic;

        float plateScale;
        float plate2Scale;
        float terrainScale=0.0f;

		if((oceanPlate && !oceanPlate2) || (!oceanPlate&&oceanPlate2))
			calculateCurve(m_influenceMap[i].plateDistanceValue, plateScale, plate2Scale, 0.7f);
		else
			calculateCurve(m_influenceMap[i].plateDistanceValue, plateScale, plate2Scale, 0.5f);
        
        plateScaleMap[i]=plateScale;
        plate2ScaleMap[i]=plate2Scale;
//        if(i>51450)
//            i=i;

        OrogenyType orogeny=OrogenyType::None;

        if(index != borderIndex)
        {
            //relief scales with how fast the plates are actually closing or opening. A boundary
            //that is mostly shear builds almost nothing, which is what breaks a mid ocean ridge
            //into offset segments where the spreading direction turns away from the normal.
            float belt=0.0f;

            if(convergence>0.0f)
            {
                float roughness=0.0f;

                belt=calculateConvergentCurve(m_influenceMap[i].plateDistanceValue, details, details2, convergence, orogeny, roughness);

                //broken topography inside the belt and nowhere else, so laramide basins and
                //himalayan deformation stay in the range instead of spilling onto the lowlands
                belt=belt+(belt*roughness*terrainScaleMap[i]);
            }
            else if(convergence<0.0f)
                belt=calculateDivergentCurve(m_influenceMap[i].plateDistanceValue, oceanPlate, oceanPlate2);

            terrainScale=belt*std::abs(convergence);

            //transform boundaries throw earthquakes, not mountains, so all they leave is a scarp
            terrainScale=terrainScale+(transformCurve(m_influenceMap[i].plateDistanceValue)*shear*0.08f);
        }

        m_influenceMap[i].orogeny=orogeny;

        //centred on zero so the noise can carry a coastline either way across sea level
        float genHeight=heightMap[i]*0.09f;
        float genTerrainScale=(terrainScaleMap[i]+1.0f)*0.2f+0.2f;

        //blend of the two plate heights either side of the boundary, this is the crust the cell is
        //actually built on and so decides whether it can carry a shelf
        float continentality=(details.height*plateScale)+(details2.height*plate2Scale);

        m_influenceMap[i].heightBase=((details.height+genHeight)*plateScale)+(details2.height*plate2Scale)+(terrainScale*genTerrainScale);// *0.4f);

        //the fine fractal layer, over everything the tectonics decided
        m_influenceMap[i].heightBase=m_influenceMap[i].heightBase+(detailMap[i]*m_descriptorValues.m_terrainDetail);

        m_influenceMap[i].plateHeight=details.height;
        m_influenceMap[i].terrainScale=terrainScale;

        //the shelf is drowned continental crust, so it goes on before the volcanoes that stand on it
        m_influenceMap[i].heightBase=applyContinentalShelf(m_influenceMap[i].heightBase, continentality, 0.5f, m_descriptorValues.m_continentalShelf);

        m_influenceMap[i].hotspot=hotspotMap[i];
        m_influenceMap[i].heightBase=m_influenceMap[i].heightBase+hotspotMap[i];

		if(m_influenceMap[i].heightBase>1.0f)
			m_influenceMap[i].heightBase=1.0f;
		if(m_influenceMap[i].heightBase<0.0f)
			m_influenceMap[i].heightBase=0.0f;

//temperature. Latitude sets the baseline; altitude and continentality are folded in later, once
//the distance to the sea is known.
        m_influenceMap[i].temperature=latitudeTemperature(latitude, m_descriptorValues.m_globalTemperature, m_descriptorValues.m_polarAmplification);

//moisture
        float bandMoisture=weather.getMoisture(latLong);

        m_influenceMap[i].moistureCapacity=bandMoisture*0.5f;
        if(m_influenceMap[i].heightBase<0.5)
            moistureMap[i]=1.0f;
        //        if(m_influenceMap[i].heightBase<0.5)
        //            moistureMap[i]=1.0f*(m_influenceMap[i].heightBase-0.25f)/0.25f;
        else
            moistureMap[i]=(bandMoisture*0.9)+(nsAirCurrent[i]*0.1f);// *0.5f;

        point.x++;
        if(point.x>=influenceSize.x)
        {
            point.x=0;
            point.y++;
        }
    }

    progress.update("Generating pressure centres", 68, false);

    //The zonal bands give a first approximation, but land breaks the highs up into discrete cells
    //and the winds spiral out of those rather than running straight along the band. Blend the two,
    //so at weight 0 this is exactly the banded model and at weight 1 it is all cells.
    if(m_descriptorValues.m_pressureCentreWeight>0.0f)
    {
        placePressureCentres(m_descriptorValues, influenceSize, m_influenceMap, m_pressureCentres);

        float outflow=rads(m_descriptorValues.m_anticycloneOutflow);
        float spin=(m_descriptorValues.m_rotationDirection<0.0f)?-1.0f:1.0f;
        float weight=std::min(m_descriptorValues.m_pressureCentreWeight, 1.0f);

        point={0, 0};
        for(size_t i=0; i<influenceMapSize; i++)
        {
            glm::vec2 position((float)(2.0*M_PI)*(((float)point.x+0.5f)/(float)influenceSize.x),
                (float)M_PI_2-((float)M_PI*(((float)point.y+0.5f)/(float)influenceSize.y)));

            float strongest=0.0f;
            glm::vec2 spiral(0.0f, 0.0f);
            float total=0.0f;

            for(size_t c=0; c<m_pressureCentres.size(); ++c)
            {
                float influence=pressureCentreInfluence(m_pressureCentres[c], position);

                if(influence<=0.001f)
                    continue;

                //the cells cannot flow into one another, so take the nearest rather than piling
                //them up, and the ground between them is left as the low the winds converge on
                strongest=std::max(strongest, influence);
                spiral+=pressureCentreWind(m_pressureCentres[c], position, outflow, spin)*influence;
                total+=influence;
            }

            if(total>0.0f)
                spiral=spiral/total;

            m_influenceMap[i].pressure=m_influenceMap[i].pressure+((strongest-m_influenceMap[i].pressure)*weight);

            float windWeight=weight*std::min(total, 1.0f);

            m_influenceMap[i].airDirection=m_influenceMap[i].airDirection+((spiral-m_influenceMap[i].airDirection)*windWeight);

            point.x++;
            if(point.x>=influenceSize.x)
            {
                point.x=0;
                point.y++;
            }
        }
    }

    progress.update("Generating moisture", 70, false);
//    std::vector<float> *mapPointer1=&moistureMap;
//    std::vector<float> *mapPointer2=&moistureMap2;
    std::vector<float> &map1=moistureMap;
    for(size_t i=0; i<influenceMapSize; i++)
    {
        if(m_influenceMap[i].heightBase>0.5f)
            moistureDeltaMap[i]=0.0f;
        else
            moistureDeltaMap[i]=1.0f;
    }

    glm::ivec2 lowerImageBound(0, 0);
    glm::ivec2 upperImageBound(influenceSize.x-1, influenceSize.y-1);


    size_t breakX=186;
    size_t breakY=168;
    size_t breakIndex=breakY*influenceSize.x+breakX;
    size_t loops=10;

    std::vector<float> floodScale={0.25f, 0.5f, 0.25f};
    for(size_t loop=0; loop<loops; ++loop)
    {
//        std::vector<float> &map1=*mapPointer1;
//        std::vector<float> &map2=*mapPointer2;

        size_t i=0;
        for(size_t y=0; y<influenceSize.y; y++)
        {
            for(size_t x=0; x<influenceSize.x; x++)
            {
                if((x==breakX)&&(y>=breakY))
                    x=x;

                if(map1[i]>0.0f)
                {
                    glm::vec2 &airDirection=m_influenceMap[i].airDirection;
                    float magnitude=glm::length(airDirection);
                    float moisture=moistureDeltaMap[i];// map1[i];
                    float moistureCaptured=moisture*m_influenceMap[i].moistureCapacity*magnitude;

                    if(m_influenceMap[i].heightBase>0.5f)
                        moistureDeltaMap[i]=moistureDeltaMap[i]-moistureCaptured;

                    fillPoints(glm::vec2(x, y), airDirection, moistureDeltaMap, influenceSize, moistureCaptured);
//                    const std::array<glm::ivec2, 3> &floodPoints=getFloodPoints(airDirection);
                }
                ++i;
            }
        }

        //remove moisture that was moved
//        for(size_t i=0; i<influenceMapSize; i++)
//        {
//            if(i==breakIndex)
//                i=i;
//
//            map1[i]=map1[i]+moistureDeltaMap[i];
//            moistureDeltaMap[i]=0.0f;
//        }

//        std::swap(mapPointer1, mapPointer2);
    }

    for(size_t i=0; i<influenceMapSize; i++)
    {
//        std::vector<float> &map=*mapPointer1;
        
        float moisture=std::max(std::min(map1[i]+moistureDeltaMap[i], 1.0f), 0.0f);

        //Air sinking into a high is warming as it goes and so holds onto its water, which is why
        //the subtropical highs sit over the deserts; air rising through a low drops it. This is
        //what makes the pressure cells show up on the ground.
        moisture=moisture*(1.0f-(m_descriptorValues.m_pressureDryness*m_influenceMap[i].pressure));

        //a colder planet carries less water in the first place, so everything dries out with it
        m_influenceMap[i].moisture=std::min(applyGlobalMoisture(moisture, m_descriptorValues.m_globalTemperature), 1.0f);
    }

    progress.update("Generating ocean currents", 80, false);

    //Surface currents. The wind drags the top of the ocean along with it, Coriolis swings that drag
    //aside, and the coastlines bend what is left into closed loops - the gyres. Then the water
    //carries its temperature with it, which is the whole point: a current running up from the
    //tropics warms the coast it washes, one coming back down from the poles chills and dries it.
    {
        const int offsets[8][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
        float deflection=rads(m_descriptorValues.m_current.m_deflection);
        float spin=(m_descriptorValues.m_rotationDirection<0.0f)?-1.0f:1.0f;

        std::vector<float> seaTemperature(influenceMapSize, 0.0f);
        std::vector<float> warmth(influenceMapSize, 0.0f);

        point={0, 0};
        for(size_t i=0; i<influenceMapSize; i++)
        {
            float latitude=(float)M_PI_2-((float)M_PI*(((float)point.y+0.5f)/(float)influenceSize.y));

            point.x++;
            if(point.x>=influenceSize.x)
            {
                point.x=0;
                point.y++;
            }

            //open water only. Sea surface temperature is the plain latitude figure - no lapse
            //rate and no continentality out here.
            seaTemperature[i]=latitudeTemperature(latitude, m_descriptorValues.m_globalTemperature,
                m_descriptorValues.m_polarAmplification);
            warmth[i]=seaTemperature[i];

            if(m_influenceMap[i].heightBase>=0.5f)
            {
                m_influenceMap[i].current=glm::vec2(0.0f, 0.0f);
                continue;
            }

            m_influenceMap[i].current=windDrivenCurrent(m_influenceMap[i].airDirection, latitude, deflection, spin);
        }

        //Water cannot flow into a headland, it turns and runs along it. Pushing the into-shore
        //component out repeatedly is what closes the wind belts into basin-sized loops instead of
        //leaving them as straight streams running aground.
        for(int pass=0; pass<m_descriptorValues.m_current.m_coastRelaxation; ++pass)
        {
            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(m_influenceMap[i].heightBase>=0.5f)
                    continue;

                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);
                glm::vec2 shore(0.0f, 0.0f);

                for(size_t n=0; n<8; ++n)
                {
                    int ny=y+offsets[n][1];

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    if(m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)].heightBase<0.5f)
                        continue;

                    //image +y runs down while world +y runs up
                    shore+=glm::vec2((float)offsets[n][0], -(float)offsets[n][1]);
                }

                float length=glm::length(shore);

                if(length<=0.0f)
                    continue; //open water, nothing in the way

                shore=shore/length;

                float into=glm::dot(m_influenceMap[i].current, shore);

                if(into>0.0f) //heading at the land, take that part out and let it run along instead
                    m_influenceMap[i].current=m_influenceMap[i].current-(shore*into);
            }

            //Water is viscous: a stream turned by a headland drags the water beside it round too.
            //Without this the steering stays pinned to the cells actually touching the shore and
            //the flow never closes into a loop around a basin.
            std::vector<glm::vec2> smoothed(influenceMapSize);

            for(size_t i=0; i<influenceMapSize; i++)
            {
                smoothed[i]=m_influenceMap[i].current;

                if(m_influenceMap[i].heightBase>=0.5f)
                    continue;

                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);
                glm::vec2 total=m_influenceMap[i].current*2.0f;
                float weight=2.0f;

                for(size_t n=0; n<8; ++n)
                {
                    int ny=y+offsets[n][1];

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    size_t neighborIndex=((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                    if(m_influenceMap[neighborIndex].heightBase>=0.5f)
                        continue;

                    total+=m_influenceMap[neighborIndex].current;
                    weight+=1.0f;
                }

                smoothed[i]=total/weight;
            }

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(m_influenceMap[i].heightBase<0.5f)
                    m_influenceMap[i].current=smoothed[i];
            }
        }

        //Upwelling. Where the wind blows off the land it drags the surface water out to sea with
        //it, and colder water rises from below to take its place. Those coasts end up cold, arid
        //and extremely productive - the desert shore with the great fishery off it.
        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];

            cell.upwelling=false;

            if(cell.heightBase>=0.5f)
                continue;

            int x=(int)(i%influenceSize.x);
            int y=(int)(i/influenceSize.x);
            glm::vec2 seaward(0.0f, 0.0f);
            bool touchesLand=false;

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                if(m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)].heightBase<0.5f)
                    continue;

                touchesLand=true;
                //away from that land, in world axes where +y is up and the image runs down
                seaward-=glm::vec2((float)offsets[n][0], -(float)offsets[n][1]);
            }

            if(!touchesLand)
                continue;

            float length=glm::length(seaward);

            if(length<=0.0f)
                continue;

            seaward=seaward/length;

            //wind carrying the surface water out to sea rather than pressing it against the shore
            if(glm::dot(cell.airDirection, seaward)>=m_descriptorValues.m_current.m_upwellingWind)
                cell.upwelling=true;
        }

        //Carry the heat. Each parcel keeps most of the temperature it arrived with and picks up a
        //little from the latitude it is passing through, so warmth is dragged a long way from
        //where it started - which is what makes a current warm or cold relative to its
        //surroundings rather than just matching them.
        std::vector<float> next(influenceMapSize, 0.0f);

        for(int pass=0; pass<m_descriptorValues.m_current.m_advectionPasses; ++pass)
        {
            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(m_influenceMap[i].heightBase>=0.5f)
                {
                    next[i]=warmth[i];
                    continue;
                }

                glm::vec2 flow=m_influenceMap[i].current;
                float speed=glm::length(flow);

                if(speed<=1e-4f)
                {
                    next[i]=seaTemperature[i];
                    continue;
                }

                //where this water came from, one cell upstream
                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);
                int upX=x-(int)((flow.x/speed)+((flow.x<0.0f)?-0.5f:0.5f));
                int upY=y+(int)((flow.y/speed)+((flow.y<0.0f)?-0.5f:0.5f)); //+y down in the image

                upY=clamp(upY, 0, influenceSize.y-1);

                size_t upstream=((size_t)upY*influenceSize.x)+wrapIndex(upX, influenceSize.x);

                if(m_influenceMap[upstream].heightBase>=0.5f)
                {//upstream is land, so this water is not being fed from anywhere
                    next[i]=seaTemperature[i];
                    continue;
                }

                //Retention is a property of the water, not of how hard the wind happens to be
                //blowing on this particular cell. Scaling it by speed made the anomaly decay
                //within a few cells of its source and no current was ever warm or cold.
                float retention=m_descriptorValues.m_current.m_retention;

                next[i]=(warmth[upstream]*retention)+(seaTemperature[i]*(1.0f-retention));
            }

            warmth.swap(next);
        }

        for(size_t i=0; i<influenceMapSize; i++)
        {
            if(m_influenceMap[i].heightBase>=0.5f)
            {
                m_influenceMap[i].currentWarmth=0.0f;
                m_influenceMap[i].currentKind=CurrentKind::None;
                continue;
            }

            //The overturning limb, before the anomaly is measured. Deep circulation moves nine
            //tenths of the ocean's water and carries heat far further poleward than the wind
            //driven surface flow manages on its own - which matters here, because these worlds
            //have no enclosed basins for surface gyres to close around.
            {
                float latitude=(float)M_PI_2-((float)M_PI*((((float)(i/influenceSize.x))+0.5f)/(float)influenceSize.y));
                float delivered=overturningWarmth(m_influenceMap[i].current, latitude, m_descriptorValues.m_current);

                if(delivered>0.0f)
                {
                    warmth[i]+=delivered;
                    m_influenceMap[i].overturning=true;
                }
                else
                    m_influenceMap[i].overturning=false;
            }

            //Water risen from below is colder than the surface it replaced. Chilling it here
            //rather than chilling the reference is the point: the anomaly is measured against what
            //this latitude's surface SHOULD be, so lowering both cancels out and the upwelling
            //never reads as a cold current at all.
            if(m_influenceMap[i].upwelling)
                warmth[i]-=m_descriptorValues.m_current.m_upwellingChill;

            m_influenceMap[i].currentWarmth=warmth[i]-seaTemperature[i];
            m_influenceMap[i].currentKind=classifyCurrent(m_influenceMap[i].currentWarmth,
                glm::length(m_influenceMap[i].current), m_descriptorValues.m_current);
        }

        //What the current does to the shore it washes. This is the payoff: a cold current running
        //up a coast takes the temperature down and the rain with it, which is how a desert ends up
        //against an ocean, and a warm one leaves the opposite.
        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];

            cell.coastCurrent=CurrentKind::None;

            if(cell.heightBase<0.5f)
                continue;

            int x=(int)(i%influenceSize.x);
            int y=(int)(i/influenceSize.x);
            float anomaly=0.0f;
            int washed=0;
            glm::vec2 seaward(0.0f, 0.0f);

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                if(neighbor.heightBase>=0.5f)
                    continue;

                anomaly+=neighbor.currentWarmth;
                washed++;
                seaward+=glm::vec2((float)offsets[n][0], -(float)offsets[n][1]); //towards the water
            }

            if(washed<=0)
                continue;

            anomaly=anomaly/(float)washed;
            cell.coastCurrent=classifyCurrent(anomaly, 1.0f, m_descriptorValues.m_current);

            //A current only reaches the land through the air above it. Where the wind blows in off
            //that water it carries the temperature and the moisture inland; where it blows the
            //other way the sea alongside makes very little difference to the shore.
            float length=glm::length(seaward);
            float onshoreWind=0.0f;

            if(length>0.0f)
                onshoreWind=glm::dot(cell.airDirection, seaward/length);

            float carried=clamp((onshoreWind+1.0f)*0.5f, 0.15f, 1.0f);

            cell.temperature=cell.temperature+(anomaly*m_descriptorValues.m_current.m_coastTemperature*carried);

            //cold water holds its moisture rather than giving it to the air over it
            cell.moisture=clamp(cell.moisture+(anomaly*m_descriptorValues.m_current.m_coastMoisture*carried), 0.0f, 1.0f);
        }
    }

    progress.update("Generating climate", 85, false);

    //How far inland a cell sits, flood filled out from the coast. One field, three uses: it damps
    //the temperature near the sea and lets interiors run to extremes, it tells the classifier
    //which cells are continental interiors, and its gradient gives the direction of the coast so
    //the wind can be called onshore or offshore.
    {
        const float noDistance=std::numeric_limits<float>::max();
        std::vector<float> oceanDistance(influenceMapSize, noDistance);
        std::vector<size_t> frontier;

        for(size_t i=0; i<influenceMapSize; i++)
        {
            if(m_influenceMap[i].heightBase<0.5f)
            {
                oceanDistance[i]=0.0f;
                frontier.push_back(i);
            }
        }

        for(size_t head=0; head<frontier.size(); ++head)
        {
            size_t index=frontier[head];
            int x=(int)(index%influenceSize.x);
            int y=(int)(index/influenceSize.x);
            const int offsets[4][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

            for(size_t n=0; n<4; ++n)
            {
                int neighborY=y+offsets[n][1];

                if((neighborY<0)||(neighborY>=influenceSize.y))
                    continue;

                size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                if(oceanDistance[neighborIndex]!=noDistance)
                    continue;

                oceanDistance[neighborIndex]=oceanDistance[index]+1.0f;
                frontier.push_back(neighborIndex);
            }
        }

        //a world with no ocean at all leaves the field untouched, treat it as all interior
        float range=std::max(1.0f, m_descriptorValues.m_continentalityRange*(float)influenceSize.y);

        point={0, 0};
        for(size_t i=0; i<influenceMapSize; i++)
        {
            float distance=(oceanDistance[i]==noDistance)?range:oceanDistance[i];

            m_influenceMap[i].continentality=std::min(distance/range, 1.0f);

            //the coast lies down the gradient of the distance field, so that is the direction the
            //wind has to be blowing along to be coming in off the sea
            int x=point.x;
            int y=point.y;
            float left=(oceanDistance[((size_t)y*influenceSize.x)+wrapIndex(x-1, influenceSize.x)]==noDistance)?range:oceanDistance[((size_t)y*influenceSize.x)+wrapIndex(x-1, influenceSize.x)];
            float right=(oceanDistance[((size_t)y*influenceSize.x)+wrapIndex(x+1, influenceSize.x)]==noDistance)?range:oceanDistance[((size_t)y*influenceSize.x)+wrapIndex(x+1, influenceSize.x)];
            int downY=clamp(y-1, 0, influenceSize.y-1);
            int upY=clamp(y+1, 0, influenceSize.y-1);
            float down=(oceanDistance[((size_t)downY*influenceSize.x)+x]==noDistance)?range:oceanDistance[((size_t)downY*influenceSize.x)+x];
            float up=(oceanDistance[((size_t)upY*influenceSize.x)+x]==noDistance)?range:oceanDistance[((size_t)upY*influenceSize.x)+x];

            //image +y runs down while world +y runs up, so the north/south term is negated
            glm::vec2 inland(right-left, -(up-down));
            float length=glm::length(inland);

            if(length>0.0f)
                m_influenceMap[i].onshore=glm::dot(glm::normalize(m_influenceMap[i].airDirection+glm::vec2(1e-6f, 0.0f)), inland/length);
            else
                m_influenceMap[i].onshore=0.0f;

            //inland points away from the water, so a positive x means the sea lies to the west
            m_influenceMap[i].westernFlank=(inland.x>0.0f);

            //altitude and continentality on top of the latitude baseline
            float elevation=heightToElevation(m_influenceMap[i].heightBase, 0.5f, m_descriptorValues.m_maxElevation, m_descriptorValues.m_elevationCurve);
            float latitude=(float)M_PI_2-((float)M_PI*(((float)point.y+0.5f)/(float)influenceSize.y));

            m_influenceMap[i].temperature=m_influenceMap[i].temperature
                +elevationTemperature(elevation, m_descriptorValues.m_lapseRate)
                +continentalTemperature(m_influenceMap[i].continentality, latitude, m_descriptorValues.m_continentalityStrength);

            if(m_influenceMap[i].heightBase<0.5f)
            {//open water, unless it is cold enough to be under permanent ice
                m_influenceMap[i].climate=(m_influenceMap[i].temperature<m_descriptorValues.m_climate.m_iceCapTemperature)
                    ?ClimateZone::EF:ClimateZone::Ocean;
            }
            else
            {
                //-1 warm water washing this coast, +1 cold, 0 neither (inland, or a neutral sea)
                int coldCurrent=0;

                if(m_influenceMap[i].coastCurrent==CurrentKind::Cold)
                    coldCurrent=1;
                else if(m_influenceMap[i].coastCurrent==CurrentKind::Warm)
                    coldCurrent=-1;

                m_influenceMap[i].climate=classifyClimate(latitude, m_influenceMap[i].temperature,
                    m_influenceMap[i].moisture, m_influenceMap[i].continentality, m_influenceMap[i].onshore,
                    m_descriptorValues.m_climate, coldCurrent, m_influenceMap[i].westernFlank);
            }

            point.x++;
            if(point.x>=influenceSize.x)
            {
                point.x=0;
                point.y++;
            }
        }
    }

    progress.update("Generating rivers", 88, false);

    //Water flows downhill, so the whole river map falls out of the height field once it has been
    //made drainable. Three steps: flood the pits so nothing traps water in a one cell hole, send
    //every cell to its steepest downhill neighbour, then walk the cells from high to low adding
    //each one's catchment to whatever it drains into.
    {
        std::vector<float> filled(influenceMapSize);
        std::vector<bool> resolved(influenceMapSize, false);
        //the order the flood reached each cell, which increases with distance from the outlet and
        //so gives flat filled surfaces a direction to drain in
        std::vector<size_t> floodOrder(influenceMapSize, 0);
        std::vector<float> cellArea(influenceMapSize);
        std::vector<float> cellRain(influenceMapSize);

        //Priority flood, working inward from the sea. Popping the lowest cell still on the rim
        //means every cell is reached over the lowest ridge that leads to it, so raising it to that
        //level is exactly enough to let its water out and no more.
        typedef std::pair<float, size_t> FloodEntry;
        std::priority_queue<FloodEntry, std::vector<FloodEntry>, std::greater<FloodEntry>> flood;

        for(size_t i=0; i<influenceMapSize; i++)
        {
            filled[i]=m_influenceMap[i].heightBase;

            if(m_influenceMap[i].heightBase<0.5f)
            {
                resolved[i]=true;
                flood.push(FloodEntry(filled[i], i));
            }
        }

        const int offsets[8][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

        size_t floodStep=0;

        while(!flood.empty())
        {
            FloodEntry entry=flood.top();

            flood.pop();

            size_t index=entry.second;

            floodOrder[index]=floodStep++;

            int x=(int)(index%influenceSize.x);
            int y=(int)(index/influenceSize.x);

            for(size_t n=0; n<8; ++n)
            {
                int neighborY=y+offsets[n][1];

                if((neighborY<0)||(neighborY>=influenceSize.y))
                    continue;

                size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                if(resolved[neighborIndex])
                    continue;

                resolved[neighborIndex]=true;
                filled[neighborIndex]=std::max(m_influenceMap[neighborIndex].heightBase, entry.first);
                flood.push(FloodEntry(filled[neighborIndex], neighborIndex));
            }
        }

        //how much ground each cell covers and how much rain lands on it
        float blockSize=m_descriptorValues.m_metresPerBlock;
        float cellWidth=(float)m_descriptorValues.m_influenceGridSize.x*blockSize;
        float cellHeight=(float)m_descriptorValues.m_influenceGridSize.y*blockSize;

        point={0, 0};
        for(size_t i=0; i<influenceMapSize; i++)
        {
            float latitude=(float)M_PI_2-((float)M_PI*(((float)point.y+0.5f)/(float)influenceSize.y));

            //cells converge towards the poles, so they cover less ground than the grid suggests
            float areaMetres=cellWidth*cellHeight*std::max(cos(latitude), 0.01f);

            cellArea[i]=areaMetres/1.0e6f; //square kilometres

            float rainfallMetres=(m_influenceMap[i].moisture*m_descriptorValues.m_river.m_maximumRainfall)/1000.0f;

            cellRain[i]=(rainfallMetres*areaMetres)/1.0e6f; //millions of cubic metres a year

            m_influenceMap[i].drainageArea=cellArea[i];
            m_influenceMap[i].riverDischarge=cellRain[i];
            m_influenceMap[i].flowDirection=-1;

            point.x++;
            if(point.x>=influenceSize.x)
            {
                point.x=0;
                point.y++;
            }
        }

        //steepest descent on the filled surface
        std::vector<size_t> order(influenceMapSize);

        for(size_t i=0; i<influenceMapSize; i++)
        {
            order[i]=i;

            if(m_influenceMap[i].heightBase<0.5f)
                continue;

            int x=(int)(i%influenceSize.x);
            int y=(int)(i/influenceSize.x);
            float steepest=0.0f;
            int best=-1;
            size_t flattestOrder=floodOrder[i];
            int flattest=-1;

            for(size_t n=0; n<8; ++n)
            {
                int neighborY=y+offsets[n][1];

                if((neighborY<0)||(neighborY>=influenceSize.y))
                    continue;

                size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                //diagonals cover more ground for the same drop, so they need a steeper one
                float run=((offsets[n][0]!=0)&&(offsets[n][1]!=0))?1.41421356f:1.0f;
                float drop=(filled[i]-filled[neighborIndex])/run;

                //Water wants downhill and the direct route at once, not just downhill - on a
                //perfectly even slope it does not wander off in a spiral. Where two directions
                //drop the same, take the one the flood reached sooner, which is the one nearer
                //the outlet.
                if((drop>steepest)||((best>=0)&&(drop==steepest)&&(floodOrder[neighborIndex]<floodOrder[((size_t)clamp(y+offsets[best][1], 0, influenceSize.y-1)*influenceSize.x)+wrapIndex(x+offsets[best][0], influenceSize.x)])))
                {
                    steepest=drop;
                    best=(int)n;
                }

                //Filling a pit leaves a dead flat surface with no downhill neighbour anywhere on
                //it. The flood reached those cells from their outlet, so whichever neighbour it
                //reached first is the way out.
                if(floodOrder[neighborIndex]<flattestOrder)
                {
                    flattestOrder=floodOrder[neighborIndex];
                    flattest=(int)n;
                }
            }

            if(best<0)
                best=flattest;

            m_influenceMap[i].flowDirection=best;
        }

        //accumulate from the top down, so a cell always has its full catchment before it passes it on
        //Process downstream-last. Sorting on the filled height alone ties across a flat, so the
        //flood order breaks it - and that is exactly the direction the flats drain in.
        std::sort(order.begin(), order.end(), [&filled, &floodOrder](size_t a, size_t b)
        {
            if(filled[a]!=filled[b])
                return filled[a]>filled[b];
            return floodOrder[a]>floodOrder[b];
        });

        //Strahler order alongside the accumulation. A headwater is order 1; a confluence only
        //raises the order when two branches of equal order meet, which is what separates a main
        //stem from something joining it.
        std::vector<int> branchOrder(influenceMapSize, 0);
        std::vector<int> branchCount(influenceMapSize, 0);

        for(size_t k=0; k<order.size(); ++k)
        {
            size_t index=order[k];
            int direction=m_influenceMap[index].flowDirection;

            if(m_influenceMap[index].heightBase>=0.5f)
            {
                //every contributor upstream has already reported in, this cell's order is settled
                int own=(branchOrder[index]<=0)?1:(branchOrder[index]+((branchCount[index]>=2)?1:0));

                m_influenceMap[index].streamOrder=own;

                if(direction>=0)
                {
                    int oy=clamp((int)(index/influenceSize.x)+offsets[direction][1], 0, influenceSize.y-1);
                    size_t downstream=((size_t)oy*influenceSize.x)+wrapIndex((int)(index%influenceSize.x)+offsets[direction][0], influenceSize.x);

                    if(own>branchOrder[downstream])
                    {
                        branchOrder[downstream]=own;
                        branchCount[downstream]=1;
                    }
                    else if(own==branchOrder[downstream])
                        branchCount[downstream]++;
                }
            }

            if((direction<0)||(m_influenceMap[index].heightBase<0.5f))
                continue;

            int x=(int)(index%influenceSize.x);
            int y=(int)(index/influenceSize.x);
            int neighborY=y+offsets[direction][1];

            if((neighborY<0)||(neighborY>=influenceSize.y))
                continue;

            size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[direction][0], influenceSize.x);

            //The ground a river drains is the ground it drains, so area always accumulates. Water
            //does not: in an arid basin the river is losing, giving up more to the sand and the
            //air than it picks up, and it shrinks downstream instead of growing. That is the
            //difference between a permanent gaining river and an ephemeral wadi.
            float retention=1.0f;

            if(m_influenceMap[index].moisture<m_descriptorValues.m_river.m_permanentMoisture)
                retention=m_descriptorValues.m_river.m_ephemeralRetention;

            m_influenceMap[neighborIndex].drainageArea+=m_influenceMap[index].drainageArea;
            m_influenceMap[neighborIndex].riverDischarge+=(m_influenceMap[index].riverDischarge*retention);
        }

        //turn the catchments into rivers, lakes and the basins that never reach the sea
        point={0, 0};
        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];

            if(cell.heightBase<0.5f)
            {
                cell.water=WaterBody::None;
                cell.riverWidth=0.0f;
                point.x++;
                if(point.x>=influenceSize.x)
                {
                    point.x=0;
                    point.y++;
                }
                continue;
            }

            //a cell the flood had to raise is one that was sitting in a hollow, so it is under water
            bool ponded=(filled[i]>cell.heightBase+0.0005f);

            if(cell.drainageArea>=m_descriptorValues.m_river.m_minimumDrainage)
            {
                cell.riverWidth=riverWidth(cell.riverDischarge, cell.drainageArea/1000.0f);

                //Where the ground is too dry the river never gets out of its own basin - the water
                //arrives and evaporates. Wet enough and it stands as a salt lake, drier and it
                //spreads into swamp and soaks away, the way the Okavango does.
                if(ponded&&(cell.moisture<m_descriptorValues.m_river.m_endorheicMoisture))
                    cell.water=(cell.moisture>=m_descriptorValues.m_river.m_saltLakeMoisture)?WaterBody::SaltLake:WaterBody::InlandDelta;
                else if(ponded)
                    cell.water=WaterBody::Lake;
                else
                    cell.water=WaterBody::River;
            }
            else if(ponded)
            {
                cell.water=WaterBody::Lake;
                cell.riverWidth=0.0f;
            }
            else
            {
                cell.water=WaterBody::None;
                cell.riverWidth=0.0f;
            }

            //the gradient it is running down decides which of the three stages it is in
            int direction=cell.flowDirection;
            float gradient=0.0f;

            if(direction>=0)
            {
                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);
                int neighborY=clamp(y+offsets[direction][1], 0, influenceSize.y-1);
                size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[direction][0], influenceSize.x);
                float run=((offsets[direction][0]!=0)&&(offsets[direction][1]!=0))?1.41421356f:1.0f;

                //A river running into the sea has its surface at sea level, not at the bottom of
                //whatever trench lies offshore. Measuring the drop to the sea bed would call every
                //river mouth on the planet a steep mountain torrent.
                float downstream=std::max(filled[neighborIndex], 0.5f);

                gradient=std::max((filled[i]-downstream)/run, 0.0f);
            }

            cell.riverStage=(cell.water==WaterBody::River)?riverStageForGradient(gradient, m_descriptorValues.m_river):RiverStage::None;

            //temperate and tropical rivers run all year and gain downstream; arid ones are
            //ephemeral, losing what they carry to the ground and the air
            cell.permanentRiver=(cell.moisture>=m_descriptorValues.m_river.m_permanentMoisture);

            point.x++;
            if(point.x>=influenceSize.x)
            {
                point.x=0;
                point.y++;
            }
        }

        //Re-derive the stream order over the river network alone. Taken over every land cell the
        //order counts hillslopes as branches, so a river was already order two or three by the
        //time it had enough catchment to be called one. Counting only river cells makes a river
        //headwater order 1, which is what the ordering is for.
        {
            std::vector<int> riverOrder(influenceMapSize, 0);
            std::vector<int> riverCount(influenceMapSize, 0);

            for(size_t k=0; k<order.size(); ++k)
            {
                size_t index=order[k];

                if(m_influenceMap[index].water!=WaterBody::River)
                {
                    m_influenceMap[index].streamOrder=0;
                    continue;
                }

                int own=(riverOrder[index]<=0)?1:(riverOrder[index]+((riverCount[index]>=2)?1:0));

                m_influenceMap[index].streamOrder=own;

                int direction=m_influenceMap[index].flowDirection;

                if(direction<0)
                    continue;

                int oy=(int)(index/influenceSize.x)+offsets[direction][1];

                if((oy<0)||(oy>=influenceSize.y))
                    continue;

                size_t downstream=((size_t)oy*influenceSize.x)+wrapIndex((int)(index%influenceSize.x)+offsets[direction][0], influenceSize.x);

                if(own>riverOrder[downstream])
                {
                    riverOrder[downstream]=own;
                    riverCount[downstream]=1;
                }
                else if(own==riverOrder[downstream])
                    riverCount[downstream]++;
            }
        }

        //Drainage basins: everything that drains out through one point. Walk each land cell down
        //its flow path to whatever it ends at and label the whole path with that terminus. Basins
        //ending at the sea are exoreic, the rest endoreic - the reference wants no more than about
        //a fifth of them to be the latter.
        {
            const size_t noBasin=std::numeric_limits<size_t>::max();
            std::vector<size_t> basinOf(influenceMapSize, noBasin);
            std::vector<size_t> path;

            m_basins.clear();

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if((m_influenceMap[i].heightBase<0.5f)||(basinOf[i]!=noBasin))
                    continue;

                path.clear();

                size_t current=i;
                size_t found=noBasin;
                bool reachedSea=false;

                while(true)
                {
                    if(m_influenceMap[current].heightBase<0.5f)
                    {//walked off the land, so this whole path drains to the sea
                        reachedSea=true;
                        break;
                    }

                    //Pit filling gives every cell a path downhill to the sea, so nothing is
                    //endoreic on the topography alone. What actually stops the water is the air:
                    //a basin whose river ends in a terminal salt lake or soaks away into swamp
                    //never gets any of it to the coast.
                    if((m_influenceMap[current].water==WaterBody::SaltLake)
                        ||(m_influenceMap[current].water==WaterBody::InlandDelta))
                    {
                        if(!path.empty())
                        {
                            path.push_back(current);
                            basinOf[current]=noBasin-1;
                        }
                        break;
                    }

                    if(basinOf[current]!=noBasin)
                    {//run into ground already labelled, the rest of the path joins it
                        found=basinOf[current];
                        break;
                    }

                    path.push_back(current);
                    basinOf[current]=noBasin-1; //mark as on the current path, guards against a cycle

                    int direction=m_influenceMap[current].flowDirection;

                    if(direction<0)
                        break; //nowhere left to go, the water stays here

                    int cy=(int)(current/influenceSize.x)+offsets[direction][1];

                    if((cy<0)||(cy>=influenceSize.y))
                        break;

                    size_t next=((size_t)cy*influenceSize.x)+wrapIndex((int)(current%influenceSize.x)+offsets[direction][0], influenceSize.x);

                    if(basinOf[next]==noBasin-1) //looped back onto this same path
                        break;

                    current=next;
                }

                if(found==noBasin)
                {//a terminus of its own, so this is a new basin
                    DrainageBasin basin;

                    basin.outlet=path.empty()?i:path.back(); //the last cell still on land
                    basin.area=0.0f;
                    basin.discharge=0.0f;
                    basin.riverCells=0;
                    basin.endoreic=!reachedSea;

                    found=m_basins.size();
                    m_basins.push_back(basin);
                }

                for(size_t k=0; k<path.size(); ++k)
                    basinOf[path[k]]=found;
            }

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if((m_influenceMap[i].heightBase<0.5f)||(basinOf[i]>=m_basins.size()))
                {
                    m_influenceMap[i].basin=0;
                    continue;
                }

                m_influenceMap[i].basin=basinOf[i];

                DrainageBasin &basin=m_basins[basinOf[i]];

                basin.area+=cellArea[i];

                if(m_influenceMap[i].water==WaterBody::River)
                    basin.riverCells++;
            }

            for(size_t b=0; b<m_basins.size(); ++b)
                m_basins[b].discharge=m_influenceMap[m_basins[b].outlet].riverDischarge;
        }

        //Glacial lakes. An ice sheet grinds hollows into the ground and, when it goes, its
        //meltwater fills them. So the question is where the ice stood at the last glacial maximum,
        //which is the same cold world the temperature model already knows how to build - run it
        //that much colder and see what freezes.
        {
            point={0, 0};
            for(size_t i=0; i<influenceMapSize; i++)
            {
                InfluenceCell &cell=m_influenceMap[i];
                float latitude=(float)M_PI_2-((float)M_PI*(((float)point.y+0.5f)/(float)influenceSize.y));
                float latitudeDegrees=(float)fabs(latitude)*180.0f/(float)M_PI;

                point.x++;
                if(point.x>=influenceSize.x)
                {
                    point.x=0;
                    point.y++;
                }

                if((cell.heightBase<0.5f)||(cell.water!=WaterBody::None))
                    continue;

                //Ice never reached this far equatorward. If it had, the planet would have tipped
                //into a runaway freeze rather than a glacial maximum.
                if(latitudeDegrees<m_descriptorValues.m_river.m_glacialLatitude)
                    continue;

                if(cell.moisture<m_descriptorValues.m_river.m_glacialMoisture)
                    continue;

                //what this ground's temperature was when the planet ran at its glacial minimum
                float glacialTemperature=latitudeTemperature(latitude,
                    m_descriptorValues.m_globalTemperature+m_descriptorValues.m_river.m_glacialDrop,
                    m_descriptorValues.m_polarAmplification)
                    +elevationTemperature(heightToElevation(cell.heightBase, 0.5f, m_descriptorValues.m_maxElevation, m_descriptorValues.m_elevationCurve), m_descriptorValues.m_lapseRate)
                    +continentalTemperature(cell.continentality, latitude, m_descriptorValues.m_continentalityStrength);

                if(glacialTemperature>=m_descriptorValues.m_climate.m_iceCapTemperature)
                    continue; //no ice stood here even then

                //The reference looks for these at or near the headwaters of a river already on the
                //map, so the hollow has to sit against one - and against its upper reaches, not
                //down the main stem where the valley has been cut through rather than dammed.
                int gx=(int)(i%influenceSize.x);
                int gy=(int)(i/influenceSize.x);
                bool byHeadwater=false;

                for(size_t n=0; n<8; ++n)
                {
                    int ny=gy+offsets[n][1];

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(gx+offsets[n][0], influenceSize.x)];

                    if((neighbor.water==WaterBody::River)&&(neighbor.streamOrder>0)
                        &&(neighbor.streamOrder<=m_descriptorValues.m_river.m_glacialOrder))
                        byHeadwater=true;
                }

                if(!byHeadwater)
                    continue;

                cell.water=WaterBody::GlacialLake;
            }
        }

        //Deltas. A river hitting standing water loses its speed all at once and drops what it was
        //carrying, so the sediment piles up at the mouth and the coast builds outward over it.
        //Slow rivers deposit the most, which is why this only runs on a river already in its old
        //age stage. The land it makes is why a coastline is almost never straight at a delta.
        std::vector<size_t> deltaFront;

        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &mouth=m_influenceMap[i];

            //A torrent still plunging out of the mountains carries its load straight past the
            //shore; anything slower drops it. Size follows from how much water arrives and how
            //slowly, since the slower it runs the more it lets go of.
            if((mouth.heightBase<0.5f)||(mouth.water!=WaterBody::River)
                ||(mouth.riverStage==RiverStage::Youthful)||(mouth.riverStage==RiverStage::None)
                ||(mouth.riverDischarge<m_descriptorValues.m_river.m_deltaMinDischarge))
                continue;

            int x=(int)(i%influenceSize.x);
            int y=(int)(i/influenceSize.x);
            bool meetsWater=false;

            for(size_t n=0; n<8; ++n)
            {
                int neighborY=y+offsets[n][1];

                if((neighborY<0)||(neighborY>=influenceSize.y))
                    continue;

                if(m_influenceMap[((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)].heightBase<0.5f)
                    meetsWater=true;
            }

            if(!meetsWater)
                continue;

            //The contest that decides the shape: what the river is delivering against what the
            //sea can carry away. Both are to hand now - discharge from the drainage solve, and
            //the surface current from the circulation pass.
            glm::vec2 seaCurrent(0.0f, 0.0f);
            glm::vec2 seaward(0.0f, 0.0f);
            int water=0;

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                if(neighbor.heightBase>=0.5f)
                    continue;

                seaCurrent+=neighbor.current;
                seaward+=glm::vec2((float)offsets[n][0], -(float)offsets[n][1]);
                water++;
            }

            if(water>0)
            {
                seaCurrent=seaCurrent/(float)water;
                seaward=seaward/(float)water;
            }

            float riverPower=std::min(mouth.riverDischarge/m_descriptorValues.m_river.m_deltaMinDischarge, 6.0f);
            float currentPower=glm::length(seaCurrent)*3.0f;
            float seawardLength=glm::length(seaward);

            DeltaShape shape=deltaShapeFor(riverPower, currentPower);

            if(shape==DeltaShape::None)
                continue; //the sea carries it off as fast as the river brings it

            float slowness=(mouth.riverStage==RiverStage::OldAge)?1.0f:0.45f;
            int budget=(int)((mouth.riverDischarge/m_descriptorValues.m_river.m_deltaDischargePerCell)*slowness);

            budget=clamp(budget, 1, m_descriptorValues.m_river.m_deltaMaxCells);

            //grow the fan outward from the mouth, cell by cell, the way the sediment actually
            //accumulates - each new bar extends the ground the next one builds on
            deltaFront.clear();
            deltaFront.push_back(i);

            int placed=0;

            for(size_t head=0; (head<deltaFront.size())&&(placed<budget); ++head)
            {
                int fx=(int)(deltaFront[head]%influenceSize.x);
                int fy=(int)(deltaFront[head]/influenceSize.x);

                for(size_t n=0; (n<8)&&(placed<budget); ++n)
                {
                    int neighborY=fy+offsets[n][1];

                    if((neighborY<0)||(neighborY>=influenceSize.y))
                        continue;

                    size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(fx+offsets[n][0], influenceSize.x);
                    InfluenceCell &deposit=m_influenceMap[neighborIndex];

                    if(deposit.heightBase>=0.5f) //already land, nothing to build on it
                        continue;

                    //sediment only settles in the shallows, it cannot fill an abyss
                    if(deposit.heightBase<m_descriptorValues.m_continentalShelf)
                        continue;

                    //Shape decides how the fan spreads. A bird's foot pushes long fingers
                    //straight out because nothing is pushing back; a cuspate delta gets driven
                    //back against the shore; a fan spreads evenly between the two.
                    if(shape!=DeltaShape::Fan)
                    {
                        glm::vec2 step((float)offsets[n][0], -(float)offsets[n][1]);
                        float stepLength=glm::length(step);
                        float along=(stepLength>0.0f&&seawardLength>0.0f)
                            ?glm::dot(step/stepLength, seaward/seawardLength):0.0f;

                        //bird's foot only grows seaward, cuspate refuses to grow seaward
                        if((shape==DeltaShape::BirdsFoot)&&(along<0.35f))
                            continue;
                        if((shape==DeltaShape::Cuspate)&&(along>0.55f))
                            continue;
                    }

                    deposit.heightBase=0.505f;
                    deposit.water=WaterBody::Delta;
                    deposit.deltaShape=shape;
                    deposit.riverStage=RiverStage::OldAge;

                    //Two fans can reach the same ground. Keep the bigger river's figures rather
                    //than whichever arrived last, so water never appears to shrink flowing in.
                    deposit.riverDischarge=std::max(deposit.riverDischarge, mouth.riverDischarge);
                    deposit.drainageArea=std::max(deposit.drainageArea, mouth.drainageArea);
                    deposit.riverWidth=std::max(deposit.riverWidth, mouth.riverWidth);
                    deposit.permanentRiver=(deposit.permanentRiver||mouth.permanentRiver);

                    //new ground carries the climate of the land that built it, not the sea's
                    deposit.climate=mouth.climate;
                    deposit.temperature=mouth.temperature;
                    deposit.moisture=mouth.moisture;
                    deposit.continentality=0.0f;

                    //New ground still has to shed its water. Point it at open sea if it touches
                    //any, otherwise back at the cell that built it, which is nearer the sea.
                    int deltaX=(int)(neighborIndex%influenceSize.x);
                    int deltaY=(int)(neighborIndex/influenceSize.x);

                    deposit.flowDirection=-1;
                    for(size_t e=0; e<8; ++e)
                    {
                        int exitY=deltaY+offsets[e][1];

                        if((exitY<0)||(exitY>=influenceSize.y))
                            continue;

                        if(m_influenceMap[((size_t)exitY*influenceSize.x)+wrapIndex(deltaX+offsets[e][0], influenceSize.x)].heightBase<0.5f)
                        {
                            deposit.flowDirection=(int)e;
                            break;
                        }
                    }

                    deltaFront.push_back(neighborIndex);
                    placed++;
                }
            }
        }

        //A fan is new ground laid over what was open water, so a river that used to end at the
        //coast may now run onto someone else's delta. Push each river's figures forward into
        //whatever it drains into so the water never appears to shrink crossing onto the fan.
        //Fans are small, so a couple of sweeps settles it.
        for(size_t pass=0; pass<3; ++pass)
        {
            bool changed=false;

            for(size_t i=0; i<influenceMapSize; i++)
            {
                const InfluenceCell &cell=m_influenceMap[i];

                if((cell.heightBase<0.5f)||(cell.flowDirection<0)||(cell.riverDischarge<=0.0f))
                    continue;

                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);
                int neighborY=y+offsets[cell.flowDirection][1];

                if((neighborY<0)||(neighborY>=influenceSize.y))
                    continue;

                size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[cell.flowDirection][0], influenceSize.x);
                InfluenceCell &downstream=m_influenceMap[neighborIndex];

                if(downstream.water!=WaterBody::Delta)
                    continue;

                if(downstream.riverDischarge<cell.riverDischarge)
                {
                    downstream.riverDischarge=cell.riverDischarge;
                    downstream.drainageArea=std::max(downstream.drainageArea, cell.drainageArea);
                    downstream.riverWidth=std::max(downstream.riverWidth, cell.riverWidth);
                    downstream.permanentRiver=(downstream.permanentRiver||cell.permanentRiver);
                    changed=true;
                }
            }

            if(!changed)
                break;
        }
    }

    progress.update("Generating coasts", 92, false);

    //The shoreline itself. Everything needed is already on the map by now - whether the margin is
    //active, how hard the ground climbs away from the water, whether the climate ices over, and
    //how warm the shallows are.
    {
        point={0, 0};
        for(size_t i=0; i<influenceMapSize; i++)
        {
            int x=point.x;
            int y=point.y;
            const int offsets[8][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

            bool land=(m_influenceMap[i].heightBase>=0.5f);
            bool touchesOther=false;
            float lowest=m_influenceMap[i].heightBase;
            float highest=m_influenceMap[i].heightBase;
            BoundaryType boundary=m_influenceMap[i].boundaryType;
            bool glaciated=false;

            for(size_t n=0; n<8; ++n)
            {
                int neighborY=y+offsets[n][1];

                if((neighborY<0)||(neighborY>=influenceSize.y))
                    continue;

                size_t neighborIndex=((size_t)neighborY*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);
                const InfluenceCell &neighbor=m_influenceMap[neighborIndex];

                if((neighbor.heightBase>=0.5f)!=land)
                {
                    touchesOther=true;
                    continue; //the shore step is not relief, every coast has one
                }

                lowest=std::min(lowest, neighbor.heightBase);
                highest=std::max(highest, neighbor.heightBase);

                //the boundary a coastal cell answers to may be a step inland rather than under it
                if(activeMargin(neighbor.boundaryType))
                    boundary=neighbor.boundaryType;

                //ice that carved the valleys need not still be sitting in them
                if(glaciatedClimate(neighbor.climate))
                    glaciated=true;
            }

            if(!touchesOther)
            {
                m_influenceMap[i].coast=(int)CoastType::None;
                point.x++;
                if(point.x>=influenceSize.x)
                {
                    point.x=0;
                    point.y++;
                }
                continue;
            }

            float relief=highest-lowest;

            if(glaciatedClimate(m_influenceMap[i].climate))
                glaciated=true;

            //Fjords are a mid to high latitude thing. The lapse rate puts icecap climates on
            //tropical peaks too, but no ice sheet ever ground a valley into an equatorial coast.
            float coastLatitude=(float)fabs((float)M_PI_2-((float)M_PI*(((float)point.y+0.5f)/(float)influenceSize.y)))*180.0f/(float)M_PI;

            if(coastLatitude<m_descriptorValues.m_coast.m_fjordLatitude)
                glaciated=false;

            if(land)
            {
                //A river reaching the sea drowns its own valley mouth. That outranks whatever the
                //shore would otherwise have been, which is why estuaries turn up on rocky and
                //sandy coasts alike.
                if(m_influenceMap[i].water==WaterBody::Delta)
                    m_influenceMap[i].coast=(int)CoastType::Delta;
                else if((m_influenceMap[i].water==WaterBody::River)&&(m_influenceMap[i].riverWidth>0.0f)
                    &&(relief<m_descriptorValues.m_coast.m_fjordRelief))
                    m_influenceMap[i].coast=(int)CoastType::Estuarine;
                else
                    m_influenceMap[i].coast=(int)classifyCoast(m_influenceMap[i], boundary, relief, glaciated, m_descriptorValues.m_coast);
            }
            else
            {
                //the water side. Coral wants warm clear shallows, so it rides the shelf off a warm
                //shore and nowhere else.
                if((m_influenceMap[i].temperature>=m_descriptorValues.m_coast.m_reefTemperature)
                    &&(m_influenceMap[i].heightBase>=m_descriptorValues.m_continentalShelf))
                    m_influenceMap[i].coast=(int)CoastType::CoralReef;
                else if(!activeMargin(boundary)&&(relief<m_descriptorValues.m_coast.m_rockyRelief)
                    &&(m_influenceMap[i].heightBase>=m_descriptorValues.m_continentalShelf))
                    m_influenceMap[i].coast=(int)CoastType::BarrierIsland; //bars build on passive margins
                else
                    m_influenceMap[i].coast=(int)CoastType::None;
            }

            point.x++;
            if(point.x>=influenceSize.x)
            {
                point.x=0;
                point.y++;
            }
        }
    }

    progress.update("Generating rocks", 96, false);

    //What is actually underfoot. Everything needed is on the map by now: which belt built the
    //mountains, where the plumes came up, what the ice scoured, and which ground was under a sea
    //when the sea stood higher. Igneous is laid down first because it overprints whatever it
    //erupted through, then metamorphic where the heat and pressure went, then the sedimentary
    //cover over everything left - which is most of the planet.
    {
        const int offsets[8][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

        point={0, 0};
        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];

            int x=point.x;
            int y=point.y;

            point.x++;
            if(point.x>=influenceSize.x)
            {
                point.x=0;
                point.y++;
            }

            float elevation=heightToElevation(cell.heightBase, 0.5f, m_descriptorValues.m_maxElevation, m_descriptorValues.m_elevationCurve);

            //ground that was drowned when the sea last stood a couple of hundred metres higher
            cell.shallowSea=(cell.heightBase>=0.5f)&&(elevation<=m_descriptorValues.m_rock.m_shallowSeaElevation);

            if(cell.heightBase<0.5f)
            {//the reference skips the ocean floor, and so does this
                cell.rock=RockType::Sediment;
                cell.craton=CratonPart::None;
                continue;
            }

            //local relief, which stands in for how much mountain is left after erosion
            float lowest=cell.heightBase;
            float highest=cell.heightBase;
            bool glaciated=glaciatedClimate(cell.climate);
            bool nearHotspot=(cell.hotspot>m_descriptorValues.m_rock.m_hotspotBasalt);
            bool nearSubduction=(cell.boundaryType==BoundaryType::Convergent);
            bool nearRift=(cell.boundaryType==BoundaryType::Divergent);

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                lowest=std::min(lowest, neighbor.heightBase);
                highest=std::max(highest, neighbor.heightBase);

                if(neighbor.hotspot>m_descriptorValues.m_rock.m_hotspotBasalt)
                    nearHotspot=true;
                if(neighbor.boundaryType==BoundaryType::Convergent)
                    nearSubduction=true;
                if(neighbor.boundaryType==BoundaryType::Divergent)
                    nearRift=true;
                if(glaciatedClimate(neighbor.climate))
                    glaciated=true;
            }

            float relief=highest-lowest;

            //An old block of crust that has sat out the tectonics: well inland, off any boundary,
            //and without the relief that says something has been happening to it lately.
            bool craton=(cell.continentality>=m_descriptorValues.m_rock.m_cratonContinentality)
                &&(cell.boundaryType==BoundaryType::None)
                &&(relief<m_descriptorValues.m_rock.m_cratonRelief);

            cell.craton=cratonPart(craton, glaciated, cell.shallowSea, cell.moisture);

            //--- igneous, where something actually erupted or intruded ---
            //Kept to the ground that is genuinely volcanic. Igneous and metamorphic together only
            //hold about a quarter of a planet's land; spreading them over everything merely
            //adjacent to a plume or a boundary buries the sedimentary cover that should dominate.
            if((cell.hotspot>m_descriptorValues.m_rock.m_hotspotBasalt)
                ||((cell.boundaryType==BoundaryType::Divergent)
                    &&(cell.plateDistanceValue>=m_descriptorValues.m_rock.m_riftDistance)))
            {//plume and rift country floods basalt over whatever was there
                cell.rock=RockType::Basalt;
                continue;
            }

            if((cell.orogeny==OrogenyType::Laramide)&&(cell.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain))
            {//a ridge going down the trench, which the reference says to spam andesite over
                cell.rock=RockType::Andesite;
                continue;
            }

            if(cell.orogeny==OrogenyType::Andean)
            {//more granite here than in a laramide belt, though far from all of it
                if(cell.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain)
                    cell.rock=RockType::Granite;
                else if(cell.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain*0.6f)
                    cell.rock=RockType::Andesite;
            }

            //island arcs run andesitic, same as the subduction belts on land
            if(nearSubduction&&(cell.orogeny==OrogenyType::None)&&(cell.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain))
            {
                cell.rock=RockType::Andesite;
                continue;
            }

            //--- metamorphic, where the collisions cooked it ---
            if(((cell.orogeny==OrogenyType::Himalayan)||(cell.orogeny==OrogenyType::Ural))
                &&(cell.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain))
            {
                //Mostly metamorphic, the core of the range where it was squeezed hardest - but
                //thickened crust melts at depth and that melt rises and freezes as granite bodies
                //within the belt. Those intrusions are where the tin is.
                cell.rock=(oreChance(i, m_descriptorValues.seed, 90)<m_descriptorValues.m_rock.m_beltIntrusion)
                    ?RockType::Granite:RockType::Metamorphic;
                continue;
            }

            if(cell.craton==CratonPart::Shield)
            {//basement at the surface: the old igneous and metamorphic floor of the continent
                cell.rock=(cell.moisture>0.5f)?RockType::Metamorphic:RockType::Granite;
                continue;
            }

            if(cell.rock==RockType::Granite||cell.rock==RockType::Andesite)
                continue; //an andean belt already claimed this one

            //--- sedimentary, which is three quarters of the planet's land ---
            //Sediment fills whatever is low and collecting: basin floors, flood plains, the aprons
            //shed off the mountains. Carbonate is warm shallow marine, so it is confined to ground
            //that was under a sea AND warm enough to build one. Sandstone takes the rest.
            bool collecting=(cell.water==WaterBody::River)||(cell.water==WaterBody::Lake)
                ||(cell.water==WaterBody::GlacialLake)||(cell.water==WaterBody::Delta)
                ||(relief<m_descriptorValues.m_rock.m_erodedRelief);

            if(collecting)
                cell.rock=RockType::Sediment;
            else if(cell.shallowSea&&(cell.temperature>=18.0f)
                &&(relief<m_descriptorValues.m_rock.m_mountainRelief*0.7f))
                cell.rock=RockType::Carbonate; //limestone wants a warm shallow sea
            else
                cell.rock=RockType::Sandstone;
        }

        //A large igneous province bakes what it touches, so a rind of metamorphic rock forms round
        //the basalt. Done as its own pass, once all the basalt is placed.
        std::vector<size_t> aureole;

        for(size_t i=0; i<influenceMapSize; i++)
        {
            if((m_influenceMap[i].heightBase<0.5f)||(m_influenceMap[i].rock!=RockType::Sandstone))
                continue;

            int x=(int)(i%influenceSize.x);
            int y=(int)(i/influenceSize.x);

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                if(m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)].rock==RockType::Basalt)
                {
                    aureole.push_back(i);
                    break;
                }
            }
        }

        for(size_t k=0; k<aureole.size(); ++k)
            m_influenceMap[aureole[k]].rock=RockType::Metamorphic;
    }

    progress.update("Generating ore deposits", 98, false);

    //Where the diggable things are. Every rule here keys on ground the map already knows about:
    //which belt raised the mountains, what the rock is, whether the crust is ancient and quiet,
    //what the climate does to the soil, and where the water went.
    {
        const int offsets[8][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
        int seed=m_descriptorValues.seed;

        //A mountain belt is heavy enough to press the crust down either side of it, and the trough
        //that makes fills with sediment. Those basins carry oil, gas and the far end of an MVT.
        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];

            cell.forelandBasin=false;

            if((cell.heightBase<0.5f)||!sedimentaryRock(cell.rock))
                continue;

            int x=(int)(i%influenceSize.x);
            int y=(int)(i/influenceSize.x);

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                if((neighbor.orogeny!=OrogenyType::None)&&(neighbor.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain))
                    cell.forelandBasin=true;
            }
        }

        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];

            cell.deposit=DepositType::None;

            if(cell.heightBase<0.5f)
                continue;

            int x=(int)(i%influenceSize.x);
            int y=(int)(i/influenceSize.x);
            bool volcanicArc=false;
            bool nearArc=false;
            bool nearRift=(cell.boundaryType==BoundaryType::Divergent);
            bool nearBasalt=(cell.rock==RockType::Basalt);
            float relief=0.0f;

            //an arc is the overriding side of a subduction zone, or a hotspot
            if(((cell.orogeny==OrogenyType::Andean)||(cell.orogeny==OrogenyType::Laramide))
                &&(cell.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain))
                volcanicArc=true;
            if(cell.hotspot>m_descriptorValues.m_rock.m_hotspotBasalt)
                volcanicArc=true;

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                relief=std::max(relief, (float)fabs(neighbor.heightBase-cell.heightBase));

                if(((neighbor.orogeny==OrogenyType::Andean)||(neighbor.orogeny==OrogenyType::Laramide))
                    &&(neighbor.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain))
                    nearArc=true;
                if(neighbor.hotspot>m_descriptorValues.m_rock.m_hotspotBasalt)
                    nearArc=true;
                if(neighbor.boundaryType==BoundaryType::Divergent)
                    nearRift=true;
                if(neighbor.rock==RockType::Basalt)
                    nearBasalt=true;
            }

            bool mountain=(cell.orogeny!=OrogenyType::None)&&(cell.terrainScale>=m_descriptorValues.m_rock.m_beltTerrain*0.5f);

            //--- the volcanic arc family, zoned outward from the vent ---
            if(volcanicArc&&(oreChance(i, seed, 1)<m_descriptorValues.m_ore.m_arcRarity))
            {//porphyry sits right in the arc itself
                cell.deposit=DepositType::PorphyryCopper;
                cell.depositExposed=true; //an active arc is being uplifted and stripped at once
                continue;
            }

            if(nearArc&&(oreChance(i, seed, 2)<m_descriptorValues.m_ore.m_arcRarity))
            {//gold rings the porphyry, a step further out
                cell.deposit=DepositType::EpithermalGold;
                cell.depositExposed=true;
                continue;
            }

            if((nearArc||nearRift)&&(oreChance(i, seed, 3)<m_descriptorValues.m_ore.m_iocgRarity))
            {//IOCG surrounds those, and takes the continental rifts as well
                cell.deposit=DepositType::IOCG;
                continue;
            }

            //--- ancient stable crust ---
            if((cell.craton!=CratonPart::None)&&(cell.boundaryType==BoundaryType::None)
                &&(oreChance(i, seed, 4)<m_descriptorValues.m_ore.m_cratonRarity))
            {//nickel, platinum, chrome and the diamonds, all rare
                cell.deposit=DepositType::NickelPGE;
                cell.depositExposed=(cell.craton==CratonPart::Shield); //only where the basement is bare
                continue;
            }

            cell.tinBelt=tinBelt(cell.orogeny);

            //Skarn: a pluton pushing up into limestone bakes the contact into ore. So it wants
            //igneous rock with carbonate country right beside it, which is a thing that only
            //happens along an orogenic belt.
            //Limestone is the classic host, but carbonate is a thin slice of any land and inside a
            //mountain belt it is thinner still, so keying on it alone left half the worlds with no
            //skarn at all. Marine sediment that was laid down in the same shallow seas is
            //calcareous enough to bake the same way, so it counts too.
            bool nearCarbonate=false;
            bool igneous=(cell.rock==RockType::Granite)||(cell.rock==RockType::Andesite)||(cell.rock==RockType::Basalt);

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                if((neighbor.rock==RockType::Carbonate)
                    ||(sedimentaryRock(neighbor.rock)&&neighbor.shallowSea))
                    nearCarbonate=true;
            }

            if(igneous&&nearCarbonate&&(cell.orogeny!=OrogenyType::None)
                &&(oreChance(i, seed, 16)<m_descriptorValues.m_ore.m_skarnRarity))
            {
                cell.deposit=DepositType::Skarn;
                //an active belt is still being lifted, an old one has mostly been worn off it
                cell.depositExposed=(cell.terrainScale>=m_descriptorValues.m_ore.m_upliftExposure);
                continue;
            }

            //--- tin, and the ground where it meets copper ---
            if(cell.tinBelt&&(cell.rock==RockType::Granite))
            {
                //Copper already in the same rock as the tin, so the two can be alloyed straight
                //out of the ground without trading for either. This is the rare case and has to be
                //tested as one - checking it first with the looser threshold, as I had it, let it
                //swallow the ordinary tin lodes and there was almost no plain tin anywhere.
                if(oreChance(i, seed, 17)<m_descriptorValues.m_ore.m_tinBronzeRarity)
                {
                    cell.deposit=DepositType::TinBronze;
                    cell.depositExposed=true;
                    continue;
                }

                if(oreChance(i, seed, 18)<m_descriptorValues.m_ore.m_tinRarity)
                {
                    cell.deposit=DepositType::TinLode;
                    cell.depositExposed=(cell.terrainScale>=m_descriptorValues.m_ore.m_upliftExposure);
                    continue;
                }
            }

            //--- iron, in the forms that actually made an iron age possible ---
            //Iron laterite: the same leaching that makes bauxite, but over basalt and under a
            //climate that alternates wet and dry rather than staying wet all year.
            if(nearBasalt&&((cell.climate==ClimateZone::Aw)||(cell.climate==ClimateZone::Am))
                &&(oreChance(i, seed, 19)<m_descriptorValues.m_ore.m_ironLateriteRarity))
            {
                cell.deposit=DepositType::IronLaterite;
                cell.depositExposed=true; //the soil itself
                continue;
            }

            //Oolitic iron: dissolved iron settling grain by grain into a shallow marine sediment.
            //The reference expects a lot of a world's iron to be this.
            if(cell.shallowSea&&((cell.rock==RockType::Carbonate)||sedimentaryRock(cell.rock))
                &&!mountain&&(oreChance(i, seed, 20)<m_descriptorValues.m_ore.m_ooliticRarity))
            {
                cell.deposit=DepositType::OoliticIron;
                cell.depositExposed=true;
                continue;
            }

            //Meteoric iron. Metallic the moment it lands and needs no smelting at all, but
            //vanishingly rare - a continent might have one. Findable where nothing grows over it,
            //which is why the Cape York iron sat on the Greenland tundra for people to work.
            if(((cell.climate==ClimateZone::ET)||(cell.climate==ClimateZone::BWh)||(cell.climate==ClimateZone::BWk))
                &&(oreChance(i, seed, 21)<m_descriptorValues.m_ore.m_meteoricRarity))
            {
                cell.deposit=DepositType::MeteoricIron;
                cell.depositExposed=true;
                continue;
            }

            //--- old sea floor caught up in a mountain belt ---
            if(mountain)
            {
                if(oreChance(i, seed, 5)<m_descriptorValues.m_ore.m_mountainRarity)
                {//black smoker sulphides thrust up to where they can be dug
                    cell.deposit=DepositType::VMS;
                    cell.depositExposed=true; //thrust up into the range, which is the whole reason it is mineable
                    continue;
                }

                //banded iron went down on an ancient sea floor and shows up in the old ranges,
                //so it wants a worn belt rather than a raw one
                if((cell.terrainScale<m_descriptorValues.m_rock.m_beltTerrain)
                    &&(oreChance(i, seed, 6)<m_descriptorValues.m_ore.m_mountainRarity))
                {
                    cell.deposit=DepositType::BIF;
                    continue;
                }

                //no coal in the interior of a range - that pressure makes graphite, not coal
            }

            //--- fossil fuels ---
            //Oil starts as plankton on a warm drowned shelf, then needs burying and squeezing. A
            //foreland basin or a passive margin basin does both.
            if(sedimentaryRock(cell.rock)&&cell.shallowSea
                &&(cell.forelandBasin||(cell.coast!=(int)CoastType::None)||(cell.temperature>=m_descriptorValues.m_ore.m_coalTemperature))
                &&(oreChance(i, seed, 7)<m_descriptorValues.m_ore.m_fuelRarity))
            {
                cell.deposit=DepositType::OilGas;
                continue;
            }

            //Coal was a tropical swamp: low, wet, warm. The good coal is the stuff that later got
            //buried, so it turns up in the foothills of a belt rather than out on the flat.
            if(sedimentaryRock(cell.rock)&&(cell.temperature>=m_descriptorValues.m_ore.m_coalTemperature)
                &&(cell.moisture>=m_descriptorValues.m_ore.m_coalMoisture)&&!mountain
                &&(oreChance(i, seed, 8)<m_descriptorValues.m_ore.m_fuelRarity))
            {
                //still a living wetland rather than a buried one, so it is peat and not coal yet
                cell.deposit=((cell.water!=WaterBody::None)&&!cell.forelandBasin)?DepositType::Peat:DepositType::Coal;

                //Peat is being cut off the surface. Coal has to have been lifted or eroded back
                //into reach - the reference puts the good seams in the foothills of a belt, which
                //is exactly where later uplift exposed them.
                cell.depositExposed=(cell.deposit==DepositType::Peat)||cell.forelandBasin
                    ||(cell.orogeny!=OrogenyType::None);
                continue;
            }

            //--- evaporites ---
            //A salt flat is a lake that dried up and left its minerals behind, so it wants an arid
            //climate and a hollow for the water to have stood in. The reference names three sorts
            //of hollow: any local depression, the undulating ground behind a laramide belt (the
            //Bonneville and Bolivian flats both sit in country like that), and a foreland basin.
            bool arid=(cell.moisture<m_descriptorValues.m_ore.m_saltMoisture);
            bool hollow=(cell.water==WaterBody::SaltLake)||(cell.water==WaterBody::InlandDelta)
                ||cell.forelandBasin||(cell.orogeny==OrogenyType::Laramide)||(relief<=0.01f);

            if(arid&&hollow&&sedimentaryRock(cell.rock)
                &&(oreChance(i, seed, 9)<m_descriptorValues.m_ore.m_fuelRarity))
            {
                cell.deposit=DepositType::SaltFlat;
                cell.depositExposed=true; //it is lying on the ground
                continue;
            }

            //Rock salt is an ancient salt flat: warm shallow sea, dried out, then buried. What
            //makes it worth anything to a pre-industrial people is whether a later collision
            //pushed it back up - buried beds stay lost, uplifted ones become the salt mines and
            //the salt glaciers of the Zagros.
            if(arid&&cell.shallowSea&&sedimentaryRock(cell.rock)
                &&(oreChance(i, seed, 14)<m_descriptorValues.m_ore.m_fuelRarity))
            {
                cell.deposit=DepositType::Halite;
                cell.depositExposed=(cell.terrainScale>=m_descriptorValues.m_ore.m_upliftExposure)
                    ||(cell.orogeny!=OrogenyType::None);
                continue;
            }

            //--- basins ---
            if(cell.forelandBasin&&(oreChance(i, seed, 10)<m_descriptorValues.m_ore.m_basinRarity))
            {//carried out from under the range and dropped on the far side
                cell.deposit=DepositType::MVT;
                continue;
            }

            if(sedimentaryRock(cell.rock)&&(cell.continentality>0.15f)
                &&(oreChance(i, seed, 11)<m_descriptorValues.m_ore.m_basinRarity))
            {
                cell.deposit=DepositType::SEDEX;
                continue;
            }

            //Native copper. Water carries copper out of a basalt province and drops it in the
            //carbonate country around it. This is the deposit that actually starts a copper age:
            //it is metallic already, so it needs no smelting, and it lies near the surface where
            //people can find it. Mostly poor, occasionally worth something.
            //Carbonate is the reference's example, but it is a thin slice of any world's land, and
            //a hotspot counts here too - "what is a hotspot if not a very small large igneous
            //province". Any sedimentary country downhill of erupted basalt will do.
            if(sedimentaryRock(cell.rock)&&(nearBasalt||(cell.hotspot>m_descriptorValues.m_rock.m_hotspotBasalt))
                &&(oreChance(i, seed, 15)<m_descriptorValues.m_ore.m_nativeCopperRarity))
            {
                cell.deposit=DepositType::NativeCopper;
                cell.depositExposed=true; //shallow, and cold workable without a furnace
                continue;
            }

            //Sediment-hosted copper: copper accumulated in sandstone and shale beds in a basin,
            //with no volcano anywhere near it. Michigan's native copper comes out of shale like
            //this. It is the one copper that does not need an arc.
            if((cell.rock==RockType::Sandstone)&&(cell.orogeny==OrogenyType::None)
                &&(cell.continentality>0.15f)
                &&(oreChance(i, seed, 23)<m_descriptorValues.m_ore.m_sedimentaryCopperRarity))
            {
                cell.deposit=DepositType::SedimentaryCopper;
                cell.depositExposed=cell.shallowSea; //bedded, so only where erosion has cut into it
                continue;
            }

            //--- surface processes ---
            //Leach a rainforest soil hard enough for long enough and the soil itself is the ore.
            if(((cell.climate==ClimateZone::Af)||(cell.climate==ClimateZone::Am))
                &&(oreChance(i, seed, 12)<m_descriptorValues.m_ore.m_lateriteRarity))
            {
                cell.deposit=DepositType::Laterite;
                cell.depositExposed=true; //the soil itself is the ore
                continue;
            }
        }

        //Hydrothermal iron. Rather than a type of its own the reference upgrades a few of the VMS
        //and SEDEX deposits already on the map to carry iron as well, and those run rich.
        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];

            cell.ironBearing=false;

            if(((cell.deposit==DepositType::VMS)||(cell.deposit==DepositType::SEDEX))
                &&(oreChance(i, seed, 22)<0.25f))
                cell.ironBearing=true;
        }

        //Placers and secondary enrichment both work by water moving something that was already
        //there. Walk downstream from each hard-rock deposit and drop concentrations along the way -
        //the gold in the gravels, and the uranium leached out and put down again lowerdown.
        std::vector<size_t> sources;

        for(size_t i=0; i<influenceMapSize; i++)
        {
            DepositType deposit=m_influenceMap[i].deposit;

            if((deposit==DepositType::PorphyryCopper)||(deposit==DepositType::EpithermalGold)
                ||(deposit==DepositType::IOCG)||(deposit==DepositType::VMS)
                ||(deposit==DepositType::NickelPGE)||(deposit==DepositType::TinLode))
                sources.push_back(i);
        }

        for(size_t k=0; k<sources.size(); ++k)
        {
            size_t current=sources[k];
            DepositType source=m_influenceMap[current].deposit;

            for(int step=0; step<6; ++step)
            {
                int direction=m_influenceMap[current].flowDirection;

                if(direction<0)
                    break;

                int ny=(int)(current/influenceSize.x)+offsets[direction][1];

                if((ny<0)||(ny>=influenceSize.y))
                    break;

                size_t next=((size_t)ny*influenceSize.x)+wrapIndex((int)(current%influenceSize.x)+offsets[direction][0], influenceSize.x);

                if(m_influenceMap[next].heightBase<0.5f)
                    break; //reached the sea, nothing further to concentrate

                current=next;

                if(m_influenceMap[current].deposit!=DepositType::None)
                    continue; //already carries something of its own

                if(m_influenceMap[current].water==WaterBody::River)
                {
                    if(oreChance(current, seed, 13+step)<m_descriptorValues.m_ore.m_placerRarity)
                    {
                        //Cassiterite is heavy and survives the trip, so a tin lode upstream leaves
                        //alluvial tin in the gravels - which is how most bronze age tin was won,
                        //panned rather than mined.
                        m_influenceMap[current].deposit=(source==DepositType::TinLode)
                            ?DepositType::TinPlacer:DepositType::Placer;
                        m_influenceMap[current].depositExposed=true; //lying in the gravel of a riverbed
                    }
                }
                else if((source==DepositType::IOCG)
                    &&(oreChance(current, seed, 30+step)<m_descriptorValues.m_ore.m_secondaryRarity))
                {//the reference wants the main uranium to be a secondary, reconcentrated one
                    m_influenceMap[current].deposit=DepositType::SecondaryUranium;
                }
                else if(((m_influenceMap[current].water==WaterBody::Lake)
                    ||(m_influenceMap[current].water==WaterBody::GlacialLake)
                    ||(m_influenceMap[current].fuel==FuelSource::Peat)
                    ||(m_influenceMap[current].coast==(int)CoastType::SaltMarsh))
                    &&(oreChance(current, seed, 40+step)<m_descriptorValues.m_ore.m_bogIronRarity))
                {
                    //Bog iron: groundwater carries iron out of an ore-bearing belt and it
                    //oxidises out again in the still water of a marsh downhill of it. Shallow,
                    //easy to dig and easy to smelt, which is a large part of why iron spread.
                    m_influenceMap[current].deposit=DepositType::BogIron;
                    m_influenceMap[current].depositExposed=true;
                }
            }
        }
    }

    progress.update("Generating fuel", 99, false);

    //What can be burned. A people sitting on copper with nothing to burn cannot smelt it, so this
    //matters as much as the ore does. Wood wherever trees will grow, peat in the cool wet ground
    //the ice sheets left behind, coal where the map already put it.
    {
        point={0, 0};
        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];
            float latitude=(float)M_PI_2-((float)M_PI*(((float)point.y+0.5f)/(float)influenceSize.y));

            point.x++;
            if(point.x>=influenceSize.x)
            {
                point.x=0;
                point.y++;
            }

            cell.forest=false;
            cell.fuel=FuelSource::None;

            if(cell.heightBase<0.5f)
                continue;

            float elevation=heightToElevation(cell.heightBase, 0.5f, m_descriptorValues.m_maxElevation,
                m_descriptorValues.m_elevationCurve);

            cell.forest=treesGrow(cell.climate, latitude, elevation);

            //Peat wants a cool wet climate sitting on ground the last ice sheet worked over. That
            //is the same glacial reconstruction the lakes used - run the planet colder and see
            //what froze - so the bogs land where the ice actually was.
            bool glacialGround=false;

            if(peatClimate(cell.climate))
            {
                float glacialTemperature=latitudeTemperature(latitude,
                    m_descriptorValues.m_globalTemperature+m_descriptorValues.m_river.m_glacialDrop,
                    m_descriptorValues.m_polarAmplification)
                    +elevationTemperature(elevation, m_descriptorValues.m_lapseRate)
                    +continentalTemperature(cell.continentality, latitude, m_descriptorValues.m_continentalityStrength);

                glacialGround=(glacialTemperature<m_descriptorValues.m_climate.m_iceCapTemperature);
            }

            //Coal and peat are the notable fuels - somewhere worth going for. Wood is the
            //background everywhere trees stand, so it is checked last: otherwise the forest
            //covers the peat bogs and the coal seams and neither ever shows on the map.
            if((cell.deposit==DepositType::Coal)&&cell.depositExposed)
                cell.fuel=FuelSource::Coal;
            else if(glacialGround||(cell.deposit==DepositType::Peat))
                cell.fuel=FuelSource::Peat;
            else if(cell.forest)
                cell.fuel=FuelSource::Wood;
        }
    }

    progress.update("Generating habitability", 99, false);

    //Who could live here, and who used to. Everything this needs is already on the map: water,
    //soil, grazing, timber, stone, fuel and ore are the site factors settlement geography asks
    //for, and each of them is a field the earlier passes produced.
    {
        const int offsets[8][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
        const std::vector<SpeciesHabitat> &species=m_descriptorValues.m_species;

        std::vector<float> relief(influenceMapSize, 0.0f);
        std::vector<float> elevation(influenceMapSize, 0.0f);

        for(size_t i=0; i<influenceMapSize; i++)
        {
            elevation[i]=heightToElevation(m_influenceMap[i].heightBase, 0.5f,
                m_descriptorValues.m_maxElevation, m_descriptorValues.m_elevationCurve);

            if(m_influenceMap[i].heightBase<0.5f)
                continue;

            int x=(int)(i%influenceSize.x);
            int y=(int)(i/influenceSize.x);
            float lowest=m_influenceMap[i].heightBase;
            float highest=m_influenceMap[i].heightBase;

            for(size_t n=0; n<8; ++n)
            {
                int ny=y+offsets[n][1];

                if((ny<0)||(ny>=influenceSize.y))
                    continue;

                const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                lowest=std::min(lowest, neighbor.heightBase);
                highest=std::max(highest, neighbor.heightBase);
            }
            relief[i]=highest-lowest;
        }

        //Raw scores are not comparable between profiles. A people who care about one thing very
        //much - elves and forest - carry a high baseline anywhere that thing is present, and would
        //simply outscore a narrower people everywhere. So each profile is measured against the
        //best ground THAT profile can find on this world, and the comparison is which people this
        //cell suits best relative to what is available to them.
        std::vector<float> speciesBest(species.size(), 0.0f);

        for(size_t k=0; k<species.size(); ++k)
        {
            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(m_influenceMap[i].heightBase<0.5f)
                    continue;

                speciesBest[k]=std::max(speciesBest[k], habitability(m_influenceMap[i], species[k], elevation[i], relief[i]));
            }
        }

        for(size_t i=0; i<influenceMapSize; i++)
        {
            InfluenceCell &cell=m_influenceMap[i];

            cell.habitability=0.0f;
            cell.dominantSpecies=-1;

            if(cell.heightBase<0.5f)
                continue;

            //whoever this ground suits best. A cell is not owned by anyone here - this only says
            //which people would find it most liveable, and callers can ask about any of them
            //anywhere by calling habitability() directly.
            float bestFit=0.0f;

            for(size_t k=0; k<species.size(); ++k)
            {
                float score=habitability(cell, species[k], elevation[i], relief[i]);
                float fit=(speciesBest[k]>0.0f)?(score/speciesBest[k]):0.0f;

                if(fit>bestFit)
                {
                    bestFit=fit;
                    cell.habitability=score;   //the raw score, which is the meaningful figure
                    cell.dominantSpecies=(int)k;
                }
            }

            if(cell.habitability<m_descriptorValues.m_settleThreshold)
                cell.dominantSpecies=-1; //nobody would bother
        }

        //--- what divides one people from the next ---
        //Two passes, because the reference is careful that the two sorts of divide do not mean the
        //same thing. The hard divides - sea, mountain wall, desert - bound the physiographic
        //regions, and nothing much crosses them. Those are then subdivided wherever the climate
        //changes family, giving cultural regions: trade crosses those freely, but a people rarely
        //expands into ground that will not grow what it knows how to grow.
        {
            const size_t noRegion=std::numeric_limits<size_t>::max();
            std::vector<size_t> physio(influenceMapSize, noRegion);
            std::vector<size_t> cultural(influenceMapSize, noRegion);
            std::vector<size_t> fill;

            m_regions.clear();

            for(size_t i=0; i<influenceMapSize; i++)
            {
                m_influenceMap[i].barrier=(int)geographicBarrier(m_influenceMap[i], elevation[i],
                    relief[i], m_descriptorValues.m_barriers);
                m_influenceMap[i].physiographicRegion=-1;
                m_influenceMap[i].culturalRegion=-1;
            }

            //hard-bounded clusters: everything crossable, stopped by any barrier
            size_t physioCount=0;

            for(size_t seed=0; seed<influenceMapSize; seed++)
            {
                if((physio[seed]!=noRegion)||(m_influenceMap[seed].barrier!=(int)BarrierType::None))
                    continue;

                fill.clear();
                fill.push_back(seed);
                physio[seed]=physioCount;

                for(size_t head=0; head<fill.size(); ++head)
                {
                    size_t index=fill[head];
                    int x=(int)(index%influenceSize.x);
                    int y=(int)(index/influenceSize.x);

                    for(size_t n=0; n<8; ++n)
                    {
                        int ny=y+offsets[n][1];

                        if((ny<0)||(ny>=influenceSize.y))
                            continue;

                        size_t neighborIndex=((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                        if((physio[neighborIndex]!=noRegion)
                            ||(m_influenceMap[neighborIndex].barrier!=(int)BarrierType::None))
                            continue;

                        physio[neighborIndex]=physioCount;
                        fill.push_back(neighborIndex);
                    }
                }
                physioCount++;
            }

            //and now the softer divide inside each: same cluster AND same climate family
            for(size_t seed=0; seed<influenceMapSize; seed++)
            {
                if((cultural[seed]!=noRegion)||(physio[seed]==noRegion))
                    continue;

                ClimateGroup group=climateGroup(m_influenceMap[seed].climate);
                size_t id=m_regions.size();

                CultureRegion region;

                region.m_cells=0;
                region.m_area=0.0f;
                region.m_dominantSpecies=-1;
                region.m_climate=group;
                region.m_physiographic=physio[seed];
                region.m_meanHabitability=0.0f;
                region.m_riverValley=false;

                fill.clear();
                fill.push_back(seed);
                cultural[seed]=id;

                std::vector<int> speciesTally(m_descriptorValues.m_species.size(), 0);
                int riverCells=0;

                for(size_t head=0; head<fill.size(); ++head)
                {
                    size_t index=fill[head];

                    region.m_cells++;

                    //cells cover less ground towards the poles, so weight by latitude
                    {
                        float cellLatitude=(float)M_PI_2-((float)M_PI*((((float)(index/influenceSize.x))+0.5f)/(float)influenceSize.y));
                        float side=(float)m_descriptorValues.m_influenceGridSize.x*m_descriptorValues.m_metresPerBlock;

                        region.m_area+=(side*side*std::max(cos(cellLatitude), 0.01f))/1.0e6f;
                    }
                    region.m_meanHabitability+=m_influenceMap[index].habitability;

                    if(m_influenceMap[index].dominantSpecies>=0)
                        speciesTally[m_influenceMap[index].dominantSpecies]++;
                    if(m_influenceMap[index].water==WaterBody::River)
                        riverCells++;

                    int x=(int)(index%influenceSize.x);
                    int y=(int)(index/influenceSize.x);

                    for(size_t n=0; n<8; ++n)
                    {
                        int ny=y+offsets[n][1];

                        if((ny<0)||(ny>=influenceSize.y))
                            continue;

                        size_t neighborIndex=((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                        if((cultural[neighborIndex]!=noRegion)||(physio[neighborIndex]!=physio[index]))
                            continue;

                        if(climateGroup(m_influenceMap[neighborIndex].climate)!=group)
                            continue;

                        cultural[neighborIndex]=id;
                        fill.push_back(neighborIndex);
                    }
                }

                region.m_meanHabitability=region.m_meanHabitability/(float)region.m_cells;

                int best=0;

                for(size_t k=0; k<speciesTally.size(); ++k)
                {
                    if(speciesTally[k]>best)
                    {
                        best=speciesTally[k];
                        region.m_dominantSpecies=(int)k;
                    }
                }

                //An arid region carried by a river is a different proposition from the desert
                //around it - it will farm where its neighbours cannot. Egypt and Mesopotamia are
                //the cases the reference names.
                region.m_riverValley=((group==ClimateGroup::Arid)&&(riverCells*4>=(int)region.m_cells));

                m_regions.push_back(region);
            }

            //Do not cut tiny slivers off as regions of their own - the reference says so outright.
            //They stay part of the map but carry no region of their own.
            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(cultural[i]==noRegion)
                    continue;

                if((int)m_regions[cultural[i]].m_cells<m_descriptorValues.m_barriers.m_minimumRegion)
                    continue;

                m_influenceMap[i].physiographicRegion=(int)physio[i];
                m_influenceMap[i].culturalRegion=(int)cultural[i];
            }
        }

        //--- the people who were here before ---
        //Each fallen civilization lived in a world that was not this one. Running the climate
        //model at their era's temperature and scoring the ground as THEY would have found it puts
        //their ruins where the land suited them then - which is why a dead city can sit in what is
        //now desert, or under forest that was steppe when they built it.
        m_fallen.clear();

        for(size_t i=0; i<influenceMapSize; i++)
        {
            m_influenceMap[i].ruinCondition=0.0f;
            m_influenceMap[i].ruinSpecies=-1;
            m_influenceMap[i].ruinCivilization=-1;
            m_influenceMap[i].ruinKind=0;
        }

        if(!species.empty()&&(m_descriptorValues.m_ruinCivilizations>0))
        {
            std::uniform_int_distribution<size_t> speciesPick(0, species.size()-1);
            std::vector<float> eraScore(influenceMapSize, 0.0f);

            for(int c=0; c<m_descriptorValues.m_ruinCivilizations; ++c)
            {
                FallenCivilization fallen;

                fallen.m_species=speciesPick(generator);
                //the older it is, the further its world sat from this one
                fallen.m_age=((float)c+1.0f)/(float)m_descriptorValues.m_ruinCivilizations;
                fallen.m_globalTemperature=m_descriptorValues.m_globalTemperature
                    +((distribution(generator))*m_descriptorValues.m_ruinClimateSwing*fallen.m_age);
                fallen.m_name=species[fallen.m_species].m_name+" ruins";
                fallen.m_sites=0;
                fallen.m_fate=(int)CivilizationFate::Unknown;
                fallen.m_seat=influenceMapSize;
                fallen.m_temperatureSwing=fallen.m_globalTemperature-m_descriptorValues.m_globalTemperature;
                fallen.m_moistureSwing=0.0f;
                fallen.m_workedOut=0.0f;
                fallen.m_drownedSites=0;
                fallen.m_barrierSplit=0;
                fallen.m_nearestRival=-1;
                fallen.m_rivalDistance=-1.0f;
                fallen.m_refuge=-1;

                const SpeciesHabitat &habitat=species[fallen.m_species];

                //score the whole map as it would have been in their era
                point={0, 0};
                for(size_t i=0; i<influenceMapSize; i++)
                {
                    eraScore[i]=0.0f;

                    float latitude=(float)M_PI_2-((float)M_PI*(((float)point.y+0.5f)/(float)influenceSize.y));

                    point.x++;
                    if(point.x>=influenceSize.x)
                    {
                        point.x=0;
                        point.y++;
                    }

                    if(m_influenceMap[i].heightBase<0.5f)
                        continue;

                    //rebuild that era's climate for this cell, the same way the climate pass did
                    InfluenceCell era=m_influenceMap[i];

                    era.temperature=latitudeTemperature(latitude, fallen.m_globalTemperature,
                        m_descriptorValues.m_polarAmplification)
                        +elevationTemperature(elevation[i], m_descriptorValues.m_lapseRate)
                        +continentalTemperature(era.continentality, latitude, m_descriptorValues.m_continentalityStrength);
                    era.moisture=clamp(applyGlobalMoisture(era.moisture,
                        fallen.m_globalTemperature-m_descriptorValues.m_globalTemperature), 0.0f, 1.0f);

                    eraScore[i]=habitability(era, habitat, elevation[i], relief[i]);
                }

                //take the best sites, kept apart so a civilization is spread over a territory
                //rather than piled onto one hill
                for(int siteIndex=0; siteIndex<m_descriptorValues.m_ruinSitesPerCivilization; ++siteIndex)
                {
                    size_t best=influenceMapSize;
                    float bestScore=m_descriptorValues.m_settleThreshold;

                    for(size_t i=0; i<influenceMapSize; i++)
                    {
                        if(eraScore[i]>bestScore)
                        {
                            bestScore=eraScore[i];
                            best=i;
                        }
                    }

                    if(best>=influenceMapSize)
                        break; //nowhere left that would have held them

                    int bx=(int)(best%influenceSize.x);
                    int by=(int)(best/influenceSize.x);
                    int spacing=m_descriptorValues.m_ruinSpacing;

                    for(int dy=-spacing; dy<=spacing; ++dy)
                    {
                        int ny=by+dy;

                        if((ny<0)||(ny>=influenceSize.y))
                            continue;

                        for(int dx=-spacing; dx<=spacing; ++dx)
                            eraScore[((size_t)ny*influenceSize.x)+wrapIndex(bx+dx, influenceSize.x)]=0.0f;
                    }

                    //what is left of it now depends on how long ago it fell and on what has been
                    //growing over it since
                    float survival=ruinSurvival(fallen.m_age, m_influenceMap[best]);

                    if(survival<=0.05f)
                        continue; //gone without trace

                    //a later civilization builds over an earlier one rather than beside it
                    if(survival<=m_influenceMap[best].ruinCondition)
                        continue;

                    m_influenceMap[best].ruinCondition=survival;
                    m_influenceMap[best].ruinSpecies=(int)fallen.m_species;
                    m_influenceMap[best].ruinCivilization=(int)m_fallen.size();

                    //what the thing actually was. The specialised table is tried first and only
                    //falls through to the default where the ground says nothing in particular.
                    {
                        bool mountainous=(elevation[best]>=m_descriptorValues.m_barriers.m_mountainElevation);
                        std::vector<WeightedEntry> table=ruinKindsForSite(m_influenceMap[best], relief[best], mountainous);

                        if(table.empty())
                            table=defaultRuinKinds(m_influenceMap[best]);

                        unsigned int roll=(unsigned int)((best*2654435761u)
                            ^((unsigned int)m_descriptorValues.seed*2246822519u)
                            ^((unsigned int)c*3266489917u));

                        m_influenceMap[best].ruinKind=rollTable(table, roll);
                    }

                    fallen.m_sites++;
                }

                m_fallen.push_back(fallen);
            }
        }

        //--- the states that grow out of all this ---
        //The reference's method, near enough word for word: "when creating an empire or state on a
        //map, I always start out with the core regions of power. This should be well in control of
        //the state, and far away from other empire's core regions of power. From this area, the
        //state's power should emanate." So no outline is drawn. Each state is a core and a budget,
        //the budget is spent walking outward, and wherever it runs out is the border. That the
        //borders then land on rivers, mountain fronts and desert edges is a consequence rather
        //than a rule, which is the whole point: "the border was as far as the Lord could project
        //his power".
        m_polities.clear();

        for(size_t i=0; i<influenceMapSize; i++)
        {
            m_influenceMap[i].polity=-1;
            m_influenceMap[i].control=0.0f;
            m_influenceMap[i].march=false;
        }

        if(!species.empty()&&(m_descriptorValues.m_polityRules.m_count>0))
        {
            const PolityThresholds &rules=m_descriptorValues.m_polityRules;

            //A core wants a heartland around it, not one good hill. Averaging the ground within a
            //few cells is what separates a river valley that can feed an empire from a lucky pass.
            std::vector<float> heartland(influenceMapSize, 0.0f);
            std::vector<int> variety(influenceMapSize, 0);
            const int heartlandRadius=3;
            const int varietyRadius=2;

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(m_influenceMap[i].heightBase<0.5f)
                    continue;

                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);
                float sum=0.0f;
                int count=0;
                bool seen[(size_t)DepositTypeCount]={false};
                int distinct=0;

                for(int dy=-heartlandRadius; dy<=heartlandRadius; ++dy)
                {
                    int ny=y+dy;

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    for(int dx=-heartlandRadius; dx<=heartlandRadius; ++dx)
                    {
                        size_t index=((size_t)ny*influenceSize.x)+wrapIndex(x+dx, influenceSize.x);

                        sum+=m_influenceMap[index].habitability;
                        count++;

                        if((abs(dx)<=varietyRadius)&&(abs(dy)<=varietyRadius))
                        {
                            const InfluenceCell &near=m_influenceMap[index];

                            if((near.deposit!=DepositType::None)&&(!seen[(size_t)near.deposit]))
                            {
                                seen[(size_t)near.deposit]=true;
                                distinct++;
                            }
                        }
                    }
                }

                heartland[i]=(count>0)?(sum/(float)count):0.0f;
                variety[i]=distinct;
            }

            //pick the seats of power, keeping them apart so no two empires share a heartland
            std::vector<float> seat(influenceMapSize, 0.0f);

            for(size_t i=0; i<influenceMapSize; i++)
                seat[i]=coreScore(m_influenceMap[i], m_influenceMap[i].habitability, heartland[i], variety[i]);

            std::vector<size_t> cores;

            for(int p=0; p<rules.m_count; ++p)
            {
                size_t best=influenceMapSize;
                float bestScore=m_descriptorValues.m_settleThreshold*0.5f;

                for(size_t i=0; i<influenceMapSize; i++)
                {
                    if(seat[i]>bestScore)
                    {
                        bestScore=seat[i];
                        best=i;
                    }
                }

                if(best>=influenceMapSize)
                    break; //no ground left that could carry a state

                cores.push_back(best);

                int bx=(int)(best%influenceSize.x);
                int by=(int)(best/influenceSize.x);

                for(int dy=-rules.m_coreSeparation; dy<=rules.m_coreSeparation; ++dy)
                {
                    int ny=by+dy;

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    for(int dx=-rules.m_coreSeparation; dx<=rules.m_coreSeparation; ++dx)
                        seat[((size_t)ny*influenceSize.x)+wrapIndex(bx+dx, influenceSize.x)]=0.0f;
                }
            }

            for(size_t p=0; p<cores.size(); ++p)
            {
                const InfluenceCell &core=m_influenceMap[cores[p]];

                Polity polity;

                polity.m_core=cores[p];
                polity.m_species=(core.dominantSpecies>=0)?core.dominantSpecies:0;
                polity.m_homeClimate=climateGroup(core.climate);
                polity.m_mountainCore=((elevation[cores[p]]>=rules.m_mountainElevation)
                    ||(relief[cores[p]]>=rules.m_mountainRelief));
                polity.m_cells=0;
                polity.m_area=0.0f;
                polity.m_marchCells=0;
                polity.m_minorityCells=0;
                polity.m_meanControl=0.0f;
                polity.m_enclave=false;
                polity.m_name=species[polity.m_species].m_name+" realm";

                m_polities.push_back(polity);
            }

            //Spend each state's budget outward from its core. Whoever reaches a cell for the least
            //power holds it, and the ground where every state's budget is already gone is the
            //no-man's-land between them.
            struct PowerFront
            {
                float m_spent;
                size_t m_index;
                int m_polity;

                bool operator>(const PowerFront &other) const { return m_spent>other.m_spent; }
            };

            std::vector<float> spent(influenceMapSize, std::numeric_limits<float>::max());
            std::vector<int> owner(influenceMapSize, -1);
            std::priority_queue<PowerFront, std::vector<PowerFront>, std::greater<PowerFront>> front;

            for(size_t p=0; p<m_polities.size(); ++p)
            {
                spent[m_polities[p].m_core]=0.0f;
                owner[m_polities[p].m_core]=(int)p;
                front.push(PowerFront{0.0f, m_polities[p].m_core, (int)p});
            }

            while(!front.empty())
            {
                PowerFront current=front.top();
                front.pop();

                if(current.m_spent>spent[current.m_index])
                    continue;

                const Polity &polity=m_polities[current.m_polity];
                const SpeciesHabitat &habitat=species[polity.m_species];

                int x=(int)(current.m_index%influenceSize.x);
                int y=(int)(current.m_index/influenceSize.x);

                for(size_t n=0; n<8; ++n)
                {
                    int ny=y+offsets[n][1];

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    size_t index=((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);
                    const InfluenceCell &cell=m_influenceMap[index];

                    float step=expansionCost(cell, elevation[index], relief[index],
                        habitability(cell, habitat, elevation[index], relief[index]),
                        polity.m_homeClimate, rules);

                    if((offsets[n][0]!=0)&&(offsets[n][1]!=0))
                        step*=1.41421f;

                    float total=current.m_spent+step;

                    if((total>rules.m_reach)||(total>=spent[index]))
                        continue;

                    spent[index]=total;
                    owner[index]=current.m_polity;
                    front.push(PowerFront{total, index, current.m_polity});
                }
            }

            //A state holds land. It can put an army across a strait but it does not rule the water,
            //so the sea is crossed in the search above and claimed by nobody here.
            for(size_t i=0; i<influenceMapSize; i++)
            {
                if((owner[i]<0)||(m_influenceMap[i].heightBase<0.5f))
                    continue;

                m_influenceMap[i].polity=owner[i];
                m_influenceMap[i].control=projectedControl(spent[i], rules.m_reach);
            }

            //--- the holdouts ---
            //"You can see in a map of Aztec holdings that they're... well, missing a few parts."
            //Tlaxcala sat surrounded by Aztec tributaries and was never taken. Defensible ground
            //that a state's power only barely reaches, and that the state has entirely enclosed,
            //is exactly that case, and leaving it out is what stops every border reading as a
            //clean blob.
            {
                std::vector<bool> visited(influenceMapSize, false);
                std::vector<size_t> blob;
                std::vector<std::pair<float, std::vector<size_t>>> holdouts;

                for(size_t i=0; i<influenceMapSize; i++)
                {
                    InfluenceCell &start=m_influenceMap[i];

                    if(visited[i]||(start.polity<0))
                        continue;

                    if((relief[i]<rules.m_enclaveRelief)||(start.control>=rules.m_enclaveControl))
                        continue;

                    if(start.habitability<m_descriptorValues.m_settleThreshold)
                        continue;

                    //gather the contiguous patch of ground that is defensible in the same way
                    blob.clear();
                    blob.push_back(i);
                    visited[i]=true;

                    int surrounding=start.polity;
                    bool enclosed=true;

                    for(size_t head=0; head<blob.size(); ++head)
                    {
                        int bx=(int)(blob[head]%influenceSize.x);
                        int by=(int)(blob[head]/influenceSize.x);

                        for(size_t n=0; n<8; ++n)
                        {
                            int ny=by+offsets[n][1];

                            if((ny<0)||(ny>=influenceSize.y))
                                continue;

                            size_t index=((size_t)ny*influenceSize.x)+wrapIndex(bx+offsets[n][0], influenceSize.x);
                            InfluenceCell &neighbor=m_influenceMap[index];

                            if(neighbor.heightBase<0.5f)
                                continue; //the sea is a wall, not a way out

                            if(neighbor.polity!=surrounding)
                            {
                                enclosed=false; //it touches open ground or a second power
                                continue;
                            }

                            if(visited[index]||(relief[index]<rules.m_enclaveRelief)
                                ||(neighbor.control>=rules.m_enclaveControl)
                                ||(neighbor.habitability<m_descriptorValues.m_settleThreshold))
                                continue;

                            visited[index]=true;
                            blob.push_back(index);
                        }
                    }

                    if((!enclosed)||((int)blob.size()>rules.m_enclaveMaxCells))
                        continue;

                    //rank by how defensible the patch is and how much of it there is, so the few
                    //that survive are the ones that had the best reason to
                    float defence=0.0f;

                    for(size_t b=0; b<blob.size(); ++b)
                        defence=std::max(defence, relief[blob[b]]);

                    defence*=(float)blob.size();

                    holdouts.push_back(std::make_pair(defence, blob));
                }

                std::sort(holdouts.begin(), holdouts.end(),
                    [](const std::pair<float, std::vector<size_t>> &a, const std::pair<float, std::vector<size_t>> &b)
                    { return a.first>b.first; });

                if((int)holdouts.size()>rules.m_enclaveLimit)
                    holdouts.resize(rules.m_enclaveLimit);

                for(size_t h=0; h<holdouts.size(); ++h)
                {
                    const std::vector<size_t> &patch=holdouts[h].second;
                    const InfluenceCell &start=m_influenceMap[patch[0]];

                    Polity holdout;

                    holdout.m_core=patch[0];
                    holdout.m_species=(start.dominantSpecies>=0)?start.dominantSpecies:m_polities[start.polity].m_species;
                    holdout.m_homeClimate=climateGroup(start.climate);
                    holdout.m_mountainCore=true;
                    holdout.m_cells=0;
                    holdout.m_area=0.0f;
                    holdout.m_marchCells=0;
                    holdout.m_minorityCells=0;
                    holdout.m_meanControl=0.0f;
                    holdout.m_enclave=true;
                    holdout.m_name=species[holdout.m_species].m_name+" holdout";

                    int id=(int)m_polities.size();

                    for(size_t b=0; b<patch.size(); ++b)
                    {
                        m_influenceMap[patch[b]].polity=id;
                        m_influenceMap[patch[b]].control=1.0f; //small, but entirely its own
                    }

                    m_polities.push_back(holdout);
                }
            }

            //--- the marches ---
            //"Since marking an exact boundary is difficult for large areas, oftentimes the line
            //would blur out to the land around it and the entire region would become a buffer
            //zone." Two things make a march: rule that has thinned out with distance, and a
            //neighbour on the far side pulling the other way.
            for(size_t i=0; i<influenceMapSize; i++)
            {
                InfluenceCell &cell=m_influenceMap[i];

                if(cell.polity<0)
                    continue;

                if(cell.control<rules.m_marchControl)
                {
                    cell.march=true;
                    continue;
                }

                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);

                for(size_t n=0; n<8; ++n)
                {
                    int ny=y+offsets[n][1];

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                    if((neighbor.heightBase>=0.5f)&&(neighbor.polity!=cell.polity))
                    {
                        cell.march=true;
                        break;
                    }
                }
            }

            for(size_t i=0; i<influenceMapSize; i++)
            {
                const InfluenceCell &cell=m_influenceMap[i];

                if(cell.polity<0)
                    continue;

                Polity &polity=m_polities[cell.polity];

                polity.m_cells++;
                polity.m_meanControl+=cell.control;

                float cellLatitude=(float)M_PI_2-((float)M_PI*((((float)(i/influenceSize.x))+0.5f)/(float)influenceSize.y));
                float side=(float)m_descriptorValues.m_influenceGridSize.x*m_descriptorValues.m_metresPerBlock;

                polity.m_area+=(side*side*std::max(cos(cellLatitude), 0.01f))/1.0e6f;

                if(cell.march)
                    polity.m_marchCells++;

                //"there are groups of people that are taken over, but they still get to exist in
                //the fringes of the land" - ground that suits somebody other than the people who
                //rule it, which is where a Brittany or a Wales survives
                if((cell.dominantSpecies>=0)&&(cell.dominantSpecies!=polity.m_species))
                    polity.m_minorityCells++;
            }

            for(size_t p=0; p<m_polities.size(); ++p)
            {
                if(m_polities[p].m_cells>0)
                    m_polities[p].m_meanControl/=(float)m_polities[p].m_cells;
            }
        }

        //--- what moves between them ---
        //Every route in the reference is one want met by one place: jade out of Turkestan against
        //Chinese silk and tea, incense off Arabia against Indian pepper, gold from the south
        //against salt from the north. So a route is not drawn between two states because they are
        //near each other - it is drawn because each has something the other has not, and then it
        //takes the cheapest way there. The cost of that way is NOT the cost of holding ground: a
        //caravan only finds a mountain expensive where an army finds it impassable, and the sea
        //reverses the pair entirely.
        m_tradeRoutes.clear();

        for(size_t i=0; i<influenceMapSize; i++)
        {
            m_influenceMap[i].goods=0;
            m_influenceMap[i].traffic=0.0f;
            m_influenceMap[i].chokepoint=false;
        }

        if(m_polities.size()>1)
        {
            const TradeThresholds &trade=m_descriptorValues.m_tradeRules;

            for(size_t i=0; i<influenceMapSize; i++)
                m_influenceMap[i].goods=cellGoods(m_influenceMap[i], relief[i]);

            //Coasting within sight of land is how nearly all trade went by water before deep-water
            //seamanship, so the shelf and the open ocean are not the same road.
            std::vector<bool> nearShore(influenceMapSize, false);

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(m_influenceMap[i].heightBase>=0.5f)
                    continue;

                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);

                for(int dy=-trade.m_coastalRange; (dy<=trade.m_coastalRange)&&(!nearShore[i]); ++dy)
                {
                    int ny=y+dy;

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    for(int dx=-trade.m_coastalRange; dx<=trade.m_coastalRange; ++dx)
                    {
                        if(m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+dx, influenceSize.x)].heightBase>=0.5f)
                        {
                            nearShore[i]=true;
                            break;
                        }
                    }
                }
            }

            std::vector<float> carry(influenceMapSize, 0.0f);

            for(size_t i=0; i<influenceMapSize; i++)
                carry[i]=travelCost(m_influenceMap[i], elevation[i], relief[i], nearShore[i], trade);

            //What each state can offer, and what it genuinely lacks. A state ruling a thousand
            //miles of ground has a little of nearly everything somewhere in it, so the union of
            //its cells is useless here - what matters is what it has in quantity against what it
            //has almost none of.
            std::vector<unsigned int> offer(m_polities.size(), 0);
            std::vector<unsigned int> want(m_polities.size(), 0);

            {
                std::vector<std::vector<size_t>> tally(m_polities.size(),
                    std::vector<size_t>(TradeGoodCount, 0));

                for(size_t i=0; i<influenceMapSize; i++)
                {
                    int held=m_influenceMap[i].polity;

                    if(held<0)
                        continue;

                    for(size_t g=0; g<TradeGoodCount; ++g)
                    {
                        if(m_influenceMap[i].goods&(1u<<(unsigned int)g))
                            tally[held][g]++;
                    }
                }

                for(size_t k=0; k<m_polities.size(); ++k)
                {
                    if(m_polities[k].m_cells==0)
                        continue;

                    for(size_t g=0; g<TradeGoodCount; ++g)
                    {
                        float share=(float)tally[k][g]/(float)m_polities[k].m_cells;

                        if(share>=trade.m_exportShare)
                            offer[k]|=(1u<<(unsigned int)g);
                        else if(share<=trade.m_scarceShare)
                            want[k]|=(1u<<(unsigned int)g);
                    }
                }
            }

            //One search per seat of power gives the cheapest way from it to everywhere, and the
            //route to any other seat is then read back off the trail.
            struct Carrier
            {
                float m_cost;
                size_t m_index;

                bool operator>(const Carrier &other) const { return m_cost>other.m_cost; }
            };

            std::vector<float> traffic(influenceMapSize, 0.0f);
            std::vector<float> reach(influenceMapSize);
            std::vector<size_t> came(influenceMapSize);
            std::vector<float> landReach(influenceMapSize);
            std::vector<size_t> landCame(influenceMapSize);
            float heaviest=0.0f;

            for(size_t from=0; from<m_polities.size(); ++from)
            {
                if(m_polities[from].m_cells==0)
                    continue;

                //the cheapest way by any means, and then the cheapest way that never leaves land
                for(int overland=0; overland<2; ++overland)
                {
                    std::vector<float> &distance=overland?landReach:reach;
                    std::vector<size_t> &trail=overland?landCame:came;

                    std::fill(distance.begin(), distance.end(), std::numeric_limits<float>::max());
                    std::priority_queue<Carrier, std::vector<Carrier>, std::greater<Carrier>> queue;

                    distance[m_polities[from].m_core]=0.0f;
                    trail[m_polities[from].m_core]=m_polities[from].m_core;
                    queue.push(Carrier{0.0f, m_polities[from].m_core});

                    while(!queue.empty())
                    {
                        Carrier current=queue.top();
                        queue.pop();

                        if(current.m_cost>distance[current.m_index])
                            continue;

                        int x=(int)(current.m_index%influenceSize.x);
                        int y=(int)(current.m_index/influenceSize.x);

                        for(size_t n=0; n<8; ++n)
                        {
                            int ny=y+offsets[n][1];

                            if((ny<0)||(ny>=influenceSize.y))
                                continue;

                            size_t index=((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                            if(overland&&(m_influenceMap[index].heightBase<0.5f))
                                continue;

                            float step=carry[index];

                            if((offsets[n][0]!=0)&&(offsets[n][1]!=0))
                                step*=1.41421f;

                            float total=current.m_cost+step;

                            if(total>=distance[index])
                                continue;

                            distance[index]=total;
                            trail[index]=current.m_index;
                            queue.push(Carrier{total, index});
                        }
                    }
                }

                for(size_t to=from+1; to<m_polities.size(); ++to)
                {
                    if(m_polities[to].m_cells==0)
                        continue;

                    unsigned int outbound=offer[from]&want[to];
                    unsigned int inbound=offer[to]&want[from];

                    //"nothing moves because two places are near each other" - each end has to want
                    //what the other has, or there is no route to draw
                    if((tradeGoodCount(outbound)<trade.m_minimumComplement)
                        ||(tradeGoodCount(inbound)<trade.m_minimumComplement))
                        continue;

                    size_t target=m_polities[to].m_core;

                    if(reach[target]>=std::numeric_limits<float>::max())
                        continue; //no way through at all

                    TradeRoute route;

                    route.m_from=from;
                    route.m_to=to;
                    route.m_cost=reach[target];
                    route.m_outbound=outbound;
                    route.m_inbound=inbound;
                    route.m_length=0;
                    route.m_seaCells=0;
                    route.m_intermediaries=0;
                    route.m_overlandCost=-1.0f;
                    route.m_overlandIntermediaries=0;

                    //how many hands the same trade would have to pass through if it had to go the
                    //whole way by land
                    if(landReach[target]<std::numeric_limits<float>::max())
                    {
                        std::vector<bool> overlandCrossed(m_polities.size(), false);
                        size_t walk=target;

                        route.m_overlandCost=landReach[target];

                        while(true)
                        {
                            int through=m_influenceMap[walk].polity;

                            if((through>=0)&&(through!=(int)from)&&(through!=(int)to)&&(!overlandCrossed[through]))
                            {
                                overlandCrossed[through]=true;
                                route.m_overlandIntermediaries++;
                            }

                            if(walk==landCame[walk])
                                break;

                            walk=landCame[walk];
                        }
                    }

                    //The value of a route falls off with what it costs to run: the whole history in
                    //the reference is people finding a cheaper way and ruining whoever held the
                    //old one.
                    float value=(float)(tradeGoodCount(outbound)+tradeGoodCount(inbound))
                        /(1.0f+(route.m_cost*0.02f));

                    std::vector<bool> crossed(m_polities.size(), false);
                    size_t step=target;

                    while(true)
                    {
                        route.m_length++;
                        traffic[step]+=value;
                        heaviest=std::max(heaviest, traffic[step]);

                        if(m_influenceMap[step].heightBase<0.5f)
                            route.m_seaCells++;
                        else
                        {
                            int through=m_influenceMap[step].polity;

                            if((through>=0)&&(through!=(int)from)&&(through!=(int)to)&&(!crossed[through]))
                            {
                                crossed[through]=true;
                                route.m_intermediaries++;
                            }
                        }

                        if(step==came[step])
                            break;

                        step=came[step];
                    }

                    m_tradeRoutes.push_back(route);
                }
            }

            //A crossroads is made by the ground, not by the traffic alone. Petra, Alexandria,
            //Constantinople, Malacca, Suez, Panama - every one of them is a place where the world
            //narrows to a gap and everything has to go through it. So a chokepoint here is heavy
            //traffic across ground that has no way round: an isthmus with sea to both sides, a
            //strait with land to both sides, a pass with mountain walls.
            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(heaviest<=0.0f)
                    break;

                m_influenceMap[i].traffic=traffic[i]/heaviest;

                if(m_influenceMap[i].traffic<trade.m_chokepointTraffic)
                    continue;

                bool overLand=(m_influenceMap[i].heightBase>=0.5f);
                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);
                int blocked=0;

                for(size_t n=0; n<8; ++n)
                {
                    int ny=y+offsets[n][1];

                    if((ny<0)||(ny>=influenceSize.y))
                    {
                        blocked++;
                        continue;
                    }

                    size_t index=((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);
                    bool neighborLand=(m_influenceMap[index].heightBase>=0.5f);

                    if(overLand)
                    {
                        //for a road, water stops it, and so does a mountain wall
                        if((!neighborLand)||(elevation[index]>=trade.m_mountainElevation)
                            ||(relief[index]>=trade.m_mountainRelief))
                            blocked++;
                    }
                    else if(neighborLand)
                        blocked++;   //for a ship, the land is the wall
                }

                if(blocked>=trade.m_constriction)
                    m_influenceMap[i].chokepoint=true;
            }
        }

        //--- the places that take rather than make ---
        //"Sea transport costs 40 times less than land transport. That gap creates a flow, and
        //wherever flow concentrates something builds to catch it. The city is a drain." The trade
        //pass above produced the flow; this one finds the drains. Four kinds of ground make one,
        //and three ceilings decide how big it can get - "they stack, and the smallest one wins".
        m_cities.clear();

        for(size_t i=0; i<influenceMapSize; i++)
            m_influenceMap[i].city=-1;

        {
            const CityThresholds &rules=m_descriptorValues.m_cityRules;

            //a good almost nowhere else has is a monopoly, and a monopoly is what turns a producer
            //from somebody else's supplier into a drain of his own
            std::vector<size_t> everywhere(TradeGoodCount, 0);
            size_t landCells=0;

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(m_influenceMap[i].heightBase<0.5f)
                    continue;

                landCells++;

                for(size_t g=0; g<TradeGoodCount; ++g)
                {
                    if(m_influenceMap[i].goods&(1u<<(unsigned int)g))
                        everywhere[g]++;
                }
            }

            unsigned int rare=0;

            for(size_t g=0; g<TradeGoodCount; ++g)
            {
                if((everywhere[g]>0)&&((float)everywhere[g]/(float)std::max<size_t>(landCells, 1)<0.02f))
                    rare|=(1u<<(unsigned int)g);
            }

            std::vector<City> candidates;

            for(size_t i=0; i<influenceMapSize; i++)
            {
                const InfluenceCell &cell=m_influenceMap[i];

                if((cell.heightBase<0.5f)||(cell.coast==(int)CoastType::None))
                    continue;

                int x=(int)(i%influenceSize.x);
                int y=(int)(i/influenceSize.x);

                //"Ships stop at natural harbours" - but only where the rest of the coast gives them
                //no choice, which is what makes the anchorage lord's position worth anything
                bool sheltered=shelteredCoast((CoastType)cell.coast);
                bool onNarrows=cell.chokepoint;
                int hostile=0;

                //a port's traffic is the traffic of the water beside it, not of the ground it
                //stands on
                float traffic=cell.traffic;

                for(size_t n=0; n<8; ++n)
                {
                    int ny=y+offsets[n][1];

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x)];

                    if(hostileCoast((CoastType)neighbor.coast))
                        hostile++;

                    if(neighbor.chokepoint)
                        onNarrows=true;

                    traffic=std::max(traffic, neighbor.traffic);
                }

                City city;

                city.m_cell=i;
                city.m_monopoly=cell.goods&rare;
                city.m_node=classifyNode(cell, cell.riverDischarge>=rules.m_navigableDischarge,
                    onNarrows, sheltered&&(hostile>=2), city.m_monopoly!=0);

                if(city.m_node==NodeType::None)
                    continue;

                city.m_polity=cell.polity;
                city.m_traffic=traffic;
                city.m_water=waterCeiling(cell, rules);
                city.m_river=riverCeiling(cell, rules);

                //"A coastal city with rich farmland in its hinterland is a large city in the
                //making. A city with no fertile hinterlands hits its ceiling fast."
                {
                    float fed=0.0f;
                    int counted=0;

                    for(int dy=-rules.m_hinterland; dy<=rules.m_hinterland; ++dy)
                    {
                        int ny=y+dy;

                        if((ny<0)||(ny>=influenceSize.y))
                            continue;

                        for(int dx=-rules.m_hinterland; dx<=rules.m_hinterland; ++dx)
                        {
                            size_t index=((size_t)ny*influenceSize.x)+wrapIndex(x+dx, influenceSize.x);

                            if(m_influenceMap[index].heightBase<0.5f)
                                continue; //the sea grows no bread

                            fed+=siteArable(m_influenceMap[index], relief[index]);
                            counted++;
                        }
                    }

                    city.m_food=(counted>0)?(fed/(float)counted):0.0f;
                }

                float natural=std::min(city.m_water, std::min(city.m_food, city.m_river));

                //"Rome did not eat Italian bread. It ate Egyptian bread." A place whose own fields
                //are what stops it, but which sits on a lane that can carry grain, refuses the
                //limit and takes its bread off the water instead.
                city.m_parasite=false;
                city.m_size=natural;

                float otherwise=std::min(city.m_water, city.m_river);

                if((city.m_food<otherwise-rules.m_parasiteShortfall)&&(traffic>=rules.m_parasiteTraffic))
                {
                    city.m_parasite=true;
                    city.m_size=otherwise;
                }

                if(city.m_size<rules.m_minimumSize)
                    continue;

                candidates.push_back(city);
            }

            //the strongest drains first, and none of them close enough to be feeding on the same
            //flow as the last
            std::sort(candidates.begin(), candidates.end(),
                [](const City &a, const City &b) { return a.m_size>b.m_size; });

            std::vector<bool> taken(influenceMapSize, false);

            for(size_t c=0; c<candidates.size(); ++c)
            {
                if((int)m_cities.size()>=rules.m_count)
                    break;

                if(taken[candidates[c].m_cell])
                    continue;

                City city=candidates[c];

                city.m_reason=cityReason(city);

                int cx=(int)(city.m_cell%influenceSize.x);
                int cy=(int)(city.m_cell/influenceSize.x);

                for(int dy=-rules.m_spacing; dy<=rules.m_spacing; ++dy)
                {
                    int ny=cy+dy;

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    for(int dx=-rules.m_spacing; dx<=rules.m_spacing; ++dx)
                        taken[((size_t)ny*influenceSize.x)+wrapIndex(cx+dx, influenceSize.x)]=true;
                }

                m_influenceMap[city.m_cell].city=(int)m_cities.size();
                m_cities.push_back(city);
            }
        }

        //--- the beaten track, and everywhere that is not it ---
        //The reference's "hot path": once you know the route through, you build a second distance
        //map out from every point ON that route, and then everything near it is where people go
        //and everything far from it is where you put what you want FOUND rather than passed. "The
        //gray rooms could be where you hide bonus things." The trade network is this world's hot
        //path, so the same distance map answers the same question at world scale.
        {
            const TradeThresholds &trade=m_descriptorValues.m_tradeRules;

            struct Traveller
            {
                float m_cost;
                size_t m_index;

                bool operator>(const Traveller &other) const { return m_cost>other.m_cost; }
            };

            std::vector<float> distance(influenceMapSize, std::numeric_limits<float>::max());
            std::priority_queue<Traveller, std::vector<Traveller>, std::greater<Traveller>> queue;

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if((m_influenceMap[i].traffic<=0.0f)&&(m_influenceMap[i].city<0))
                    continue;

                distance[i]=0.0f;
                queue.push(Traveller{0.0f, i});
            }

            //walking off the road costs what the ground costs, the same as travelling it does
            while(!queue.empty())
            {
                Traveller current=queue.top();
                queue.pop();

                if(current.m_cost>distance[current.m_index])
                    continue;

                int x=(int)(current.m_index%influenceSize.x);
                int y=(int)(current.m_index/influenceSize.x);

                for(size_t n=0; n<8; ++n)
                {
                    int ny=y+offsets[n][1];

                    if((ny<0)||(ny>=influenceSize.y))
                        continue;

                    size_t index=((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);

                    if(m_influenceMap[index].heightBase<0.5f)
                        continue;   //this is about walking, not sailing

                    //nearShore is irrelevant here: this walk never enters the water
                    float step=travelCost(m_influenceMap[index], elevation[index], relief[index],
                        false, trade);

                    if((offsets[n][0]!=0)&&(offsets[n][1]!=0))
                        step*=1.41421f;

                    float total=current.m_cost+step;

                    if(total>=distance[index])
                        continue;

                    distance[index]=total;
                    queue.push(Traveller{total, index});
                }
            }

            //Ground the walk never reached is not merely remote, it is cut off - an island with no
            //port on it, or country walled in behind something impassable. The reference culls
            //exactly this case before placing anything: "all of the tiles that are open but have
            //the sentinel value can't be reached."
            float furthest=0.0f;

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if((m_influenceMap[i].heightBase<0.5f)||(distance[i]>=std::numeric_limits<float>::max()))
                    continue;

                furthest=std::max(furthest, distance[i]);
            }

            for(size_t i=0; i<influenceMapSize; i++)
            {
                InfluenceCell &cell=m_influenceMap[i];

                cell.onTheBeatenTrack=false;

                if(cell.heightBase<0.5f)
                {
                    cell.remoteness=0.0f;
                    continue;
                }

                if(distance[i]>=std::numeric_limits<float>::max())
                {
                    cell.remoteness=1.0f;   //nobody comes here at all
                    continue;
                }

                cell.remoteness=(furthest>0.0f)?(distance[i]/furthest):0.0f;
                cell.onTheBeatenTrack=(distance[i]<=trade.m_hotPath);
            }
        }


        //--- the places people actually live ---
        //A realm is a thousand miles of ground and a city is a drain built where flow concentrates.
        //Neither is what somebody standing in a valley has for a neighbour, so there is a tier
        //below both: steadings, hamlets, villages and towns, a few miles apart.
        //
        //Sited by weighted interest with a hard veto (Emilien et al): every criterion runs from -1
        //to 1, any single -1 makes the place impossible however good the rest of it is, and the
        //result is not a decision but a probability - a site is ACCEPTED in proportion to its
        //interest rather than the best cell being taken, so a good region fills in unevenly the way
        //a real one does.
        m_settlements.clear();

        for(size_t i=0; i<influenceMapSize; i++)
            m_influenceMap[i].settlement=-1;

        if(!species.empty())
        {
            const SettlementThresholds &rules=m_descriptorValues.m_settlementRules;

            //"Either an indicator of social superiority or as necessity for defense": how far this
            //ground stands over what surrounds it, weighted by the inverse square of the distance.
            std::vector<float> domination(influenceMapSize, 0.0f);

            {
                int radius=rules.m_dominationRadius;
                float highest=0.0f;

                for(size_t i=0; i<influenceMapSize; i++)
                {
                    if(m_influenceMap[i].heightBase<0.5f)
                        continue;

                    int x=(int)(i%influenceSize.x);
                    int y=(int)(i/influenceSize.x);
                    float sum=0.0f;

                    for(int dy=-radius; dy<=radius; ++dy)
                    {
                        int ny=y+dy;

                        if((ny<0)||(ny>=influenceSize.y))
                            continue;

                        for(int dx=-radius; dx<=radius; ++dx)
                        {
                            if((dx==0)&&(dy==0))
                                continue;

                            float distanceSquared=(float)((dx*dx)+(dy*dy));

                            if(distanceSquared>(float)(radius*radius))
                                continue;

                            size_t index=((size_t)ny*influenceSize.x)+wrapIndex(x+dx, influenceSize.x);

                            sum+=(elevation[i]-elevation[index])/(1.0f+distanceSquared);
                        }
                    }

                    domination[i]=sum;
                    highest=std::max(highest, fabsf(sum));
                }

                if(highest>0.0f)
                {
                    for(size_t i=0; i<influenceMapSize; i++)
                        domination[i]=clamp(domination[i]/highest, -1.0f, 1.0f);
                }
            }

            //Sociability is the one criterion that changes as the map fills, so it is carried in a
            //field that later placements read and every placement writes to.
            std::vector<float> company(influenceMapSize, 0.0f);

            //Biggest first. A town is the hardest to please and wants the most room, and placing it
            //before the villages lets the smaller places gather around it rather than the other way
            //about - which is the order settlement hierarchies actually form in.
            const SettlementKind order[4]=
            {
                SettlementKind::Town, SettlementKind::Village,
                SettlementKind::Hamlet, SettlementKind::Steading
            };

            //A shuffled walk, so no part of the map is preferred merely for being early in memory.
            std::vector<size_t> visit(influenceMapSize);

            for(size_t i=0; i<influenceMapSize; i++)
                visit[i]=i;

            for(size_t k=0; k<4; ++k)
            {
                SettlementKind kind=order[k];
                const SettlementPreference &preference=rules.preference(kind);

                for(size_t i=visit.size(); i>1; --i)
                {
                    std::uniform_int_distribution<size_t> pick(0, i-1);
                    std::swap(visit[i-1], visit[pick(generator)]);
                }

                for(size_t v=0; v<visit.size(); ++v)
                {
                    size_t index=visit[v];
                    const InfluenceCell &cell=m_influenceMap[index];

                    if(cell.settlement>=0)
                        continue;

                    if(settlementImpossible(cell, relief[index], rules))
                        continue;

                    //whose people would live here at all
                    int who=cell.dominantSpecies;

                    if(who<0)
                        continue;

                    int x=(int)(index%influenceSize.x);
                    int y=(int)(index/influenceSize.x);

                    //nothing of this size stands within its own spacing of another of its size
                    bool crowded=false;

                    for(int dy=-preference.m_spacing; (dy<=preference.m_spacing)&&(!crowded); ++dy)
                    {
                        int ny=y+dy;

                        if((ny<0)||(ny>=influenceSize.y))
                            continue;

                        for(int dx=-preference.m_spacing; dx<=preference.m_spacing; ++dx)
                        {
                            int near=m_influenceMap[((size_t)ny*influenceSize.x)
                                +wrapIndex(x+dx, influenceSize.x)].settlement;

                            if((near>=0)&&(m_settlements[near].m_kind==kind))
                            {
                                crowded=true;
                                break;
                            }
                        }
                    }

                    if(crowded)
                        continue;

                    SettlementCriteria criteria;

                    criteria.m_water=toInterest(siteWater(cell));
                    criteria.m_arable=toInterest(siteArable(cell, relief[index]));
                    criteria.m_grazing=toInterest(siteGrazing(cell));
                    criteria.m_timber=toInterest(siteTimber(cell));
                    criteria.m_stone=toInterest(siteStone(cell, relief[index]));
                    criteria.m_fuel=toInterest(siteFuel(cell));
                    criteria.m_ore=toInterest(siteOre(cell));

                    //flat enough to build on and to plough, without being a bog
                    criteria.m_slope=toInterest(1.0f-clamp(relief[index]/rules.m_slopeVeto, 0.0f, 1.0f));

                    criteria.m_sociability=clamp(company[index], -1.0f, 1.0f);
                    criteria.m_domination=domination[index];

                    //the roads and lanes: how near this place is to ground people already cross
                    criteria.m_accessibility=cell.onTheBeatenTrack
                        ?1.0f:toInterest(1.0f-clamp(cell.remoteness*3.0f, 0.0f, 1.0f));

                    float interest=settlementInterest(criteria, species[who], preference);

                    if(interest<preference.m_threshold)
                        continue;

                    //Accepted in proportion to what the place is worth, not because it was the best
                    //cell going. This is the whole point of the model: taking the argmax fills a
                    //region from its single best site outward and looks it, where a probability
                    //that follows interest leaves the good country busy and the poor country thin.
                    if(distributionNorm(generator)>(interest*rules.m_acceptance))
                        continue;

                    Settlement settlement;

                    settlement.m_cell=index;
                    settlement.m_kind=kind;
                    settlement.m_species=who;
                    settlement.m_polity=cell.polity;
                    settlement.m_interest=interest;
                    settlement.m_criteria=criteria;
                    settlement.m_population=(size_t)((float)preference.m_population
                        *(0.55f+(interest*0.9f)));
                    settlement.m_reason=settlementReason(settlement);

                    m_influenceMap[index].settlement=(int)m_settlements.size();
                    m_settlements.push_back(settlement);

                    //Somewhere with people in it draws more people. The pull falls off with
                    //distance and with how small the place is.
                    {
                        int reach=rules.m_sociabilityRadius;
                        float weight=(float)preference.m_population/1400.0f;

                        for(int dy=-reach; dy<=reach; ++dy)
                        {
                            int ny=y+dy;

                            if((ny<0)||(ny>=influenceSize.y))
                                continue;

                            for(int dx=-reach; dx<=reach; ++dx)
                            {
                                float distanceSquared=(float)((dx*dx)+(dy*dy));

                                if(distanceSquared>(float)(reach*reach))
                                    continue;

                                company[((size_t)ny*influenceSize.x)+wrapIndex(x+dx, influenceSize.x)]
                                    +=weight/(1.0f+distanceSquared);
                            }
                        }
                    }
                }
            }
        }


        //--- the ways between them ---
        //Nothing here decides that four lanes should meet at a town. Each place is joined to a
        //LARGER one near it, every road is the cheapest line the ground allows, and a road somebody
        //else has already cut is nearly free to follow. Roads meet at towns because each of them was
        //separately cheapest, which is the same mechanism that made the borders land on mountains.
        m_roads.clear();

        for(size_t i=0; i<influenceMapSize; i++)
        {
            m_influenceMap[i].road=false;
            m_influenceMap[i].roadTraffic=0.0f;
        }

        if(m_settlements.size()>1)
        {
            const RoadThresholds &roadRules=m_descriptorValues.m_roadRules;
            const TradeThresholds &carriage=m_descriptorValues.m_tradeRules;

            //biggest first, so the trunk roads exist for the smaller ways to join
            std::vector<size_t> byKind(m_settlements.size());

            for(size_t i=0; i<m_settlements.size(); ++i)
                byKind[i]=i;

            std::sort(byKind.begin(), byKind.end(), [&](size_t a, size_t b)
                { return (int)m_settlements[a].m_kind>(int)m_settlements[b].m_kind; });

            struct Walker
            {
                float m_estimate;
                float m_cost;
                size_t m_index;

                bool operator>(const Walker &other) const { return m_estimate>other.m_estimate; }
            };

            std::vector<float> spent(influenceMapSize);
            std::vector<size_t> came(influenceMapSize);
            std::vector<size_t> stamp(influenceMapSize, 0);
            size_t visitStamp=0;

            for(size_t s=0; s<byKind.size(); ++s)
            {
                const Settlement &from=m_settlements[byKind[s]];
                int ax=(int)(from.m_cell%influenceSize.x);
                int ay=(int)(from.m_cell/influenceSize.x);

                //the nearest place bigger than this one - a steading walks to its hamlet, a hamlet
                //to its village, and a town to another town
                size_t target=m_settlements.size();
                float nearest=roadRules.m_reach;

                for(size_t t=0; t<m_settlements.size(); ++t)
                {
                    if(t==byKind[s])
                        continue;

                    const Settlement &other=m_settlements[t];

                    if((int)other.m_kind<(int)from.m_kind)
                        continue;

                    if(((int)other.m_kind==(int)from.m_kind)&&(from.m_kind!=SettlementKind::Town))
                        continue;

                    int bx=(int)(other.m_cell%influenceSize.x);
                    int by=(int)(other.m_cell/influenceSize.x);
                    int dx=abs(bx-ax);

                    if(dx>(int)influenceSize.x/2)
                        dx=(int)influenceSize.x-dx;

                    float distance=sqrtf((float)((dx*dx)+((by-ay)*(by-ay))));

                    if(distance<nearest)
                    {
                        nearest=distance;
                        target=t;
                    }
                }

                if(target>=m_settlements.size())
                    continue;

                size_t goal=m_settlements[target].m_cell;
                int gx=(int)(goal%influenceSize.x);
                int gy=(int)(goal/influenceSize.x);

                //A stamp rather than clearing two arrays of the whole map for every one of a
                //thousand settlements.
                visitStamp++;

                std::priority_queue<Walker, std::vector<Walker>, std::greater<Walker>> open;

                spent[from.m_cell]=0.0f;
                came[from.m_cell]=from.m_cell;
                stamp[from.m_cell]=visitStamp;
                open.push(Walker{0.0f, 0.0f, from.m_cell});

                bool arrived=false;

                while(!open.empty())
                {
                    Walker current=open.top();

                    open.pop();

                    if(current.m_cost>spent[current.m_index])
                        continue;

                    if(current.m_index==goal)
                    {
                        arrived=true;
                        break;
                    }

                    int x=(int)(current.m_index%influenceSize.x);
                    int y=(int)(current.m_index/influenceSize.x);

                    for(size_t n=0; n<8; ++n)
                    {
                        int ny=y+offsets[n][1];

                        if((ny<0)||(ny>=influenceSize.y))
                            continue;

                        size_t index=((size_t)ny*influenceSize.x)+wrapIndex(x+offsets[n][0], influenceSize.x);
                        const InfluenceCell &step=m_influenceMap[index];

                        //a road is overland; a ferry is somebody else's problem
                        if(step.heightBase<0.5f)
                            continue;

                        float length=((offsets[n][0]!=0)&&(offsets[n][1]!=0))?1.41421f:1.0f;
                        float run=length*(float)m_descriptorValues.m_influenceGridSize.x
                            *m_descriptorValues.m_metresPerBlock;
                        float rise=elevation[index]-elevation[current.m_index];
                        float climb=(run>0.0f)?std::max(rise/run, 0.0f):0.0f;

                        float cost=travelCost(step, elevation[index], relief[index], false, carriage)
                            *length*(1.0f+(roadRules.m_climbPenalty*climb));

                        //following a way already cut is nearly free, which is what makes lanes join
                        //instead of running alongside each other
                        if(step.road)
                            cost*=roadRules.m_reuseWeight;

                        float total=current.m_cost+cost;

                        if(total>roadRules.m_searchLimit*carriage.m_landCost*8.0f)
                            continue;

                        if((stamp[index]==visitStamp)&&(total>=spent[index]))
                            continue;

                        spent[index]=total;
                        came[index]=current.m_index;
                        stamp[index]=visitStamp;

                        int hx=abs(gx-((int)(index%influenceSize.x)));

                        if(hx>(int)influenceSize.x/2)
                            hx=(int)influenceSize.x-hx;

                        int hy=gy-((int)(index/influenceSize.x));
                        float remaining=sqrtf((float)((hx*hx)+(hy*hy)))*roadRules.m_reuseWeight;

                        open.push(Walker{total+remaining, total, index});
                    }
                }

                if(!arrived)
                    continue;

                Road road;

                road.m_from=byKind[s];
                road.m_to=target;
                road.m_cost=spent[goal];
                road.m_length=0;

                size_t step=goal;

                while(true)
                {
                    m_influenceMap[step].road=true;
                    m_influenceMap[step].roadTraffic+=1.0f;
                    road.m_length++;

                    if(came[step]==step)
                        break;

                    step=came[step];
                }

                m_roads.push_back(road);
            }
        }

        //--- resolving what the ruins meant ---
        //The reference splits history in two, and the split is what makes it work. The abstract
        //part happens during world generation - the village worships a legendary creature, but
        //"at this point in generation they don't even know what creature they worship, they simply
        //know that they worship a creature". Only once the world has real things in it are those
        //abstract links bound to them. So the fall of each civilization is decided here, after the
        //states, the regions and the coasts exist to be pointed at.
        //
        //And it is decided in the reference's own inverted order: "we decide how an event ends and
        //then create a rationalization for why it ended that way". The fate is rolled first. The
        //evidence for it is then LOOKED FOR on the map rather than invented, and where the map has
        //nothing to say the history admits as much.
        for(size_t f=0; f<m_fallen.size(); ++f)
        {
            FallenCivilization &fallen=m_fallen[f];

            std::vector<size_t> sites;

            for(size_t i=0; i<influenceMapSize; i++)
            {
                if(m_influenceMap[i].ruinCivilization==(int)f)
                    sites.push_back(i);
            }

            if(sites.empty())
                continue;

            //the ending, chosen before anything is known about why
            {
                std::vector<WeightedEntry> fates;

                fates.push_back(WeightedEntry{(int)CivilizationFate::Conquest, 30});
                fates.push_back(WeightedEntry{(int)CivilizationFate::Drought, 20});
                fates.push_back(WeightedEntry{(int)CivilizationFate::Exhaustion, 15});
                fates.push_back(WeightedEntry{(int)CivilizationFate::Sundered, 12});
                fates.push_back(WeightedEntry{(int)CivilizationFate::Ice, 8});
                fates.push_back(WeightedEntry{(int)CivilizationFate::Drowned, 8});
                fates.push_back(WeightedEntry{(int)CivilizationFate::Plague, 7});

                unsigned int roll=(unsigned int)(((f+1)*2654435761u)
                    ^((unsigned int)m_descriptorValues.seed*374761393u));

                fallen.m_fate=rollTable(fates, roll);
            }

            //now go looking for what would account for it
            float best=-1.0f;
            float moisture=0.0f;
            int worked=0;
            std::vector<int> regions;

            for(size_t k=0; k<sites.size(); ++k)
            {
                const InfluenceCell &site=m_influenceMap[sites[k]];

                if(site.ruinCondition>best)
                {
                    best=site.ruinCondition;
                    fallen.m_seat=sites[k];   //the greatest of them, or what is left of it
                }

                //what their rain did between their world and this one
                moisture+=applyGlobalMoisture(site.moisture, fallen.m_temperatureSwing)-site.moisture;

                if(site.deposit!=DepositType::None)
                    worked++;

                if(site.shallowSea)
                    fallen.m_drownedSites++;

                if(site.culturalRegion>=0)
                {
                    bool known=false;

                    for(size_t r=0; r<regions.size(); ++r)
                        known=known||(regions[r]==site.culturalRegion);

                    if(!known)
                        regions.push_back(site.culturalRegion);
                }
            }

            fallen.m_moistureSwing=moisture/(float)sites.size();
            fallen.m_workedOut=(float)worked/(float)sites.size();
            fallen.m_barrierSplit=(int)regions.size();

            if(fallen.m_seat>=influenceMapSize)
                fallen.m_seat=sites[0];

            //"we looked at nearby creatures who are good candidates for the hatred of this
            //village" - the same reification, against the states this world actually grew
            {
                int sx=(int)(fallen.m_seat%influenceSize.x);
                int sy=(int)(fallen.m_seat/influenceSize.x);

                for(size_t k=0; k<m_polities.size(); ++k)
                {
                    if(m_polities[k].m_cells==0)
                        continue;

                    int px=(int)(m_polities[k].m_core%influenceSize.x);
                    int py=(int)(m_polities[k].m_core/influenceSize.x);
                    int dx=abs(px-sx);

                    if(dx>(int)influenceSize.x/2)
                        dx=(int)influenceSize.x-dx;   //the map is a cylinder

                    float distance=sqrtf((float)((dx*dx)+((py-sy)*(py-sy))));

                    if((fallen.m_rivalDistance<0.0f)||(distance<fallen.m_rivalDistance))
                    {
                        fallen.m_rivalDistance=distance;
                        fallen.m_nearestRival=(int)k;
                    }
                }

                //where anyone who walked away would have ended up: the first country over the
                //divide that is not the one they were living in
                //A neighbouring region is not a refuge - the next valley over is where you go for
                //market day. It only counts as somewhere survivors FLED to if getting there meant
                //crossing the hard ground that divides one people from the next, so the walk is
                //tracked and only paths that cross a barrier are accepted.
                //Two walks. The first stays on open ground: if another country can be reached
                //without crossing anything hard, then nobody FLED anywhere, they simply moved, and
                //there is no refuge to record. Only where the open walk finds nothing does the
                //second walk go over the divide, and that is the refuge.
                int home=m_influenceMap[fallen.m_seat].culturalRegion;
                bool reachedInTheOpen=false;

                for(int overBarriers=0; overBarriers<2; ++overBarriers)
                {
                    if(reachedInTheOpen||(fallen.m_refuge>=0))
                        break;

                    std::vector<bool> seen(influenceMapSize, false);
                    std::vector<size_t> wave;

                    wave.push_back(fallen.m_seat);
                    seen[fallen.m_seat]=true;

                    for(size_t head=0; head<wave.size(); ++head)
                    {
                        int wx=(int)(wave[head]%influenceSize.x);
                        int wy=(int)(wave[head]/influenceSize.x);
                        bool done=false;

                        for(size_t n=0; n<8; ++n)
                        {
                            int ny=wy+offsets[n][1];

                            if((ny<0)||(ny>=influenceSize.y))
                                continue;

                            size_t index=((size_t)ny*influenceSize.x)+wrapIndex(wx+offsets[n][0], influenceSize.x);
                            const InfluenceCell &step=m_influenceMap[index];

                            if(seen[index]||(step.heightBase<0.5f))
                                continue;

                            if((!overBarriers)&&(step.barrier!=(int)BarrierType::None))
                                continue;   //the open walk stops at hard ground

                            seen[index]=true;

                            if((step.culturalRegion>=0)&&(step.culturalRegion!=home))
                            {
                                if(overBarriers)
                                    fallen.m_refuge=step.culturalRegion;
                                else
                                    reachedInTheOpen=true;

                                done=true;
                                break;
                            }

                            wave.push_back(index);
                        }

                        if(done)
                            break;
                    }
                }
            }

            //and only now is the story written
            fallen.m_name=species[fallen.m_species].m_name+" of the "
                +civilizationFateName((CivilizationFate)fallen.m_fate);
        }
    }

    time2=chrono::high_resolution_clock::now();
    processingTime=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

    m_plateCount=plateDetails.size();

    //The corner values getBaseHeight and generateChunk interpolate between. Every cell keeps the
    //four values of its own corners, each being the average of the four cells that meet there,
    //which is the same arrangement a tile renderer calls a dual grid - the type is carried by the
    //corners rather than by the cell, so neighbouring cells agree along their shared edge and
    //nothing has to be resolved at draw time.
    //
    //This was previously only reached down the persistence path, which nothing instantiates, so a
    //world built by create() left the map empty and getBaseHeight indexed off the end of it.
    updateInfluenceNeighbors();
}


void EquiRectWorldGenerator::generateContinents(Progress &progress)
{
//    m_hidden->m_cellularNoise->SetFrequency(m_descriptorValues.m_continentFrequency);
//    m_hidden->m_cellularNoise->SetFractalLacunarity(m_descriptorValues.m_plateLacunarity);
//    m_hidden->m_cellularNoise->SetFractalOctaves(m_descriptorValues.m_plateOctaves);
}

template<bool useStride>
int getBlockType(int z, size_t columnHeight, size_t stride)
{
    int blockType=1;// =(columnHeight-z)/8+2;

    if(columnHeight>10)
        blockType=3;
    if(columnHeight>2)
        blockType=2;

    return blockType;
}

template<>
inline int getBlockType<true>(int z, size_t columnHeight, size_t stride)
{
    int blockType=1;

//    for(int z=0; z<stride; ++z)
//    {
//        for(int y=0; y<stride; ++y)
//        {
//            for(int y=0; y<stride; ++y)
//            {
//
//            }
//        }
//    }
    if(columnHeight>10)
        blockType=3;
    if(columnHeight>2)
        blockType=2;

    return blockType;
}


unsigned int EquiRectWorldGenerator::generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod)
{
//    if(!m_threadStorage.vectorSet)
//        m_threadStorage.vectorSet=std::make_unique<HastyNoise::VectorSet>(m_simdLevel);
//    
//
////    glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;
//    glm::vec3 scaledOffset=startPos*m_descriptorValues.m_noiseScale;
//    glm::vec3 position=scaledOffset;
//    glm::ivec2 influenceIPos(startPos.x/m_descriptorValues.m_influenceGridSize.x, startPos.y/m_descriptorValues.m_influenceGridSize.y);
//    size_t influenceIndex=((influenceIPos.y*m_descriptorValues.m_influenceSize.x)+influenceIPos.x);
//    glm::vec2 influenceOffset=glm::vec2(startPos.x, startPos.y)-glm::vec2(influenceIPos*m_descriptorValues.m_influenceGridSize);
//    glm::vec2 influenceGridSize(m_descriptorValues.m_influenceGridSize);
//
//    influenceOffset=influenceOffset/influenceGridSize;
//    glm::vec2 influenceBlockScale((float)ChunkType::sizeX::value/influenceGridSize.x, (float)ChunkType::sizeY::value/influenceGridSize.y);
//    
//    InfluenceCell &influence=m_influenceMap[influenceIndex];
//    float *neighborHeight=&m_influenceNeighborMap[influenceIndex*NeighborCount];
//
////    UniqueChunkType chunk=std::make_unique<ChunkType>(hash, 0, chunkIndex, startPos);
////    ChunkType::Cells &cells=chunk->getCells();
//    
//    typename ChunkType::CellType *cells=(typename ChunkType::CellType *)buffer;
//    size_t stride=glm::pow(2u, (unsigned int)lod);
//    glm::ivec3 lodChunkSize=chunkSize/(int)stride;
//    float noiseScale=m_descriptorValues.m_noiseScale/stride;
//
//    //verify chunkSize matches template chunk size
//    assert(chunkSize==glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
//    //verify buffer is large enough for data
//    //assert(bufferSize>=(ChunkType::sizeX::value*ChunkType::sizeY::value*ChunkType::sizeZ::value)*sizeof(ChunkType::CellType));
//    assert(bufferSize>=(lodChunkSize.x*lodChunkSize.y*lodChunkSize.z)*sizeof(typename ChunkType::CellType));
//
////    m_continentPerlin->FillNoiseSetMap(heightMap.data(), xMap.data(), yMap.data(), zMap.data(), lodChunkSize.x, lodChunkSize.y, 1);
//    buildHeightMap(startPos, lodChunkSize, stride);
//    if(m_threadStorage.blockHeightMap.size()!=m_threadStorage.heightMap.size())
//        m_threadStorage.blockHeightMap.resize(m_threadStorage.heightMap.size());
//    if(m_threadStorage.blockScaleMap.size()!=m_threadStorage.heightMap.size())
//        m_threadStorage.blockScaleMap.resize(m_threadStorage.heightMap.size());
//
//    int chunkMapSize=HastyNoise::AlignedSize(lodChunkSize.x*lodChunkSize.y*lodChunkSize.z, m_simdLevel);
//
////    std::vector<float> layerMap(chunkMapSize);
//    m_threadStorage.layerMap.resize(chunkMapSize);
//
////    m_layersPerlin->FillNoiseSet(layerMap.data(), offset.x, offset.y, offset.z, _Chunk::sizeX::value, _Chunk::sizeY::value, _Chunk::sizeZ::value);
//
//    glm::ivec3 &size=m_descriptors.getSize();
//    int seaLevel=(size.z/2);
////    float heightScale=((float)size.z/2.0f);
//    float heightScale=(float)size.z;
//
////    float heightScale=1.0f-neighborHeight;
    unsigned int validCells=0;
//    glm::ivec3 blockIndex;
//
//    size_t heightIndex=0;
//    for(int y=0; y<ChunkType::sizeY::value; y+=stride)
//    {
//        for(int x=0; x<ChunkType::sizeX::value; x+=stride)
//        {
//            glm::vec2 influencePos=influenceOffset+(glm::vec2(x, y)*influenceBlockScale);
//
//            float scale=1.0f/3.0f;
//            float heightBase=bi_lerp(neighborHeight[0], neighborHeight[1], neighborHeight[2], neighborHeight[3], influencePos.x, influencePos.y);
//            float influenceScale=abs(heightBase-0.5f)*scale;
//
//            m_threadStorage.blockHeightMap[heightIndex]=(heightBase-influenceScale)*heightScale;
//            m_threadStorage.blockScaleMap[heightIndex]=influenceScale*heightScale;
////            blockHeight[heightIndex]=(int)(heightMap[heightIndex]*heightScale)+heightBase;
//
//            heightIndex++;
//        }
//    }
//
//    size_t index=0;
//    heightIndex=0;
//    position.z=scaledOffset.z;
//    for(int z=0; z<ChunkType::sizeZ::value; z+=stride)
//    {
//        heightIndex=0;
//        position.y=scaledOffset.y;
//
//        int blockZ=(int)startPos.z+z;
//        for(int y=0; y<ChunkType::sizeY::value; y+=stride)
//        {
//            position.x=scaledOffset.x;
//            for(int x=0; x<ChunkType::sizeX::value; x+=stride)
//            {
//                unsigned int blockType;
//                int blockHeight=m_threadStorage.blockHeightMap[heightIndex]+(m_threadStorage.heightMap[heightIndex]*m_threadStorage.blockScaleMap[heightIndex]);// (int)(heightMap[heightIndex]*heightScale)+seaLevel;
//
////                if(position.z > heightMap[heightIndex]) //larger than height map, air
//                if(blockZ>blockHeight)
//                    blockType=0;
//                else
//                {
//                    if(stride == 1)
//                        blockType=getBlockType<false>(blockZ, blockHeight-blockZ, stride);
//                    else
//                        blockType=getBlockType<true>(blockZ, blockHeight-blockZ, stride);
//
////                    if(blockZ<seaLevel)
////                        blockType=1;
////                    blockType=(blockHeight-blockZ)/13+2;
//                    if(blockType!=0)
//                        validCells++;
//                }
//
//                type(cells[index])=blockType;
//                index++;
//                heightIndex++;
//                position.x+=noiseScale;
//            }
//            position.y+=noiseScale;
//        }
//        position.z+=noiseScale;
//    }
//
    return validCells;
////    chunk->setValidCellCount(validCells);
////    return chunk;
}


unsigned int EquiRectWorldGenerator::generateRegion(const glm::vec3 &startPos, const glm::ivec3 &regionSize, void *buffer, size_t bufferSize, size_t lod)
{
//    if(!m_threadStorage.regionVectorSet)
//        m_threadStorage.regionVectorSet=std::make_unique<HastyNoise::VectorSet>(m_simdLevel);
//
//    size_t stride=glm::pow(2u, (unsigned int)lod);
//    glm::ivec2 lodSize=glm::ivec2(regionSize.x, regionSize.y)/(int)stride;
//
//    typename _Region, typename _Chunk::RegionHandleType::Cell *cells=(typename _Region, typename _Chunk::RegionHandleType::Cell *)buffer;
//    assert(bufferSize>=(lodSize.x*lodSize.y)*sizeof(typename _Region, typename _Chunk::RegionHandleType::Cell));
//
//    int heightMapSize=HastyNoise::AlignedSize(lodSize.x*lodSize.y, m_simdLevel);
//
//    m_threadStorage.regionHeightMap.resize(heightMapSize);
//    m_threadStorage.regionVectorSet->SetSize(heightMapSize);
//
//    size_t index=0;
//    glm::vec3 mapPos;
//    glm::ivec3 size=m_descriptors.getSize();
//    float heightScale=((float)size.z/2.0f);
//    int seaLevel=(size.z/2);
    unsigned int validCells=0;
//
//    mapPos.z=(float)size.x/2.0f;
//    for(int y=0; y<regionSize.y; y+=stride)
//    {
//        mapPos.y=startPos.y+y;
//        for(int x=0; x<regionSize.x; x+=stride)
//        {
//            mapPos.x=startPos.x+x;
//
//            glm::vec3 pos=getCylindricalCoords(size.x, size.y, mapPos);
//
//            m_threadStorage.regionVectorSet->xSet[index]=pos.x;
//            m_threadStorage.regionVectorSet->ySet[index]=pos.y;
//            m_threadStorage.regionVectorSet->zSet[index]=pos.z;
//            index++;
//        }
//    }
//
//    m_continentPerlin->FillSet(m_threadStorage.regionHeightMap.data(), m_threadStorage.regionVectorSet.get());
//
//    index=0;
//    for(int y=0; y<regionSize.y; y+=stride)
//    {
//        for(int x=0; x<regionSize.x; x+=stride)
//        {
//            unsigned int blockType;
//            int blockHeight=(int)(m_threadStorage.regionHeightMap[index]*heightScale)+seaLevel;
//
//            if((blockHeight<startPos.z)||(blockHeight>startPos.z+regionSize.z))
//                blockType=0;
//            else
//            {
//                if(stride==1)
//                    blockType=getBlockType<false>(blockHeight, 0, stride);
//                else
//                    blockType=getBlockType<true>(blockHeight, 0, stride);
//            }
//
//            if(blockType!=0)
//                validCells++;
//
//            type(cells[index])=blockType;
//            height(cells[index])=blockHeight;
//
//            index++;
//        }
//    }
//
    return validCells;
}


int EquiRectWorldGenerator::getBaseHeight(const glm::vec2 &pos)
{
    glm::ivec2 influenceIPos=glm::ivec2(pos)/m_descriptorValues.m_influenceGridSize;
    glm::vec2 influenceOffset=pos-glm::vec2(influenceIPos*m_descriptorValues.m_influenceGridSize);
    size_t influenceIndex=((influenceIPos.y*m_descriptorValues.m_influenceSize.x)+influenceIPos.x);
    float *neighborHeight=&m_influenceNeighborMap[influenceIndex*NeighborCount];

    glm::vec2 influencePos=influenceOffset/glm::vec2(m_descriptorValues.m_influenceGridSize);
    glm::ivec3 size=m_descriptors.getSize();

    float scale=1.0f/3.0f;
    float heightBase=bi_lerp(neighborHeight[0], neighborHeight[1], neighborHeight[2], neighborHeight[3], influencePos.x, influencePos.y);

    return heightBase*(float)size.z;
}

//
//unsigned int EquiRectWorldGenerator::generateHeightMap(const glm::vec3 &startPos, const glm::ivec3 &regionSize, void *buffer, size_t bufferSize, size_t lod)
//{
//    glm::vec3 scaledOffset=startPos*m_descriptorValues.m_noiseScale;
//    glm::vec3 position=scaledOffset;
//
//    HeightMapCell *cells=(HeightMapCell *)buffer;
//
//    size_t stride=glm::pow(2u, (unsigned int)lod);
//    glm::ivec3 lodSize=regionSize/(int)stride;
//    float noiseScale=m_descriptorValues.m_noiseScale/stride;
//
//    //verify chunkSize matches template chunk size
//    assert(chunkSize==glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value));
//
//    //verify buffer is large enough for data
//    assert(bufferSize>=(lodSize.x*lodSize.y*lodSize.z)*sizeof(HeightMapCell));
//
//    buildHeightMap(startPos, lodSize, stride);
//
//    size_t index=0;
//    int heightIndex=0;
//    position.y=scaledOffset.y;
//
//    int blockZ=(int)startPos.z+z;
//    for(int y=0; y<ChunkType::sizeY::value; y+=stride)
//    {
//        position.x=scaledOffset.x;
//        for(int x=0; x<ChunkType::sizeX::value; x+=stride)
//        {
//            unsigned int blockType;
//            int blockHeight=(int)(heightMap[heightIndex]*heightScale)+seaLevel;
//
//            if(stride==1)
//                blockType=getBlockType<false>(blockHeight, 0, stride);
//            else
//                blockType=getBlockType<true>(blockHeight, 0, stride);
//
//            if(blockType!=0)
//                validCells++;
//
//            cells[index].type=blockType;
//            cells[index].height=blockHeight;
//
//            index++;
//            heightIndex++;
//            position.x+=noiseScale;
//        }
//        position.y+=noiseScale;
//    }
//}


void EquiRectWorldGenerator::buildHeightMap(const glm::vec3 &startPos, const glm::ivec3 &lodSize, size_t stride)
{
//    int heightMapSize=HastyNoise::AlignedSize(lodSize.x*lodSize.y, m_simdLevel);
//
//    m_threadStorage.heightMap.resize(heightMapSize);
//    m_threadStorage.xMap.resize(heightMapSize);
//    m_threadStorage.yMap.resize(heightMapSize);
//    m_threadStorage.zMap.resize(heightMapSize);
//
//    if(m_threadStorage.vectorSet->size !=heightMapSize)
//        m_threadStorage.vectorSet->SetSize(heightMapSize);
//
//    size_t index=0;
//    glm::vec3 mapPos;
//    glm::ivec3 size=m_descriptors.getSize();
//
//    mapPos.z=(float)size.x/2.0f;
//    for(int y=0; y<ChunkType::sizeY::value; y+=stride)
//    {
//        mapPos.y=startPos.y+y;
//        for(int x=0; x<ChunkType::sizeX::value; x+=stride)
//        {
//            mapPos.x=startPos.x+x;
//
//            glm::vec3 pos=getCylindricalCoords(size.x, size.y, mapPos);
//
//            m_threadStorage.vectorSet->xSet[index]=pos.x;
//            m_threadStorage.vectorSet->ySet[index]=pos.y;
//            m_threadStorage.vectorSet->zSet[index]=pos.z;
//            index++;
//        }
//    }
//
//    m_continentPerlin->FillSet(m_threadStorage.heightMap.data(), m_threadStorage.vectorSet.get());
////    m_continentPerlin->FillNoiseSetMap(heightMap.data(), xMap.data(), yMap.data(), zMap.data(), lodSize.x, lodSize.y, 1);
}

//
//void EquiRectWorldGenerator::generate()
//{
//
//}


void EquiRectWorldGenerator::updateInfluenceNeighbors()
{
    if(m_influenceNeighborMap.size()/NeighborCount!= m_influenceMap.size())
        m_influenceNeighborMap.resize(m_influenceMap.size()*NeighborCount);

    glm::ivec2 influenceSize=m_descriptorValues.m_influenceSize;
    glm::ivec2 influencePos(0, 0);
    size_t index=0;

    for(influencePos.y=0; influencePos.y<influenceSize.y; ++influencePos.y)
    {
        influencePos.x=0;
        for(influencePos.x=0; influencePos.x<influenceSize.x; ++influencePos.x)
        {
            std::vector<size_t> neighborIndexes=get2DCellNeighbors_eq(influencePos, influenceSize);

            float neighborHeight[9];
            float *neighborHeightMap=&m_influenceNeighborMap[index];
            
            for(size_t i=0; i<9; ++i)
            {
                neighborHeight[i]=m_influenceMap[neighborIndexes[i]].heightBase;
            }

            //TODO: could be replaced with (neighborHeight[0]+neighborHeight[1]+neighborHeight[3]+neighborHeight[4])*0.25f
            neighborHeightMap[0]=bi_lerp(neighborHeight[0], neighborHeight[1], neighborHeight[3], neighborHeight[4], 0.5f, 0.5f);
            neighborHeightMap[1]=bi_lerp(neighborHeight[1], neighborHeight[2], neighborHeight[4], neighborHeight[5], 0.5f, 0.5f);
            neighborHeightMap[2]=bi_lerp(neighborHeight[3], neighborHeight[4], neighborHeight[6], neighborHeight[7], 0.5f, 0.5f);
            neighborHeightMap[3]=bi_lerp(neighborHeight[4], neighborHeight[5], neighborHeight[7], neighborHeight[8], 0.5f, 0.5f);

            index+=NeighborCount;
        }
    }

}

}//namespace worldgen

