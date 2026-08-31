#include "mapgen.h"

#include "worldgen/utils/randomcolor.h"
#include "worldgen/utils/colorMap.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "imguiHelpers.h"

#include <imglib/simpleImage.h>
#include <imglib/draw.h>

#include <tuple>


namespace mapgen
{


App::App():
    m_show(true)
{
    m_worldWidth=4194304;
    m_worldHeight=2097152;
    m_worldDepth=2560;

//    worldgen::ColorScale tempColorScale;
//
//    tempColorScale.addColorPosition(glm::ivec4(255, 255, 255, 255), 0.0f);
//    tempColorScale.addColorPosition(glm::ivec4(255, 0, 255, 255), 0.2f);
//    tempColorScale.addColorPosition(glm::ivec4(0, 0, 255, 255), 0.4f);
//    tempColorScale.addColorPosition(glm::ivec4(0, 255, 0, 255), 0.6f);
//    tempColorScale.addColorPosition(glm::ivec4(255, 0, 0, 255), 0.8f);
//    tempColorScale.addColorPosition(glm::ivec4(0, 0, 0, 255), 1.0f);
//
//    m_tempColorMap=generateColorMap(tempColorScale, 64);
//
//    worldgen::ColorScale moistureColorScale;
//
//    moistureColorScale.addColorPosition(glm::ivec4(255, 0, 0, 255), 0.0f);
//    moistureColorScale.addColorPosition(glm::ivec4(0, 255, 0, 255), 0.5f);
//    moistureColorScale.addColorPosition(glm::ivec4(0, 0, 255, 255), 1.0f);
//
//    m_moistureColorMap=worldgen::generateColorMap(moistureColorScale, 64);
//
//    m_biomeColorMap=worldgen::generateHieghtMoistureColorMap(64, 64);
//
//    m_plateCount=16;
}

void App::initialize()
{
    //    WorldGeneratorTemplate *genTemplate=(WorldGeneratorTemplate *)&m_world.getGenerator();
    //    m_worldGenerator=(WorldGenerator *)genTemplate->get();
    newWorld();

    m_worldControls.setName("WorldControls");
    m_worldControls.initialize(&m_uiData);
    m_worldViewer.setName("WorldViewer");
    m_worldViewer.initialize(&m_uiData);
    m_regionViewer.setName("RegionViewer");
    m_regionViewer.initialize(&m_uiData);

    m_legendViewer.setName("LegendViewer");
    m_legendViewer.initialize(&m_uiData);
    m_chunkViewer.setName("ChunkViewer");
    m_chunkViewer.initialize(&m_uiData);
    m_infoViewer.setName("InfoViewer");
    m_infoViewer.initialize(&m_uiData);

//    m_layerIndex=0;
//    m_info=0;
//    m_overlay=0;
//    m_overlayVector=0;
//    m_layerNames=packVectorString({"Terrain", "Plate Info", "Tectonic Plates", "Plates Distance", "Terrain Scale"});

//    updateTexture();
}

void App::newWorld()
{
    m_uiData.generator.reset(new WorldGenerator());
    m_worldGenerator=m_uiData.generator.get();

    worldgen::WorldDescriptors descriptors;
    worldgen::Progress progress;

//    descriptors.setSize({4194304, 2097152, 2560});
    descriptors.setSize({1048576, 524288, 1024});
    descriptors.setRegionSize({16, 16, 16});
    descriptors.setChunkSize({64, 64, 16});

    m_worldGenerator->create(&descriptors, progress);
}

void App::setSize(int width, int height)
{
    m_width=width;
    m_height=height;

    int controlsWidth=350;
    int infoHeight=150;
    int displayWidth=std::max(m_width-controlsWidth, 0);
    int halfHeight=m_height/2;
    int halfDisplayWidth=displayWidth/2;

    m_worldControls.setPosition(0, 0);
    m_worldControls.setSize(controlsWidth, m_height-infoHeight);

    m_worldViewer.setPosition(controlsWidth, 0);
    m_worldViewer.setSize(displayWidth, halfHeight);

    m_regionViewer.setPosition(controlsWidth, halfHeight);
    m_regionViewer.setSize(halfDisplayWidth, halfHeight);
    
    //the legend sits beside the detail view, where the chunk viewer stub used to be
    m_legendViewer.setPosition(controlsWidth+halfDisplayWidth, halfHeight);
    m_legendViewer.setSize(halfDisplayWidth, halfHeight);

    m_infoViewer.setPosition(0, m_height-infoHeight);
    m_infoViewer.setSize(controlsWidth, infoHeight);
}

void App::draw()
{
    bool forceUpdate=false;

    worldgen::EquiRectDescriptors &descriptors=m_worldGenerator->getDecriptors();

    ImVec2 mousePosition=ImGui::GetMousePos();

    m_uiData.mousePos.x=mousePosition.x;
    m_uiData.mousePos.y=mousePosition.y;

    m_worldControls.draw();
    m_worldViewer.draw();
    m_regionViewer.draw();
    m_legendViewer.draw();
    m_infoViewer.draw();
}


//void App::generate()
//{
//    worldgen::Progress progress;
//
//    m_worldGenerator->m_plateSeed=m_noiseSeed;
//    m_worldGenerator->m_plateCount=m_plateCount;
//    m_worldGenerator->generateWorldOverview(progress);
//
//    //    updateTexture();
//}

}//namespace mapgen
