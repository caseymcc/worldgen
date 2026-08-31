#include "legend.h"

namespace mapgen
{

namespace
{

void addOcean(Legend &legend)
{
    legend.m_entries.push_back(LegendEntry(legendOceanColor(), "sea"));
}

}//unnamed namespace

Legend buildLayerLegend(DisplayLayers layer, WorldGenerator *generator)
{
    Legend legend;

    switch(layer)
    {
    case DisplayLayers::Climate:
        legend.m_title="Koppen climate";
        legend.m_description="Classified from the annual mean temperature and moisture of each cell.";

        //Ocean is the first enumerator, so the land zones run from the one after it to the end.
        for(size_t z=0; z<worldgen::ClimateZoneCount; ++z)
        {
            worldgen::ClimateZone zone=(worldgen::ClimateZone)z;

            legend.m_entries.push_back(LegendEntry(worldgen::climateZoneColor(zone),
                worldgen::climateZoneName(zone)));
        }
        break;

    case DisplayLayers::Coast:
        legend.m_title="Coast type";
        legend.m_description="What the shoreline is made of, from relief, margin and climate.";

        for(size_t c=1; c<=(size_t)worldgen::CoastType::CoralReef; ++c)
        {
            worldgen::CoastType coast=(worldgen::CoastType)c;

            legend.m_entries.push_back(LegendEntry(worldgen::coastTypeColor(coast),
                worldgen::coastTypeName(coast)));
        }
        break;

    case DisplayLayers::Rocks:
        legend.m_title="Surface rock";
        legend.m_description="What is under the soil, and what a quarry or a mine would find.";

        for(size_t r=0; r<(size_t)worldgen::RockTypeCount; ++r)
        {
            worldgen::RockType rock=(worldgen::RockType)r;

            legend.m_entries.push_back(LegendEntry(worldgen::rockTypeColor(rock),
                worldgen::rockTypeName(rock)));
        }
        break;

    case DisplayLayers::Deposits:
        legend.m_title="Ore and mineral deposits";
        legend.m_description="Where each deposit type can form, given the rock and the history that made it.";

        for(size_t d=1; d<(size_t)worldgen::DepositTypeCount; ++d)
        {
            worldgen::DepositType deposit=(worldgen::DepositType)d;

            legend.m_entries.push_back(LegendEntry(worldgen::depositTypeColor(deposit),
                worldgen::depositTypeName(deposit)));
        }
        break;

    case DisplayLayers::Fuel:
        legend.m_title="Fuel and forest";
        legend.m_description="The best thing that can be burned here - which decides whether metal can be worked at all.";

        for(size_t f=0; f<=(size_t)worldgen::FuelSource::Coal; ++f)
        {
            worldgen::FuelSource fuel=(worldgen::FuelSource)f;

            legend.m_entries.push_back(LegendEntry(worldgen::fuelSourceColor(fuel),
                worldgen::fuelSourceName(fuel)));
        }
        break;

    case DisplayLayers::Barriers:
        legend.m_title="Barriers";
        legend.m_description="Ground hard enough to keep peoples apart. These bound the physiographic regions.";
        legend.m_entries.push_back(LegendEntry(legendBarrierColor(worldgen::BarrierType::None), "crossable"));
        legend.m_entries.push_back(LegendEntry(legendBarrierColor(worldgen::BarrierType::Mountain), "mountain wall"));
        legend.m_entries.push_back(LegendEntry(legendBarrierColor(worldgen::BarrierType::Desert), "desert or rough steppe"));
        legend.m_entries.push_back(LegendEntry(legendBarrierColor(worldgen::BarrierType::Ocean), "open sea"));
        break;

    case DisplayLayers::Ruins:
        legend.m_title="Ruins";
        legend.m_description="What the fallen civilizations left. Hue is what the thing was, brightness how much still stands.";
        legend.m_entries.push_back(LegendEntry(legendRuinKindColor(worldgen::RuinKind::Monument), "monument"));
        legend.m_entries.push_back(LegendEntry(legendRuinKindColor(worldgen::RuinKind::Fortress), "fortress"));
        legend.m_entries.push_back(LegendEntry(legendRuinKindColor(worldgen::RuinKind::MineHead), "mine head"));
        legend.m_entries.push_back(LegendEntry(legendRuinKindColor(worldgen::RuinKind::Harbour), "harbour works"));
        legend.m_entries.push_back(LegendEntry(legendRuinKindColor(worldgen::RuinKind::Terraces), "terraces"));
        legend.m_entries.push_back(LegendEntry(legendRuinKindColor(worldgen::RuinKind::Tomb), "tomb"));
        legend.m_entries.push_back(LegendEntry(legendRuinKindColor(worldgen::RuinKind::Bridge), "bridge"));
        break;

    case DisplayLayers::Cities:
        legend.m_title="Coastal cities";
        legend.m_description="Drains built where flow concentrates. Traffic glows along the sea lanes behind them.";
        legend.m_entries.push_back(LegendEntry(legendCityColor(worldgen::NodeType::Mouth, false),
            "mouth", "a river meets the sea, so two supply chains do"));
        legend.m_entries.push_back(LegendEntry(legendCityColor(worldgen::NodeType::Strait, false),
            "strait", "a gap every hull pays to pass"));
        legend.m_entries.push_back(LegendEntry(legendCityColor(worldgen::NodeType::Anchorage, false),
            "anchorage", "shelter on a coast that kills ships"));
        legend.m_entries.push_back(LegendEntry(legendCityColor(worldgen::NodeType::Extraction, false),
            "extraction", "the world sails here for what is in the ground"));
        legend.m_entries.push_back(LegendEntry(legendCityColor(worldgen::NodeType::Mouth, true),
            "parasite", "outgrew its own fields and eats imported grain"));

        if(generator)
        {
            const std::vector<worldgen::City> &cities=generator->getCities();
            size_t parasites=0;

            for(size_t c=0; c<cities.size(); ++c)
            {
                if(cities[c].m_parasite)
                    parasites++;
            }

            legend.m_description+="  "+std::to_string(cities.size())+" cities, "
                +std::to_string(parasites)+" of them parasites.";
        }
        break;

    case DisplayLayers::Remoteness:
        legend.m_title="Beaten track";
        legend.m_description="Distance from the trade network, walked at the cost the ground actually charges.";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(235, 205, 140), "on the beaten track",
            "within reach of a route or a city"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(105, 110, 85), "backcountry",
            "fading out the further it is from anywhere"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(150, 50, 60), "cut off",
            "nothing walks here from anywhere"));
        addOcean(legend);
        break;

    case DisplayLayers::States:
        legend.m_title="States";
        legend.m_description="Each power gets its own colour, radiating from its seat until its budget runs out. "
            "The marches - frontier ground held loosely, or with a rival next to it - are drawn dimmer.";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(54, 52, 48), "beyond any state's reach"));
        addOcean(legend);

        if(generator)
        {
            const std::vector<worldgen::Polity> &polities=generator->getPolities();
            size_t enclaves=0;

            for(size_t p=0; p<polities.size(); ++p)
            {
                if(polities[p].m_enclave)
                    enclaves++;
            }

            legend.m_description+="  "+std::to_string(polities.size()-enclaves)+" realms and "
                +std::to_string(enclaves)+" holdouts.";
        }
        break;

    case DisplayLayers::Regions:
        legend.m_title="Cultural regions";
        legend.m_description="Clusters bounded by the hard divides, then cut again wherever the climate changes family. "
            "Colours only tell one region from its neighbour.";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(52, 50, 48), "barrier ground, or too small to name"));
        addOcean(legend);
        break;

    case DisplayLayers::Peoples:
        legend.m_title="Peoples";
        legend.m_description="Which people this ground suits best, brighter where it suits them more.";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(58, 58, 62), "nobody would settle"));
        addOcean(legend);

        if(generator)
        {
            const std::vector<worldgen::SpeciesHabitat> &species=generator->getSpecies();

            for(size_t k=0; k<species.size(); ++k)
            {
                //the same hash the viewer paints with
                glm::ivec3 color(((k*97)%200)+55, ((k*151)%200)+55, ((k*211)%200)+55);

                legend.m_entries.push_back(LegendEntry(color, species[k].m_name));
            }
        }
        break;

    case DisplayLayers::Trade:
        legend.m_title="Trade routes";
        legend.m_description="Routes between states that each have what the other lacks, taking the cheapest way. "
            "The busier a road, the brighter it burns.";
        legend.m_gradient=true;
        legend.m_low=glm::ivec3(100, 80, 60);
        legend.m_high=glm::ivec3(255, 240, 130);
        legend.m_lowLabel="quiet";
        legend.m_highLabel="busiest";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(255, 70, 70), "chokepoint",
            "heavy traffic with no way round it"));
        break;

    case DisplayLayers::Habitability:
        legend.m_title="Habitability";
        legend.m_description="How well the ground suits whichever people it suits best - water, soil, grazing, "
            "timber, stone, fuel and ore together.";
        legend.m_gradient=true;
        legend.m_low=glm::ivec3(40, 35, 45);
        legend.m_high=glm::ivec3(130, 250, 115);
        legend.m_lowLabel="uninhabitable";
        legend.m_highLabel="rich";
        addOcean(legend);
        break;

    case DisplayLayers::Control:
        legend.m_title="Power projection";
        legend.m_description="How firmly a state holds its ground: full at the seat of power, falling away to "
            "nothing where its reach ends.";
        legend.m_gradient=true;
        legend.m_low=glm::ivec3(60, 50, 45);
        legend.m_high=glm::ivec3(250, 190, 105);
        legend.m_lowLabel="frontier";
        legend.m_highLabel="heartland";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(44, 42, 40), "unclaimed"));
        break;

    case DisplayLayers::Rivers:
        legend.m_title="Rivers";
        legend.m_description="Drainage after depression filling and flow routing. Wider where more water passes.";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(60, 130, 220), "river"));
        addOcean(legend);
        break;

    case DisplayLayers::Currents:
        legend.m_title="Ocean currents";
        legend.m_description="Wind driven at the surface, warm where they run from the tropics and cold where "
            "they run back.";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(215, 105, 85), "warm"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(85, 150, 225), "cold"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(120, 230, 200), "upwelling",
            "cold nutrient rich water rising, and the great fisheries"));
        break;

    case DisplayLayers::Hotspots:
        legend.m_title="Hotspots";
        legend.m_description="Relief raised by a mantle plume, independent of any plate boundary.";
        legend.m_gradient=true;
        legend.m_low=glm::ivec3(30, 30, 40);
        legend.m_high=glm::ivec3(255, 160, 60);
        legend.m_lowLabel="none";
        legend.m_highLabel="strongest";
        break;

    case DisplayLayers::Height:
    case DisplayLayers::Map:
        legend.m_title="Terrain";
        legend.m_description="Height above and below sea level.";
        legend.m_gradient=true;
        legend.m_low=glm::ivec3(20, 34, 64);
        legend.m_high=glm::ivec3(245, 245, 245);
        legend.m_lowLabel="deep sea";
        legend.m_highLabel="peaks";
        break;

    case DisplayLayers::Plates:
        legend.m_title="Tectonic plates";
        legend.m_description="One colour per plate, found by flood filling the cellular noise into connected pieces.";

        if(generator)
            legend.m_description+="  "+std::to_string(generator->getPlateCount())+" plates.";
        break;

    default:
        legend.m_title="Layer";
        legend.m_description="A raw field, drawn dark for low values and bright for high ones.";
        legend.m_gradient=true;
        legend.m_low=glm::ivec3(20, 20, 20);
        legend.m_high=glm::ivec3(245, 245, 245);
        legend.m_lowLabel="low";
        legend.m_highLabel="high";
        break;
    }

    return legend;
}

Legend buildOverlayLegend(DisplayOverlays overlay)
{
    Legend legend;

    legend.m_gradient=true;

    switch(overlay)
    {
    case DisplayOverlays::Collision:
    case DisplayOverlays::CollisionDistance:
        legend.m_title="Collision";
        legend.m_description="Plate motion normal to the boundary: apart where it is negative, together where positive.";
        legend.m_low=glm::ivec3(70, 110, 220);
        legend.m_high=glm::ivec3(220, 80, 70);
        legend.m_lowLabel="divergent";
        legend.m_highLabel="convergent";
        break;

    case DisplayOverlays::Shear:
        legend.m_title="Shear";
        legend.m_description="Motion along the boundary rather than across it, which is what makes a transform fault.";
        legend.m_low=glm::ivec3(30, 30, 40);
        legend.m_high=glm::ivec3(230, 200, 90);
        legend.m_lowLabel="none";
        legend.m_highLabel="strong";
        break;

    case DisplayOverlays::BoundaryType:
        legend.m_title="Boundary type";
        legend.m_gradient=false;
        legend.m_description="How the two plates meet, decided by the normal and tangential parts of their motion.";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(220, 80, 70), "convergent"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(70, 110, 220), "divergent"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(230, 200, 90), "transform"));
        break;

    case DisplayOverlays::Orogeny:
        legend.m_title="Orogeny";
        legend.m_gradient=false;
        legend.m_description="Which of the four mountain building styles a convergent boundary is running.";
        legend.m_entries.push_back(LegendEntry(glm::ivec3(225, 120, 60), "Andean", "ocean under continent"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(200, 175, 80), "Laramide", "shallow subduction, far inland"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(140, 190, 100), "Ural", "two continents, long since worn down"));
        legend.m_entries.push_back(LegendEntry(glm::ivec3(230, 230, 235), "Himalayan", "two continents, still rising"));
        break;

    case DisplayOverlays::Scale:
        legend.m_title="Terrain scale";
        legend.m_description="How much relief the boundary is entitled to raise here.";
        legend.m_low=glm::ivec3(30, 30, 40);
        legend.m_high=glm::ivec3(245, 245, 245);
        legend.m_lowLabel="flat";
        legend.m_highLabel="mountainous";
        break;

    case DisplayOverlays::Temp:
        legend.m_title="Temperature";
        legend.m_description="Annual mean, from latitude, elevation and how far the cell sits from the sea.";
        legend.m_low=glm::ivec3(60, 90, 200);
        legend.m_high=glm::ivec3(220, 70, 60);
        legend.m_lowLabel="-90 C";
        legend.m_highLabel="+70 C";
        break;

    case DisplayOverlays::Moisture:
    case DisplayOverlays::MoistureGreyScale:
        legend.m_title="Moisture";
        legend.m_description="Water carried in off the sea and dropped as the air rises and cools.";
        legend.m_low=glm::ivec3(120, 100, 60);
        legend.m_high=glm::ivec3(60, 140, 220);
        legend.m_lowLabel="arid";
        legend.m_highLabel="wet";
        break;

    case DisplayOverlays::WeatherCells:
        legend.m_title="Weather cells";
        legend.m_gradient=false;
        legend.m_description="Polar, Ferrel and Hadley, north and south - the six circulation cells the winds come from.";
        break;

    case DisplayOverlays::WeatherBands:
        legend.m_title="Weather bands";
        legend.m_gradient=false;
        legend.m_description="The bands and fronts between those cells, perturbed by noise so they are not straight lines.";
        break;

    case DisplayOverlays::Pressure:
        legend.m_title="Pressure";
        legend.m_description="Where air rises and leaves a low, and where it sinks into a high.";
        legend.m_low=glm::ivec3(70, 110, 220);
        legend.m_high=glm::ivec3(220, 80, 70);
        legend.m_lowLabel="low";
        legend.m_highLabel="high";
        break;

    default:
        break;
    }

    return legend;
}

}//namespace mapgen
