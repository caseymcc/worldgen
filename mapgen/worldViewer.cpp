#include "mapgen.h"

#include "worldgen/utils/randomcolor.h"
#include "worldgen/utils/colorMap.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "imguiHelpers.h"

#include <imglib/simpleImage.h>
#include <imglib/imageIo.h>
#include <imglib/draw.h>

#include <tuple>

namespace mapgen
{

WorldViewer::WorldViewer():
    m_textureValid(false),
    m_dragging(false),
    m_lastDrag(0.0f, 0.0f)
{
    m_worldWidth=4194304;
    m_worldHeight=2097152;
    m_worldDepth=2560;

    worldgen::ColorScale tempColorScale;

    tempColorScale.addColorPosition(glm::ivec4(255, 255, 255, 255), 0.0f);
    tempColorScale.addColorPosition(glm::ivec4(255, 0, 255, 255), 0.2f);
    tempColorScale.addColorPosition(glm::ivec4(0, 0, 255, 255), 0.4f);
    tempColorScale.addColorPosition(glm::ivec4(0, 255, 0, 255), 0.6f);
    tempColorScale.addColorPosition(glm::ivec4(255, 0, 0, 255), 0.8f);
    tempColorScale.addColorPosition(glm::ivec4(0, 0, 0, 255), 1.0f);

    m_tempColorMap=generateColorMap(tempColorScale, 64);

    worldgen::ColorScale moistureColorScale;

    moistureColorScale.addColorPosition(glm::ivec4(255, 0, 0, 255), 0.0f);
    moistureColorScale.addColorPosition(glm::ivec4(0, 255, 0, 255), 0.5f);
    moistureColorScale.addColorPosition(glm::ivec4(0, 0, 255, 255), 1.0f);

    m_moistureColorMap=worldgen::generateColorMap(moistureColorScale, 64);

    m_biomeColorMap=worldgen::generateHieghtMoistureColorMap(64, 64);

    m_plateCount=16;
}

void WorldViewer::onInitialize()
{
//    WorldGeneratorTemplate *genTemplate=(WorldGeneratorTemplate *)&m_world.getGenerator();
//    m_worldGenerator=(WorldGenerator *)genTemplate->get();
//    worldgen::WorldDescriptors descriptors;
//    worldgen::Progress progress;
//
//    m_worldGenerator.reset(new WorldGenerator());
//
//    descriptors.setSize({4194304, 2097152, 2560});
//    descriptors.setRegionSize({16, 16, 16});
//    descriptors.setChunkSize({64, 64, 16});
//
//    m_worldGenerator->create(&descriptors, progress);
//
    m_imageView.setName("WorldViewerImage");
    m_imageView.initialize();
    m_imageView.setWrapX(true);

    m_layerIndex=0;
//    m_info=0;
//    m_overlay=0;
//    m_overlayVector=0;
//    m_layerNames=packVectorString({"Terrain", "Plate Info", "Tectonic Plates", "Plates Distance", "Terrain Scale"});

    m_worldGenerator=m_uiData->generator.get();
	updateTexture();
}

void WorldViewer::onPosition()
{
    m_imageView.setPosition(m_x+1, m_y+1);
}

void WorldViewer::onSize()
{
    m_imageView.setSize(m_width-1, m_height-1);
}

void WorldViewer::onDraw()
{
    m_worldGenerator=m_uiData->generator.get();
    worldgen::EquiRectDescriptors &descriptors=m_worldGenerator->getDecriptors();

    if(m_uiData->forceUpdate)
        updateTexture();

    m_imageView.draw();

    //--- zoom and pan ---
    //The map is one cell per pixel, so most of what the generator produces is invisible at fit
    //width. The wheel magnifies about the pointer, which is the only zoom that feels like it is
    //doing what you asked; dragging with a button other than the left one moves the view, because
    //the left button already means select this cell.
    ImVec2 mouse=ImGui::GetMousePos();
    bool overImage=m_imageView.isHovered(mouse);
    float zoom=m_imageView.getZoom();
    ImVec2 center=m_imageView.getCenter();

    if(overImage)
    {
        float wheel=ImGui::GetIO().MouseWheel;

        if(wheel!=0.0f)
        {
            ImVec2 unit;

            m_imageView.screenToUnit(mouse, unit);

            float wanted=zoom*powf(1.2f, wheel);

            wanted=std::max(1.0f, std::min(wanted, 64.0f));

            //hold whatever is under the pointer still while the magnification changes
            float half=0.5f/zoom;
            float atX=(center.x-half)+(unit.x/zoom);
            float atY=(center.y-half)+(unit.y/zoom);

            center.x=atX+((0.5f-unit.x)/wanted);
            center.y=atY+((0.5f-unit.y)/wanted);
            zoom=wanted;

            m_imageView.setView(center, zoom);
        }
    }

    if(ImGui::IsMouseDragging(ImGuiMouseButton_Right)||ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
    {
        ImVec2 drag=ImGui::IsMouseDragging(ImGuiMouseButton_Right)
            ?ImGui::GetMouseDragDelta(ImGuiMouseButton_Right)
            :ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);

        if(m_dragging||overImage)
        {
            ImVec2 moved=ImVec2(drag.x-m_lastDrag.x, drag.y-m_lastDrag.y);
            //The drawn rectangle keeps the map's 2:1 shape, so a vertical drag covers half as many
            //pixels as the same horizontal one. Each axis has to be divided by its own span or the
            //view creeps up and down at half the speed it moves sideways.
            float spanX=m_imageView.getImageWidth();
            float spanY=m_imageView.getImageHeight();

            if((spanX>0.0f)&&(spanY>0.0f))
            {
                center.x-=(moved.x/spanX)/zoom;
                center.y-=(moved.y/spanY)/zoom;
                m_imageView.setView(center, zoom);
            }

            m_dragging=true;
            m_lastDrag=drag;
        }
    }
    else
    {
        m_dragging=false;
        m_lastDrag=ImVec2(0.0f, 0.0f);
    }

    ImVec2 imagePos;

    m_imageView.screenToTexel(mouse, imagePos);

    int texturePosX=(int)imagePos.x;
    int texturePosY=(int)imagePos.y;

    m_uiData->worldPos.x=texturePosX;
    m_uiData->worldPos.y=texturePosY;

    //Clicking a cell asks for the ground inside it. The click has to be tested against this
    //widget's own rectangle - the mouse position above is clamped into range whether or not the
    //pointer was ever over the map, so acting on it unguarded would select a cell from a click
    //anywhere in the window.
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)&&overImage)
    {
        m_uiData->selectedCell.x=texturePosX;
        m_uiData->selectedCell.y=texturePosY;
        m_uiData->detailValid=false;
    }

    //the view state, and how to get back out of it
    if(m_imageView.getZoom()>1.0f)
    {
        ImGui::SetCursorPos(ImVec2(8.0f, 6.0f));
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.75f), "%.1fx  (wheel zoom, right drag pan)", m_imageView.getZoom());
        ImGui::SameLine();

        if(ImGui::SmallButton("fit"))
            m_imageView.resetView();
    }
//    m_uiData->worldPos.x=mousePosition.x;
//    m_uiData->worldPos.y=mousePosition.y;

    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
    size_t index=texturePosY*influenceMapSize.x+texturePosX;
    float value;
    float overlayValue;

    if((index<0)||(index>=influenceMapSize.x*influenceMapSize.y))
        index=0;

    if((m_uiData->displayLayer==0) || (m_uiData->displayLayer==1))
        value=influenceMap[index].heightBase;
    else if(m_uiData->displayLayer==2)
        value=influenceMap[index].plateHeight;
    else if(m_uiData->displayLayer==3)
        value=(float)influenceMap[index].tectonicPlate;
    else if(m_uiData->displayLayer==5)
        value=m_worldGenerator->plateMap_noise[index];
    else if(m_uiData->displayLayer==6)
        value=m_worldGenerator->plate2Map_noise[index];
    else if(m_uiData->displayLayer==7)
        value=m_worldGenerator->plateDistance_noise[index];
    else if(m_uiData->displayLayer==8)
        value=m_worldGenerator->continentMap_noise[index];
    else if(m_uiData->displayLayer==9)
        value=m_worldGenerator->heightMap_noise[index];
    else if(m_uiData->displayLayer==10)
        value=influenceMap[index].plateDistanceValue;
    else if(m_uiData->displayLayer==11)
        value=m_worldGenerator->plateScaleMap[index];
    else
        value=0.0f;

    m_uiData->worldValue=value;

    if(m_uiData->displayOverlay > 0)
    {
        overlayValue=0.0f;

        if(m_overlay==1)
            overlayValue=influenceMap[index].collision;
        else if(m_overlay==2)
            overlayValue=influenceMap[index].collision*influenceMap[index].plateDistanceValue;
        else if(m_overlay==3)
            overlayValue=influenceMap[index].terrainScale;
        else if(m_overlay==4)
            overlayValue=(influenceMap[index].temperature+90.0f)/160.0f;
        else if((m_overlay==5) || (m_overlay==6))
            overlayValue=std::min(influenceMap[index].moisture, 1.0f);
        else if(m_overlay==7)
            overlayValue=(float)influenceMap[index].weatherCell;
        else if(m_overlay==8)
            overlayValue=(float)influenceMap[index].weatherBand;
        
        m_uiData->worldOverlay=true;
        m_uiData->worldOverlayValue=overlayValue;
    }
    else
        m_uiData->worldOverlay=false;
}

void WorldViewer::updateTexture()
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();

    std::vector<GLubyte> textureBuffer;

    textureBuffer.resize(influenceMap.size()*4);

    //    int min=std::numeric_limits<int>::max();
    //    int max=0;
    //
    //    for(size_t i=0; i<influenceMap.size(); ++i)
    //    {
    //        int value=influenceMap[i].tectonicPlate;
    //
    //        min=std::min(min, value);
    //        max=std::max(max, value);
    //    }
    //m_layerNames=packVectorString({"Terrain", "Plate Info", "Tectonic Plates", "Plates Distance"});

    if(m_layerIndex==0)
        updatePlateInfoTexture(textureBuffer);
    if(m_layerIndex==1)
        updatePlateInfoTexture(textureBuffer);
    else if(m_layerIndex==2)
        updatePlateTexture(textureBuffer);
    else if(m_layerIndex==3)
        updatePlateDistanceTexture(textureBuffer);

#if 1
    imglib::SimpleImage image;

    image.format=imglib::Format::RGBA;
    image.depth=imglib::Depth::Bit8;
    image.width=influenceMapSize.x;
    image.height=influenceMapSize.y;
    image.data=&textureBuffer[0];

    imglib::save(image, "test.png");
#endif
}

void WorldViewer::updatePlateTexture(std::vector<GLubyte> &textureBuffer)
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();

    int plateCount=m_worldGenerator->getPlateCount();

    worldgen::RandomColorGenerator colorGenerator;

    if(m_plateColors.size() <plateCount)
        m_plateColors=colorGenerator.randomColors(plateCount);

    size_t index=0;
    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        auto &color=m_plateColors[influenceMap[i].tectonicPlate];

        textureBuffer[index++]=(GLubyte)std::get<0>(color);
        textureBuffer[index++]=(GLubyte)std::get<1>(color);
        textureBuffer[index++]=(GLubyte)std::get<2>(color);
        textureBuffer[index++]=255;
    }

    imglib::SimpleImage image;

    image.width=influenceMapSize.x;
    image.height=influenceMapSize.y;
    image.data=&textureBuffer[0];

    m_imageView.updateImage(&image);
}

void WorldViewer::updatePlateInfoTexture(std::vector<GLubyte> &textureBuffer)
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
    int plateCount=m_worldGenerator->getPlateCount();
    worldgen::RandomColorGenerator colorGenerator;

    if(m_plateColors.size()<plateCount)
        m_plateColors=colorGenerator.randomColors(plateCount);

    DisplayLayers displayLayer=getEnumFromIndex(uiDisplayLayers, m_uiData->displayLayer);

    size_t index=0;
    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        glm::ivec4 color;
        
        if(displayLayer == DisplayLayers::Map)
        {
            color=m_biomeColorMap.color((size_t)(64.0f*influenceMap[i].heightBase), (size_t)(64.0f*influenceMap[i].moisture));

            float value=(influenceMap[i].temperature+90.0f)/160.0f;

            if((value<0.4f) && (influenceMap[i].heightBase>0.5f)) //polar caps
            {
                if(value<0.2f)
                {
                    color.r=255;
                    color.g=255;
                    color.b=255;
                }
                else
                {
                    value=5.0f*(0.4f-value);
                    color.r=(255*value)+(color.r*(1.0f-value));
                    color.g=(255*value)+(color.g*(1.0f-value));
                    color.b=(255*value)+(color.b*(1.0f-value));
                }
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Height)
        {
            unsigned char value=255*influenceMap[i].heightBase;

            color.r=value;
            color.g=value;
            color.b=value;
            color.a=255;
        }
        else if(displayLayer==DisplayLayers::Value)
            color=m_biomeColorMap.color((size_t)64*influenceMap[i].plateHeight, 32);
        else if(displayLayer==DisplayLayers::Plates)
        {
            auto &plateColor=m_plateColors[influenceMap[i].tectonicPlate];

            color.r=std::get<0>(plateColor);
            color.g=std::get<1>(plateColor);
            color.b=std::get<2>(plateColor);
            color.a=255;
        }
        else if((displayLayer == DisplayLayers::PlateMapNoise) ||
            (displayLayer == DisplayLayers::PlateMap2Noise) ||
            (displayLayer == DisplayLayers::PlateDistanceNoise) ||
            (displayLayer == DisplayLayers::ContinentMapNoise) ||
            (displayLayer == DisplayLayers::HeightMapNoise) ||
            (displayLayer == DisplayLayers::PlateDistanceValue))
        {
            float fValue;

            if(displayLayer == DisplayLayers::PlateMapNoise)
                fValue=m_worldGenerator->plateMap_noise[i];
            else if(displayLayer == DisplayLayers::PlateMap2Noise)
                fValue=m_worldGenerator->plate2Map_noise[i];
            else if(displayLayer == DisplayLayers::PlateDistanceNoise)
                fValue=m_worldGenerator->plateDistance_noise[i];
            else if(displayLayer == DisplayLayers::ContinentMapNoise)
                fValue=m_worldGenerator->continentMap_noise[i];
            else if(displayLayer == DisplayLayers::HeightMapNoise)
                fValue=m_worldGenerator->heightMap_noise[i];
            else if(displayLayer == DisplayLayers::PlateDistanceValue)
                fValue=influenceMap[i].plateDistanceValue;

            unsigned char value=255*fValue;

            color.r=value;
            color.g=value;
            color.b=value;
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Climate)
        {
            glm::ivec3 climate=worldgen::climateZoneColor(influenceMap[i].climate);

            color.r=climate.r;
            color.g=climate.g;
            color.b=climate.b;
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Regions)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=20; color.g=34; color.b=64;
            }
            else if(influenceMap[i].culturalRegion<0)
            {//barrier ground, or a sliver too small to be a region of its own
                color.r=52; color.g=50; color.b=48;
            }
            else
            {
                int r=influenceMap[i].culturalRegion;

                color.r=((r*67)%180)+60;
                color.g=((r*151)%180)+60;
                color.b=((r*211)%180)+60;
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Barriers)
        {
            worldgen::BarrierType barrier=(worldgen::BarrierType)influenceMap[i].barrier;

            if(barrier == worldgen::BarrierType::Ocean)
            {
                color.r=20; color.g=34; color.b=64;
            }
            else if(barrier == worldgen::BarrierType::Mountain)
            {
                color.r=170; color.g=150; color.b=130;
            }
            else if(barrier == worldgen::BarrierType::Desert)
            {
                color.r=210; color.g=160; color.b=70;
            }
            else
            {//ground people can move across
                color.r=70; color.g=140; color.b=80;
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::States)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=20; color.g=34; color.b=64;
            }
            else if(influenceMap[i].polity<0)
            {//beyond the reach of any state
                color.r=54; color.g=52; color.b=48;
            }
            else
            {
                int s=influenceMap[i].polity;

                //the marches are drawn dimmer, because that is what they are: the same state,
                //held loosely
                float hold=influenceMap[i].march?0.55f:1.0f;

                color.r=(unsigned char)((((s*83)%170)+70)*hold);
                color.g=(unsigned char)((((s*157)%170)+70)*hold);
                color.b=(unsigned char)((((s*197)%170)+70)*hold);
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Control)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=20; color.g=34; color.b=64;
            }
            else if(influenceMap[i].polity<0)
            {
                color.r=44; color.g=42; color.b=40;
            }
            else
            {//bright in the heartland, fading out to the frontier
                float control=influenceMap[i].control;

                color.r=(unsigned char)(60.0f+(190.0f*control));
                color.g=(unsigned char)(50.0f+(140.0f*control));
                color.b=(unsigned char)(45.0f+(60.0f*control));
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Trade)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=16; color.g=28; color.b=54;
            }
            else
            {
                unsigned char ground=44+(unsigned char)(30.0f*(influenceMap[i].heightBase-0.5f)*2.0f);

                color.r=ground; color.g=ground; color.b=ground;
            }

            if(influenceMap[i].traffic>0.0f)
            {//the busier the road, the brighter it burns
                float traffic=influenceMap[i].traffic;

                color.r=(unsigned char)std::min(255.0f, (float)color.r+(60.0f+(195.0f*traffic)));
                color.g=(unsigned char)std::min(255.0f, (float)color.g+(45.0f+(150.0f*traffic)));
                color.b=(unsigned char)std::min(255.0f, (float)color.b+(20.0f+(50.0f*traffic)));
            }

            if(influenceMap[i].chokepoint)
            {//a gap the world has to squeeze through, and somebody will tax it
                color.r=255; color.g=70; color.b=70;
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Cities)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=16; color.g=28; color.b=54;
            }
            else
            {
                unsigned char ground=(influenceMap[i].coast!=(int)worldgen::CoastType::None)?78:48;

                color.r=ground; color.g=ground; color.b=ground;
            }

            if(influenceMap[i].traffic>0.0f)
            {
                color.r=(unsigned char)std::min(255.0f, (float)color.r+(40.0f*influenceMap[i].traffic));
                color.g=(unsigned char)std::min(255.0f, (float)color.g+(35.0f*influenceMap[i].traffic));
                color.b=(unsigned char)std::min(255.0f, (float)color.b+(15.0f*influenceMap[i].traffic));
            }

            if(influenceMap[i].city>=0)
            {//each of the four drains gets its own colour, and a parasite burns white
                const std::vector<worldgen::City> &cities=m_uiData->generator->getCities();
                const worldgen::City &city=cities[influenceMap[i].city];

                if(city.m_parasite)
                {
                    color.r=255; color.g=255; color.b=230;
                }
                else if(city.m_node == worldgen::NodeType::Mouth)
                {
                    color.r=90; color.g=200; color.b=255;
                }
                else if(city.m_node == worldgen::NodeType::Strait)
                {
                    color.r=255; color.g=80; color.b=80;
                }
                else if(city.m_node == worldgen::NodeType::Anchorage)
                {
                    color.r=120; color.g=255; color.b=140;
                }
                else
                {
                    color.r=255; color.g=200; color.b=60;
                }
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Remoteness)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=16; color.g=28; color.b=54;
            }
            else if(influenceMap[i].remoteness>=1.0f)
            {//cut off from the network entirely - nobody walks here from anywhere
                color.r=150; color.g=50; color.b=60;
            }
            else if(influenceMap[i].onTheBeatenTrack)
            {
                color.r=235; color.g=205; color.b=140;
            }
            else
            {//backcountry, fading out the further it is from anywhere people go
                float away=1.0f-worldgen::clamp(influenceMap[i].remoteness*3.0f, 0.0f, 1.0f);

                color.r=(unsigned char)(40.0f+(90.0f*away));
                color.g=(unsigned char)(45.0f+(85.0f*away));
                color.b=(unsigned char)(40.0f+(60.0f*away));
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Habitability)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=20; color.g=34; color.b=64;
            }
            else
            {//dark where nobody could live, bright where anybody could
                float h=influenceMap[i].habitability;

                color.r=(int)(40.0f+(90.0f*h));
                color.g=(int)(35.0f+(215.0f*h));
                color.b=(int)(45.0f+(70.0f*h));
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Peoples)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=20; color.g=34; color.b=64;
            }
            else if(influenceMap[i].dominantSpecies<0)
            {//land nobody would settle
                color.r=58; color.g=58; color.b=62;
            }
            else
            {//one colour per profile, so a caller adding a sixth people still gets a colour
                int k=influenceMap[i].dominantSpecies;
                float shade=0.55f+(0.45f*influenceMap[i].habitability);

                color.r=(int)(((k*97)%200+55)*shade);
                color.g=(int)(((k*151)%200+55)*shade);
                color.b=(int)(((k*211)%200+55)*shade);
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Ruins)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=18; color.g=30; color.b=58;
            }
            else
            {
                unsigned char value=48+(unsigned char)(40.0f*(influenceMap[i].heightBase-0.5f)*2.0f);

                color.r=value; color.g=value; color.b=value;
            }

            if(influenceMap[i].ruinCondition>0.0f)
            {//the hue says what it was, the brightness how much of it is still standing
                float condition=0.35f+(0.65f*influenceMap[i].ruinCondition);
                int base[3]={200, 170, 120};

                switch((worldgen::RuinKind)influenceMap[i].ruinKind)
                {
                case worldgen::RuinKind::Fortress: base[0]=210; base[1]=90;  base[2]=80;  break;
                case worldgen::RuinKind::MineHead: base[0]=150; base[1]=140; base[2]=210; break;
                case worldgen::RuinKind::Harbour:  base[0]=80;  base[1]=190; base[2]=225; break;
                case worldgen::RuinKind::Terraces: base[0]=140; base[1]=205; base[2]=110; break;
                case worldgen::RuinKind::Tomb:     base[0]=235; base[1]=215; base[2]=120; break;
                case worldgen::RuinKind::Bridge:   base[0]=225; base[1]=150; base[2]=205; break;
                default: break;   //Monument keeps the sandstone default
                }

                color.r=(int)(base[0]*condition);
                color.g=(int)(base[1]*condition);
                color.b=(int)(base[2]*condition);
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Fuel)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=20; color.g=34; color.b=64;
            }
            else
            {
                glm::ivec3 fuel=worldgen::fuelSourceColor(influenceMap[i].fuel);

                color.r=fuel.r; color.g=fuel.g; color.b=fuel.b;
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Currents)
        {
            const auto &cell=influenceMap[i];

            if(cell.heightBase>=0.5f)
            {//land, tinted by whatever current washes it
                unsigned char value=70;

                color.r=value; color.g=value; color.b=value;

                if(cell.coastCurrent == worldgen::CurrentKind::Cold)
                {
                    color.r=60; color.g=90; color.b=150;
                }
                else if(cell.coastCurrent == worldgen::CurrentKind::Warm)
                {
                    color.r=150; color.g=80; color.b=60;
                }
            }
            else
            {//warm water red, cold water blue, by how far off the local sea temperature it is
                float warmth=std::max(-6.0f, std::min(6.0f, cell.currentWarmth))/6.0f;

                color.r=40+((warmth>0.0f)?(int)(200.0f*warmth):0);
                color.g=40;
                color.b=40+((warmth<0.0f)?(int)(200.0f*-warmth):0);
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Deposits)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=20; color.g=34; color.b=64;
            }
            else if(influenceMap[i].deposit == worldgen::DepositType::None)
            {//barren ground, dimmed so the deposits stand out against it
                unsigned char value=55+(unsigned char)(70.0f*(influenceMap[i].heightBase-0.5f)*2.0f);

                color.r=value; color.g=value; color.b=value;
            }
            else
            {
                glm::ivec3 deposit=worldgen::depositTypeColor(influenceMap[i].deposit);

                color.r=deposit.r; color.g=deposit.g; color.b=deposit.b;
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Rocks)
        {
            if(influenceMap[i].heightBase<0.5f)
            {
                color.r=22; color.g=38; color.b=72;
            }
            else
            {
                glm::ivec3 rock=worldgen::rockTypeColor(influenceMap[i].rock);

                color.r=rock.r; color.g=rock.g; color.b=rock.b;
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Rivers)
        {
            const auto &cell=influenceMap[i];

            if(cell.heightBase<0.5f)
            {
                color.r=18; color.g=32; color.b=64;
            }
            else
            {//land shaded by height so the valleys the rivers run down are readable
                unsigned char value=60+(unsigned char)(120.0f*(cell.heightBase-0.5f)*2.0f);

                color.r=value; color.g=value; color.b=value;
            }

            if(cell.water == worldgen::WaterBody::River)
            {//wider rivers draw brighter
                float scale=std::min(1.0f, (float)cell.streamOrder/4.0f);

                if(cell.permanentRiver)
                {
                    color.r=30; color.g=110+(int)(120*scale); color.b=200+(int)(55*scale);
                }
                else
                {//ephemeral, a wadi for most of the year
                    color.r=170+(int)(60*scale); color.g=140+(int)(60*scale); color.b=40;
                }
            }
            else if(cell.water == worldgen::WaterBody::Delta)
            {
                color.r=210; color.g=240; color.b=120;
            }
            else if(cell.water == worldgen::WaterBody::Lake)
            {
                color.r=40; color.g=90; color.b=220;
            }
            else if(cell.water == worldgen::WaterBody::GlacialLake)
            {
                color.r=150; color.g=225; color.b=245;
            }
            else if(cell.water == worldgen::WaterBody::SaltLake)
            {
                color.r=220; color.g=220; color.b=235;
            }
            else if(cell.water == worldgen::WaterBody::InlandDelta)
            {
                color.r=200; color.g=150; color.b=60;
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Coast)
        {
            worldgen::CoastType coast=(worldgen::CoastType)influenceMap[i].coast;

            if(coast == worldgen::CoastType::None)
            {//everything that is not a shoreline, dimmed so the coasts stand out
                unsigned char value=(influenceMap[i].heightBase>=0.5f)?70:25;

                color.r=value;
                color.g=value;
                color.b=value+15;
            }
            else
            {
                glm::ivec3 coastColor=worldgen::coastTypeColor(coast);

                color.r=coastColor.r;
                color.g=coastColor.g;
                color.b=coastColor.b;
            }
            color.a=255;
        }
        else if(displayLayer == DisplayLayers::Hotspots)
        {
            float hotspot=influenceMap[i].hotspot;

            color.r=0;
            color.g=0;
            color.b=0;
            color.a=255;

            if(hotspot>0.0f)
                color.r=std::min((int)(255.0f*(hotspot/0.25f)), 255);
            else if(hotspot<0.0f)
                color.b=std::min((int)(255.0f*(-hotspot/0.25f)), 255);
        }
        else if(displayLayer == DisplayLayers::PlateScale)
        {
            color.r=255*m_worldGenerator->plateScaleMap[i];
            color.g=255*m_worldGenerator->plate2ScaleMap[i];
            color.b=0;
            color.a=255;
        }
        else
        {
            color.r=0;
            color.g=0;
            color.b=0;
            color.a=255;
        }

        DisplayOverlays overlay=getEnumFromIndex(uiDisplayOverlays, m_uiData->displayOverlay);
        
        if(overlay == DisplayOverlays::Collision)
        {
            if(influenceMap[i].collision<0.0f)
                color.r=color.r+(255.0f*-influenceMap[i].collision);
            else
                color.g=color.g+(255.0f*influenceMap[i].collision);
        }
        else if(overlay == DisplayOverlays::CollisionDistance)
        {
            if(influenceMap[i].collision<0.0f)
                color.r=color.r+(255.0f*-influenceMap[i].collision*influenceMap[i].plateDistanceValue);
            else
                color.g=color.g+(255.0f*influenceMap[i].collision*influenceMap[i].plateDistanceValue);
        }
        else if(overlay == DisplayOverlays::Shear)
        {
            color.b=color.b+(255.0f*influenceMap[i].shear);
        }
        else if(overlay == DisplayOverlays::BoundaryType)
        {//green convergent, red divergent, blue transform
            worldgen::BoundaryType boundaryType=influenceMap[i].boundaryType;
            float strength=std::abs(influenceMap[i].collision)+influenceMap[i].shear;

            if(boundaryType == worldgen::BoundaryType::Convergent)
                color.g=color.g+(255.0f*strength);
            else if(boundaryType == worldgen::BoundaryType::Divergent)
                color.r=color.r+(255.0f*strength);
            else if(boundaryType == worldgen::BoundaryType::Transform)
                color.b=color.b+(255.0f*strength);
        }
        else if(overlay == DisplayOverlays::Orogeny)
        {//andean yellow, laramide orange, ural cyan, himalayan magenta
            worldgen::OrogenyType orogeny=influenceMap[i].orogeny;
            float strength=std::min(1.0f, std::abs(influenceMap[i].terrainScale)*3.0f);

            if(orogeny == worldgen::OrogenyType::Andean)
            {
                color.r=color.r+(255.0f*strength);
                color.g=color.g+(220.0f*strength);
            }
            else if(orogeny == worldgen::OrogenyType::Laramide)
            {
                color.r=color.r+(255.0f*strength);
                color.g=color.g+(120.0f*strength);
            }
            else if(orogeny == worldgen::OrogenyType::Ural)
            {
                color.g=color.g+(220.0f*strength);
                color.b=color.b+(255.0f*strength);
            }
            else if(orogeny == worldgen::OrogenyType::Himalayan)
            {
                color.r=color.r+(255.0f*strength);
                color.b=color.b+(255.0f*strength);
            }
        }
        else if(overlay == DisplayOverlays::Scale)
        {
            if(influenceMap[i].terrainScale<0.0f)
                color.r=color.r+(255.0f*-influenceMap[i].terrainScale);
            else
                color.g=color.g+(255.0f*influenceMap[i].terrainScale);
        }
        else if(overlay == DisplayOverlays::Temp)
        {
            //color scale intended for -50 to 60, temp runs -90 to 60
            float value=(influenceMap[i].temperature+50.0f)/110.0f;

            value=std::max(value, 0.0f);
            value=std::min(value, 1.0f);

            color=color+(m_tempColorMap.color((size_t)63*value));
        }
        else if(overlay == DisplayOverlays::Moisture)
        {
            float moisture=std::min(influenceMap[i].moisture, 1.0f);
            color=color+(m_moistureColorMap.color((size_t)63*moisture));
        }
        else if(overlay == DisplayOverlays::MoistureGreyScale)
        {
            unsigned char value=255*influenceMap[i].moisture;

            if(influenceMap[i].heightBase > 0.5f)
                color=color+glm::ivec4(value, value, value, 255);
        }
        else if(overlay == DisplayOverlays::WeatherCells)
        {
            auto &plateColor=m_plateColors[influenceMap[i].weatherCell];

            color.r=std::get<0>(plateColor);
            color.g=std::get<1>(plateColor);
            color.b=std::get<2>(plateColor);
            color.a=255;
        }
        else if(overlay == DisplayOverlays::Pressure)
        {//blue where air rises into a low, red where it sinks into a high
            float pressure=influenceMap[i].pressure;

            color.r=color.r+(255.0f*pressure);
            color.b=color.b+(255.0f*(1.0f-pressure));
        }
        else if(overlay == DisplayOverlays::WeatherBands)
        {
            auto &plateColor=m_plateColors[influenceMap[i].weatherBand];

            color.r=std::get<0>(plateColor);
            color.g=std::get<1>(plateColor);
            color.b=std::get<2>(plateColor);
            color.a=255;
        }

        
        color.r=std::min(color.r, 255);
        color.g=std::min(color.g, 255);
        color.b=std::min(color.b, 255);

        textureBuffer[index++]=(GLubyte)color.r;
        textureBuffer[index++]=(GLubyte)color.g;
        textureBuffer[index++]=(GLubyte)color.b;
        textureBuffer[index++]=255;
    }

    DisplayVectors displayVector=getEnumFromIndex(uiDisplayVectors, m_uiData->displayVector);

    if(displayVector != DisplayVectors::NoVector)
    {
        imglib::SimpleImage textureImage(imglib::Format::RGBA, imglib::Depth::Bit8, influenceMapSize.x, influenceMapSize.y, &textureBuffer[0], influenceMap.size());
        glm::tvec4<unsigned char> colorRed(255, 0, 0, 255);
        glm::tvec4<unsigned char> colorWhite(255, 255, 255, 255);

//        colorRed.r=255;
//        colorRed.g=0;
//        colorRed.b=0;
//        colorRed.a=255;

        glm::vec2 point;
        index=0;

        size_t skip=10;
        float fskip=(float)skip;

        point.y=0.0f;
        for(size_t y=0; y<influenceMapSize.y; y+=skip)
        {
            index=y*influenceMapSize.x;
            point.x=0.0f;
            for(size_t x=0; x<influenceMapSize.x; x+=skip)
            {
                glm::vec2 endPoint;
                glm::vec2 direction;

                if(displayVector == DisplayVectors::Direction)
                    direction=influenceMap[index].direction;
                else
                    direction=influenceMap[index].airDirection;
                
                direction.y=-direction.y;//for images y origin is top left
                endPoint=(direction*8.0f)+point;

                endPoint.x=std::max(endPoint.x, 0.0f);
                endPoint.x=std::min(endPoint.x, (float)influenceMapSize.x);
                endPoint.y=std::max(endPoint.y, 0.0f);
                endPoint.y=std::min(endPoint.y, (float)influenceMapSize.y);

                imglib::drawLine(point, endPoint, colorRed, textureImage);

//                auto &plateColor=m_plateColors[influenceMap[index].tectonicPlate];
//
//                colorWhite.r=std::get<0>(plateColor);
//                colorWhite.g=std::get<1>(plateColor);
//                colorWhite.b=std::get<2>(plateColor);
//                colorWhite.a=255;

                imglib::drawPoint(point, colorWhite, textureImage);
                index+=skip;
                point.x+=fskip;
            }
            point.y+=fskip;
        }
    }

    imglib::SimpleImage image;

    image.width=influenceMapSize.x;
    image.height=influenceMapSize.y;
    image.data=&textureBuffer[0];

    m_imageView.updateImage(&image);
}

void WorldViewer::updatePlateDistanceTexture(std::vector<GLubyte> &textureBuffer)
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();

    float min=std::numeric_limits<float>::max();
    float max=0.0f;

    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        float value=influenceMap[i].plateDistanceValue;

        min=std::min(min, value);
        max=std::max(max, value);
    }

    float delta=255/(max-min);
    size_t index=0;

    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        GLubyte value=std::max(0, std::min(255, (int)((influenceMap[i].plateDistanceValue-min)*delta)));

        textureBuffer[index++]=value;
        textureBuffer[index++]=value;
        textureBuffer[index++]=value;
        textureBuffer[index++]=255;
    }

    imglib::SimpleImage image;

    image.width=influenceMapSize.x;
    image.height=influenceMapSize.y;
    image.data=&textureBuffer[0];

    m_imageView.updateImage(&image);
}

//void WorldViewer::updateContinentTexture(std::vector<GLubyte> &textureBuffer)
//{
//    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
//    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
//
//    int plateCount=m_worldGenerator->getPlateCount();
//
//    RandomColorGenerator colorGenerator;
//
//    if(m_plateColors.size()<plateCount)
//        m_plateColors=colorGenerator.randomColors(plateCount);
//
//    size_t index=0;
//
//    for(size_t i=0; i<influenceMap.size(); ++i)
//    {
//        int elevation=32+32*influenceMap[i].continentValue;
//        glm::ivec4 color=m_biomeColorMap.color(elevation, 32);
//
//        textureBuffer[index++]=(GLubyte)color.r;
//        textureBuffer[index++]=(GLubyte)color.g;
//        textureBuffer[index++]=(GLubyte)color.b;
//        textureBuffer[index++]=255;
//    }
//
//    m_textureWidth=influenceMapSize.x;
//    m_textureHeight=influenceMapSize.y;
//
//    glBindTexture(GL_TEXTURE_2D, m_textureId);
//
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureBuffer[0]);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//
//    m_textureValid=true;
//}

void WorldViewer::updateHeightMapTexture(std::vector<GLubyte> &textureBuffer)
{
    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
    imglib::SimpleImage textureImage(imglib::Format::RGBA, imglib::Depth::Bit8, influenceMapSize.x, influenceMapSize.y, &textureBuffer[0], influenceMap.size());

    size_t index=0;

    for(size_t i=0; i<influenceMap.size(); ++i)
    {
        const float &height=influenceMap[i].heightBase;

        glm::ivec4 color=m_biomeColorMap.color((size_t)64*height, 32);

        textureBuffer[index++]=(GLubyte)color.r;
        textureBuffer[index++]=(GLubyte)color.g;
        textureBuffer[index++]=(GLubyte)color.b;
        //		textureBuffer[index++]=(GLubyte)255.0f*height;
        //		textureBuffer[index++]=(GLubyte)255.0f*height;
        //		textureBuffer[index++]=(GLubyte)255.0f*height;
        textureBuffer[index++]=255;
    }

//        imglib::SimpleImage image;
//
//        image.width=influenceMapSize.x;
//        image.height=influenceMapSize.y;
//        image.data=&textureBuffer[0];

    m_imageView.updateImage(&textureImage);
}


//void WorldViewer::updateGeometryTexture(std::vector<GLubyte> &textureBuffer)
//{
//    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
//    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
//	imglib::SimpleImage textureImage(imglib::Format::RGBA, imglib::Depth::Bit8, influenceMapSize.x, influenceMapSize.y, &textureBuffer[0], influenceMap.size());
//
//    int plateCount=m_worldGenerator->getPlateCount();
//
//    RandomColorGenerator colorGenerator;
//
//    if(m_plateColors.size()<plateCount)
//        m_plateColors=colorGenerator.randomColors(plateCount);
//
////    size_t index=0;
////
////    for(size_t i=0; i<influenceMap.size(); ++i)
////    {
////        const bool &isPoint=influenceMap[i].point;
////
////        if(isPoint)
////        {
////            textureBuffer[index++]=(GLubyte)255;
////            textureBuffer[index++]=(GLubyte)255;
////            textureBuffer[index++]=(GLubyte)255;
////        }
////        else
////        {
////            textureBuffer[index++]=(GLubyte)0;
////            textureBuffer[index++]=(GLubyte)0;
////            textureBuffer[index++]=(GLubyte)0;
////        }
////        textureBuffer[index++]=255;
//
////    }
//    std::vector<glm::vec2> &points=m_worldGenerator->m_influencePoints;
//	std::vector<std::vector<glm::vec2>> &lines=m_worldGenerator->m_influenceLines;
//	
////	int32_t color=0xff0000ff;
//    glm::tvec4<unsigned char> color;
//
//    color.r=255;
//    color.g=0;
//    color.b=0;
//    color.a=255;
//
//	for(std::vector<glm::vec2> &line:lines)
//	{
//		for(size_t i=1; i<line.size(); ++i)
//		{
//			imglib::drawLine(line[i-1], line[i], color, textureImage);
//		}
//	}
//
//    color.r=255;
//    color.g=255;
//    color.b=255;
//    color.a=255;
//
//    for(auto &point:points)
//        imglib::drawPoint(point, color, textureImage);
//
//    m_textureWidth=influenceMapSize.x;
//    m_textureHeight=influenceMapSize.y;
//
//    glBindTexture(GL_TEXTURE_2D, m_textureId);
//
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureBuffer[0]);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//
//    m_textureValid=true;
//}

}//namespace mapgen
