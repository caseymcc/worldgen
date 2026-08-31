#ifndef _mapgen_worldViewer_h_
#define _mapgen_worldViewer_h_

#include "uiDataWidget.h"
#include "imageWidget.h"

namespace mapgen
{

class WorldViewer:public UiDataWidget
{
public:
    WorldViewer();

    void onInitialize() override;

    void onPosition() override;
    void onSize() override;

    void onDraw() override;

private:
    void updateTexture();
    void updatePlateTexture(std::vector<GLubyte> &textureBuffer);
    void updatePlateInfoTexture(std::vector<GLubyte> &textureBuffer);
    void updatePlateDistanceTexture(std::vector<GLubyte> &textureBuffer);
    //    void updateContinentTexture(std::vector<GLubyte> &textureBuffer);
    //    void updateGeometryTexture(std::vector<GLubyte> &textureBuffer);
    void updateHeightMapTexture(std::vector<GLubyte> &textureBuffer);
    //    void updateCollisionInfoTexture(std::vector<GLubyte> &textureBuffer);
    //    void updatePlateWithInfoTexture(std::vector<GLubyte> &textureBuffer);

    void generate();

    ImageWidget m_imageView;

    WorldGenerator *m_worldGenerator;

    //    World m_world;
    int m_worldWidth;
    int m_worldHeight;
    int m_worldDepth;

//    int m_noiseSeed;
//    float m_plateFrequency;
//    float m_continentFrequency;
    imglib::SimpleImage layerImage;
    imglib::SimpleImage overlayImage;
    imglib::SimpleImage vectorImage;

    std::vector<GLubyte> layerData;
    std::vector<GLubyte> overlayData;
    std::vector<GLubyte> vectorData;

    bool m_textureValid;
    GLuint m_textureId;
    int m_textureWidth;
    int m_textureHeight;

    int m_layerIndex;
    std::vector<char> m_layerNames;
    int m_info;
    int m_overlay;
    int m_overlayVector;

    int m_plateCount;
    std::vector<std::tuple<int, int, int>> m_plateColors;

    worldgen::ColorMap m_tempColorMap;
    worldgen::ColorMap m_moistureColorMap;
    worldgen::ColorMap2D m_biomeColorMap;
    worldgen::Progress m_progress;

    ImVec2 lastDrawPos;
    ImVec2 lastDrawSize;

    bool m_dragging;
    ImVec2 m_lastDrag;
};

}//namespace mapgen

#endif//_mapgen_worldViewer_h_
