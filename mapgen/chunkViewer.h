#ifndef _mapgen_chunkViewer_h_
#define _mapgen_chunkViewer_h_

#include "mapgen/uiDataWidget.h"

#include "worldgen/utils/colorMap.h"

#include <imgui.h>

namespace mapgen
{

class ChunkViewer:public UiDataWidget
{
public:
    ChunkViewer();


    void onDraw();

private:
};

}//namespace mapgen

#endif//_mapgen_chunkViewer_h_