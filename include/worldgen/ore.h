#ifndef _worldgen_ore_h_
#define _worldgen_ore_h_

#include "worldgen/export.h"
#include "worldgen/rock.h"
#include "worldgen/climate.h"

#include <glm/glm.hpp>

namespace worldgen
{

//What can be dug out of the ground, and what it is worth digging for. A deposit is not just a
//metal - it is a metal that turned up somewhere concentrated enough and shallow enough to be worth
//the effort, and where that happens is decided by what the rock has been through.
enum class DepositType
{
    None,

    //fossil fuels
    Peat,             //modern low lying wetland, not yet coal
    Coal,             //ancient tropical coal swamp, buried and compressed
    OilGas,           //drowned tropical shelf for the source rock, then burial and a trap

    //evaporites
    SaltFlat,         //and lithium. A lake evaporated away in an arid basin, salt left on the surface
    Halite,           //rock salt. An ancient salt flat buried, and worth mining where it was lifted back up

    //volcanic arc and hotspot family, the recent stuff
    PorphyryCopper,   //Cu, Cu-Au, Cu-Mo, with minor Pb/Zn/Ag and a little Sn-W
    EpithermalGold,   //Au-Ag, ringed outward from the porphyry
    IOCG,             //iron oxide copper gold, and the primary uranium

    //ancient stable crust
    NickelPGE,        //Ni-Cu and platinum group with chrome, and the diamonds

    //iron, in the several forms that made an iron age possible
    BogIron,          //iron oxidised out of groundwater into a marsh. Shallow, and easy to smelt
    OoliticIron,      //iron laid down grain by grain in a shallow marine sediment
    IronLaterite,     //iron concentrated in soil under alternating wet and dry seasons
    MeteoricIron,     //fell out of the sky. Metallic already, and vanishingly rare

    //ancient sea floor, since thrust up
    VMS,              //Cu-Zn from black smokers caught in a mountain belt
    BIF,              //banded iron, in and around the old ranges

    //contact between magma and limestone
    Skarn,            //a pluton baked into carbonate country rock. Copper, iron, and more

    //tin, which is what turns copper into bronze and is far rarer than it
    TinLode,          //cassiterite in the granites of a collisional belt - the tin belts
    TinPlacer,        //tin weathered out of those granites and carried downriver
    TinBronze,        //copper and tin in the same ground, alloyable straight out of the rock

    //basins
    SEDEX,            //Pb-Zn-Ag laid down in a continental sedimentary basin
    MVT,              //Mississippi valley type, carried out under a foreland basin

    //surface processes
    Laterite,         //bauxite. Rainforest soil leached until the soil is the ore
    NativeCopper,     //metallic copper, no smelting needed. What actually starts a copper age
    SedimentaryCopper,//copper accumulated in sandstone and shale beds, away from any volcano
    Placer,           //eroded out, carried downriver, concentrated in the gravels
    SecondaryUranium  //leached out of an older deposit and reconcentrated downhill
};

constexpr size_t DepositTypeCount=27; //None plus the twenty-six kinds

inline const char *depositTypeName(DepositType deposit)
{
    switch(deposit)
    {
    case DepositType::Peat:             return "Peat";
    case DepositType::Coal:             return "Coal";
    case DepositType::OilGas:           return "Oil and gas";
    case DepositType::SaltFlat:         return "Salt flat";
    case DepositType::Halite:           return "Rock salt (halite)";
    case DepositType::PorphyryCopper:   return "Porphyry copper";
    case DepositType::EpithermalGold:   return "Epithermal gold";
    case DepositType::IOCG:             return "IOCG (uranium)";
    case DepositType::NickelPGE:        return "Nickel-PGE, diamond";
    case DepositType::BogIron:          return "Bog iron";
    case DepositType::OoliticIron:      return "Oolitic iron";
    case DepositType::IronLaterite:     return "Iron laterite";
    case DepositType::MeteoricIron:     return "Meteoric iron";
    case DepositType::VMS:              return "VMS copper-zinc";
    case DepositType::BIF:              return "Banded iron";
    case DepositType::Skarn:            return "Skarn";
    case DepositType::TinLode:          return "Tin lode";
    case DepositType::TinPlacer:        return "Alluvial tin";
    case DepositType::TinBronze:        return "Natural tin-bronze";
    case DepositType::SEDEX:            return "SEDEX lead-zinc";
    case DepositType::MVT:              return "MVT lead-zinc";
    case DepositType::Laterite:         return "Laterite bauxite";
    case DepositType::NativeCopper:     return "Native copper";
    case DepositType::SedimentaryCopper: return "Sediment-hosted copper";
    case DepositType::Placer:           return "Placer gold";
    case DepositType::SecondaryUranium: return "Secondary uranium";
    default: break;
    }
    return "None";
}

inline glm::ivec3 depositTypeColor(DepositType deposit)
{
    switch(deposit)
    {
    case DepositType::Peat:             return glm::ivec3(120, 90, 50);
    case DepositType::Coal:             return glm::ivec3(30, 30, 30);
    case DepositType::OilGas:           return glm::ivec3(80, 40, 120);
    case DepositType::SaltFlat:         return glm::ivec3(240, 240, 250);
    case DepositType::Halite:           return glm::ivec3(200, 195, 215);
    case DepositType::PorphyryCopper:   return glm::ivec3(200, 110, 50);
    case DepositType::EpithermalGold:   return glm::ivec3(255, 215, 0);
    case DepositType::IOCG:             return glm::ivec3(140, 200, 60);
    case DepositType::NickelPGE:        return glm::ivec3(180, 220, 230);
    case DepositType::BogIron:          return glm::ivec3(140, 80, 45);
    case DepositType::OoliticIron:      return glm::ivec3(175, 75, 55);
    case DepositType::IronLaterite:     return glm::ivec3(200, 95, 65);
    case DepositType::MeteoricIron:     return glm::ivec3(245, 245, 255);
    case DepositType::VMS:              return glm::ivec3(0, 170, 160);
    case DepositType::BIF:              return glm::ivec3(160, 50, 40);
    case DepositType::Skarn:            return glm::ivec3(230, 100, 140);
    case DepositType::TinLode:          return glm::ivec3(150, 210, 220);
    case DepositType::TinPlacer:        return glm::ivec3(190, 235, 240);
    case DepositType::TinBronze:        return glm::ivec3(205, 145, 60);
    case DepositType::SEDEX:            return glm::ivec3(110, 130, 200);
    case DepositType::MVT:              return glm::ivec3(70, 90, 160);
    case DepositType::Laterite:         return glm::ivec3(220, 130, 110);
    case DepositType::NativeCopper:     return glm::ivec3(230, 130, 70);
    case DepositType::SedimentaryCopper: return glm::ivec3(190, 120, 95);
    case DepositType::Placer:           return glm::ivec3(255, 250, 150);
    case DepositType::SecondaryUranium: return glm::ivec3(180, 255, 80);
    default: break;
    }
    return glm::ivec3(0, 0, 0);
}

struct OreThresholds
{
    OreThresholds()
    {
        //An ore deposit is a rare accident of geology, so the settings below say "somewhere in
        //this kind of country" and the rarity decides which cells of it actually carry one.
        m_arcRarity=0.10f;        //porphyry and epithermal, along the volcanic arcs
        m_iocgRarity=0.05f;
        m_cratonRarity=0.03f;     //nickel-PGE and diamond, few and far between
        m_mountainRarity=0.08f;   //VMS and BIF in the ranges
        m_basinRarity=0.04f;      //SEDEX, uncommon by the reference's own note
        m_fuelRarity=0.10f;
        m_lateriteRarity=0.10f;
        m_placerRarity=0.20f;     //locally common once a river is carrying the stuff
        m_secondaryRarity=0.20f;

        //a coal swamp was tropical, low and wet; the coal is only good where it was then buried
        m_coalTemperature=17.0f;
        m_coalMoisture=0.55f;

        //salt wants somewhere that evaporates faster than it fills
        m_saltMoisture=0.30f;

        //An ancient deposit is only worth anything to people without drills if something lifted it
        //back to where they can reach it. The reference makes this point about halite: the beds
        //that stayed buried are lost to a pre-industrial world, the ones a collision thrust up are
        //the salt mines and the salt glaciers.
        m_upliftExposure=0.12f;

        //native copper carried out of a basalt province and dropped in the carbonate around it
        m_nativeCopperRarity=0.22f;

        //Tin is the whole difficulty of the bronze age: it is rare, and it does not turn up
        //where copper does. Ground where both occur together is rarer still and enormously
        //valuable, because it can be alloyed straight out of the rock.
        m_skarnRarity=0.16f;
        m_tinRarity=0.55f;
        m_tinBronzeRarity=0.10f;

        //Iron is the common metal - the reference's advice is that if you think you have enough
        //deposits you should put in more - except for the one that fell out of the sky.
        m_sedimentaryCopperRarity=0.30f;
        m_bogIronRarity=0.55f;
        m_ooliticRarity=0.10f;   //still the commonest iron, but one type must not swamp the map
        m_ironLateriteRarity=0.30f;
        m_meteoricRarity=0.0016f;
    }

    float m_arcRarity;
    float m_iocgRarity;
    float m_cratonRarity;
    float m_mountainRarity;
    float m_basinRarity;
    float m_fuelRarity;
    float m_lateriteRarity;
    float m_placerRarity;
    float m_secondaryRarity;

    float m_coalTemperature;
    float m_coalMoisture;
    float m_saltMoisture;
    float m_upliftExposure;
    float m_nativeCopperRarity;
    float m_skarnRarity;
    float m_tinRarity;
    float m_tinBronzeRarity;
    float m_sedimentaryCopperRarity;
    float m_bogIronRarity;
    float m_ooliticRarity;
    float m_ironLateriteRarity;
    float m_meteoricRarity;
};

//How rich the rock is, which is a different question from how much of it there is. A porphyry is
//the world's great source of copper and is still low grade - vast tonnages of poor rock, worth
//nothing at all to anyone who cannot move mountains of it. A small rich vein is worth far more to
//a people working by hand.
enum class OreGrade
{
    Low,        //high tonnage, poor rock. Needs industry to be worth anything
    Moderate,
    Rich        //worth working with hand tools
};

inline const char *oreGradeName(OreGrade grade)
{
    switch(grade)
    {
    case OreGrade::Low:      return "low grade";
    case OreGrade::Moderate: return "moderate";
    default: break;
    }
    return "rich";
}

//What a deposit is typically worth per tonne of rock. Porphyry is the type case for low grade and
//high tonnage; the concentrated and the already-metallic things are the rich ones.
inline OreGrade depositGrade(DepositType deposit)
{
    switch(deposit)
    {
    case DepositType::PorphyryCopper:
    case DepositType::OoliticIron:
    case DepositType::Laterite:
    case DepositType::IronLaterite:
    case DepositType::OilGas:
        return OreGrade::Low;

    case DepositType::NativeCopper:
    case DepositType::MeteoricIron:
    case DepositType::Placer:
    case DepositType::TinPlacer:
    case DepositType::TinBronze:
    case DepositType::EpithermalGold:
    case DepositType::BogIron:
    case DepositType::SaltFlat:
        return OreGrade::Rich;

    default:
        break;
    }
    return OreGrade::Moderate;
}

//Whether a people without industry can actually get anything out of this. It has to be within
//reach AND concentrated enough to be worth the digging - a buried seam is no use, and neither is
//a mountain of rock that is half a percent copper.
inline bool workableByHand(DepositType deposit, bool exposed)
{
    if(!exposed)
        return false;

    return depositGrade(deposit)!=OreGrade::Low;
}

//A deposit is not spread evenly over the country that can host it, so this scatters them: a
//repeatable per-cell value that decides whether this particular patch of the right sort of ground
//actually carries anything.
inline float oreChance(size_t index, int seed, int salt)
{
    unsigned int h=(unsigned int)(index*2654435761u);

    h^=(unsigned int)(seed*40503)+(unsigned int)(salt*2246822519u);
    h^=h>>13;
    h*=2654435761u;
    h^=h>>16;

    return (float)(h&0xFFFFFF)/(float)0xFFFFFF;
}

}//namespace worldgen

#endif //_worldgen_ore_h_
