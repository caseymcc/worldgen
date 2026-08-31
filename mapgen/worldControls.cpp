#include "worldControls.h"

namespace mapgen
{

WorldControls::WorldControls()
{
    
}

void WorldControls::onInitialize()
{
    setName("Controls");
}

void WorldControls::onDraw()
{
    bool generateWorld=false;
    m_uiData->forceUpdate=false;

    WorldGenerator *worldGenerator=m_uiData->generator.get();
    worldgen::EquiRectDescriptors &descriptors=worldGenerator->getDecriptors();

    int controlsWidth=350;

    if(ImGui::Button("Generate"))
        generateWorld=true;

    ImGui::Separator();

    ImGui::InputInt("Seed", &worldGenerator->m_plateSeed);
    if(ImGui::SliderInt("Plate Count", &worldGenerator->m_plateCount, 1000, 10000))
        generateWorld=true;
    ImGui::SliderFloat("Plate Frequency", &descriptors.m_plateFrequency, 0.0001f, 1.0f, "%.4f", 3.0f);
    ImGui::SliderFloat("Continent Frequency", &descriptors.m_continentFrequency, 0.001f, 1.0f, "%.3f", 3.0f);

    if(generateWorld)
    {
        generate();
        m_uiData->forceUpdate=true;
    }

    ImGui::Separator();

//    if(ImGui::Combo("Layer", &m_layerIndex, &m_layerNames[0]))
//        m_uiData->forceUpdate=true;

    int info=m_uiData->displayLayer;
    ImGui::Text("Terrain");

    for(size_t i=0; i<uiDisplayLayers.size(); ++i)
    {
        ImGui::RadioButton(uiDisplayLayers[i].second.c_str(), &info, i);
    }
    if(info!=m_uiData->displayLayer)
    {
        m_uiData->displayLayer=info;
        m_uiData->forceUpdate=true;
    }

    int overlay=m_uiData->displayOverlay;
    ImGui::Text("Overlay");

    for(size_t i=0; i<uiDisplayOverlays.size(); ++i)
    {
        ImGui::RadioButton(uiDisplayOverlays[i].second.c_str(), &overlay, i);
    }
    if(overlay != m_uiData->displayOverlay)
    {
        m_uiData->displayOverlay=overlay;
        m_uiData->forceUpdate=true;
    }

    int displayVector=m_uiData->displayVector;
    ImGui::Text("Vector");
    
    for(size_t i=0; i<uiDisplayVectors.size(); ++i)
    {
        ImGui::RadioButton(uiDisplayVectors[i].second.c_str(), &displayVector, i);
    }
    if(displayVector != m_uiData->displayVector)
    {
        m_uiData->displayVector=displayVector;
        m_uiData->forceUpdate=true;
    }

//    ImVec2 mousePosition=ImGui::GetMousePos();
//
//    mousePosition.x=mousePosition.x-controlsWidth-lastDrawPos.x-1.0f;//-1.of border
//    mousePosition.y=mousePosition.y-lastDrawPos.y-1.0f;//-1.of border
//
//    if(mousePosition.x<0.0f)
//        mousePosition.x=0.0f;
//    if(mousePosition.y<0.0f)
//        mousePosition.y=0.0f;
//
//    if(mousePosition.x>lastDrawSize.x-1)
//        mousePosition.x=lastDrawSize.x-1;
//    if(mousePosition.y>lastDrawSize.y-1)
//        mousePosition.y=lastDrawSize.y-1;
//
//    int texturePosX=mousePosition.x/lastDrawSize.x*m_textureWidth;
//    int texturePosY=mousePosition.y/lastDrawSize.y*m_textureHeight;
//
//    const typename WorldGenerator::InfluenceMap &influenceMap=m_worldGenerator->getInfluenceMap();
//    const glm::ivec2 &influenceMapSize=m_worldGenerator->getInfluenceMapSize();
//    size_t index=texturePosY*influenceMapSize.x+texturePosX;
//    float value;
//    float overlayValue;
//
//    if((index<0)||(index>=influenceMapSize.x*influenceMapSize.y))
//        index=0;
//
//    if((m_info==0) || (m_info==1))
//        value=influenceMap[index].heightBase;
//    else if(m_info==2)
//        value=influenceMap[index].plateHeight;
//    else if(m_info==3)
//        value=(float)influenceMap[index].tectonicPlate;
//    else if(m_info==5)
//        value=m_worldGenerator->plateMap_noise[index];
//    else if(m_info==6)
//        value=m_worldGenerator->plate2Map_noise[index];
//    else if(m_info==7)
//        value=m_worldGenerator->plateDistance_noise[index];
//    else if(m_info==8)
//        value=m_worldGenerator->continentMap_noise[index];
//    else if(m_info==9)
//        value=m_worldGenerator->heightMap_noise[index];
//    else if(m_info==10)
//        value=influenceMap[index].plateDistanceValue;
//    else if(m_info==11)
//        value=m_worldGenerator->plateScaleMap[index];
//    else
//        value=0.0f;
//
//    ImGui::Text("Position: %d, %d : %f", texturePosX, texturePosY, value);
//
//    if(m_overlay > 0)
//    {
//        overlayValue=0.0f;
//
//        if(m_overlay==1)
//            overlayValue=influenceMap[index].collision;
//        else if(m_overlay==2)
//            overlayValue=influenceMap[index].collision*influenceMap[index].plateDistanceValue;
//        else if(m_overlay==3)
//            overlayValue=influenceMap[index].terrainScale;
//        else if(m_overlay==4)
//            overlayValue=(influenceMap[index].temperature+90.0f)/160.0f;
//        else if((m_overlay==5) || (m_overlay==6))
//            overlayValue=std::min(influenceMap[index].moisture, 1.0f);
//        else if(m_overlay==7)
//            overlayValue=(float)influenceMap[index].weatherCell;
//        else if(m_overlay==8)
//            overlayValue=(float)influenceMap[index].weatherBand;
//        ImGui::Text("Overlay: %f", overlayValue);
//    }
}

void WorldControls::generate()
{
    WorldGenerator *worldGenerator=m_uiData->generator.get();
    worldgen::Progress progress;

    worldGenerator->generateWorldOverview(progress);
}

}//namespace mapgen
