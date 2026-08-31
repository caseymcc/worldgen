#ifndef _mapgen_legendViewer_h_
#define _mapgen_legendViewer_h_

#include "uiDataWidget.h"
#include "legend.h"

#include <imgui.h>

namespace mapgen
{

//Sits beside the detail view and says what the world map's colours mean - the layer being drawn,
//and the overlay over it if there is one.
class LegendViewer:public UiDataWidget
{
public:
    LegendViewer();

    void onDraw() override;

private:
    void drawLegend(const Legend &legend);

    int m_shownLayer;
    int m_shownOverlay;
    Legend m_layerLegend;
    Legend m_overlayLegend;
};

}//namespace mapgen

#endif//_mapgen_legendViewer_h_
