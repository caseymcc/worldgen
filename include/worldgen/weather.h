#ifndef _worldgen_weather_h_
#define _worldgen_weather_h_

#include "worldgen/export.h"
#include "worldgen/sortedVector.h"
#include "worldgen/maths/math_helpers.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <algorithm>
#include <limits>
#include <vector>

namespace worldgen
{

struct WeatherBand
{
    std::string name;
    float lowerLatitude;
    float upperLatitude;
    float size;
    float moistureUpper;
    float moistureMiddle;
    float moistureLower;
};

struct WeatherCell
{
    friend bool operator<(const WeatherCell &left, const WeatherCell &right)
    {   return (left.upperLatitude<right.upperLatitude);}

    std::string name;

    float lowerLatitude;
    float upperLatitude;
//    float latitude;
//    float size;
    float moisture;

    //0 where air rises out of the cell and 1 where it sinks into it. Rising air leaves a low
    //pressure zone behind it, sinking air piles up into a high one.
    float pressureLower;
    float pressureUpper;

    glm::vec2 windDirectionLower;
    glm::vec2 windDirectionUpper;
};

class WeatherBands
{
public:
    WeatherBands(std::vector<WeatherCell> cells)
    {
        if(cells.empty())
        {
            assert(false);
            return;
        }

        for(size_t i=0; i<cells.size(); ++i)
        {
            insert_sorted(m_cells, cells[i]);
        }
        
        generateWeatherBands(m_cells, m_bands);
    }

    void generateWeatherBands(const std::vector<WeatherCell> &cells, std::vector<WeatherBand> &bands)
    {
        float prevFrontUpperLatitude=0.0f;
        float prevFrontSize=0.0f;
        float prevMoisture=m_cells[0].moisture;

        for(size_t i=0; i<cells.size()-1; ++i)
        {
            WeatherBand cellBand;
            WeatherBand front;

            const WeatherCell &current=cells[i];
            const WeatherCell &next=cells[i+1];

            cellBand.name=current.name;

            float upperLatitude=current.upperLatitude;//current.latitude+(current.size*0.5f);
//            float frontSize=(M_PI_2-abs(upperLatitude))*(20.0f/90.0f);
            float currentSize=current.upperLatitude-current.lowerLatitude;
            float nextSize=next.upperLatitude-next.lowerLatitude;

            float frontRatio=0.2f;//(M_PI_2-abs(upperLatitude))*(0.4f/M_PI_2);
            float frontLowerLatitude=current.upperLatitude-(currentSize*frontRatio);
            float frontUpperLatitude=current.upperLatitude+(nextSize*frontRatio);

//            cellBand.size=currentSize-(prevFrontSize*0.5f)-(frontSize*0.5f);
//            cellBand.latitude=current.latitude+(prevFrontSize*0.25f)-(frontSize*0.25f
//            float currentLatitude=(currentSize/2.0f)+current.lowerLatitude;
//            float cellBandLatitude=currentLatitude+(prevFrontSize*0.25f)-(frontSize*0.25f);
//            cellBand.lowerLatitude=cellBandLatitude-(cellBand.size*0.5f);
//            cellBand.upperLatitude=cellBandLatitude+(cellBand.size*0.5f);
            cellBand.lowerLatitude=prevFrontUpperLatitude;
            cellBand.upperLatitude=frontLowerLatitude;
            cellBand.size=cellBand.upperLatitude-cellBand.lowerLatitude;

            cellBand.moistureLower=(current.moisture+prevMoisture)*0.5f;
            cellBand.moistureMiddle=current.moisture;

            glm::vec2 wind=current.windDirectionUpper-next.windDirectionLower;
            float frontMoisture=abs(glm::dot(current.windDirectionUpper, next.windDirectionLower));

            if(wind.y<0.0f) //wind moving away from each other
                frontMoisture=0.2f;

            front.name=current.name+"/"+next.name+" front";
            front.size=frontUpperLatitude-frontLowerLatitude;// frontSize;
            float frontLatitude=upperLatitude;
            front.lowerLatitude=frontLowerLatitude;// frontLatitude-(front.size*0.5f);
            front.upperLatitude=frontUpperLatitude;// frontLatitude+(front.size*0.5f);
            front.moistureLower=(current.moisture+frontMoisture)*0.5f;
            front.moistureMiddle=frontMoisture;
            front.moistureUpper=(next.moisture+frontMoisture)*0.5f;

            cellBand.moistureUpper=(current.moisture+frontMoisture)*0.5f;
            bands.push_back(cellBand);

            bands.push_back(front);

            prevFrontUpperLatitude=frontUpperLatitude;
//            prevFrontSize=frontSize;
            prevMoisture=frontMoisture;
        }

        //add last cell band
        {
            const WeatherCell &current=cells[cells.size()-1];
            WeatherBand cellBand;

            cellBand.name=current.name;
//            float currentSize=current.upperLatitude-current.lowerLatitude;
//            cellBand.size=currentSize-(prevFrontSize*0.5f);
//            float currentLatitude=(currentSize/2.0f)+current.lowerLatitude;
//            float cellBandLatitude=currentLatitude+(prevFrontSize*0.25f);
//            cellBand.lowerLatitude=cellBandLatitude-(cellBand.size*0.5f);
//            cellBand.upperLatitude=cellBandLatitude+(cellBand.size*0.5f);
            cellBand.lowerLatitude=prevFrontUpperLatitude;
            cellBand.upperLatitude=current.upperLatitude;
            cellBand.size=cellBand.upperLatitude-cellBand.lowerLatitude;
            cellBand.moistureLower=(current.moisture+prevMoisture)*0.5f;
            cellBand.moistureMiddle=current.moisture;
            cellBand.moistureUpper=current.moisture;

            bands.push_back(cellBand);
        }
    }

    size_t getCellIndex(float latitude)
    {
        for(size_t i=0; i<m_cells.size(); ++i)
        {
            if(latitude<m_cells[i].upperLatitude)
                return i;
        }
        return m_cells.size()-1;
    }

    const WeatherCell &getCell(float latitude)
    {
        return m_cells[getCellIndex(latitude)];
    }

    glm::vec2 getWindDirection(float latitude)
    {
        const WeatherCell cell=getCell(latitude);

        float value=(latitude-cell.lowerLatitude)/(cell.upperLatitude-cell.lowerLatitude);

//        glm::vec2 direction;

        return (cell.windDirectionUpper-cell.windDirectionLower)*value+cell.windDirectionLower;
    }

    float getPressure(float latitude)
    {
        const WeatherCell &cell=getCell(latitude);
        float size=cell.upperLatitude-cell.lowerLatitude;

        if(size<=0.0f)
            return cell.pressureLower;

        float value=std::clamp((latitude-cell.lowerLatitude)/size, 0.0f, 1.0f);

        return cell.pressureLower+((cell.pressureUpper-cell.pressureLower)*value);
    }

    size_t getBandIndex(float latitude)
    {
        for(size_t i=0; i<m_bands.size(); ++i)
        {
            if(latitude<m_bands[i].upperLatitude)
                return i;
        }
        return m_bands.size()-1;
    }

    const WeatherBand &getBand(float latitude)
    {
        return m_bands[getBandIndex(latitude)];
    }

    float getMoisture(float latitude)
    {
        const WeatherBand band=getBand(latitude);

        float value=(latitude-band.lowerLatitude)/band.size;

        //assert(value<=1.0f);
        value=std::clamp(value, 0.0f, 1.0f);

        if(value<0.5f)
        {
            value=2*value;
            return (band.moistureMiddle-band.moistureLower)*(1.0f-pow(value-1.0f, 2))+band.moistureLower;
        }
        else
        {
            value=2*(value-0.5);
            return (band.moistureUpper-band.moistureMiddle)*(pow(value, 2))+band.moistureMiddle;
        }
    }

protected:
    std::vector<WeatherBand> m_bands;
    std::vector<WeatherCell> m_cells;
};

constexpr float rads(float degrees)
{
    return M_PI/180.0f*degrees;
}

//How many circulation cells a planet runs per hemisphere, from how fast it spins. The reference
//gives these as measured regimes rather than a formula, and points out that four times earth's
//rate breaks the otherwise linear trend without explaining why - that is kept as given rather than
//smoothed into a curve.
inline int weatherCellsForRotation(float rotationRate)
{
    if(rotationRate<1.0f)
        return 1;   //half earth's rate and slower, a single cell running pole to equator
    if(rotationRate<3.0f)
        return 3;   //earth, at one to two times its rate
    if(rotationRate<6.0f)
        return 7;   //four times, the break in the trend
    return 5;       //eight times
}

inline std::string weatherCellName(int index, int cellsPerHemisphere, bool north)
{
    std::string hemisphere=north?"North":"South";

    if(cellsPerHemisphere==3)
    {//the earthlike case has names worth keeping
        const char *names[3]={"Hadley", "Ferrell", "Polar"};

        return hemisphere+" "+names[index]+" Cell";
    }
    return hemisphere+" Cell "+std::to_string(index+1);
}

//Moisture falls off from the equatorial cell out to the polar one. The three anchors are earth's
//values, so a three cell world reproduces them exactly.
inline float weatherCellMoisture(int index, int cellsPerHemisphere)
{
    if(cellsPerHemisphere<=1)
        return 0.95f;

    float t=(float)index/(float)(cellsPerHemisphere-1);

    if(t<0.5f)
        return 0.95f+((0.75f-0.95f)*(t/0.5f));
    return 0.75f+((0.65f-0.75f)*((t-0.5f)/0.5f));
}

//Builds a hemisphere-symmetric set of circulation cells. Cells divide each hemisphere evenly,
//which reproduces earth's 0/30/60/90 boundaries exactly at three cells per hemisphere. They
//alternate direct (air rising on the equatorward edge, sinking on the poleward one) and indirect,
//so the boundaries alternate low and high pressure outward from the equator - the intertropical
//convergence zone, the subtropical ridge, the polar front, and so on.
//rotationDirection is +1 for an earthlike prograde spin and -1 for a retrograde one, which
//reverses the coriolis deflection and with it every east-west wind component.
inline std::vector<WeatherCell> generateWeatherCells(int cellsPerHemisphere, float rotationDirection=1.0f)
{
    std::vector<WeatherCell> cells;

    if(cellsPerHemisphere<1)
        cellsPerHemisphere=1;

    float step=(float)M_PI_2/(float)cellsPerHemisphere;
    float spin=(rotationDirection<0.0f)?-1.0f:1.0f;

    cells.reserve((size_t)cellsPerHemisphere*2);

    //south first, polar-most cell leading, so the list runs south to north
    for(int i=cellsPerHemisphere-1; i>=0; --i)
    {
        WeatherCell cell;

        cell.name=weatherCellName(i, cellsPerHemisphere, false);
        cell.lowerLatitude=-step*(float)(i+1);
        cell.upperLatitude=-step*(float)i;
        cell.moisture=weatherCellMoisture(i, cellsPerHemisphere);

        if((i%2)==0)
        {//direct cell
            cell.windDirectionLower=glm::vec2(0.0f, 1.0f);
            cell.windDirectionUpper=glm::vec2(-spin, 0.0f);
        }
        else
        {
            cell.windDirectionLower=glm::vec2(spin, 0.0f);
            cell.windDirectionUpper=glm::vec2(0.0f, -1.0f);
        }

        //a boundary an even number of cells out from the equator is one the air rises through
        cell.pressureLower=(((i+1)%2)==0)?0.0f:1.0f;
        cell.pressureUpper=((i%2)==0)?0.0f:1.0f;

        cells.push_back(cell);
    }

    for(int i=0; i<cellsPerHemisphere; ++i)
    {
        WeatherCell cell;

        cell.name=weatherCellName(i, cellsPerHemisphere, true);
        cell.lowerLatitude=step*(float)i;
        cell.upperLatitude=step*(float)(i+1);
        cell.moisture=weatherCellMoisture(i, cellsPerHemisphere);

        if((i%2)==0)
        {//direct cell
            cell.windDirectionLower=glm::vec2(-spin, 0.0f);
            cell.windDirectionUpper=glm::vec2(0.0f, -1.0f);
        }
        else
        {
            cell.windDirectionLower=glm::vec2(0.0f, 1.0f);
            cell.windDirectionUpper=glm::vec2(spin, 0.0f);
        }

        cell.pressureLower=((i%2)==0)?0.0f:1.0f;
        cell.pressureUpper=(((i+1)%2)==0)?0.0f:1.0f;

        cells.push_back(cell);
    }

    return cells;
}

//const CellBand g_cellBands[]=
//{
//    {"Equator", rads(0.0f) , rads(10.0f), 0.7f, false},
//    {"Hadley Cell" , rads(17.5f), rads(25.0f), 1.0f, true},
//    {"Horse Latitude"  , rads(35.0f), rads(10.0f), 0.2f, false},
//    {"Ferrell Cell", rads(55.0f), rads(20.0f), 0.6f, true},
//    {"Polar Front" , rads(68.5f), rads(5.0f) , 0.8f, false},
//    {"Polar Cell"  , rads(80.0f), rads(20.0f), 0.45f, true}
//};
//
//
//inline size_t getCellIndex(float latitude, bool onlyCells=false)
//{
//    static bool init=false;
//    static std::vector<float> limits;
//
//    if(!init)
//    {
//        limits.resize(6);
//        for(size_t i=0; i<6; ++i)
//            limits[i]=g_cellBands[i].latitude+(g_cellBands[i].size*0.5f);
//        init=true;
//    }
//
//    size_t index=0;
//
//    for(size_t i=0; i<6; ++i)
//    {
//        if(latitude<limits[i])
//        {
//            if(onlyCells)
//                return index;
//            else
//                return i;
//        }
//
//        if(g_cellBands[i].cell)
//            index++;
//    }
//    return g_cellBands.size()-1;
//}
//
//inline size_t gellCellBoundaryCount
//
//inline const CellBand &getCellBand(float latitude)
//{
//    size_t index=getCellIndex(latitude);
//
//    return g_cellBands[index];
//}

//A real planet does not run a smooth pressure band around each latitude - the highs break up into
//discrete cells sitting over the oceans and over cold continental interiors, and the winds spiral
//out of them rather than blowing straight down the band. This is one of those cells.
struct PressureCentre
{
    glm::vec2 position;  //longitude, latitude in radians
    glm::vec3 point3d;
    float strength;
    float radius;        //angular radius in latitude, longitude is stretched against this
};

//How strongly a centre is felt at a point. Cells are kept elliptical, wider in longitude than in
//latitude, because that is the shape these systems actually take.
inline float pressureCentreInfluence(const PressureCentre &centre, const glm::vec2 &position, float longitudeStretch=1.8f)
{
    float deltaLatitude=position.y-centre.position.y;
    float deltaLongitude=position.x-centre.position.x;

    //the map wraps, take the short way round
    while(deltaLongitude>M_PI)
        deltaLongitude-=2.0f*(float)M_PI;
    while(deltaLongitude<-M_PI)
        deltaLongitude+=2.0f*(float)M_PI;

    deltaLongitude=deltaLongitude*cos(position.y); //longitude closes up towards the poles

    if(centre.radius<=0.0f)
        return 0.0f;

    float scaledLatitude=deltaLatitude/centre.radius;
    float scaledLongitude=deltaLongitude/(centre.radius*longitudeStretch);
    float distance2=(scaledLatitude*scaledLatitude)+(scaledLongitude*scaledLongitude);

    return centre.strength*exp(-distance2);
}

//Surface wind spiralling out of a high. Outflow turns clockwise in the northern hemisphere and
//anticlockwise in the southern one, so the direction is the outward radial turned by the outflow
//angle. Returned in local (east, north) axes.
inline glm::vec2 pressureCentreWind(const PressureCentre &centre, const glm::vec2 &position, float outflowAngle, float spin=1.0f)
{
    float deltaLongitude=position.x-centre.position.x;

    while(deltaLongitude>M_PI)
        deltaLongitude-=2.0f*(float)M_PI;
    while(deltaLongitude<-M_PI)
        deltaLongitude+=2.0f*(float)M_PI;

    //bearing from the centre out to the point, in local east/north axes
    float east=cos(position.y)*sin(deltaLongitude);
    float north=(cos(centre.position.y)*sin(position.y))-(sin(centre.position.y)*cos(position.y)*cos(deltaLongitude));
    glm::vec2 radial(east, north);

    float length=glm::length(radial);

    if(length<=std::numeric_limits<float>::epsilon())
        return glm::vec2(0.0f, 0.0f);

    radial=radial/length;

    //clockwise in the north, anticlockwise in the south, and a retrograde planet reverses both
    float angle=outflowAngle*spin;

    if(position.y<0.0f)
        angle=-angle;

    float cosAngle=cos(angle);
    float sinAngle=sin(angle);

    return glm::vec2((radial.x*cosAngle)+(radial.y*sinAngle), (radial.y*cosAngle)-(radial.x*sinAngle));
}

inline float getTemperature(float latitude)
{
    float magnitude=abs(latitude/M_PI_2);

    magnitude=magnitude*magnitude;
    return 60.0f-(150.0f*magnitude);
}

}//namespace worldgen

#endif //_worldgen_weather_h_