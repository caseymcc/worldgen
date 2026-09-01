#include "worldgen/worldgen_c.h"
#include "worldgen/generators/equiRectWorldGenerator.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{

const uint32_t WG_MAGIC=0x4E454757u;   //"WGEN"
const uint32_t WG_VERSION=1u;

std::string g_error;

void setError(const std::string &why)
{
    g_error=why;
}

//Fowler-Noll-Vo over the body, so a file that says one world and holds another is refused rather
//than quietly believed.
uint32_t hashBytes(const void *data, size_t size, uint32_t seed)
{
    const unsigned char *bytes=(const unsigned char *)data;
    uint32_t hash=seed?seed:2166136261u;

    for(size_t i=0; i<size; ++i)
    {
        hash^=bytes[i];
        hash*=16777619u;
    }

    return hash;
}

void copyName(char *out, size_t size, const std::string &from)
{
    if(size==0)
        return;

    size_t take=(from.size()<(size-1))?from.size():(size-1);

    memcpy(out, from.data(), take);
    out[take]='\0';
}

}//unnamed namespace

//The handle. A caller never sees inside it, which is the point: nothing about the layout of a
//std::vector or a std::string escapes this file.
struct wgWorld
{
    worldgen::EquiRectWorldGenerator m_generator;
    worldgen::WorldDescriptors m_descriptors;
    int32_t m_seed;
};

//--- keeping it ---

namespace
{

struct FileHeader
{
    uint32_t m_magic;
    uint32_t m_version;
    uint32_t m_cellBytes;   //refuse a file written by a build whose cell was a different shape
    int32_t m_seed;
    int32_t m_sizeX, m_sizeY, m_sizeZ;
    int32_t m_influenceX, m_influenceY;
    uint32_t m_cellCount;
    uint32_t m_hash;
};

void writeString(std::vector<unsigned char> &body, const std::string &value)
{
    uint32_t length=(uint32_t)value.size();

    body.insert(body.end(), (unsigned char *)&length, ((unsigned char *)&length)+sizeof(length));
    body.insert(body.end(), value.begin(), value.end());
}

bool readString(const std::vector<unsigned char> &body, size_t &at, std::string &value)
{
    uint32_t length=0;

    if((at+sizeof(length))>body.size())
        return false;

    memcpy(&length, &body[at], sizeof(length));
    at+=sizeof(length);

    if((at+length)>body.size())
        return false;

    value.assign((const char *)&body[at], length);
    at+=length;

    return true;
}

template<typename Type>
void writePod(std::vector<unsigned char> &body, const Type &value)
{
    body.insert(body.end(), (const unsigned char *)&value,
        ((const unsigned char *)&value)+sizeof(Type));
}

template<typename Type>
bool readPod(const std::vector<unsigned char> &body, size_t &at, Type &value)
{
    if((at+sizeof(Type))>body.size())
        return false;

    memcpy(&value, &body[at], sizeof(Type));
    at+=sizeof(Type);

    return true;
}

}//unnamed namespace

extern "C"
{

wgWorld *wgWorldCreate(int32_t seed, int32_t sizeX, int32_t sizeY, int32_t sizeZ)
{
    setError("");

    wgWorld *world=new wgWorld();
    worldgen::Progress progress;

    if(sizeX<=0)
        sizeX=1048576;
    if(sizeY<=0)
        sizeY=524288;
    if(sizeZ<=0)
        sizeZ=1024;

    world->m_seed=seed;
    world->m_descriptors.setSize({sizeX, sizeY, sizeZ});
    world->m_descriptors.setRegionSize({16, 16, 16});
    world->m_descriptors.setChunkSize({64, 64, 16});
    world->m_generator.getDecriptors().seed=seed;
    world->m_generator.create(&world->m_descriptors, progress);

    return world;
}

void wgWorldDestroy(wgWorld *world)
{
    delete world;
}

void wgInfluenceSize(const wgWorld *world, int32_t *x, int32_t *y)
{
    if(!world)
        return;

    const glm::ivec2 &size=((wgWorld *)world)->m_generator.getInfluenceMapSize();

    if(x)
        *x=size.x;
    if(y)
        *y=size.y;
}

int32_t wgSeed(const wgWorld *world)
{
    return world?world->m_seed:0;
}

int32_t wgCellCount(const wgWorld *world)
{
    return world?(int32_t)((wgWorld *)world)->m_generator.getInfluenceMap().size():0;
}

static void fillCell(const worldgen::InfluenceCell &from, const worldgen::EquiRectDescriptors &d,
    wgCell &to)
{
    memset(&to, 0, sizeof(to));

    to.height=from.heightBase;
    to.elevation=worldgen::heightToElevation(from.heightBase, 0.5f, d.m_maxElevation, d.m_elevationCurve);
    to.temperature=from.temperature;
    to.moisture=from.moisture;

    to.climate=(int32_t)from.climate;
    to.coast=from.coast;
    to.rock=(int32_t)from.rock;
    to.deposit=(int32_t)from.deposit;
    to.fuel=(int32_t)from.fuel;
    to.water=(int32_t)from.water;

    to.riverDischarge=from.riverDischarge;
    to.riverWidth=from.riverWidth;
    to.drainageArea=from.drainageArea;

    to.forest=from.forest?1:0;
    to.depositExposed=from.depositExposed?1:0;
    to.road=from.road?1:0;
    to.onTheBeatenTrack=from.onTheBeatenTrack?1:0;

    to.polity=from.polity;
    to.culturalRegion=from.culturalRegion;
    to.settlement=from.settlement;
    to.dominantSpecies=from.dominantSpecies;
    to.habitability=from.habitability;
    to.control=from.control;
    to.remoteness=from.remoteness;
    to.traffic=from.traffic;
    to.barrier=from.barrier;
    to.ruinKind=from.ruinKind;
}

int32_t wgReadCells(const wgWorld *world, wgCell *out, int32_t max)
{
    if(!world)
        return 0;

    const worldgen::EquiRectWorldGenerator::InfluenceMap &cells=
        ((wgWorld *)world)->m_generator.getInfluenceMap();

    if(!out)
        return (int32_t)cells.size();

    const worldgen::EquiRectDescriptors &d=((wgWorld *)world)->m_generator.getDecriptors();
    int32_t count=((int32_t)cells.size()<max)?(int32_t)cells.size():max;

    for(int32_t i=0; i<count; ++i)
        fillCell(cells[i], d, out[i]);

    return count;
}

int32_t wgReadCell(const wgWorld *world, int32_t x, int32_t y, wgCell *out)
{
    if((!world)||(!out))
        return 0;

    const glm::ivec2 &size=((wgWorld *)world)->m_generator.getInfluenceMapSize();

    //the map joins up in x and stops at the poles
    x=x%size.x;

    if(x<0)
        x+=size.x;

    if((y<0)||(y>=size.y))
        return 0;

    const worldgen::EquiRectWorldGenerator::InfluenceMap &cells=
        ((wgWorld *)world)->m_generator.getInfluenceMap();

    fillCell(cells[((size_t)y*size.x)+x], ((wgWorld *)world)->m_generator.getDecriptors(), *out);

    return 1;
}

int32_t wgSettlementCount(const wgWorld *world)
{
    return world?(int32_t)((wgWorld *)world)->m_generator.getSettlements().size():0;
}

int32_t wgReadSettlements(const wgWorld *world, wgSettlement *out, int32_t max)
{
    if(!world)
        return 0;

    const std::vector<worldgen::Settlement> &from=((wgWorld *)world)->m_generator.getSettlements();

    if(!out)
        return (int32_t)from.size();

    const glm::ivec2 &size=((wgWorld *)world)->m_generator.getInfluenceMapSize();
    int32_t count=((int32_t)from.size()<max)?(int32_t)from.size():max;

    for(int32_t i=0; i<count; ++i)
    {
        memset(&out[i], 0, sizeof(wgSettlement));

        out[i].cellX=(int32_t)(from[i].m_cell%size.x);
        out[i].cellY=(int32_t)(from[i].m_cell/size.x);
        out[i].kind=(int32_t)from[i].m_kind;
        out[i].species=from[i].m_species;
        out[i].polity=from[i].m_polity;
        out[i].population=(uint32_t)from[i].m_population;
        out[i].interest=from[i].m_interest;

        copyName(out[i].name, WG_NAME_MAX, from[i].m_name);
        copyName(out[i].reason, sizeof(out[i].reason), from[i].m_reason);
    }

    return count;
}

int32_t wgPolityCount(const wgWorld *world)
{
    return world?(int32_t)((wgWorld *)world)->m_generator.getPolities().size():0;
}

int32_t wgReadPolities(const wgWorld *world, wgPolity *out, int32_t max)
{
    if(!world)
        return 0;

    const std::vector<worldgen::Polity> &from=((wgWorld *)world)->m_generator.getPolities();

    if(!out)
        return (int32_t)from.size();

    const glm::ivec2 &size=((wgWorld *)world)->m_generator.getInfluenceMapSize();
    int32_t count=((int32_t)from.size()<max)?(int32_t)from.size():max;

    for(int32_t i=0; i<count; ++i)
    {
        memset(&out[i], 0, sizeof(wgPolity));

        out[i].cellX=(int32_t)(from[i].m_core%size.x);
        out[i].cellY=(int32_t)(from[i].m_core/size.x);
        out[i].species=from[i].m_species;
        out[i].mountainCore=from[i].m_mountainCore?1:0;
        out[i].cells=(uint32_t)from[i].m_cells;
        out[i].area=from[i].m_area;
        out[i].marchCells=(uint32_t)from[i].m_marchCells;
        out[i].meanControl=from[i].m_meanControl;
        out[i].enclave=from[i].m_enclave?1:0;

        copyName(out[i].name, WG_NAME_MAX, from[i].m_name);
    }

    return count;
}

int32_t wgCityCount(const wgWorld *world)
{
    return world?(int32_t)((wgWorld *)world)->m_generator.getCities().size():0;
}

int32_t wgReadCities(const wgWorld *world, wgCity *out, int32_t max)
{
    if(!world)
        return 0;

    const std::vector<worldgen::City> &from=((wgWorld *)world)->m_generator.getCities();

    if(!out)
        return (int32_t)from.size();

    const glm::ivec2 &size=((wgWorld *)world)->m_generator.getInfluenceMapSize();
    int32_t count=((int32_t)from.size()<max)?(int32_t)from.size():max;

    for(int32_t i=0; i<count; ++i)
    {
        memset(&out[i], 0, sizeof(wgCity));

        out[i].cellX=(int32_t)(from[i].m_cell%size.x);
        out[i].cellY=(int32_t)(from[i].m_cell/size.x);
        out[i].node=(int32_t)from[i].m_node;
        out[i].polity=from[i].m_polity;
        out[i].parasite=from[i].m_parasite?1:0;
        out[i].size=from[i].m_size;
        out[i].water=from[i].m_water;
        out[i].food=from[i].m_food;
        out[i].river=from[i].m_river;

        copyName(out[i].reason, sizeof(out[i].reason), from[i].m_reason);
    }

    return count;
}

int32_t wgRoadCount(const wgWorld *world)
{
    return world?(int32_t)((wgWorld *)world)->m_generator.getRoads().size():0;
}

int32_t wgReadRoads(const wgWorld *world, wgRoad *out, int32_t max)
{
    if(!world)
        return 0;

    const std::vector<worldgen::Road> &from=((wgWorld *)world)->m_generator.getRoads();

    if(!out)
        return (int32_t)from.size();

    const std::vector<worldgen::Settlement> &places=((wgWorld *)world)->m_generator.getSettlements();
    const glm::ivec2 &size=((wgWorld *)world)->m_generator.getInfluenceMapSize();
    int32_t count=((int32_t)from.size()<max)?(int32_t)from.size():max;

    for(int32_t i=0; i<count; ++i)
    {
        memset(&out[i], 0, sizeof(wgRoad));

        if((from[i].m_from<places.size())&&(from[i].m_to<places.size()))
        {
            out[i].fromCellX=(int32_t)(places[from[i].m_from].m_cell%size.x);
            out[i].fromCellY=(int32_t)(places[from[i].m_from].m_cell/size.x);
            out[i].toCellX=(int32_t)(places[from[i].m_to].m_cell%size.x);
            out[i].toCellY=(int32_t)(places[from[i].m_to].m_cell/size.x);
        }

        out[i].cost=from[i].m_cost;
        out[i].length=(uint32_t)from[i].m_length;
    }

    return count;
}

int32_t wgFindFoundingSites(wgWorld *world, int32_t species, int32_t spacing,
    wgFoundingSite *out, int32_t max)
{
    if((!world)||(!out)||(max<=0))
        return 0;

    std::vector<worldgen::FoundingSite> sites;

    world->m_generator.findFoundingSites(species, max, spacing, sites);

    const glm::ivec2 &size=world->m_generator.getInfluenceMapSize();
    int32_t count=((int32_t)sites.size()<max)?(int32_t)sites.size():max;

    for(int32_t i=0; i<count; ++i)
    {
        memset(&out[i], 0, sizeof(wgFoundingSite));

        out[i].cellX=(int32_t)(sites[i].m_cell%size.x);
        out[i].cellY=(int32_t)(sites[i].m_cell/size.x);
        out[i].interest=sites[i].m_interest;
        out[i].polity=sites[i].m_polity;
        out[i].nearestSettlement=sites[i].m_nearestSettlement;
        out[i].neighbours=(uint32_t)sites[i].m_neighbours;
        out[i].neighbourPopulation=(uint32_t)sites[i].m_neighbourPopulation;

        copyName(out[i].reason, sizeof(out[i].reason), sites[i].m_reason);
        copyName(out[i].character, sizeof(out[i].character), sites[i].m_character);
    }

    return count;
}

//--- keeping it ---

int32_t wgWorldSave(const wgWorld *world, const char *path)
{
    setError("");

    if((!world)||(!path))
    {
        setError("nothing to save, or nowhere to save it");
        return 0;
    }

    worldgen::EquiRectWorldGenerator &generator=((wgWorld *)world)->m_generator;
    const worldgen::EquiRectWorldGenerator::InfluenceMap &cells=generator.getInfluenceMap();
    std::vector<unsigned char> body;

    //The cells go down whole - they are trivially copyable, and the header records how big one is
    //so a build whose cell has changed shape refuses the file rather than misreading it.
    body.insert(body.end(), (const unsigned char *)cells.data(),
        ((const unsigned char *)cells.data())+(cells.size()*sizeof(worldgen::InfluenceCell)));

    const std::vector<worldgen::Settlement> &settlements=generator.getSettlements();

    writePod(body, (uint32_t)settlements.size());

    for(size_t i=0; i<settlements.size(); ++i)
    {
        writePod(body, (uint32_t)settlements[i].m_cell);
        writePod(body, (int32_t)settlements[i].m_kind);
        writePod(body, (int32_t)settlements[i].m_species);
        writePod(body, (int32_t)settlements[i].m_polity);
        writePod(body, settlements[i].m_interest);
        writePod(body, (uint32_t)settlements[i].m_population);
        writeString(body, settlements[i].m_name);
        writeString(body, settlements[i].m_reason);
    }

    const std::vector<worldgen::Road> &roads=generator.getRoads();

    writePod(body, (uint32_t)roads.size());

    for(size_t i=0; i<roads.size(); ++i)
    {
        writePod(body, (uint32_t)roads[i].m_from);
        writePod(body, (uint32_t)roads[i].m_to);
        writePod(body, roads[i].m_cost);
        writePod(body, (uint32_t)roads[i].m_length);
    }

    FileHeader header;

    memset(&header, 0, sizeof(header));
    header.m_magic=WG_MAGIC;
    header.m_version=WG_VERSION;
    header.m_cellBytes=(uint32_t)sizeof(worldgen::InfluenceCell);
    header.m_seed=((wgWorld *)world)->m_seed;

    glm::ivec3 size=((wgWorld *)world)->m_descriptors.getSize();

    header.m_sizeX=size.x;
    header.m_sizeY=size.y;
    header.m_sizeZ=size.z;

    const glm::ivec2 &influence=generator.getInfluenceMapSize();

    header.m_influenceX=influence.x;
    header.m_influenceY=influence.y;
    header.m_cellCount=(uint32_t)cells.size();
    header.m_hash=hashBytes(body.data(), body.size(), 0);

    FILE *file=fopen(path, "wb");

    if(!file)
    {
        setError(std::string("could not open ")+path+" to write");
        return 0;
    }

    bool ok=(fwrite(&header, sizeof(header), 1, file)==1);

    if(ok&&(!body.empty()))
        ok=(fwrite(body.data(), 1, body.size(), file)==body.size());

    fclose(file);

    if(!ok)
    {
        setError("the world did not write completely");
        return 0;
    }

    return 1;
}

wgWorld *wgWorldLoad(const char *path)
{
    setError("");

    if(!path)
    {
        setError("no path given");
        return 0;
    }

    FILE *file=fopen(path, "rb");

    if(!file)
    {
        setError(std::string("could not open ")+path);
        return 0;
    }

    FileHeader header;

    if(fread(&header, sizeof(header), 1, file)!=1)
    {
        fclose(file);
        setError("that file is too short to be a world");
        return 0;
    }

    if(header.m_magic!=WG_MAGIC)
    {
        fclose(file);
        setError("that is not a worldgen file");
        return 0;
    }

    if(header.m_version!=WG_VERSION)
    {
        fclose(file);
        setError("that world was written by a different version");
        return 0;
    }

    if(header.m_cellBytes!=(uint32_t)sizeof(worldgen::InfluenceCell))
    {
        fclose(file);
        setError("that world was written by a build whose cell was a different shape");
        return 0;
    }

    std::vector<unsigned char> body;

    {
        long at=ftell(file);

        fseek(file, 0, SEEK_END);

        long end=ftell(file);

        fseek(file, at, SEEK_SET);
        body.resize((size_t)(end-at));

        if((!body.empty())&&(fread(body.data(), 1, body.size(), file)!=body.size()))
        {
            fclose(file);
            setError("that world is truncated");
            return 0;
        }
    }

    fclose(file);

    if(hashBytes(body.data(), body.size(), 0)!=header.m_hash)
    {
        setError("that world does not hash to what it says it does");
        return 0;
    }

    size_t needed=(size_t)header.m_cellCount*sizeof(worldgen::InfluenceCell);

    if(body.size()<needed)
    {
        setError("that world is missing cells");
        return 0;
    }

    //Rebuilt without generating: the whole reason a world is written down is that plate tectonics
    //and drainage are whole-world computations, and that the noise underneath is not promised to be
    //identical on two different processors.
    wgWorld *world=new wgWorld();

    world->m_seed=header.m_seed;
    world->m_descriptors.setSize({header.m_sizeX, header.m_sizeY, header.m_sizeZ});
    world->m_descriptors.setRegionSize({16, 16, 16});
    world->m_descriptors.setChunkSize({64, 64, 16});
    world->m_generator.getDecriptors().seed=header.m_seed;
    world->m_generator.loadDescriptors(&world->m_descriptors);

    worldgen::EquiRectWorldGenerator::InfluenceMap &cells=world->m_generator.getInfluenceMapForWrite();

    cells.resize(header.m_cellCount);
    memcpy(cells.data(), body.data(), needed);

    size_t at=needed;
    uint32_t count=0;

    std::vector<worldgen::Settlement> &settlements=world->m_generator.getSettlementsForWrite();

    settlements.clear();

    if(readPod(body, at, count))
    {
        for(uint32_t i=0; i<count; ++i)
        {
            worldgen::Settlement settlement;
            uint32_t cell=0;
            int32_t kind=0;
            uint32_t population=0;

            if(!readPod(body, at, cell)) break;
            if(!readPod(body, at, kind)) break;
            if(!readPod(body, at, settlement.m_species)) break;
            if(!readPod(body, at, settlement.m_polity)) break;
            if(!readPod(body, at, settlement.m_interest)) break;
            if(!readPod(body, at, population)) break;
            if(!readString(body, at, settlement.m_name)) break;
            if(!readString(body, at, settlement.m_reason)) break;

            settlement.m_cell=cell;
            settlement.m_kind=(worldgen::SettlementKind)kind;
            settlement.m_population=population;

            settlements.push_back(settlement);
        }
    }

    std::vector<worldgen::Road> &roads=world->m_generator.getRoadsForWrite();

    roads.clear();

    if(readPod(body, at, count))
    {
        for(uint32_t i=0; i<count; ++i)
        {
            worldgen::Road road;
            uint32_t from=0;
            uint32_t to=0;
            uint32_t length=0;

            if(!readPod(body, at, from)) break;
            if(!readPod(body, at, to)) break;
            if(!readPod(body, at, road.m_cost)) break;
            if(!readPod(body, at, length)) break;

            road.m_from=from;
            road.m_to=to;
            road.m_length=length;

            roads.push_back(road);
        }
    }

    world->m_generator.rebuildAfterLoad();

    return world;
}

const char *wgLastError(void)
{
    return g_error.c_str();
}

}//extern "C"
