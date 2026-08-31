#include "infoViewer.h"

namespace mapgen
{

InfoViewer::InfoViewer()
{
    
}

void InfoViewer::onInitialize()
{
    setName("InfoViewer");
}

void InfoViewer::onDraw()
{
    ImGui::Text("Mouse Position: %d, %d", m_uiData->mousePos.x, m_uiData->mousePos.y);

    ImGui::Text("World");
    ImGui::Text("  Position: %d, %d", m_uiData->worldPos.x, m_uiData->worldPos.y);
    ImGui::Text("  Value: %f", m_uiData->worldValue);
    if(m_uiData->worldOverlay)
    {
        ImGui::Text("  Overlay: %f", m_uiData->worldOverlayValue);
    }
}

}//namespace mapgen
