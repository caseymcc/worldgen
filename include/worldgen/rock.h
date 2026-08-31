#ifndef _worldgen_rock_h_
#define _worldgen_rock_h_

#include "worldgen/export.h"
#include "worldgen/climate.h"

#include <glm/glm.hpp>

namespace worldgen
{

//The rock actually underfoot. Three families - igneous, metamorphic, sedimentary - split just far
//enough to say something useful about where ore, karst and building stone turn up. About three
//quarters of land is sediment or sedimentary rock and only a quarter igneous or metamorphic, which
//is the proportion this is calibrated against.
enum class RockType
{
    Sediment,     //loose, not yet rock proper. Basin floors, lake beds, flood plains
    Sandstone,    //sedimentary, laid down on land and in shallow water
    Carbonate,    //sedimentary, marine. Limestone, and so the karst country
    Basalt,       //igneous, erupted. Large igneous provinces, hotspots, rifts
    Andesite,     //igneous, subduction. Island arcs and the laramide belts
    Granite,      //igneous, intruded, taken with rhyolite. Andean roots and craton basement
    Metamorphic   //cooked and squeezed. Craton shields, collision cores, contact aureoles
};

constexpr size_t RockTypeCount=7;

//An ancient block of continental crust that has sat out the last billion years of tectonics. It
//has a basement of igneous and metamorphic rock and a cover of sediment on top; where erosion has
//stripped the cover away the basement is exposed and that part is a shield.
enum class CratonPart
{
    None,
    Shield,      //basement at the surface, scraped clean
    Platform     //basement still buried under its sedimentary cover
};

inline const char *rockTypeName(RockType rock)
{
    switch(rock)
    {
    case RockType::Sediment:    return "Sediment";
    case RockType::Sandstone:   return "Sandstone";
    case RockType::Carbonate:   return "Carbonate";
    case RockType::Basalt:      return "Basalt";
    case RockType::Andesite:    return "Andesite";
    case RockType::Granite:     return "Granite";
    case RockType::Metamorphic: return "Metamorphic";
    default: break;
    }
    return "Unknown";
}

//roughly the palette geological maps use, so the output reads like one
inline glm::ivec3 rockTypeColor(RockType rock)
{
    switch(rock)
    {
    case RockType::Sediment:    return glm::ivec3(250, 240, 190);
    case RockType::Sandstone:   return glm::ivec3(240, 200, 110);
    case RockType::Carbonate:   return glm::ivec3(215, 160, 70);
    case RockType::Basalt:      return glm::ivec3(70, 70, 85);
    case RockType::Andesite:    return glm::ivec3(150, 110, 190);
    case RockType::Granite:     return glm::ivec3(230, 140, 190);
    case RockType::Metamorphic: return glm::ivec3(110, 175, 120);
    default: break;
    }
    return glm::ivec3(0, 0, 0);
}

inline bool sedimentaryRock(RockType rock)
{
    return (rock==RockType::Sediment)||(rock==RockType::Sandstone)||(rock==RockType::Carbonate);
}

//limestone dissolves, which is what makes caves, sinkholes and rivers that vanish underground
inline bool karstRock(RockType rock)
{
    return (rock==RockType::Carbonate);
}

struct RockThresholds
{
    RockThresholds()
    {
        //Sea level stood 150-200m higher when the shallow seas were at their peak, and the ground
        //that was under them took marine carbonate.
        m_shallowSeaElevation=200.0f;

        //a craton is old, quiet crust well away from any boundary
        m_cratonContinentality=0.30f;
        m_cratonRelief=0.06f;

        //where a large igneous province is young enough to still be basalt rather than worn to
        //sediment, and how strongly a hotspot has to have marked the ground
        m_hotspotBasalt=0.06f;

        //How close to the boundary line itself a rift has to be to be flooding basalt, and how
        //much relief an orogenic belt has actually raised before its core rock shows through.
        //Both matter because a cell knowing which boundary is nearest is not the same as it being
        //at that boundary - most of a plate is nearest to something.
        m_riftDistance=0.90f;
        m_beltTerrain=0.22f;

        //A continental collision does not only cook rock. Melt rises through the thickened crust
        //and freezes as granite bodies inside the belt - the Cornish granites, the tin granites of
        //southeast Asia. Without these a collisional belt is metamorphic all the way through and
        //there is nowhere for tin to be.
        m_beltIntrusion=0.22f;

        //how much orogenic relief still counts as a mountain core rather than its worn skirts
        m_mountainRelief=0.10f;
        m_erodedRelief=0.04f;
    }

    float m_shallowSeaElevation;
    float m_cratonContinentality;
    float m_cratonRelief;
    float m_hotspotBasalt;
    float m_riftDistance;
    float m_beltTerrain;
    float m_beltIntrusion;
    float m_mountainRelief;
    float m_erodedRelief;
};

//Ice scours a craton back to its basement - the reference's reasoning for the Canadian Shield is
//that the ice sheet scratched it clean on its way out. Deserts and drowned ground get the opposite
//treatment, burying the basement under what settles on top.
inline CratonPart cratonPart(bool craton, bool glaciated, bool shallowSea, float moisture)
{
    if(!craton)
        return CratonPart::None;

    if(glaciated)
        return CratonPart::Shield;

    //ground that has been under a sea, or that is desert, is where the cover survives
    if(shallowSea||(moisture<0.30f))
        return CratonPart::Platform;

    return (moisture>0.55f)?CratonPart::Shield:CratonPart::Platform;
}

}//namespace worldgen

#endif //_worldgen_rock_h_
