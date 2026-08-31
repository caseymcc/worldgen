[![discord](https://img.shields.io/discord/495955797872869376.svg?logo=discord "Discord")](https://discord.gg/BfceAsX)

# worldgen

World generator is a procedural world generator built on the [FastNoise2 library](https://github.com/Auburn/FastNoise2).
It starts from plate tectonics and keeps going. Plates are given Euler poles and rotated as rigid
bodies on a sphere; the motion at each boundary is split into the part acting across it and the part
acting along it, and what that raises becomes the height map. Weather cells drive wind, moisture and
temperature. From there the world grows rivers, coasts, rocks, ores, forests, climates, peoples,
states, trade and ruins — each layer reading the ones under it rather than being sprinkled on
independently.

Pick any cell on the world map and it expands into ground you could stand on, at 16 m per sample,
consistent with what the overview said was there.

- Documentation (WIP) - https://github.com/caseymcc/worldgen/wiki

## Lots of thanks go to
- [RedBlobGames](https://www.redblobgames.com/x/1843-planet-generation/)
- [Experilous](http://experilous.com/1/blog/post/procedural-planet-generation)

## Building

CMake (>= 3.15) + [vcpkg](https://github.com/microsoft/vcpkg) in manifest mode, C++17.

```bash
git submodule update --init --recursive
export VCPKG_ROOT=/path/to/vcpkg
./scripts/configure.sh          # or --debug
./scripts/build.sh
./scripts/run.sh
```

Builds go to `build/<arch>/<platform>/<type>`, so debug and release live side by side.

If you would rather not install a toolchain at all, everything needed to build is in a container:

```bash
./run.sh          # builds the image, builds the project, starts the viewer
```

The host needs only docker. The repository is bind mounted rather than copied, the container runs as
you so nothing ends up owned by root, and compiled vcpkg ports are cached in a volume so only the
first build pays for them. `./run.sh --native` skips docker and uses the host's own toolchain.

## Features

**The world**
- Plate tectonics with Euler poles, convergent/divergent/transform boundaries, and four orogeny
  styles (Andean, Laramide, Ural, Himalayan)
- Mantle hotspots, independent of any boundary
- Weather cells, wind, pressure centres, moisture and rain shadow
- Köppen-Geiger climate, 18 zones
- Ocean currents, upwelling and overturning
- Rivers: depression filling, flow routing, Strahler order, drainage basins, deltas, channel width
  from discharge
- Coast classification, 11 types
- Surface geology and cratons; 26 ore and mineral deposit types with grade and exposure
- Treeline, forest, and what can be burned — wood, peat or coal

**The people in it**
- Configurable per-species habitat preferences, so a world can be settled by more than humans
- Cultural and physiographic regions, bounded by the ground that keeps peoples apart
- States that grow outward from a seat of power until their reach runs out, with frontier marches
  and the occasional holdout nobody ever took
- Trade routes between states that each have what the other lacks, with chokepoints
- Coastal cities sited on what geography gives for free, including cities that outgrew their own
  fields and live on imported grain
- Ruins of fallen civilizations, each with a cause of death the map can account for

**The viewer**
- Zoom and pan the world map; a legend for whatever layer and overlay is on screen
- Click any cell to expand it into detailed terrain

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_climate.png)

## Detail view

The overview map says what a cell averages, which river crosses it and how much water it carries.
Selecting a cell expands that into a 256x256 region — 4096 x 4096 m at 16 m per sample — and the two
have to agree. The base is interpolated *through* the surrounding cell heights so a coastal cell
keeps its coastline; detail is added as a residual sampled in world space, so two neighbouring
regions match along their shared edge and a region regenerates identically from the seed without
being stored anywhere.

The river is not painted on. Where the overview says water enters and leaves the cell are fixed
boundary conditions, and the channel is routed between them by least-cost path over ground that
charges for climbing. Its width comes from the discharge the overview already computed, so the same
river is a stream you could step over in one cell and a hundred metres across in another. Inflow and
outflow balance against the overview at both edges.

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_detail.png)

Depression filling conditions a copy rather than the ground, so terrain keeps its shape — and where
the fill had to hold water back, that is a lake. Whether a basin actually holds one is a question
about climate, not about the shape of the ground: in dry country it does not, and the same hollow is
a salt pan. Ground colour follows the climate, and takes the temperature at each sample's own height,
so a mountain in the tropics still wears snow above its treeline.

## Rivers
![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_rivers.png)

## Ocean currents
![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_currents.png)

## Ore and mineral deposits
![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_ore.png)

## Peoples
Which people the ground suits best, given each species' own preferences — elves toward forest,
dwarves toward hills and mountains, and so on.

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_peoples.png)

## Cultural regions
Clusters bounded by the hard divides — sea, mountain wall, desert — then cut again wherever the
climate changes family, because a people rarely expands into country that will not grow what it
knows how to grow.

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_regions.png)

## States
No border is ever drawn. Each state is a seat of power and a budget, the budget is spent walking
outward over ground that costs what it costs to hold, and wherever it runs out is the border. That
the borders then land on mountain fronts and desert edges is a consequence rather than a rule.

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_states.png)

## Trade routes
Routes run between states that each have something the other lacks, and take the cheapest way there —
which is usually the water. Chokepoints are the places the world narrows to a gap and everything has
to go through.

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_trade.png)

## Coastal cities
Four kinds of place that take rather than make: a river mouth where two supply chains meet, a strait
every hull pays to pass, shelter on a coast that kills ships, and a coast the world sails to for
what is in the ground.

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_cities.png)

## Ruins
Civilizations that rose and fell, sited by the world as it was in *their* era rather than this one —
which is why a dead city can sit in what is now desert.

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_ruins.png)

## Beaten track
Distance from the trade network, walked at the cost the ground actually charges. Where the roads are,
and where the backcountry starts.

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_beatentrack.png)

## Plates
![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_plates.png)

## Collisions
![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_collisions.png)

## Heightmap after tectonics
![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_heightmap.png)

## Weather Cells
![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_weathercells.png)

## Moisture
![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot_moisture.png)
