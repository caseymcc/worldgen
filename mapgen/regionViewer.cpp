#include "regionViewer.h"
#include "imguiHelpers.h"
#include "terrainPalette.h"

#include <algorithm>
#include <cmath>

namespace mapgen
{

RegionViewer::RegionViewer():
    m_shownCell(-1, -1),
    m_layerIndex(0),
    m_shownLayer(-1)
{}

void RegionViewer::onInitialize()
{
    m_imageView.setName("RegionViewerImage");
    m_imageView.initialize();
}

void RegionViewer::onPosition()
{
    m_imageView.setPosition(m_x+1, m_y+22);
}

void RegionViewer::onSize()
{
    m_imageView.setSize(m_width-1, m_height-22);
}

void RegionViewer::updateTexture()
{
    const worldgen::DetailMap &detail=m_uiData->detail;
    int size=detail.m_size;

    if(size<=0)
        return;

    m_textureBuffer.resize((size_t)size*size*4);

    //Shade the land against the land's own range. Taking it against the whole range instead would
    //let a deep shelf on one edge flatten every hill in the region into the same colour.
    float landLow=0.0f;
    float landHigh=1.0f;
    float deepest=0.0f;
    bool anyLand=false;

    for(size_t i=0; i<detail.m_cells.size(); ++i)
    {
        const worldgen::DetailCell &sample=detail.m_cells[i];

        if(sample.sea)
        {
            deepest=std::max(deepest, sample.water);
            continue;
        }

        if(!anyLand)
        {
            landLow=sample.elevation;
            landHigh=sample.elevation;
            anyLand=true;
        }

        landLow=std::min(landLow, sample.elevation);
        landHigh=std::max(landHigh, sample.elevation);
    }

    float range=std::max(landHigh-landLow, 1.0f);

    if(deepest<=0.0f)
        deepest=1.0f;

    //The climate this region sits in, and the temperature at the influence cell's own height, so a
    //sample's temperature can be worked out from how far above or below that it stands.
    const worldgen::EquiRectWorldGenerator::InfluenceMap &influenceMap=
        m_uiData->generator->getInfluenceMap();
    const glm::ivec2 &influenceSize=m_uiData->generator->getInfluenceMapSize();
    worldgen::EquiRectDescriptors &descriptors=m_uiData->generator->getDecriptors();
    size_t cellIndex=((size_t)detail.m_cell.y*influenceSize.x)+detail.m_cell.x;
    const worldgen::InfluenceCell &cell=influenceMap[cellIndex];
    float latitude=(float)M_PI_2-((float)M_PI*(((float)detail.m_cell.y+0.5f)/(float)influenceSize.y));
    float cellElevation=worldgen::heightToElevation(cell.heightBase, 0.5f,
        descriptors.m_maxElevation, descriptors.m_elevationCurve);
    float cellLapse=worldgen::elevationTemperature(cellElevation, descriptors.m_lapseRate);

    for(int y=0; y<size; ++y)
    {
        for(int x=0; x<size; ++x)
        {
            const worldgen::DetailCell &sample=detail.at(x, y);
            size_t index=(((size_t)y*size)+x)*4;
            int r=0;
            int g=0;
            int b=0;

            //Slope shading off the conditioned surface. Without it an amplified heightmap reads as
            //a flat colour ramp and none of the detail the pass just added is visible.
            float across=detail.at(std::min(x+1, size-1), y).elevation
                -detail.at(std::max(x-1, 0), y).elevation;
            float down=detail.at(x, std::min(y+1, size-1)).elevation
                -detail.at(x, std::max(y-1, 0)).elevation;
            float shade=std::max(0.15f, std::min(1.6f, 1.0f-((across+down)*0.06f)));
            float lift=(sample.elevation-landLow)/range;

            if(m_layerIndex==(int)DetailLayers::Elevation)
            {
                if(sample.sea)
                {//below the waterline, measured downwards on its own scale
                    float deep=std::min(sample.water/deepest, 1.0f);

                    r=(int)(30.0f-(14.0f*deep));
                    g=(int)(60.0f-(28.0f*deep));
                    b=(int)(110.0f-(40.0f*deep));
                }
                else
                {
                    int value=(int)(40.0f+(215.0f*lift));

                    r=value; g=value; b=value;
                }
            }
            else if(m_layerIndex==(int)DetailLayers::Water)
            {
                if(sample.sea)
                {
                    float deep=std::min(sample.water/deepest, 1.0f);

                    r=(int)(30.0f-(12.0f*deep));
                    g=(int)(75.0f-(35.0f*deep));
                    b=(int)(135.0f-(45.0f*deep));
                }
                else if(sample.saltFlat)
                {
                    r=235; g=230; b=215;
                }
                else if(sample.water>0.0f)
                {
                    float deep=std::min(sample.water/4.0f, 1.0f);

                    r=(int)(70.0f-(45.0f*deep));
                    g=(int)(150.0f-(70.0f*deep));
                    b=(int)(215.0f-(45.0f*deep));
                }
                else if(sample.floodplain)
                {
                    r=150; g=175; b=120;
                }
                else
                {
                    int value=(int)(45.0f+(60.0f*lift));

                    r=value; g=value; b=value;
                }
            }
            else if(m_layerIndex==(int)DetailLayers::Base)
            {
                //the coarse value this region was amplified from, for comparison
                int value=(int)(255.0f*worldgen::clamp(sample.base, 0.0f, 1.0f));

                r=value; g=value; b=value;
            }
            else
            {
                if(sample.sea)
                {//the sea, darkening with depth away from the shore
                    float deep=std::min(sample.water/deepest, 1.0f);

                    r=(int)(46.0f-(24.0f*deep));
                    g=(int)(104.0f-(54.0f*deep));
                    b=(int)(160.0f-(58.0f*deep));
                }
                else if(sample.saltFlat)
                {//a crust of what the water left behind
                    r=(int)(232.0f*shade);
                    g=(int)(228.0f*shade);
                    b=(int)(213.0f*shade);
                }
                else if(sample.water>0.0f)
                {
                    float deep=std::min(sample.water/4.0f, 1.0f);

                    r=(int)(70.0f-(40.0f*deep));
                    g=(int)(140.0f-(60.0f*deep));
                    b=(int)(200.0f-(40.0f*deep));
                }
                else
                {
                    //the temperature at this sample's own height, not the cell's
                    float local=cell.temperature
                        +worldgen::elevationTemperature(sample.elevation, descriptors.m_lapseRate)
                        -cellLapse;
                    glm::ivec3 ground=terrainColor(cell.climate, lift, local, sample.elevation,
                        latitude, sample.floodplain);

                    r=(int)((float)ground.r*shade);
                    g=(int)((float)ground.g*shade);
                    b=(int)((float)ground.b*shade);
                }
            }

            m_textureBuffer[index]=(GLubyte)std::min(255, std::max(0, r));
            m_textureBuffer[index+1]=(GLubyte)std::min(255, std::max(0, g));
            m_textureBuffer[index+2]=(GLubyte)std::min(255, std::max(0, b));
            m_textureBuffer[index+3]=255;
        }
    }

    imglib::SimpleImage image(imglib::Format::RGBA, imglib::Depth::Bit8, size, size,
        &m_textureBuffer[0], (size_t)size*size);

    m_imageView.updateImage(&image);
}

void RegionViewer::onDraw()
{
    if((m_uiData->selectedCell.x<0)||(m_uiData->selectedCell.y<0))
    {
        ImGui::TextDisabled("Click a cell on the world map to open it.");
        return;
    }

    worldgen::EquiRectWorldGenerator *generator=m_uiData->generator.get();

    if((!m_uiData->detailValid)||(m_shownCell!=m_uiData->selectedCell))
    {
        generator->generateDetail(m_uiData->selectedCell, m_uiData->detail);

        m_uiData->detailValid=true;
        m_shownCell=m_uiData->selectedCell;
        m_shownLayer=-1;
    }

    const worldgen::DetailMap &detail=m_uiData->detail;

    ImGui::SetNextItemWidth(180.0f);

    if(ImGui::BeginCombo("##detailLayer", uiDetailLayers[m_layerIndex].second.c_str()))
    {
        for(size_t i=0; i<uiDetailLayers.size(); ++i)
        {
            bool selected=(m_layerIndex==(int)i);

            if(ImGui::Selectable(uiDetailLayers[i].second.c_str(), selected))
            {
                m_layerIndex=(int)i;
                m_shownLayer=-1;
            }

            if(selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::SameLine();

    float span=(float)detail.m_size*detail.m_metresPerSample;
    float seaShare=detail.m_cells.empty()
        ?0.0f:((100.0f*(float)detail.m_seaSamples)/(float)detail.m_cells.size());

    ImGui::Text("[%d,%d]  %.0f x %.0f m at %.1f m  |  %.0f..%.0f m",
        detail.m_cell.x, detail.m_cell.y, span, span,
        detail.m_metresPerSample, detail.m_lowest, detail.m_highest);

    if(seaShare>0.0f)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.45f, 0.65f, 0.9f, 1.0f), "|  %.0f%% sea", seaShare);
    }

    if(detail.m_lakeSamples>0)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.45f, 0.65f, 0.9f, 1.0f), "|  %.1f%% lake",
            (100.0f*(float)detail.m_lakeSamples)/(float)detail.m_cells.size());
    }

    if(detail.m_saltFlatSamples>0)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.85f, 0.83f, 0.72f, 1.0f), "|  %.1f%% salt flat",
            (100.0f*(float)detail.m_saltFlatSamples)/(float)detail.m_cells.size());
    }

    if(detail.m_hasRiver&&(detail.m_widest>0.0f))
    {
        //the numbers that make this region agree with the overview and with its neighbours
        ImGui::SameLine();
        ImGui::Text("|  river in %.1f out %.1f m3/s, up to %.0f m wide",
            detail.m_inflow, detail.m_outflow, detail.m_widest);
    }

    if(m_shownLayer!=m_layerIndex)
    {
        updateTexture();
        m_shownLayer=m_layerIndex;
    }

    m_imageView.draw();
}

}//namespace mapgen
