#ifndef _mapgen_legend_h_
#define _mapgen_legend_h_

#include "uiData.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace mapgen
{

//What a layer's colours mean. A map with thirty layers on it is unreadable without this, and a
//legend that has drifted from what is actually painted is worse than none - so where the library
//supplies a colour function the legend calls the SAME function the viewer does, and the few colours
//the viewer chooses inline are defined here and used from both places.
struct LegendEntry
{
    LegendEntry(const glm::ivec3 &color, const std::string &label, const std::string &note=""):
        m_color(color), m_label(label), m_note(note)
    {}

    glm::ivec3 m_color;
    std::string m_label;
    std::string m_note;
};

struct Legend
{
    Legend(): m_gradient(false) {}

    std::string m_title;
    std::string m_description;

    //a continuous layer is described by its two ends rather than by a list
    bool m_gradient;
    glm::ivec3 m_low;
    glm::ivec3 m_high;
    std::string m_lowLabel;
    std::string m_highLabel;

    std::vector<LegendEntry> m_entries;
};

//--- colours the world viewer paints inline ---
//Defined here so the legend and the map cannot disagree about them.

inline glm::ivec3 legendOceanColor() { return glm::ivec3(20, 34, 64); }

inline glm::ivec3 legendBarrierColor(worldgen::BarrierType barrier)
{
    switch(barrier)
    {
    case worldgen::BarrierType::Ocean:    return glm::ivec3(20, 34, 64);
    case worldgen::BarrierType::Mountain: return glm::ivec3(170, 150, 130);
    case worldgen::BarrierType::Desert:   return glm::ivec3(210, 160, 70);
    default: break;
    }
    return glm::ivec3(70, 140, 80);
}

inline glm::ivec3 legendRuinKindColor(worldgen::RuinKind kind)
{
    switch(kind)
    {
    case worldgen::RuinKind::Fortress: return glm::ivec3(210, 90, 80);
    case worldgen::RuinKind::MineHead: return glm::ivec3(150, 140, 210);
    case worldgen::RuinKind::Harbour:  return glm::ivec3(80, 190, 225);
    case worldgen::RuinKind::Terraces: return glm::ivec3(140, 205, 110);
    case worldgen::RuinKind::Tomb:     return glm::ivec3(235, 215, 120);
    case worldgen::RuinKind::Bridge:   return glm::ivec3(225, 150, 205);
    default: break;
    }
    return glm::ivec3(200, 170, 120);   //Monument, the sandstone default
}

inline glm::ivec3 legendCityColor(worldgen::NodeType node, bool parasite)
{
    if(parasite)
        return glm::ivec3(255, 255, 230);

    switch(node)
    {
    case worldgen::NodeType::Mouth:     return glm::ivec3(90, 200, 255);
    case worldgen::NodeType::Strait:    return glm::ivec3(255, 80, 80);
    case worldgen::NodeType::Anchorage: return glm::ivec3(120, 255, 140);
    default: break;
    }
    return glm::ivec3(255, 200, 60);    //Extraction
}

//Built for whatever is on screen. The generator is needed because a couple of layers name things
//the world decided for itself - which peoples exist, how many states there are.
Legend buildLayerLegend(DisplayLayers layer, WorldGenerator *generator);
Legend buildOverlayLegend(DisplayOverlays overlay);

}//namespace mapgen

#endif//_mapgen_legend_h_
