// Invariants that have each caught a real defect in this generator.
//
// They are written as properties of a generated world rather than as expected values, because the
// world changes every time a parameter moves and a test that pins an exact number would have to be
// edited on every tune - at which point it stops being a test. What is asserted here is what must
// be true of ANY world this generator makes: water leaves the map, a river that enters a region
// leaves it, the same seed gives the same ground, a settlement has something to drink.
//
// Several of these exist because the obvious check was the wrong one:
//  - "every channel sample has a lower neighbour" passed while a fifth of the channel could not
//    drain, because the two banks of a meander are neighbours and are not the same part of a river.
//    What matters is whether water can GET OUT, so that is what is asked.
//  - "every entry connects to the exit through channel cells" failed on a river that reached the
//    sea, because the sample at the mouth is sea rather than channel. It follows water instead.

#include "worldgen/generators/equiRectWorldGenerator.h"
#include "worldgen/worldgen_c.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>

namespace
{

int g_checks=0;
int g_failures=0;
std::string g_group;

void group(const std::string &name)
{
    g_group=name;
    printf("\n%s\n", name.c_str());
}

void check(bool passed, const std::string &what, const std::string &detail)
{
    g_checks++;

    if(passed)
        printf("  ok    %-52s %s\n", what.c_str(), detail.c_str());
    else
    {
        g_failures++;
        printf("  FAIL  %-52s %s\n", what.c_str(), detail.c_str());
    }
}

std::string say(const char *format, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    return std::string(buffer);
}

const int g_offsets[8][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

int wrap(int value, int size)
{
    value=value%size;

    if(value<0)
        value+=size;

    return value;
}

}//unnamed namespace

using namespace worldgen;

//--- the overview ---

static void testRivers(EquiRectWorldGenerator &g, int seed)
{
    const EquiRectWorldGenerator::InfluenceMap &m=g.getInfluenceMap();
    const glm::ivec2 &s=g.getInfluenceMapSize();
    size_t deadEnd=0;
    size_t looped=0;

    for(size_t i=0; i<m.size(); ++i)
    {
        if((m[i].heightBase<0.5f)||(m[i].water!=WaterBody::River))
            continue;

        //water that arrives somewhere and cannot leave is the defect this check exists for
        if(m[i].flowDirection<0)
        {
            deadEnd++;
            continue;
        }

        //and water that goes round for ever is the other one
        size_t at=i;

        for(int step=0; step<4096; ++step)
        {
            const InfluenceCell &cell=m[at];

            if((cell.flowDirection<0)||(cell.heightBase<0.5f))
                break;

            int x=(int)(at%s.x);
            int y=(int)(at/s.x);
            int ny=y+g_offsets[cell.flowDirection][1];

            if((ny<0)||(ny>=s.y))
                break;

            at=((size_t)ny*s.x)+wrap(x+g_offsets[cell.flowDirection][0], s.x);

            if(at==i)
            {
                looped++;
                break;
            }
        }
    }

    check(deadEnd==0, "river water always has somewhere to go", say("%zu dead end(s)", deadEnd));
    check(looped==0, "no river flows in a circle", say("%zu loop(s)", looped));
}

static void testClimate(EquiRectWorldGenerator &g, int seed)
{
    const EquiRectWorldGenerator::InfluenceMap &m=g.getInfluenceMap();
    const glm::ivec2 &s=g.getInfluenceMapSize();

    //The continental zones are defined by summer warmth, so they must stack in latitude order.
    //Getting this wrong is not visible on the map: it looks like weather.
    double sum[3]={0.0, 0.0, 0.0};
    size_t count[3]={0, 0, 0};

    for(size_t i=0; i<m.size(); ++i)
    {
        int band=-1;

        if(m[i].climate==ClimateZone::Dfa) band=0;
        else if(m[i].climate==ClimateZone::Dfb) band=1;
        else if(m[i].climate==ClimateZone::Dfc) band=2;

        if(band<0)
            continue;

        sum[band]+=fabs(90.0-(180.0*(((double)(i/s.x)+0.5)/(double)s.y)));
        count[band]++;
    }

    bool present=(count[0]>0)&&(count[1]>0)&&(count[2]>0);

    if(!present)
    {
        //An absent zone is untested, not passed. Raising a threshold until a zone vanishes is how
        //an ordering check gets "fixed" without fixing anything.
        check(true, "continental zones stack poleward (absent, untested)",
            say("Dfa %zu Dfb %zu Dfc %zu", count[0], count[1], count[2]));
        return;
    }

    double a=sum[0]/count[0];
    double b=sum[1]/count[1];
    double c=sum[2]/count[2];

    check((a<b)&&(b<c), "continental zones stack poleward",
        say("Dfa %.0f -> Dfb %.0f -> Dfc %.0f degrees", a, b, c));
}

static void testSettlements(EquiRectWorldGenerator &g, int seed)
{
    const EquiRectWorldGenerator::InfluenceMap &m=g.getInfluenceMap();
    const std::vector<Settlement> &places=g.getSettlements();

    check(!places.empty(), "a world is settled", say("%zu place(s)", places.size()));

    if(places.empty())
        return;

    //The hard veto. No amount of ore makes up for having nothing to drink, and a settlement without
    //water is the one thing the interest model must never produce.
    size_t thirsty=0;
    size_t drowned=0;

    for(size_t i=0; i<places.size(); ++i)
    {
        if(siteWater(m[places[i].m_cell])<=0.02f)
            thirsty++;
        if(m[places[i].m_cell].heightBase<0.5f)
            drowned++;
    }

    check(thirsty==0, "every settlement has water within reach", say("%zu without", thirsty));
    check(drowned==0, "no settlement stands in the sea", say("%zu at sea", drowned));

    //The weights have to actually discriminate. A town wants a road far more than a steading does
    //(0.70 against 0.05), so if the two end up equally far from the network the weighting is doing
    //nothing and the model is an expensive way to scatter points.
    //
    //Measured on REMOTENESS rather than on the onTheBeatenTrack flag. The flag covers a third of
    //all land, so both kinds are usually inside it and the comparison came out 73% against 74% on
    //one world in four - which reads as the weight not working when it is the metric throwing the
    //signal away. On the continuous field the ordering holds on every world tried.
    double remoteness[5]={0.0, 0.0, 0.0, 0.0, 0.0};
    size_t total[5]={0, 0, 0, 0, 0};

    for(size_t i=0; i<places.size(); ++i)
    {
        int kind=(int)places[i].m_kind;

        remoteness[kind]+=m[places[i].m_cell].remoteness;
        total[kind]++;
    }

    if((total[(int)SettlementKind::Town]>0)&&(total[(int)SettlementKind::Steading]>0))
    {
        double town=remoteness[(int)SettlementKind::Town]/total[(int)SettlementKind::Town];
        double steading=remoteness[(int)SettlementKind::Steading]/total[(int)SettlementKind::Steading];

        check(town<steading, "towns stand nearer the network than steadings",
            say("%.4f against %.4f", town, steading));
    }
}

static void testRoads(EquiRectWorldGenerator &g, int seed)
{
    const EquiRectWorldGenerator::InfluenceMap &m=g.getInfluenceMap();
    const std::vector<Settlement> &places=g.getSettlements();

    if(places.empty())
        return;

    size_t connected=0;

    for(size_t i=0; i<places.size(); ++i)
    {
        if(m[places[i].m_cell].road)
            connected++;
    }

    check((100*connected/places.size())>=90, "settlements are joined to the network",
        say("%zu%% on a road", 100*connected/places.size()));

    //A road is not a boat. Reusing the trade layer's cost put a quarter of the road network in the
    //riverbed, because trade discounts water and a cart cannot use it.
    size_t land=0;
    size_t river=0;
    size_t road=0;
    size_t roadOnRiver=0;

    for(size_t i=0; i<m.size(); ++i)
    {
        if(m[i].heightBase<0.5f)
            continue;

        land++;

        bool wet=(m[i].water==WaterBody::River);

        if(wet)
            river++;

        if(m[i].road)
        {
            road++;

            if(wet)
                roadOnRiver++;
        }
    }

    if((road>0)&&(land>0))
    {
        double onRiver=100.0*roadOnRiver/road;
        double baseRate=100.0*river/land;

        check(onRiver<(baseRate*1.6), "roads cross rivers rather than follow them",
            say("%.0f%% of road on river against %.0f%% of land", onRiver, baseRate));
    }
}

static void testNamesAndLand(EquiRectWorldGenerator &g, int seed)
{
    const EquiRectWorldGenerator::InfluenceMap &m=g.getInfluenceMap();
    const glm::ivec2 &s=g.getInfluenceMapSize();
    const std::vector<Settlement> &places=g.getSettlements();

    if(places.empty())
        return;

    //A name has to be a name, and two places must not share one, or a player told to go to
    //Kanham has two Kanhams to choose between.
    std::vector<std::string> names;
    size_t unnamed=0;

    for(size_t i=0; i<places.size(); ++i)
    {
        if(places[i].m_name.empty())
            unnamed++;

        names.push_back(places[i].m_name);
    }

    std::sort(names.begin(), names.end());

    size_t distinct=(size_t)(std::unique(names.begin(), names.end())-names.begin());

    check(unnamed==0, "every place has a name", say("%zu without", unnamed));
    check((100*distinct/places.size())>=95, "names are not repeated",
        say("%zu distinct of %zu", distinct, places.size()));

    //Each people names in its own sounds, so two peoples must not produce the same name for the
    //same ground - that is the whole reason names are built rather than drawn from a list.
    if(g.getSpecies().size()>=2)
    {
        Phonology first=phonologyFor(0);
        Phonology second=phonologyFor(1);
        size_t same=0;

        for(size_t i=0; i<places.size(); ++i)
        {
            uint32_t roll=nameHash((uint32_t)places[i].m_cell, (uint32_t)seed);

            if(placeName(first, m[places[i].m_cell], roll)==placeName(second, m[places[i].m_cell], roll))
                same++;
        }

        check(same==0, "two peoples do not name a place alike", say("%zu shared", same));
    }

    //Navigable water is a claim about big rivers, so it must be a small share of them.
    size_t river=0;
    size_t navigable=0;
    size_t navigableDry=0;

    for(size_t i=0; i<m.size(); ++i)
    {
        if(m[i].heightBase<0.5f)
            continue;

        if(m[i].water==WaterBody::River)
            river++;

        if(m[i].navigable)
        {
            navigable++;

            if(m[i].water!=WaterBody::River)
                navigableDry++;
        }
    }

    check(navigableDry==0, "nothing is navigable that is not a river", say("%zu dry", navigableDry));

    if(river>0)
    {
        check(navigable<(river/2), "only the big rivers carry a boat",
            say("%zu of %zu river cell(s)", navigable, river));
    }

    //Ground is worked by somebody who can actually get to it, so a catchment must be reachable
    //from its own settlement and must not be sea.
    size_t wet=0;

    for(size_t i=0; i<m.size(); ++i)
    {
        if((m[i].catchment>=0)&&(m[i].heightBase<0.5f))
            wet++;
    }

    check(wet==0, "nobody works the sea", say("%zu sea cell(s) claimed", wet));

    //And a walk between two neighbouring places has to be possible, or the network is not one.
    size_t unreachable=0;
    size_t tried=0;

    for(size_t i=0; (i<places.size())&&(tried<12); i+=(places.size()/12)+1)
    {
        size_t best=i;
        float nearest=1.0e9f;

        for(size_t j=0; j<places.size(); ++j)
        {
            if(i==j)
                continue;

            int ax=(int)(places[i].m_cell%s.x);
            int ay=(int)(places[i].m_cell/s.x);
            int bx=(int)(places[j].m_cell%s.x);
            int by=(int)(places[j].m_cell/s.x);
            int dx=abs(bx-ax);

            if(dx>s.x/2)
                dx=s.x-dx;

            float distance=(float)((dx*dx)+((by-ay)*(by-ay)));

            if(distance<nearest)
            {
                nearest=distance;
                best=j;
            }
        }

        tried++;

        if(g.travelHours(glm::ivec2((int)(places[i].m_cell%s.x), (int)(places[i].m_cell/s.x)),
            glm::ivec2((int)(places[best].m_cell%s.x), (int)(places[best].m_cell/s.x)))<0.0f)
        {
            unreachable++;
        }
    }

    check(unreachable==0, "a place can be walked to from its nearest neighbour",
        say("%zu of %zu unreachable", unreachable, tried));
}

//--- the detail pass ---

static void testDetail(EquiRectWorldGenerator &g, int seed)
{
    const EquiRectWorldGenerator::InfluenceMap &m=g.getInfluenceMap();
    const glm::ivec2 &s=g.getInfluenceMapSize();

    //the biggest river on the map, which is where every one of these has failed first
    size_t pick=0;
    float most=-1.0f;

    for(size_t i=0; i<m.size(); ++i)
    {
        if((m[i].heightBase<0.5f)||(m[i].water!=WaterBody::River))
            continue;

        if(m[i].riverDischarge>most)
        {
            most=m[i].riverDischarge;
            pick=i;
        }
    }

    if(most<0.0f)
        return;

    glm::ivec2 at((int)(pick%s.x), (int)(pick/s.x));
    DetailMap detail;

    g.generateDetail(at, detail);

    check(detail.m_size>0, "a region expands", say("%d x %d samples", detail.m_size, detail.m_size));

    if(detail.m_size==0)
        return;

    int n=detail.m_size;

    //Can water get out? Not "does every neighbour happen to be lower" - the two banks of a meander
    //are neighbours and are not the same part of the river. This is the question that matters, and
    //the one that found the carve undoing the descent pass.
    size_t stuck=0;
    size_t channel=0;

    for(int y=0; y<n; ++y)
    {
        for(int x=0; x<n; ++x)
        {
            if(!detail.at(x, y).channel)
                continue;

            channel++;

            int cx=x;
            int cy=y;
            bool out=false;

            for(int step=0; step<(n*4); ++step)
            {
                if(detail.at(cx, cy).sea||(cx==0)||(cy==0)||(cx==n-1)||(cy==n-1))
                {
                    out=true;
                    break;
                }

                int bx=-1;
                int by=-1;
                float lowest=detail.at(cx, cy).elevation;

                for(size_t k=0; k<8; ++k)
                {
                    int nx=cx+g_offsets[k][0];
                    int ny=cy+g_offsets[k][1];

                    if((nx<0)||(ny<0)||(nx>=n)||(ny>=n))
                        continue;

                    if((!detail.at(nx, ny).channel)&&(!detail.at(nx, ny).sea))
                        continue;

                    if(detail.at(nx, ny).elevation<lowest)
                    {
                        lowest=detail.at(nx, ny).elevation;
                        bx=nx;
                        by=ny;
                    }
                }

                if(bx<0)
                    break;

                cx=bx;
                cy=by;
            }

            if(!out)
                stuck++;
        }
    }

    check(stuck==0, "water can leave the region from anywhere in the channel",
        say("%zu of %zu sample(s) trapped", stuck, channel));

    //The overview says where the river enters and leaves. A region that moves either has stopped
    //agreeing with the region next to it.
    size_t reached=0;

    for(size_t e=0; e<detail.m_entries.size(); ++e)
    {
        if(detail.at(detail.m_entries[e].m_at.x, detail.m_entries[e].m_at.y).water>0.0f)
            reached++;
    }

    check(reached==detail.m_entries.size(), "every mandated entry is water",
        say("%zu of %zu", reached, detail.m_entries.size()));

    //Nothing is stored, so the same cell must come back the same or a region disagrees with itself
    //between two visits.
    DetailMap again;

    g.generateDetail(at, again);

    double difference=0.0;

    for(size_t i=0; (i<detail.m_cells.size())&&(i<again.m_cells.size()); ++i)
        difference+=fabs(detail.m_cells[i].height-again.m_cells[i].height);

    check(difference==0.0, "the same region twice is the same region",
        say("total difference %.9f", difference));

    //The detail is meant to add shape without moving the ground the overview promised.
    check(fabs(detail.m_amplifiedMean-detail.m_baseMean)<0.02f,
        "amplification does not move the ground",
        say("%.4f against %.4f", detail.m_amplifiedMean, detail.m_baseMean));

    //Two regions have to agree along the edge they share, or a seam shows.
    DetailMap east;

    g.generateDetail(glm::ivec2((at.x+1)%s.x, at.y), east);

    if(east.m_size==n)
    {
        double seam=0.0;
        double across=0.0;

        for(int y=0; y<n; ++y)
        {
            seam+=fabs(detail.at(n-1, y).base-east.at(0, y).base);
            across+=fabs(detail.at(0, y).base-east.at(0, y).base);
        }

        check(seam<(across*0.1), "neighbouring regions agree at their shared edge",
            say("step %.6f against %.6f across a whole region", seam/n, across/n));
    }
}

//--- the boundary ---

static void testBoundary(int seed)
{
    wgWorld *world=wgWorldCreate(seed, 0, 0, 0);

    check(world!=0, "a world is made through the C boundary", wgLastError());

    if(!world)
        return;

    int32_t cells=wgCellCount(world);
    std::string path="/tmp/worldgen_test_"+std::to_string(seed)+".wgen";

    check(wgWorldSave(world, path.c_str())!=0, "a world writes to disk", wgLastError());

    wgWorld *back=wgWorldLoad(path.c_str());

    check(back!=0, "and reads back", wgLastError());

    if(back)
    {
        //A world is remembered rather than re-derived because the noise underneath is not promised
        //to be identical on two processors. That is worth nothing if the round trip is lossy.
        std::vector<wgCell> before((size_t)cells);
        std::vector<wgCell> after((size_t)cells);

        wgReadCells(world, before.data(), cells);
        wgReadCells(back, after.data(), cells);

        size_t differ=0;

        for(int32_t i=0; i<cells; ++i)
        {
            if(memcmp(&before[i], &after[i], sizeof(wgCell))!=0)
                differ++;
        }

        check(differ==0, "a world read back is the world written",
            say("%zu of %d cell(s) differ", differ, cells));

        wgWorldDestroy(back);
    }

    //A file that says one world and holds another is worse than one that will not open.
    {
        FILE *source=fopen(path.c_str(), "rb");
        std::string bent=path+".bent";
        FILE *target=fopen(bent.c_str(), "wb");
        int c;
        int at=0;

        while((c=fgetc(source))!=EOF)
        {
            if(at==200)
                c^=0xFF;

            fputc(c, target);
            at++;
        }

        fclose(source);
        fclose(target);

        wgWorld *bad=wgWorldLoad(bent.c_str());

        check(bad==0, "a bent world is refused", bad?"accepted":wgLastError());

        if(bad)
            wgWorldDestroy(bad);

        remove(bent.c_str());
    }

    remove(path.c_str());
    wgWorldDestroy(world);
}

int main(int argc, char **argv)
{
    std::vector<int> seeds;

    if(argc>1)
    {
        for(int i=1; i<argc; ++i)
            seeds.push_back(atoi(argv[i]));
    }
    else
    {
        //More than one, because nearly every defect here appeared on some worlds and not others.
        int fixed[4]={1, 7, 42, 1337};

        for(int i=0; i<4; ++i)
            seeds.push_back(fixed[i]);
    }

    for(size_t i=0; i<seeds.size(); ++i)
    {
        EquiRectWorldGenerator generator;
        WorldDescriptors descriptors;
        Progress progress;

        descriptors.setSize({1048576, 524288, 1024});
        descriptors.setRegionSize({16, 16, 16});
        descriptors.setChunkSize({64, 64, 16});
        generator.getDecriptors().seed=seeds[i];
        generator.create(&descriptors, progress);

        group(say("seed %d", seeds[i]));
        testRivers(generator, seeds[i]);
        testClimate(generator, seeds[i]);
        testSettlements(generator, seeds[i]);
        testRoads(generator, seeds[i]);
        testNamesAndLand(generator, seeds[i]);
        testDetail(generator, seeds[i]);
    }

    group("the C boundary");
    testBoundary(seeds[0]);

    printf("\n%d check(s), %d failure(s)\n", g_checks, g_failures);

    return g_failures?1:0;
}
