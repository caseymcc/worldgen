#include "worldgen/generators/equiRectWorldGenerator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>

namespace worldgen
{

namespace
{

const int detailOffsets[8][2]={{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

inline int wrapDetailIndex(int value, int size)
{
    value=value%size;

    if(value<0)
        value+=size;

    return value;
}

//The residual is sampled in WORLD space rather than region space, and that single decision buys
//three of the things the detail pass has to guarantee at once: two neighbouring regions read the
//same field along their shared edge and so agree there without either knowing the other exists; a
//region regenerates identically from the world seed and its own coordinates, so nothing has to be
//stored; and nothing has to be reconciled after the fact. A hash keeps it that way - no state, no
//setup, the same answer whichever region asks first.
inline uint32_t detailHash(int x, int y, uint32_t seed)
{
    uint32_t h=seed;

    h^=(uint32_t)x*0x9E3779B1u;
    h^=(uint32_t)y*0x85EBCA77u;
    h^=h>>15;
    h*=0xC2B2AE3Du;
    h^=h>>13;
    h*=0x27D4EB2Fu;
    h^=h>>16;

    return h;
}

inline float detailValue(int x, int y, uint32_t seed)
{
    return ((float)(detailHash(x, y, seed)&0xFFFFFFu)/(float)0x7FFFFFu)-1.0f;
}

inline float detailSmooth(float t)
{
    return t*t*(3.0f-(2.0f*t));
}

inline float detailNoise(float x, float y, uint32_t seed)
{
    int x0=(int)floorf(x);
    int y0=(int)floorf(y);
    float fx=detailSmooth(x-(float)x0);
    float fy=detailSmooth(y-(float)y0);

    float v00=detailValue(x0, y0, seed);
    float v10=detailValue(x0+1, y0, seed);
    float v01=detailValue(x0, y0+1, seed);
    float v11=detailValue(x0+1, y0+1, seed);

    return (((v00*(1.0f-fx))+(v10*fx))*(1.0f-fy))+((((v01*(1.0f-fx))+(v11*fx)))*fy);
}

inline float detailFbm(float x, float y, uint32_t seed, int octaves, float lacunarity, float gain)
{
    float sum=0.0f;
    float amplitude=1.0f;
    float total=0.0f;

    for(int o=0; o<octaves; ++o)
    {
        sum+=detailNoise(x, y, seed+(uint32_t)(o*7919))*amplitude;
        total+=amplitude;

        x*=lacunarity;
        y*=lacunarity;
        amplitude*=gain;
    }

    return (total>0.0f)?(sum/total):0.0f;
}

//Where a flow crossing the region edge in a given direction meets that edge. Both sides of a
//boundary work this out from the same direction, so the point one region calls its exit is the
//sample the neighbouring region calls its entry, immediately across the seam.
inline glm::ivec2 boundaryPoint(int dx, int dy, int size)
{
    glm::ivec2 point;

    point.x=(dx<0)?0:((dx>0)?(size-1):(size/2));
    point.y=(dy<0)?0:((dy>0)?(size-1):(size/2));

    return point;
}

}//unnamed namespace

void EquiRectWorldGenerator::generateDetail(const glm::ivec2 &influenceCell, DetailMap &detail)
{
    const DetailThresholds &rules=m_descriptorValues.m_detailRules;
    const glm::ivec2 &influenceSize=m_descriptorValues.m_influenceSize;

    detail=DetailMap();

    if(m_influenceMap.empty()||(rules.m_resolution<8))
        return;

    if(m_influenceNeighborMap.size()<(m_influenceMap.size()*NeighborCount))
        updateInfluenceNeighbors();

    const int size=rules.m_resolution;
    const int cellX=wrapDetailIndex(influenceCell.x, influenceSize.x);
    const int cellY=std::max(0, std::min(influenceCell.y, influenceSize.y-1));
    const size_t cellIndex=((size_t)cellY*influenceSize.x)+cellX;
    const InfluenceCell &cell=m_influenceMap[cellIndex];

    detail.m_cell=glm::ivec2(cellX, cellY);
    detail.m_size=size;
    detail.m_metresPerSample=((float)m_descriptorValues.m_influenceGridSize.x
        *m_descriptorValues.m_metresPerBlock)/(float)size;
    detail.m_cells.assign((size_t)size*size, DetailCell());

    const float metres=detail.m_metresPerSample;

    //--- 01 · amplify the cell ---
    //The base comes from the cell's four CORNER values, each already the average of the four cells
    //meeting at that corner, so the base is continuous across every region boundary before any
    //detail is added. Amplification then only has to supply a zero-mean residual, and the ground
    //the overview promised is what the region still averages.
    {
        //A 5 by 5 patch of the surrounding cells, in METRES rather than normalized height, and
        //signed so it runs continuously through the shoreline. Averaged corners were tried first
        //and are wrong here: averaging the four cells meeting at a corner is a box blur, so a
        //coastal cell whose neighbours are sea loses its own height and the region comes out
        //underwater. Interpolating THROUGH the cell values keeps the coastline where the overview
        //put it.
        float patch[5][5];

        for(int py=0; py<5; ++py)
        {
            int ny=cellY+py-2;

            if(ny<0)
                ny=0;
            if(ny>=influenceSize.y)
                ny=influenceSize.y-1;

            for(int px=0; px<5; ++px)
            {
                patch[py][px]=heightToSigned(m_influenceMap[((size_t)ny*influenceSize.x)
                    +wrapDetailIndex(cellX+px-2, influenceSize.x)].heightBase, 0.5f,
                    m_descriptorValues.m_maxElevation, m_descriptorValues.m_elevationCurve,
                    rules.m_maxDepth, rules.m_depthCurve);
            }
        }

        //How much relief to inject, taken from how much the neighbouring ground differs by and
        //defined at the CORNERS so it is shared with the neighbouring regions and cannot step at a
        //seam. Broken country gets more than a plain, and both get some.
        float amplitude[4];

        for(int c=0; c<4; ++c)
        {
            int ox=(c&1)?0:-1;
            int oy=(c&2)?0:-1;
            float lowest=patch[2][2];
            float highest=patch[2][2];

            for(int qy=0; qy<2; ++qy)
            {
                for(int qx=0; qx<2; ++qx)
                {
                    float value=patch[2+oy+qy][2+ox+qx];

                    lowest=std::min(lowest, value);
                    highest=std::max(highest, value);
                }
            }

            //A floor of absolute relief PLUS a share of how much the neighbouring ground varies.
            //Multiplying the two instead ties flat country's detail to a number that is near zero
            //there, so one setting cannot serve both a coastal plain and a mountain front: turn it
            //down far enough to tame the mountains and the plains go dead flat.
            amplitude[c]=rules.m_minimumRelief+(rules.m_residualScale*(highest-lowest));
        }

        double baseSum=0.0;
        double amplifiedSum=0.0;
        uint32_t noiseSeed=(uint32_t)m_descriptorValues.seed*0x9E3779B1u;

        for(int y=0; y<size; ++y)
        {
            float v=((float)y+0.5f)/(float)size;

            for(int x=0; x<size; ++x)
            {
                float u=((float)x+0.5f)/(float)size;
                DetailCell &sample=detail.at(x, y);

                //Cell centres sit at u = 0.5, so the coordinate to interpolate along runs from
                //-0.5 to +0.5 across the region and the four cells in play change at the midline.
                float sx=u-0.5f;
                float sy=v-0.5f;
                int ix=(sx<0.0f)?1:2;
                int iy=(sy<0.0f)?1:2;
                float tx=sx-floorf(sx);
                float ty=sy-floorf(sy);
                float rows[4];

                for(int r=0; r<4; ++r)
                {
                    rows[r]=catmullRom(patch[iy-1+r][ix-1], patch[iy-1+r][ix],
                        patch[iy-1+r][ix+1], patch[iy-1+r][ix+2], tx);
                }

                float baseElevation=catmullRom(rows[0], rows[1], rows[2], rows[3], ty);
                float local=bilinearCorners(amplitude[0], amplitude[1], amplitude[2], amplitude[3], u, v);
                float worldX=(float)((cellX*size)+x)*rules.m_residualFrequency;
                float worldY=(float)((cellY*size)+y)*rules.m_residualFrequency;
                float residual=detailFbm(worldX, worldY, noiseSeed, rules.m_octaves,
                    rules.m_lacunarity, rules.m_gain);

                //Fold back off the top and bottom of the world rather than clipping at them. Where
                //the overview has saturated - the white ground at the very top of the range - half
                //of a symmetric residual would otherwise be cut away and the region would come out
                //a dead flat plateau at exactly the ceiling.
                float elevation=baseElevation+(local*residual);
                float ceiling=m_descriptorValues.m_maxElevation;

                if(elevation>ceiling)
                    elevation=(2.0f*ceiling)-elevation;
                if(elevation<-rules.m_maxDepth)
                    elevation=(-2.0f*rules.m_maxDepth)-elevation;

                sample.elevation=elevation;
                sample.base=signedToHeight(baseElevation, 0.5f, m_descriptorValues.m_maxElevation,
                    m_descriptorValues.m_elevationCurve, rules.m_maxDepth, rules.m_depthCurve);

                baseSum+=sample.base;
                amplifiedSum+=signedToHeight(sample.elevation, 0.5f,
                    m_descriptorValues.m_maxElevation, m_descriptorValues.m_elevationCurve,
                    rules.m_maxDepth, rules.m_depthCurve);
            }
        }

        detail.m_baseMean=(float)(baseSum/detail.m_cells.size());
        detail.m_amplifiedMean=(float)(amplifiedSum/detail.m_cells.size());
    }

    //--- 02 · condition the surface so water can leave it ---
    //Priority flood plus epsilon, then a separate pass over the flats, because filling a depression
    //leaves ground with no gradient at all and flow direction there is undefined. The rim of the
    //region is the outlet: water leaving the region is the neighbouring region's problem, and the
    //overview already decided where it goes.
    std::vector<size_t> floodOrder((size_t)size*size, 0);
    //The conditioned surface, kept apart from the terrain. Filling a pit raises it to the level of
    //its outlet, which is flat by definition - doing that to the ground itself is what turns a
    //rugged region into a scatter of dead flat plateaus. Water needs the filled surface to find its
    //way out; the ground does not need to be flattened for it.
    std::vector<float> drained((size_t)size*size, 0.0f);

    {
        typedef std::pair<float, size_t> FloodEntry;
        std::priority_queue<FloodEntry, std::vector<FloodEntry>, std::greater<FloodEntry>> flood;
        std::vector<bool> resolved((size_t)size*size, false);

        for(size_t i=0; i<detail.m_cells.size(); ++i)
            drained[i]=detail.m_cells[i].elevation;

        for(int y=0; y<size; ++y)
        {
            for(int x=0; x<size; ++x)
            {
                size_t index=((size_t)y*size)+x;
                bool rim=((x==0)||(y==0)||(x==size-1)||(y==size-1));

                //The rim is an outlet because whatever leaves the region is the next region's
                //problem. Anything already under water is an outlet too - the sea does not need
                //draining, and filling it would flatten the sea floor into a sheet.
                if((!rim)&&(detail.m_cells[index].elevation>=0.0f))
                    continue;

                resolved[index]=true;
                flood.push(FloodEntry(drained[index], index));
            }
        }

        size_t floodStep=0;
        size_t filled=0;

        while(!flood.empty())
        {
            FloodEntry entry=flood.top();

            flood.pop();

            size_t index=entry.second;

            floodOrder[index]=floodStep++;

            int x=(int)(index%size);
            int y=(int)(index/size);

            for(size_t n=0; n<8; ++n)
            {
                int nx=x+detailOffsets[n][0];
                int ny=y+detailOffsets[n][1];

                if((nx<0)||(ny<0)||(nx>=size)||(ny>=size))
                    continue;

                size_t neighborIndex=((size_t)ny*size)+nx;

                if(resolved[neighborIndex])
                    continue;

                resolved[neighborIndex]=true;

                float floor=drained[index]+rules.m_epsilon;

                if(drained[neighborIndex]<floor)
                {
                    drained[neighborIndex]=floor;
                    filled++;
                }

                flood.push(FloodEntry(drained[neighborIndex], neighborIndex));
            }
        }

        detail.m_filledSamples=filled;

        //Where the fill had to raise the surface appreciably, the water standing in it has nowhere
        //to go. Whether that is a lake depends on the climate rather than on the shape of the
        //ground: a basin holds water only where enough runs into it to beat what evaporates out,
        //and in dry country it does not. There the same hollow is a salt pan - which is exactly
        //where salt flats come from. A big enough river can still sustain a terminal lake in a
        //desert, the way the Nile and the Jordan do.
        bool holdsWater=(cell.moisture>=rules.m_lakeMoisture)
            ||(dischargePerSecond(cell.riverDischarge)>=rules.m_lakeRiver);

        for(size_t i=0; i<detail.m_cells.size(); ++i)
        {
            DetailCell &sample=detail.m_cells[i];

            if(sample.elevation<0.0f)
                continue;

            float held=drained[i]-sample.elevation;

            if(held<rules.m_lakeDepth)
                continue;

            if(holdsWater)
                sample.water=held;
            else
            {
                //a pan floors itself with what the water left behind, and that is dead flat -
                //which is the one thing everybody knows about a salt flat
                sample.saltFlat=true;
                sample.elevation=drained[i];
            }
        }
    }


    //--- boundary conditions, read off the overview ---
    //These are the numbers the detail pass is not allowed to invent. The overview already ran a
    //world-wide depression fill, flow routing and accumulation, so the global reconciliation that
    //tiled hydrology normally needs has already happened - this region reads its edges off a solve
    //that both it and its neighbours share.
    detail.m_exit.m_direction=cell.flowDirection;
    detail.m_exit.m_discharge=dischargePerSecond(cell.riverDischarge);
    detail.m_outflow=detail.m_exit.m_discharge;

    if(cell.flowDirection>=0)
    {
        detail.m_exit.m_at=boundaryPoint(detailOffsets[cell.flowDirection][0],
            detailOffsets[cell.flowDirection][1], size);
    }
    else
        detail.m_exit.m_at=glm::ivec2(size/2, size-1);

    for(size_t n=0; n<8; ++n)
    {
        int nx=cellX+detailOffsets[n][0];
        int ny=cellY+detailOffsets[n][1];

        if((ny<0)||(ny>=influenceSize.y))
            continue;

        const InfluenceCell &neighbor=m_influenceMap[((size_t)ny*influenceSize.x)
            +wrapDetailIndex(nx, influenceSize.x)];

        if(neighbor.flowDirection<0)
            continue;

        //does that neighbour drain into this cell rather than away from it
        if((detailOffsets[neighbor.flowDirection][0]!=-detailOffsets[n][0])
            ||(detailOffsets[neighbor.flowDirection][1]!=-detailOffsets[n][1]))
            continue;

        float arriving=dischargePerSecond(neighbor.riverDischarge);

        if(arriving<=0.0f)
            continue;

        DetailFlow entry;

        entry.m_direction=(int)n;
        entry.m_discharge=arriving;
        entry.m_at=boundaryPoint(detailOffsets[n][0], detailOffsets[n][1], size);

        detail.m_entries.push_back(entry);
        detail.m_inflow+=arriving;
    }

    detail.m_hasRiver=(cell.heightBase>=0.5f)
        &&((detail.m_outflow>0.0f)||(!detail.m_entries.empty()));

    if(detail.m_hasRiver)
    {
        //--- 03 · route the channel between the mandated endpoints ---
        //A fixed-endpoint least-cost path over an anisotropic cost field: the slope that matters is
        //the one in the direction of travel, because water cares about downhill and nothing else.
        //Segments come from a coprime mask rather than the eight neighbours, which is what keeps a
        //channel from reading as a staircase.
        std::vector<glm::ivec2> mask=coprimeSegmentMask(rules.m_segmentMask);

        //biggest river first, so the tributaries have a stem to join
        std::sort(detail.m_entries.begin(), detail.m_entries.end(),
            [](const DetailFlow &a, const DetailFlow &b) { return a.m_discharge>b.m_discharge; });

        std::vector<DetailFlow> sources=detail.m_entries;

        if(sources.empty())
        {
            //nothing arrives, so the river rises here. Its head is the highest ground in the
            //region, which is where the springs would be.
            DetailFlow head;
            float highest=-1.0f;

            head.m_direction=-1;
            head.m_discharge=0.0f;
            head.m_at=glm::ivec2(size/2, size/2);

            for(int y=size/8; y<size-(size/8); ++y)
            {
                for(int x=size/8; x<size-(size/8); ++x)
                {
                    if(detail.at(x, y).height>highest)
                    {
                        highest=detail.at(x, y).height;
                        head.m_at=glm::ivec2(x, y);
                    }
                }
            }

            sources.push_back(head);
        }

        std::vector<std::vector<size_t>> courses;

        for(size_t s=0; s<sources.size(); ++s)
        {
            const glm::ivec2 &from=sources[s].m_at;
            const glm::ivec2 &to=detail.m_exit.m_at;

            std::vector<float> spent((size_t)size*size, std::numeric_limits<float>::max());
            std::vector<size_t> came((size_t)size*size, 0);

            struct Front
            {
                float m_estimate;
                float m_cost;
                size_t m_index;

                bool operator>(const Front &other) const { return m_estimate>other.m_estimate; }
            };

            std::priority_queue<Front, std::vector<Front>, std::greater<Front>> open;
            size_t start=((size_t)from.y*size)+from.x;
            size_t goal=((size_t)to.y*size)+to.x;

            spent[start]=0.0f;
            came[start]=start;
            open.push(Front{0.0f, 0.0f, start});

            bool arrived=false;

            while(!open.empty())
            {
                Front current=open.top();

                open.pop();

                if(current.m_cost>spent[current.m_index])
                    continue;

                if(current.m_index==goal)
                {
                    arrived=true;
                    break;
                }

                int x=(int)(current.m_index%size);
                int y=(int)(current.m_index/size);
                const DetailCell &here=detail.m_cells[current.m_index];

                for(size_t m=0; m<mask.size(); ++m)
                {
                    int nx=x+mask[m].x;
                    int ny=y+mask[m].y;

                    if((nx<0)||(ny<0)||(nx>=size)||(ny>=size))
                        continue;

                    size_t index=((size_t)ny*size)+nx;
                    const DetailCell &there=detail.m_cells[index];
                    float length=sqrtf((float)((mask[m].x*mask[m].x)+(mask[m].y*mask[m].y)));
                    float run=length*metres;
                    float rise=there.elevation-here.elevation;
                    float climb=(run>0.0f)?std::max(rise/run, 0.0f):0.0f;

                    float step=length*(1.0f+(rules.m_climbPenalty*climb));

                    //a channel already cut is nearly free to follow, which is what makes a
                    //tributary join the stem instead of running alongside it
                    if(there.channel)
                        step*=rules.m_reuseWeight;

                    float total=current.m_cost+step;

                    if(total>=spent[index])
                        continue;

                    spent[index]=total;
                    came[index]=current.m_index;

                    //the cheapest any remaining distance can possibly be, so the estimate never
                    //overreaches and the search stays admissible
                    float remaining=sqrtf((float)(((nx-to.x)*(nx-to.x))+((ny-to.y)*(ny-to.y))))
                        *rules.m_reuseWeight;

                    open.push(Front{total+remaining, total, index});
                }
            }

            if(!arrived)
                continue;

            //walk the trail back, filling in the ground each segment jumped over
            std::vector<size_t> course;
            size_t step=goal;

            while(true)
            {
                size_t previous=came[step];

                if(previous==step)
                {
                    course.push_back(step);
                    break;
                }

                int ax=(int)(previous%size);
                int ay=(int)(previous/size);
                int bx=(int)(step%size);
                int by=(int)(step/size);
                int span=std::max(abs(bx-ax), abs(by-ay));

                for(int t=span; t>0; --t)
                {
                    int px=ax+(int)roundf((float)(bx-ax)*((float)t/(float)span));
                    int py=ay+(int)roundf((float)(by-ay)*((float)t/(float)span));

                    course.push_back(((size_t)py*size)+px);
                }

                step=previous;
            }

            std::reverse(course.begin(), course.end());

            for(size_t c=0; c<course.size(); ++c)
                detail.m_cells[course[c]].channel=true;

            courses.push_back(course);
        }

        //--- 04 · size the channel from discharge ---
        //What arrives is what the overview says arrives; what leaves is what the overview says
        //leaves. The difference is the ground's own runoff, gained along the way, so the water
        //balance closes against the overview at both edges rather than drifting.
        float gain=std::max(detail.m_outflow-detail.m_inflow, 0.0f);

        for(size_t c=0; c<courses.size(); ++c)
        {
            const std::vector<size_t> &course=courses[c];
            float carried=(c<sources.size())?sources[c].m_discharge:0.0f;
            float share=(courses.size()>0)?(gain/(float)courses.size()):0.0f;

            for(size_t i=0; i<course.size(); ++i)
            {
                float along=(course.size()>1)?((float)i/(float)(course.size()-1)):1.0f;

                detail.m_cells[course[i]].discharge+=carried+(share*along);
            }
        }

        //A routed course still has to run downhill, and least cost only discourages climbing - it
        //does not forbid it. Taking a running minimum of the bank height from the head down fixes
        //the profile so the channel descends the whole way, which is the difference between a river
        //and a chain of ponds.
        for(size_t c=0; c<courses.size(); ++c)
        {
            const std::vector<size_t> &course=courses[c];
            float bank=std::numeric_limits<float>::max();

            for(size_t i=0; i<course.size(); ++i)
            {
                DetailCell &sample=detail.m_cells[course[i]];

                bank=std::min(bank, sample.elevation);
                sample.elevation=bank;
                bank-=0.01f;   //a centimetre a sample, so it never runs level
            }
        }

        //Carve the cross section: the channel proper, then the ground it has worked over beside it.
        //Every sample takes its shape from the NEAREST point on the course. Letting any course
        //sample within reach lower it instead would let a wide river flatten its own bed - the disc
        //of a downstream sample is wider than the drop between samples, so a stretch upstream would
        //be pulled down level with it and the water would stop running.
        //also records which ground the carve touched, so the conversion back to height afterwards
        //only reworks what was actually changed
        std::vector<float> nearest(detail.m_cells.size(), std::numeric_limits<float>::max());

        for(size_t c=0; c<courses.size(); ++c)
        {
            const std::vector<size_t> &course=courses[c];

            for(size_t i=0; i<course.size(); ++i)
            {
                DetailCell &sample=detail.m_cells[course[i]];
                float width=0.0f;
                float depth=0.0f;

                channelShape(sample.discharge, rules, width, depth);

                sample.channelWidth=width;
                sample.water=depth;

                int cx=(int)(course[i]%size);
                int cy=(int)(course[i]/size);
                float bank=sample.elevation;
                float reach=((width*0.5f)+(depth*rules.m_bankSlope)
                    +(width*rules.m_floodplainWidth*0.5f))/metres;
                int radius=std::max((int)ceilf(reach), 1);

                for(int dy=-radius; dy<=radius; ++dy)
                {
                    int ny=cy+dy;

                    if((ny<0)||(ny>=size))
                        continue;

                    for(int dx=-radius; dx<=radius; ++dx)
                    {
                        int nx=cx+dx;

                        if((nx<0)||(nx>=size))
                            continue;

                        size_t nearIndex=((size_t)ny*size)+nx;
                        float distance=sqrtf((float)((dx*dx)+(dy*dy)))*metres;

                        if(distance>=nearest[nearIndex])
                            continue;   //a closer part of the course already shaped this ground

                        DetailCell &near=detail.m_cells[nearIndex];

                        if(distance<=(width*0.5f))
                        {
                            //open water
                            nearest[nearIndex]=distance;

                            if(near.elevation>bank)
                                near.elevation=bank;

                            near.water=depth;
                            near.channelWidth=width;
                            near.floodplain=true;
                            continue;
                        }

                        //Banks rising away from the water, then the floodplain beyond them. The
                        //floodplain has to fade back INTO the natural ground rather than stopping
                        //at a fixed height - a terrace that ends at a step reads as a drawn line
                        //across the hillside, which is the one thing a floodplain never looks like.
                        float out=distance-(width*0.5f);
                        float bankTop=depth*rules.m_bankSlope;
                        float plain=width*rules.m_floodplainWidth*0.5f;

                        if(out>(bankTop+plain))
                            continue;

                        float top;

                        if(out<=bankTop)
                            top=bank+(out/rules.m_bankSlope);
                        else
                        {
                            float across=(plain>0.0f)?((out-bankTop)/plain):1.0f;

                            across=across*across*(3.0f-(2.0f*across));
                            top=((bank+depth)*(1.0f-across))+(near.elevation*across);
                        }

                        nearest[nearIndex]=distance;

                        if(near.elevation>top)
                        {
                            near.elevation=top;
                            near.floodplain=true;
                        }
                    }
                }
            }
        }

    }

    //--- the sea ---
    //Everything under sea level is water, and it is a different sort of water to a river: it has a
    //floor rather than a bed, and its depth comes from how far the ground fell short of the surface
    //rather than from any discharge. Run last, so a channel carved down past sea level at a river
    //mouth becomes sea rather than staying a trench with a dry floor.
    for(size_t i=0; i<detail.m_cells.size(); ++i)
    {
        DetailCell &sample=detail.m_cells[i];

        if(sample.elevation>=0.0f)
            continue;

        sample.sea=true;
        sample.saltFlat=false;
        sample.water=std::max(sample.water, -sample.elevation);
        sample.channel=false;
        sample.floodplain=false;
    }

    //Height last, derived once from the elevation everything else worked in. Nothing round trips
    //through the curve, so nothing gets pinned by the flat part of it at the waterline.
    for(size_t i=0; i<detail.m_cells.size(); ++i)
    {
        DetailCell &sample=detail.m_cells[i];

        sample.height=clamp(signedToHeight(sample.elevation, 0.5f,
            m_descriptorValues.m_maxElevation, m_descriptorValues.m_elevationCurve,
            rules.m_maxDepth, rules.m_depthCurve), 0.0f, 1.0f);
    }

    detail.m_lowest=std::numeric_limits<float>::max();
    detail.m_highest=-std::numeric_limits<float>::max();
    detail.m_channelSamples=0;
    detail.m_seaSamples=0;
    detail.m_lakeSamples=0;
    detail.m_saltFlatSamples=0;
    detail.m_widest=0.0f;

    for(size_t i=0; i<detail.m_cells.size(); ++i)
    {
        const DetailCell &sample=detail.m_cells[i];

        detail.m_lowest=std::min(detail.m_lowest, sample.elevation);
        detail.m_highest=std::max(detail.m_highest, sample.elevation);

        if(sample.sea)
            detail.m_seaSamples++;
        else if(sample.saltFlat)
            detail.m_saltFlatSamples++;
        else if(sample.water>0.0f)
        {
            if(sample.channel)
            {
                detail.m_channelSamples++;
                detail.m_widest=std::max(detail.m_widest, sample.channelWidth);
            }
            else
                detail.m_lakeSamples++;
        }
    }
}

}//namespace worldgen
