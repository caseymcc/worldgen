#include "uiDataWidget.h"

namespace mapgen
{

UiDataWidget::UiDataWidget()
{
    
}

void UiDataWidget::initialize(UiData *uiData)
{
    m_uiData=uiData;

    onInitialize();
}

}//namespace mapgen
