#ifndef _mapgen_regionViewer_h_
#define _mapgen_regionViewer_h_

#include "mapgen/uiDataWidget.h"
#include "mapgen/imageWidget.h"

#include "worldgen/utils/colorMap.h"

#include <imgui.h>

namespace mapgen
{

//What one cell of the overview map actually looks like on the ground. Pick a cell in the world
//viewer and this expands it: the heightmap amplified from the cell's corner values, conditioned so
//water can leave it, and the river the overview promised routed across it at the width its
//discharge earns.
enum class DetailLayers
{
    Terrain,
    Elevation,
    Water,
    Base
};

static const std::vector<std::pair<DetailLayers, std::string>> uiDetailLayers=
{
    {DetailLayers::Terrain, "Terrain"},
    {DetailLayers::Elevation, "Elevation"},
    {DetailLayers::Water, "Water and floodplain"},
    {DetailLayers::Base, "Overview base"}
};

class RegionViewer:public UiDataWidget
{
public:
    RegionViewer();

    void onInitialize() override;

    void onPosition() override;
    void onSize() override;

    void onDraw() override;

private:
    void updateTexture();

    ImageWidget m_imageView;

    glm::ivec2 m_shownCell;
    int m_layerIndex;
    int m_shownLayer;
    std::vector<GLubyte> m_textureBuffer;
};

}//namespace mapgen

#endif//_mapgen_regionViewer_h_
