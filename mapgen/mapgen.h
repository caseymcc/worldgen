#ifndef _mapgen_app_h_
#define _mapgen_app_h_

#include "uiData.h"

#include "worldgen/utils/colorMap.h"

#include "worldViewer.h"
#include "worldControls.h"
#include "regionViewer.h"
#include "chunkViewer.h"
#include "legendViewer.h"
#include "infoViewer.h"

#include <glbinding/gl/gl.h>
using namespace gl;

#include <imgui.h>

namespace mapgen
{

class App
{
public:
    App();

    void initialize();
    void setSize(int width, int height);

    void draw();

private:
    void newWorld();

    WorldControls m_worldControls;
    WorldViewer m_worldViewer;
    RegionViewer m_regionViewer;
    ChunkViewer m_chunkViewer;
    LegendViewer m_legendViewer;
    InfoViewer m_infoViewer;

    bool m_show;

    int m_width;
    int m_height;

    //    World m_world;
    int m_worldWidth;
    int m_worldHeight;
    int m_worldDepth;

    UiData m_uiData;
    WorldGenerator *m_worldGenerator;
    //    WorldGenerator *m_worldGenerator;

//    int m_noiseSeed;
//    float m_plateFrequency;
//    float m_continentFrequency;
//
//    bool m_textureValid;
//    GLuint m_textureId;
//    int m_textureWidth;
//    int m_textureHeight;
//
//    int m_layerIndex;
//    std::vector<char> m_layerNames;
//    int m_info;
//    int m_overlay;
//    int m_overlayVector;
//
//    int m_plateCount;
//    std::vector<std::tuple<int, int, int>> m_plateColors;
//
//    worldgen::ColorMap m_tempColorMap;
//    worldgen::ColorMap m_moistureColorMap;
//    worldgen::ColorMap2D m_biomeColorMap;
//    worldgen::Progress m_progress;
//
//    ImVec2 lastDrawPos;
//    ImVec2 lastDrawSize;
};

}//namespace mapgen

#endif//_mapgen_app_h_