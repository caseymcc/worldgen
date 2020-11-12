#include "worldgen/generators/equiRectWorldGenerator.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <glm/gtx/projection.hpp>

#include <FastNoise/FastNoise.h>

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

    fs::Type *file=fs::open(fileName, "rb");

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

    fs::Type *file=fs::open(fileName, "wb");

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

    fs::Type *file=fs::open(fileName, "rb");

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

    fs::Type *file=fs::open(fileName, "wb");

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


void EquiRectWorldGenerator::generatePlates(Progress &progress)
{

    std::vector<WeatherCell> weatherCells=
    {
//        {"South Polar Cell"  , rads(-75.0f), rads(30.0f), 0.65f, { 0.0f,  1.0f}, {-1.0f,  0.0f}},
//        {"South Ferrell Cell", rads(-45.0f), rads(30.0f), 0.75f , { 1.0f,  0.0f}, { 0.0f, -1.0f}},
//        {"South Hadley Cell" , rads(-15.0f), rads(30.0f), 0.95f , { 0.0f,  1.0f}, {-1.0f,  0.0f}},
//        {"Noth Hadley Cell"  , rads(15.0f) , rads(30.0f), 0.95f , {-1.0f,  0.0f}, { 0.0f, -1.0f}},
//        {"North Ferrell Cell", rads(45.0f) , rads(30.0f), 0.74f , { 0.0f,  1.0f}, { 1.0f,  0.0f}},
//        {"North Polar Cell"  , rads(75.0f) , rads(30.0f), 0.65f, {-1.0f,  0.0f}, { 0.0f, -1.0f}}
        {"South Polar Cell"  , rads(-90.0f), rads(-60.0f), 0.65f, { 0.0f,  1.0f}, {-1.0f,  0.0f}},
        {"South Ferrell Cell", rads(-60.0f), rads(-30.0f), 0.75f , { 1.0f,  0.0f}, { 0.0f, -1.0f}},
        {"South Hadley Cell" , rads(-30.0f), rads(0.0f), 0.95f , { 0.0f,  1.0f}, {-1.0f,  0.0f}},
        {"Noth Hadley Cell"  , rads(0.0f) , rads(30.0f), 0.95f , {-1.0f,  0.0f}, { 0.0f, -1.0f}},
        {"North Ferrell Cell", rads(30.0f) , rads(60.0f), 0.74f , { 0.0f,  1.0f}, { 1.0f,  0.0f}},
        {"North Polar Cell"  , rads(60.0f) , rads(90.0f), 0.65f, {-1.0f,  0.0f}, { 0.0f, -1.0f}}

    };

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
    m_hidden->m_domainWarp->SetWarpAmplitude(0.5f);
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

    float last=-2.0f;
    std::vector<float> plates;

    //find and index all plates
    for(size_t i=0; i<influenceMapSize; i++)
    {
        if(plateMap[i]==last)
            continue;
        else if(contains_sorted(plates, plateMap[i]))
        {
            last=plateMap[i];
            continue;
        }

        insert_sorted(plates, plateMap[i]);
        last=plateMap[i];
    }

    last=-2.0f;
    size_t lastIndex=0;
    float last2=-2.0f;
    size_t last2Index=0;
    glm::ivec2 point={0, 0};

    //setup plates
    std::default_random_engine generator(m_descriptorValues.seed);
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distributionNorm(0.0f, 1.0f);
    std::vector<float> jitter;
    std::vector<float> plateMinDistance;
    std::vector<float> plateMaxDistance;
    std::vector<glm::ivec2> plateMinPoint;
    std::vector<int> plateNeighbors;
    std::vector<int> plateCollisions;

    plateDetails.resize(plates.size());
    plateNeighbors.resize(plates.size()*plates.size(), 0);
    plateMinDistance.resize(plates.size(), 2.0f);
    plateMaxDistance.resize(plates.size(), -2.0f);
    plateMinPoint.resize(plates.size());

    glm::vec3 zAxis(0.0f, 0.0f, 1.0f);

    progress.update("Generating height map", 50, false);

    for(size_t i=0; i<influenceMapSize; i++)
    {
        if(plateMap[i]!=last)
        {
            lastIndex=index_sorted(plates, plateMap[i]);
            last=plateMap[i];
        }

        if(plate2Map[i]!=last2)
        {
            last2Index=index_sorted(plates, plate2Map[i]);

            if(last2Index==std::numeric_limits<size_t>::max())
            {
                //plate we haven't seen, setting to current plate
                last2Index=lastIndex;
                last2=plateMap[i];
            }
            else
                last2=plate2Map[i];
        }

        if(lastIndex!=last2Index)
        {
            size_t neighborIndex=lastIndex*plates.size()+last2Index;

            if(!plateNeighbors[neighborIndex])
                plateNeighbors[neighborIndex]=1;
        }

        if(plateDistanceMap[i]<plateMinDistance[lastIndex])
        {
            plateMinDistance[lastIndex]=plateDistanceMap[i];
            plateMinPoint[lastIndex]=point;
        }

        if(plateDistanceMap[i]>plateMaxDistance[lastIndex])
            plateMaxDistance[lastIndex]=plateDistanceMap[i];

        assert(last2Index<plates.size());

        m_influenceMap[i].tectonicPlate=lastIndex;
        m_influenceMap[i].borderPlate=last2Index;
        m_influenceMap[i].plateHeight=(last)*0.1f+0.5f;
        m_influenceMap[i].plateValue=last;
        m_influenceMap[i].plateDistanceValue=plateDistanceMap[i];
        m_influenceMap[i].continentValue=continentMap[i];

//        glm::vec2 airDirection(ewAirCurrent[i], nsAirCurrent[i]);
//        
//        m_influenceMap[i].airDirection=glm::normalize(airDirection);
        m_influenceMap[i].airDirection.x=ewAirCurrent[i];
        m_influenceMap[i].airDirection.y=nsAirCurrent[i];

        plateDetails[lastIndex].points.emplace_back(point.x, point.y);

        point.x++;
        if(point.x>=influenceSize.x)
        {
            point.x=0;
            point.y++;
        }
    }

    progress.update("Generating tectonic zones", 55, false);

    for(size_t i=0; i<plateDetails.size(); i++)
    {
        PlateInfo &details=plateDetails[i];

        details.value=plates[i];
        details.point=plateMinPoint[i];

        glm::vec2 mapPoint;
        glm::vec3 point;

        //we want plate heights to be 0.5 to -0.5
//        details.height=(distributionNorm(generator))*0.3f+0.3f;
        details.height=(plates[i])*0.1f+0.5f;
        if(details.height<0.5f)
            details.height=details.height-0.05f;

        //normalize map axis
        mapPoint.x=(float)details.point.x/(float)influenceSize.x;
        mapPoint.y=(float)details.point.y/(float)influenceSize.y;

        projectPoint<Projections::Equirectangular, Projections::Cartesian>(mapPoint, point);
        details.point3d=point;

        glm::vec3 direction;
        glm::vec2 tangentPlaneCoords;

        //generate drift vector, building on x/y plane that will be rotate to the point
        //tanget to the sphere
        tangentPlaneCoords.x=distribution(generator);
        tangentPlaneCoords.y=distribution(generator);
        tangentPlaneCoords=glm::normalize(tangentPlaneCoords);

        details.driftDirection=tangentPlaneCoords;
        rotateTangetVectorToPoint(point, tangentPlaneCoords, details.direction);

//        //cross with z axis for a rotation vector to align with the z axis
//        glm::vec3 rotationVector=glm::cross(zAxis, point);
//
//        if(glm::length2(rotationVector)==0.0f)
//        {//already aligned to z axis
//            details.direction=glm::vec3(tangentPlaneCoords.x, tangentPlaneCoords.y, 0.0f);
//        }
//        else
//        {
//            //get angle to rotate vector to position
//            float theta=acos(glm::dot(point, zAxis));
//            glm::mat4 rotationMat=glm::rotate(theta, rotationVector);
//
//            details.direction=rotationMat*glm::vec4(tangentPlaneCoords.x, tangentPlaneCoords.y, 0.0f, 1.0f);
//        }

        size_t neighborIndex=i*plateDetails.size();
        for(size_t j=0; j<plateDetails.size(); j++)
        {
            if(plateNeighbors[neighborIndex+j])
                details.neighbors.push_back(j);
        }
    }

    const float invsqrt2=0.5f/sqrt(2.0f);

    for(size_t i=0; i<plateDetails.size(); i++)
    {
        PlateInfo &details=plateDetails[i];
        size_t neighborIndex=plateDetails.size()*i;

        for(size_t j=0; j<details.neighbors.size(); j++)
        {
            //this is working on the vector in between the center points, it is actually be better to rotate the 
            //vector to the point being worked on and solve from there. Skipping shear for the moment.
            PlateInfo &details2=plateDetails[details.neighbors[j]];

            glm::vec3 collisionVector=details2.point3d-details.point3d;

            glm::vec3 direction=glm::proj(details.direction, collisionVector);
            glm::vec3 direction2=glm::proj(details2.direction, collisionVector);
            
            float pointDistance=glm::distance(details2.point3d, details.point3d);
            float directionDistance=glm::distance((details2.point3d+direction2), (details.point3d+direction));

            float collision=(pointDistance-directionDistance)*invsqrt2; //scale to 0.0 to 1.0

            assert(collision<=1.0f);
            collision=std::min(collision, 1.0f);//make sure between 0.0 and 1.0
            details.neighborCollisions.push_back(collision);
        }
    }


    progress.update("Generating influence map", 60, false);

    point={0, 0};
    for(size_t i=0; i<influenceMapSize; i++)
    {
        size_t &index=m_influenceMap[i].tectonicPlate;
        size_t &borderIndex=m_influenceMap[i].borderPlate;

        PlateInfo &details=plateDetails[index];
        PlateInfo &details2=plateDetails[borderIndex];

        float collision;

        if(!details.neighbors.empty())
        {
            size_t neighborIndex=0;

            for(size_t j=0; j<details.neighbors.size(); ++j)
            {
                if(details.neighbors[j]==borderIndex)
                    neighborIndex=j;
            }
            
            collision=details.neighborCollisions[neighborIndex];
        }
        else
            collision=0.0f;

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
        
        rotateTangetVectorToPoint(cartPoint, details.driftDirection, direction);

        projectVector(direction, sphericalPoint, sphericalDirection);
        m_influenceMap[i].direction.x=sphericalDirection.y;
        m_influenceMap[i].direction.y=sphericalDirection.z;

//air currents determined by banding and random vectors from before
        glm::vec2 latLong(sphericalPoint.y, sphericalPoint.z);
        float latitude=sphericalPoint.z;
        float dir=1.0f;
        glm::vec2 bandDirection;

        if(sphericalPoint.z<0.0f)
            dir=-1.0f;

        m_influenceMap[i].weatherCell=weather.getCellIndex(latLong);
        m_influenceMap[i].weatherBand=weather.getBandIndex(latLong);

        bandDirection=weather.getWindDirection(latLong);
        m_influenceMap[i].airDirection=bandDirection;
//        m_influenceMap[i].airDirection=(bandDirection+m_influenceMap[i].airDirection)/2.0f;

//build terrain
        bool oceanPlate=(details.height<0.5f);
        bool oceanPlate2=(details2.height<0.5f);

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

        if(index != borderIndex)
        {
            m_influenceMap[i].collision=collision;

            if(collision<0.0f) //divergent boundary
            {
                collision=-(collision);//reverse negative as following is expecting collision to be a magnitude
                terrainScale=calculateDivergentCurve(m_influenceMap[i].plateDistanceValue, oceanPlate, oceanPlate2);
            }
            else if(collision>0.0f) //convergent boundary
                terrainScale=calculateConvergentCurve(m_influenceMap[i].plateDistanceValue, oceanPlate, oceanPlate2);
        }
        else
            m_influenceMap[i].collision=0.0f;

        float genHeight=(heightMap[i]+1.0f)*0.05f;
        float genTerrainScale=(terrainScaleMap[i]+1.0f)*0.2f+0.2f;

        m_influenceMap[i].heightBase=((details.height+genHeight)*plateScale)+(details2.height*plate2Scale)+(terrainScale*collision*genTerrainScale);// *0.4f);
        
        m_influenceMap[i].terrainScale=terrainScale;

		if(m_influenceMap[i].heightBase>1.0f)
			m_influenceMap[i].heightBase=1.0f;
		if(m_influenceMap[i].heightBase<0.0f)
			m_influenceMap[i].heightBase=0.0f;

//temperature
        m_influenceMap[i].temperature=getTemperature(latitude);

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
        
        m_influenceMap[i].moisture=std::max(std::min(map1[i]+moistureDeltaMap[i], 1.0f), 0.0f);
    }

    time2=chrono::high_resolution_clock::now();
    processingTime=chrono::duration_cast<chrono::milliseconds>(time2-time1).count();

    m_plateCount=plates.size();

//    updateInfluenceNeighbors();
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

