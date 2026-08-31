#include "legendViewer.h"

namespace mapgen
{

namespace
{

ImU32 toColor(const glm::ivec3 &color)
{
    return IM_COL32(color.r, color.g, color.b, 255);
}

}//unnamed namespace

LegendViewer::LegendViewer():
    m_shownLayer(-1),
    m_shownOverlay(-1)
{}

void LegendViewer::drawLegend(const Legend &legend)
{
    if(legend.m_title.empty())
        return;

    ImGui::TextUnformatted(legend.m_title.c_str());

    if(!legend.m_description.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.68f, 1.0f));
        ImGui::TextWrapped("%s", legend.m_description.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    const float swatch=13.0f;

    if(legend.m_gradient)
    {
        //A ramp rather than a list, drawn as a strip of steps so it reads as continuous without
        //needing a texture of its own.
        ImVec2 origin=ImGui::GetCursorScreenPos();
        float width=ImGui::GetContentRegionAvail().x-4.0f;
        const int steps=48;

        if(width>steps)
        {
            ImDrawList *draw=ImGui::GetWindowDrawList();
            float step=width/(float)steps;

            for(int i=0; i<steps; ++i)
            {
                float t=(float)i/(float)(steps-1);
                glm::ivec3 mixed(
                    (int)(legend.m_low.r+((legend.m_high.r-legend.m_low.r)*t)),
                    (int)(legend.m_low.g+((legend.m_high.g-legend.m_low.g)*t)),
                    (int)(legend.m_low.b+((legend.m_high.b-legend.m_low.b)*t)));

                draw->AddRectFilled(
                    ImVec2(origin.x+(step*i), origin.y),
                    ImVec2(origin.x+(step*(i+1))+1.0f, origin.y+swatch),
                    toColor(mixed));
            }

            ImGui::Dummy(ImVec2(width, swatch));
            ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.68f, 1.0f), "%s", legend.m_lowLabel.c_str());
            ImGui::SameLine();

            float used=ImGui::CalcTextSize(legend.m_highLabel.c_str()).x;

            ImGui::SetCursorPosX(ImGui::GetCursorPosX()+std::max(0.0f,
                ImGui::GetContentRegionAvail().x-used-4.0f));
            ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.68f, 1.0f), "%s", legend.m_highLabel.c_str());
        }

        if(!legend.m_entries.empty())
            ImGui::Spacing();
    }

    for(size_t i=0; i<legend.m_entries.size(); ++i)
    {
        const LegendEntry &entry=legend.m_entries[i];
        ImVec2 origin=ImGui::GetCursorScreenPos();
        ImDrawList *draw=ImGui::GetWindowDrawList();

        draw->AddRectFilled(ImVec2(origin.x, origin.y+2.0f),
            ImVec2(origin.x+swatch, origin.y+2.0f+swatch), toColor(entry.m_color));
        draw->AddRect(ImVec2(origin.x, origin.y+2.0f),
            ImVec2(origin.x+swatch, origin.y+2.0f+swatch), IM_COL32(0, 0, 0, 90));

        ImGui::Dummy(ImVec2(swatch+6.0f, swatch+2.0f));
        ImGui::SameLine();

        if(entry.m_note.empty())
            ImGui::TextUnformatted(entry.m_label.c_str());
        else
        {
            ImGui::TextUnformatted(entry.m_label.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.63f, 1.0f), "- %s", entry.m_note.c_str());
        }
    }
}

void LegendViewer::onDraw()
{
    WorldGenerator *generator=m_uiData->generator.get();

    //Rebuilt only when the selection changes; a couple of these ask the generator how many states
    //or peoples there are and there is no reason to do that every frame.
    //displayLayer is an INDEX into the ui tables, not the enum value - the same lookup the world
    //viewer uses, so the legend cannot describe a different layer to the one being drawn.
    if(m_shownLayer!=m_uiData->displayLayer)
    {
        m_layerLegend=buildLayerLegend(getEnumFromIndex(uiDisplayLayers, m_uiData->displayLayer),
            generator);
        m_shownLayer=m_uiData->displayLayer;
    }

    if(m_shownOverlay!=m_uiData->displayOverlay)
    {
        m_overlayLegend=buildOverlayLegend(getEnumFromIndex(uiDisplayOverlays, m_uiData->displayOverlay));
        m_shownOverlay=m_uiData->displayOverlay;
    }

    ImGui::BeginChild("legendScroll", ImVec2(0.0f, 0.0f), false);

    drawLegend(m_layerLegend);

    if(!m_overlayLegend.m_title.empty())
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.6f, 1.0f), "OVERLAY");

        drawLegend(m_overlayLegend);
    }

    ImGui::EndChild();
}

}//namespace mapgen
