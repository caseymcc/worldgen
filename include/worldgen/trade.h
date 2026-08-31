#ifndef _worldgen_trade_h_
#define _worldgen_trade_h_

#include "worldgen/export.h"
#include "worldgen/polity.h"

#include <string>
#include <vector>

namespace worldgen
{

//The things worth carrying a long way. Every route in the reference is one specific want met by
//one specific place: jade out of Turkestan against Chinese silk and tea, incense off the Arabian
//peninsula against Indian pepper and cinnamon, Novgorod furs and Riga grain against Scandinavian
//herring, gold from the south against salt from the north. Nothing moves because two places are
//near each other - it moves because one has what the other has not.
enum class TradeGood
{
    Grain,
    Livestock,  //and the horses that opened the silk road in the first place
    Fish,       //the herring and cod fisheries the Baltic and North Sea leagues were built on
    Timber,
    Furs,
    Salt,
    Spices,
    Incense,    //aromatics out of the hot dry country, "used for offerings and medicine"
    Gold,
    Silver,
    Copper,
    Tin,
    Iron,
    Gems,
    Stone,
    Coal
};

const size_t TradeGoodCount=16;

inline const char *tradeGoodName(TradeGood good)
{
    switch(good)
    {
    case TradeGood::Grain:     return "grain";
    case TradeGood::Livestock: return "livestock";
    case TradeGood::Fish:      return "fish";
    case TradeGood::Timber:    return "timber";
    case TradeGood::Furs:      return "furs";
    case TradeGood::Salt:      return "salt";
    case TradeGood::Spices:    return "spices";
    case TradeGood::Incense:   return "incense";
    case TradeGood::Gold:      return "gold";
    case TradeGood::Silver:    return "silver";
    case TradeGood::Copper:    return "copper";
    case TradeGood::Tin:       return "tin";
    case TradeGood::Iron:      return "iron";
    case TradeGood::Gems:      return "gems";
    case TradeGood::Stone:     return "stone";
    default: break;
    }
    return "coal";
}

inline unsigned int tradeGoodBit(TradeGood good) { return 1u<<((unsigned int)good); }

inline int tradeGoodCount(unsigned int goods)
{
    int count=0;

    for(size_t g=0; g<TradeGoodCount; ++g)
    {
        if(goods&(1u<<(unsigned int)g))
            count++;
    }
    return count;
}

//Everything below reads off fields the earlier passes already produced. The point is that a world
//which has grown its own climates, ores, forests and fisheries does not need a separate table of
//who trades what - the answer is already on the map.
inline unsigned int cellGoods(const InfluenceCell &cell, float relief)
{
    unsigned int goods=0;

    if(cell.heightBase<0.5f)
        return goods;

    if(siteArable(cell, relief)>0.6f)
        goods|=tradeGoodBit(TradeGood::Grain);

    if(siteGrazing(cell)>0.6f)
        goods|=tradeGoodBit(TradeGood::Livestock);

    //the great fisheries are cold water, and colder still where upwelling feeds them
    if((cell.coast!=0)&&((cell.climate==ClimateZone::Cfb)||(cell.climate==ClimateZone::Cfc)
        ||(cell.climate==ClimateZone::Dfb)||(cell.climate==ClimateZone::Dfc)||(cell.climate==ClimateZone::ET)))
        goods|=tradeGoodBit(TradeGood::Fish);

    if(cell.forest)
    {
        ClimateGroup group=climateGroup(cell.climate);

        if((group==ClimateGroup::Temperate)||(group==ClimateGroup::Subarctic))
            goods|=tradeGoodBit(TradeGood::Timber);

        if((group==ClimateGroup::Subarctic)||(group==ClimateGroup::Polar))
            goods|=tradeGoodBit(TradeGood::Furs);
    }

    //sea salt can be won on any coast, so it is not worth carrying. Mined salt and salt lifted off
    //a pan in a dry country is, and that is the trade the Sahara ran on
    if((cell.deposit==DepositType::SaltFlat)||(cell.deposit==DepositType::Halite))
        goods|=tradeGoodBit(TradeGood::Salt);
    else if((cell.coast!=0)&&(climateGroup(cell.climate)==ClimateGroup::Arid))
        goods|=tradeGoodBit(TradeGood::Salt);

    if((cell.climate==ClimateZone::Af)||(cell.climate==ClimateZone::Am))
        goods|=tradeGoodBit(TradeGood::Spices);

    if((cell.climate==ClimateZone::BWh)||(cell.climate==ClimateZone::BSh))
        goods|=tradeGoodBit(TradeGood::Incense);

    switch(cell.deposit)
    {
    case DepositType::EpithermalGold:
        goods|=tradeGoodBit(TradeGood::Gold)|tradeGoodBit(TradeGood::Silver);
        break;
    case DepositType::Placer:
    case DepositType::IOCG:
        goods|=tradeGoodBit(TradeGood::Gold);
        break;
    case DepositType::SEDEX:
    case DepositType::MVT:
        goods|=tradeGoodBit(TradeGood::Silver);
        break;
    case DepositType::PorphyryCopper:
    case DepositType::NativeCopper:
    case DepositType::SedimentaryCopper:
    case DepositType::VMS:
    case DepositType::Skarn:
        goods|=tradeGoodBit(TradeGood::Copper);
        break;
    case DepositType::TinLode:
    case DepositType::TinPlacer:
        goods|=tradeGoodBit(TradeGood::Tin);
        break;
    case DepositType::TinBronze:
        goods|=tradeGoodBit(TradeGood::Tin)|tradeGoodBit(TradeGood::Copper);
        break;
    case DepositType::BogIron:
    case DepositType::OoliticIron:
    case DepositType::IronLaterite:
    case DepositType::BIF:
    case DepositType::MeteoricIron:
        goods|=tradeGoodBit(TradeGood::Iron);
        break;
    case DepositType::NickelPGE:
        goods|=tradeGoodBit(TradeGood::Gems);
        break;
    case DepositType::Coal:
        goods|=tradeGoodBit(TradeGood::Coal);
        break;
    default:
        break;
    }

    if(siteStone(cell, relief)>0.8f)
        goods|=tradeGoodBit(TradeGood::Stone);

    return goods;
}

struct TradeThresholds
{
    TradeThresholds()
    {
        m_landCost=1.0f;
        m_mountainCost=5.0f;
        m_desertCost=3.0f;
        m_jungleCost=3.5f;
        m_iceCost=6.0f;

        //"in order to obtain some of these horses, the emperor agrees for the first time to trade
        //silk" - a river is the cheapest road there is, and cheaper for cargo than for an army
        m_riverDiscount=0.4f;

        //Coasting within sight of land is how nearly all pre-modern sea trade was done, and it is
        //cheaper than any road. The open sea is a different proposition and needs deep-water
        //seamanship: "the Romans come into contact with the Kingdom of Aksum, where merchants live
        //who've mastered techniques of navigation on the high seas."
        //The often quoted figure is 40 to 1 for bulk against a wagon, and it was tested here at
        //that ratio. It deletes overland trade outright - every route goes by water and crosses
        //nobody, so there is no silk road left on the map. The 40 to 1 is a GRAIN figure, and the
        //long land routes never carried grain; they carried silk, jade and spice, which is cargo
        //dense enough in value to pay for a wagon. These costs are for that mixed traffic.
        m_coastalSeaCost=0.35f;
        m_openSeaCost=2.2f;
        m_coastalRange=2;

        //Two places only trade if each has something the other LACKS, and a state that rules a
        //thousand miles of ground has a little of nearly everything somewhere in it. So what a
        //state offers is what it has in quantity, and what it wants is what it genuinely has
        //almost none of - which is what makes jade, tin, incense and silk worth carrying at all.
        m_exportShare=0.06f;
        m_scarceShare=0.01f;
        m_minimumComplement=1;

        //The "hot path": ground within this much travel cost of the trade network counts as
        //being on the beaten track. Everything beyond it is backcountry - which is exactly where
        //the reference says to put the things you want found rather than passed.
        //It is an ABSOLUTE cost, deliberately, and not a fraction of the furthest ground on the
        //map. Distance-from-network is heavily skewed - half the land sits within a tenth of the
        //maximum and one walled-off corner sets the maximum - so any threshold expressed as a
        //share of the furthest cell lands in the tail and names almost nothing.
        m_hotPath=6.0f;

        //a chokepoint is high traffic through ground that has no way round it
        m_chokepointTraffic=0.55f;
        m_constriction=4;

        m_mountainElevation=2300.0f;
        m_mountainRelief=0.24f;
    }

    float m_landCost;
    float m_mountainCost;
    float m_desertCost;
    float m_jungleCost;
    float m_iceCost;
    float m_riverDiscount;
    float m_coastalSeaCost;
    float m_openSeaCost;
    int m_coastalRange;
    float m_exportShare;
    float m_scarceShare;
    int m_minimumComplement;
    float m_hotPath;
    float m_chokepointTraffic;
    int m_constriction;
    float m_mountainElevation;
    float m_mountainRelief;
};

//What it costs to carry a load through this cell. This is NOT the cost of holding it: an army is
//stopped by things a caravan only finds expensive, and the sea reverses the two entirely - it is
//the cheapest ground of all for cargo and the dearest for rule.
inline float travelCost(const InfluenceCell &cell, float elevation, float relief, bool nearShore,
    const TradeThresholds &thresholds)
{
    if(cell.heightBase<0.5f)
        return nearShore?thresholds.m_coastalSeaCost:thresholds.m_openSeaCost;

    float cost=thresholds.m_landCost;
    ClimateGroup group=climateGroup(cell.climate);

    if((elevation>=thresholds.m_mountainElevation)||(relief>=thresholds.m_mountainRelief))
        cost=thresholds.m_mountainCost;
    else if(group==ClimateGroup::Arid)
        cost=thresholds.m_desertCost;
    else if((cell.climate==ClimateZone::Af)||(cell.climate==ClimateZone::Am))
        cost=thresholds.m_jungleCost;
    else if(group==ClimateGroup::Polar)
        cost=thresholds.m_iceCost;

    if(cell.water==WaterBody::River)
        cost*=thresholds.m_riverDiscount;

    return cost;
}

//One route between two seats of power, and what moves along it.
struct TradeRoute
{
    size_t m_from;
    size_t m_to;
    float m_cost;             //what it costs to get a load from one end to the other
    unsigned int m_outbound;  //goods the origin has that the destination lacks
    unsigned int m_inbound;   //and the other way
    size_t m_length;          //cells
    size_t m_seaCells;        //how much of it goes by water
    int m_intermediaries;     //states crossed on the way, which is what everyone tries to cut out

    //What the same trade would cost if it had to go the whole way overland. The reference turns on
    //this comparison again and again - "China turns away from land routes to concentrate on sea
    //routes, which are safer and have fewer intermediaries", Portugal going round Africa "at the
    //expense of the powers that control the passage via the Middle East". A route whose overland
    //alternative is far dearer, or crosses far more hands, is a route somebody can strangle.
    float m_overlandCost;         //-1 where there is no way round by land at all
    int m_overlandIntermediaries;
};

}//namespace worldgen

#endif //_worldgen_trade_h_
