#ifndef _mapgen_terrainViewer_h_
#define _mapgen_terrainViewer_h_

#include "mapgen/uiData.h"

#include "worldgen/utils/colorMap.h"

#include <imgui.h>

class TerrainViewer
{
public:
    MapGen();
    
    void initialize();
    void setSize(int width, int height);

    void draw();

private:


#endif//_mapgen_terrainViewer_h_