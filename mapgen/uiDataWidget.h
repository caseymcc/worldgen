#ifndef _mapgen_uiDataWidget_h_
#define _mapgen_uiDataWidget_h_

#include "imguiWidget.h"
#include "uiData.h"

namespace mapgen
{

class UiDataWidget:public ImguiWidget
{
public:
    UiDataWidget();

    void initialize(UiData *uiData);
    
protected:
    UiData *m_uiData;
};

}//namespace mapgen

#endif//_mapgen_uiDataWidget_h_
