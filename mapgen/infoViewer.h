#ifndef _mapgen_infoViewer_h_
#define _mapgen_infoViewer_h_

#include "uiDataWidget.h"

namespace mapgen
{

class InfoViewer:public UiDataWidget
{
public:
    InfoViewer();

    void onInitialize() override;

    void onDraw() override;

private:
    void generate();
};

}//namespace mapgen

#endif//_mapgen_infoViewer_h_