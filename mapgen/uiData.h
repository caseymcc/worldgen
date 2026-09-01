#ifndef _mapgen_uiData_h_
#define _mapgen_uiData_h_

#include <memory>

#include <glm/glm.hpp>

#include "worldgen/generators/equiRectWorldGenerator.h"

namespace mapgen
{

typedef worldgen::EquiRectWorldGenerator WorldGenerator;


enum class DisplayLayers
{
    NoMap,
    Map,
    Height,
    Value,
    Plates,
    PlateMapNoise,
    PlateMap2Noise,
    PlateDistanceNoise,
    ContinentMapNoise,
    HeightMapNoise,
    PlateDistanceValue,
    PlateScale,
    Hotspots,
    Climate,
    Coast,
    Rivers,
    Rocks,
    Deposits,
    Currents,
    Fuel,
    Habitability,
    Peoples,
    Ruins,
    Regions,
    Barriers,
    States,
    Control,
    Trade,
    Cities,
    Remoteness,
    Settlements
};

static const std::vector<std::pair<DisplayLayers, std::string>> uiDisplayLayers=
{
    {DisplayLayers::NoMap, "No Map"},
    {DisplayLayers::Map, "Map"},
    {DisplayLayers::Height, "Height"},
    {DisplayLayers::Value, "Value"},
    {DisplayLayers::Plates, "Plates"},
    {DisplayLayers::PlateMapNoise, "plateMap noise"},
    {DisplayLayers::PlateMap2Noise, "plateMap2 noise"},
    {DisplayLayers::PlateDistanceNoise, "plateDistance noise"},
    {DisplayLayers::ContinentMapNoise, "continentMap noise"},
    {DisplayLayers::HeightMapNoise, "HeightMap noise"},
    {DisplayLayers::PlateDistanceValue, "plateDistanceValue"},
    {DisplayLayers::PlateScale, "plateScale"},
    {DisplayLayers::Hotspots, "Hotspots"},
    {DisplayLayers::Climate, "Climate (Koppen)"},
    {DisplayLayers::Coast, "Coast types"},
    {DisplayLayers::Rivers, "Rivers"},
    {DisplayLayers::Rocks, "Rocks (geology)"},
    {DisplayLayers::Deposits, "Ore and fuel"},
    {DisplayLayers::Currents, "Ocean currents"},
    {DisplayLayers::Fuel, "Fuel and forest"},
    {DisplayLayers::Habitability, "Habitability"},
    {DisplayLayers::Peoples, "Peoples"},
    {DisplayLayers::Ruins, "Ruins"},
    {DisplayLayers::Regions, "Cultural regions"},
    {DisplayLayers::Barriers, "Barriers"},
    {DisplayLayers::States, "States"},
    {DisplayLayers::Control, "Power projection"},
    {DisplayLayers::Trade, "Trade routes"},
    {DisplayLayers::Cities, "Coastal cities"},
    {DisplayLayers::Remoteness, "Beaten track"},
    {DisplayLayers::Settlements, "Settlements"}
};

enum class DisplayOverlays
{
    None,
    Collision,
    CollisionDistance,
    Shear,
    BoundaryType,
    Orogeny,
    Scale,
    Temp,
    Moisture,
    MoistureGreyScale,
    WeatherCells,
    WeatherBands,
    Pressure
};

static const std::vector<std::pair<DisplayOverlays, std::string>> uiDisplayOverlays=
{
    {DisplayOverlays::None, "None"},
    {DisplayOverlays::Collision, "Collision"},
    {DisplayOverlays::CollisionDistance, "Collision-Distance"},
    {DisplayOverlays::Shear, "Shear"},
    {DisplayOverlays::BoundaryType, "Boundary Type"},
    {DisplayOverlays::Orogeny, "Orogeny Type"},
    {DisplayOverlays::Scale, "Scale"},
    {DisplayOverlays::Temp, "Temp"},
    {DisplayOverlays::Moisture, "Moisture"},
    {DisplayOverlays::MoistureGreyScale, "Moisture GreyScale"},
    {DisplayOverlays::WeatherCells, "Weather Cells"},
    {DisplayOverlays::WeatherBands, "Weather Bands"},
    {DisplayOverlays::Pressure, "Pressure"}
};

enum class DisplayVectors
{
    NoVector,
    Direction,
    Air
};

static const std::vector<std::pair<DisplayVectors, std::string>> uiDisplayVectors=
{
    {DisplayVectors::NoVector, "No Vector"},
    {DisplayVectors::Direction, "Direction"},
    {DisplayVectors::Air, "Air"}
};

template<typename EnumType_, typename Type_>
EnumType_ getEnum(const std::vector<std::pair<EnumType_, Type_>> &values, const Type_ &value)
{
    auto iter=std::find_if(std::begin(values), std::end(values), [&value](auto &pair){return pair.second==value; });

    if(iter != std::end(values))
        return iter->first;
    return values[0].first;
}

template<typename EnumType_, typename Type_>
EnumType_ getEnumFromIndex(const std::vector<std::pair<EnumType_, Type_>> &values, size_t index)
{
    if(index < values.size())
        return values[index].first;
    return values[0].first;
}

template<typename EnumType_, typename Type_>
Type_ getEnumName(const std::vector<std::pair<EnumType_, Type_>> &values, EnumType_ enumValue)
{
    auto iter=std::find_if(std::begin(values), std::end(values), [&enumValue](auto &pair){return pair.first==enumValue; });

    if(iter != std::end(values))
        return iter->second;
    return values[0].second;
}

struct UiData
{
    UiData():
        selectedCell(-1, -1),
        detailValid(false),
        displayLayer(1),
        displayOverlay(0),
        displayVector(0)
    {}

    std::unique_ptr<WorldGenerator> generator;

    glm::ivec2 mousePos;

    glm::ivec2 worldPos;
    float worldValue;
    bool worldOverlay;
    float worldOverlayValue;

    //Which overview cell the user picked, and the ground inside it. The overview says what a cell
    //averages and which river crosses it; the detail map is what that looks like close up, and it
    //is regenerated from the seed rather than kept, so selecting the same cell twice gives the same
    //ground.
    glm::ivec2 selectedCell;
    bool detailValid;
    worldgen::DetailMap detail;

    glm::ivec2 region;
    glm::ivec2 regionPos;
    glm::ivec2 chunk;
    glm::ivec2 chunkPos;

    bool forceUpdate;

    int displayLayer;
    int displayOverlay;
    int displayVector;

    int noiseSeed;
    float plateFrequency;
    float continentFrequency;
};

}

#endif//_mapgen_uiData_h_
