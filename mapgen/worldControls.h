#ifndef _mapgen_worldControls_h_
#define _mapgen_worldControls_h_

#include "uiDataWidget.h"

namespace mapgen
{

class WorldControls:public UiDataWidget
{
public:
    WorldControls();

    void onInitialize() override;

    void onDraw() override;

private:
    void generate();
};

}//namespace mapgen

#endif//_mapgen_worldControls_h_