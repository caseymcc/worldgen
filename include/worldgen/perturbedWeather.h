#ifndef _worldgen_weatherPerturbed_h_
#define _worldgen_weatherPerturbed_h_

#include "worldgen/weather.h"
#include "worldgen/maths/coords.h"

#include <glm/glm.hpp>

namespace worldgen
{

struct WeatherBandInfo
{
    float upperLatitude;
    float lowerLatitude;
    float size;
};

struct WeatherCellInfo
{
    float upperLatitude;
    float lowerLatitude;
    float size;
};

class PerturbedWeatherBands:public WeatherBands
{
public:
    PerturbedWeatherBands(int seed, const glm::ivec2 &size, std::vector<WeatherCell> cells);
    
    size_t getCellIndex(const glm::vec2 &coord);
    glm::vec2 getWindDirection(const glm::vec2 &coord);
    size_t getBandIndex(const glm::vec2 &coord);
    float getMoisture(const glm::vec2 &coord);

private:
    glm::ivec2 m_size;

    std::vector<std::vector<WeatherBandInfo>> m_bandsInfo;
    std::vector<std::vector<WeatherCellInfo>> m_cellsInfo;
};

}//namespace worldgen

#endif //_worldgen_weather_h_