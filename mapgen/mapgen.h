#ifndef _worldgen_mapgen_h_
#define _worldgen_mapgen_h_

#include "worldgen/utils/colorMap.h"
#include "worldgen/generators/equiRectWorldGenerator.h"

#include <glbinding/gl/gl.h>
using namespace gl;

#include <imgui.h>

struct WorldRegion
{

};

struct WorldChunk
{

};

typedef worldgen::EquiRectWorldGenerator<WorldRegion, WorldChunk> WorldGenerator;

class MapGen
{
public:
    MapGen();
    
    void initialize();
    void setSize(int width, int height);

    void draw();

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

    bool m_show;

    int m_width;
    int m_height;

//    World m_world;
    int m_worldWidth;
    int m_worldHeight;
    int m_worldDepth;

    std::unique_ptr<WorldGenerator> m_worldGenerator;
//    WorldGenerator *m_worldGenerator;

    int m_noiseSeed;
    float m_plateFrequency;
    float m_continentFrequency;

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
};

#endif//_worldgen_mapgen_h_