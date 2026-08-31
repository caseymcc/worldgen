# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

CMake (>= 3.15) + [vcpkg](https://github.com/microsoft/vcpkg) in manifest mode, C++17, no test suite.

`./run.sh` at the repo root is the one-command entry point: it fetches submodules, builds the
container image, builds the project and starts the viewer, skipping whichever of those is already
done. `./run.sh --debug`, `--rebuild`, `--rebuild-image`, and `--native` to bypass docker and use the
host's own toolchain. Everything below is what it delegates to, for driving a step at a time.

```bash
git submodule update --init --recursive          # deps/fastnoise2, deps/imglib
export VCPKG_ROOT=/path/to/vcpkg                 # picked up automatically
./scripts/configure.sh                           # or --debug, or -DWORLDGEN_MAPGEN=OFF
./scripts/build.sh                               # configures first if needed
./scripts/run.sh                                 # the viewer
```

Trees go to `build/<arch>/<platform>/<type>` — `build/x64/linux/release` by default, so debug and
release coexist and neither touches the stale tree at `build/` itself. `configure.sh` also symlinks
`compile_commands.json` to the repo root. Both scripts take `-t/--type`, `-a/--arch`,
`-p/--platform`, `-g/--generator` and `--clean`; `build.sh` also takes `--target` and `-j`. Anything
they do not recognise is passed through to cmake. `scripts/env.sh` holds the shared path and
toolchain logic and is sourced, not run.

### Building without installing anything

`docker/Dockerfile` carries the whole toolchain — compiler, cmake, ninja, vcpkg pinned to a commit,
and the X11/OpenGL development packages the mapgen ports need. The host needs only docker.

```bash
./scripts/docker.sh                              # builds the default configuration
./scripts/docker.sh ./scripts/build.sh --debug
./scripts/docker.sh bash                         # a shell in the build environment
./scripts/docker.sh --gui ./scripts/run.sh       # the viewer, on the host's display
```

The image contains no source: the repository is bind mounted at `/work`, so edits on the host are
what gets built and the image is not invalidated by them. The container runs as the calling user, so
build output is not owned by root. Compiled ports live in a named volume (`worldgen-vcpkg-cache`), so
the first run compiles them and later runs do not — delete the volume to force that again. A
container build lands in `build/x64/linux-container/release` rather than `.../linux/...`, because a
container build and a host build are not interchangeable and must not share a tree.

`--gui` passes `DISPLAY`, the X11 socket and `/dev/dri` through. On an NVIDIA host the image has no
proprietary userspace driver, so mesa falls back to software rendering (`glx: failed to create dri3
screen` in the log is this, not a failure); hardware rendering there needs the nvidia container
toolkit.

`CMakeLists.txt` selects `$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake` as the toolchain when
the caller has not passed `-DCMAKE_TOOLCHAIN_FILE`, so no extra flags are needed once `VCPKG_ROOT`
is exported. The first configure is slow: vcpkg builds every port into
`$VCPKG_ROOT/{buildtrees,packages,installed}`.

### Dependencies

Declared in `vcpkg.json`. `glm`, `rapidjson`, `libpng` and `libjpeg-turbo` are unconditional (the
last two serve the `imglib` submodule). `glfw3`, `glbinding`, `glew` and
`imgui[glfw-binding,opengl3-binding]` sit behind the `mapgen` manifest feature.

**vcpkg manifest features must be selected before `project()`** — `CMakeLists.txt` appends `mapgen`
to `VCPKG_MANIFEST_FEATURES` in the block above `project()` when `WORLDGEN_MAPGEN` is ON. A new
optional dependency has to be wired in there, not further down the file.

Target names differ from the Hunter packages this project used previously: `glm::glm` (not `glm`),
`PNG::PNG` / `JPEG::JPEG` in imglib (not `PNG::png` / `JPEG::jpeg`). vcpkg ships upstream's
`RapidJSONConfig.cmake`, which exports include dirs but *no* imported target, so `CMakeLists.txt`
synthesizes a `RapidJSON::rapidjson` INTERFACE target. Hunter also built glm with
`GLM_FORCE_CTOR_INIT` for GCC; that is now an explicit `target_compile_definitions` on `worldgen`.

`deps/fastnoise2` and `deps/imglib` still build from the submodules via `add_subdirectory`.

Options: `WORLDGEN_MAPGEN` (build the app, ON), `WORLDGEN_SHARED` (OFF),
`WORLDGEN_FORCE_RELEASERUNTIME` (MSVC `/MD` in debug).

The checked-in `build/` directory is a stale MSVC/Hunter tree from `E:/projects/worldgen`; configure
into a fresh directory instead. `cmake/targetInfo.cmake` is left over from the removed Hunter
`CreateLaunchers` package and is no longer included by anything.

## Architecture

Two pieces: `worldgen`, a projection/tectonics/weather library with no UI or GL dependencies, and
`mapgen`, a GLFW + Dear ImGui + glbinding viewer that drives it.

### Generation pipeline

Everything lives in `EquiRectWorldGenerator` (`source/generators/equiRectWorldGenerator.cpp`, ~1500
lines — this is the file that matters). `create()` → `generateWorldOverview()` → `generatePlates()`,
which runs the whole world in one pass and reports stages through `Progress`:

weather bands → coordinates → base heightmap noise → terrain → air currents → tectonic plates →
continents → height map → tectonic zones → influence map → moisture → normalize neighbors

`generateContinents()` exists but is not called; its work happens inside `generatePlates()`.

### Two resolutions

The world is sized in blocks (`WorldDescriptors::setSize`, e.g. 1048576 × 524288 × 1024) but the
overview is computed on a much coarser **influence map**: `EquiRectDescriptors::m_influenceGridSize`
(4096 blocks per cell) divides the world size into `m_influenceSize` cells.
`calculateInfluenceSize()` also derives `m_plateFrequency` / `m_continentFrequency` from it, so noise
scales with world size. `generateChunk()` / `generateRegion()` / `getBaseHeight()` bilinearly
interpolate the influence map and add per-block noise; they never re-run the overview.

`InfluenceCell` (`include/worldgen/tectonics.h`) is the single per-cell record — plate id, plate
value/distance, collision, terrain scale, weather cell/band, wind direction, temperature, moisture.
Nearly every stage reads and writes fields on this one struct.

### The detail pass

`generateDetail()` (`source/generators/detailPass.cpp`, types in `include/worldgen/detail.h`) expands
one influence cell into a `DetailMap` — 256x256 samples over the cell's 4096 blocks, so ~16 m per
sample. Five stages, in order, following `docs/RESEARCH_DETAIL_PASS.md`:

1. **Amplify** — bilinear over the cell's four corner values from `m_influenceNeighborMap` (the base
   is continuous across cell boundaries by construction), plus a zero-mean residual. The residual is
   a hash-based fBm sampled in **world** coordinates, which is what makes neighbouring regions agree
   at their shared edge and makes a region reproducible from the seed alone. Its amplitude is also
   defined at the corners (the spread of the four cells meeting there) so it cannot step at a seam.
2. **Condition** — priority-flood + epsilon out from the region rim, then flats resolution.
3. **Route** — anisotropic A* from each mandated entry to the mandated exit, over a cost field that
   pays for climbing, with Galin's coprime segment mask (k=5) instead of 8-connectivity. Entries are
   routed biggest-first and an existing channel costs `m_reuseWeight`, so tributaries join the stem.
4. **Size** — `a = Q/u` with `u ~ Q^0.1`, split into width and depth by a ratio that also grows with
   Q, giving Leopold–Maddock's `w ~ Q^0.5`, `d ~ Q^0.4`. Carving takes each sample's shape from the
   **nearest** point on the course; letting any course point within reach lower it flattens a wide
   river's own bed.
5. **Seams** — nothing is negotiated with neighbours. The overview already ran a world-wide
   depression fill, D8 routing and accumulation, so `flowDirection` / `riverDischarge` are a
   pre-solved global flow graph both sides of a boundary read the same way.

Boundary conditions come from the overview and are not parameters: where the river enters, where it
leaves, and how much water it carries. Inflow is the sum of neighbours draining in; outflow is this
cell's `riverDischarge`; the difference is local runoff gained along the course, so the water balance
closes at both edges. Measured: mean drift between the coarse and amplified means 0.0003–0.006 in
normalized height, seam step ~500x smaller than intra-cell variation, determinism exact, every entry
connected to the exit, 0% of channel samples without a lower channel neighbour, river covering
0.4–3.5% of a region. 8 ms median / 81 ms worst per region.

**The pass works in metres, not normalized height.** A unit of `heightBase` is worth a few metres
beside the sea and ~23,000 m at the top of the curve, so a residual defined in height units gives
detail of wildly different physical size depending on elevation — it produced 0–9000 m of relief
inside one 4 km cell at the ceiling. The 5x5 patch, the Catmull-Rom, the residual, the depression
fill (epsilon in metres) and the carving all work on signed elevation; `height` is derived once at
the very end via `signedToHeight`. Amplitude is `m_minimumRelief + m_residualScale * spread` — an
absolute floor plus a proportional term, because multiplying them ties flat country's detail to a
number that is near zero there and no single setting then serves both a coastal plain and a
mountain front. Elevation reflects off the top and bottom of the world rather than clipping, so
saturated ground keeps its relief instead of flattening to the ceiling.

**Depression filling conditions a COPY (`drained`), never the terrain.** Filling raises a pit to its
outlet, which is flat by definition; doing that to the ground turns a rugged region into a scatter
of dead flat plateaus. Water routes on the filled surface, the ground keeps its shape, and where the
fill had to raise the surface by more than `m_lakeDepth` the water has nowhere to go — that is a
lake — but only where the climate can keep one. Whether a closed basin holds water is a question
about evaporation, not about the shape of the ground: below `m_lakeMoisture` it does not, and the
same hollow is a salt pan (`DetailCell::saltFlat`), floored flat by what the water left behind. A
river carrying more than `m_lakeRiver` sustains a terminal lake anyway, the way the Nile and the
Jordan do. Without that gate, desert regions came out with as many lakes as rainforest ones (2.7%
against 2.4%); with it, arid cells hold 0.00% lake and 0.8-2.7% salt flat, and the rest of the world
is unchanged. Lakes overall run 1.3-2.5% of land against Earth's ~2%.

Note that river *presence* cannot be used for this test: 99% of arid land cells carry some
`riverDischarge`, so only its magnitude discriminates.

Water is two different things and the map carries both: `DetailCell::sea` with a depth from
`heightToDepth` for anything under sea level, and `water`/`channelWidth`/`discharge` for a routed
river. The sea pass runs last, so a channel carved past sea level at a mouth becomes sea. Sub-sea
samples also seed the depression fill alongside the region rim — the sea is already drained, and
filling it would flatten the sea floor into a sheet.

**The base is a Catmull-Rom through cell values, not a bilinear over averaged corners.** Averaging
the four cells that meet at a corner is a box blur: a coastal cell whose neighbours are ocean loses
its own height entirely, and a cell at 0.504 came out 100% submerged. Interpolating *through* the
cell values keeps the coastline where the overview put it, is still continuous across a boundary
(both sides evaluate the same function of world position), and is clamped between the two central
samples so a cubic cannot overshoot a shoreline. `getBaseHeight()` and `generateChunk()` still use
the corner map, which is right for them — they want a smooth field, not a coastline.

Converting between metres and normalized height uses the exact curve inverses
(`elevationToHeight` / `depthToHeight`), never a local gradient: `heightToElevation` is flat at the
waterline, so a gradient-based conversion made one metre of carving equal a full unit of height and
drove river mouths to −6000 m.

**Every `pow` in `detail.h` and `climate.h` is qualified `std::pow` deliberately.**
`maths/math_helpers.h` declares `worldgen::pow(_Type, unsigned)` — an integer power — and an
unqualified call from inside the namespace finds *that* first, silently truncating a fractional
exponent. An exponent of 1.5 becomes 1 and the curve goes linear; 0.667 becomes 0 and the function
returns 1.0 for every input, which pinned every land sample in every detail region to height 1.0.
`climate.h` is only correct today because it happens to be included before `math_helpers.h`.

### Projections and wrapping

`include/worldgen/maths/coords.h` is a templated projection layer (`Cartesian`, `Spherical`,
`Equirectangular`, `Stereographic`) with `projectPoint<From, To>` specializations, plus
`generateFibonacciSphere()` used to seed plate centers evenly on the sphere. The map is an
equirectangular cylinder: it wraps in x and must be treated as such — use `wrap()`
(`include/worldgen/wrap.h`) and `get2DCellNeighbors_eq()` rather than raw index arithmetic.
Note the y-axis convention noted in `fill.h`: world +y is up, image +y is down.

### Noise

FastNoise2 is deliberately **kept out of public headers**: all `FastNoise::SmartNode` members live in
`EquiRectWorldGenerator::Hidden`, a pimpl defined in the .cpp (commit `ded87b8`). Keep new noise
nodes there — don't reintroduce FastNoise types into `include/`. Commented-out `HastyNoise` code
throughout is the previous noise backend.

### Weather

`WeatherBands` (`weather.h`) takes six latitude-sorted `WeatherCell`s (polar/Ferrel/Hadley, north and
south), derives the bands and fronts between them, and supplies wind direction and moisture.
`PerturbedWeatherBands` (`perturbedWeather.h`) precomputes per-column noise-perturbed band/cell
boundaries so the fronts aren't straight latitude lines; the generator uses this subclass.

### mapgen app

`ImguiWidget` (name, position, size, `onInitialize`/`onPosition`/`onSize`/`onDraw` hooks) →
`UiDataWidget` (adds a `UiData*`) → `WorldControls`, `WorldViewer`, `RegionViewer`, `ChunkViewer`,
`InfoViewer`, `LegendViewer`. Left-clicking the world map sets `UiData::selectedCell` and clears
`detailValid`; `RegionViewer` then calls `generateDetail()` and draws the result (Terrain /
Elevation / Water and floodplain / Overview base).

`WorldViewer` zooms and pans: wheel magnifies about the pointer, right or middle drag moves the
view, left click still selects a cell, and a `fit` button appears once zoomed. `ImageWidget` owns
the view state (`setView`/`resetView`, centre in uv plus a magnification) and the screen-to-texel
mapping, which now accounts for letterboxing and for the view — the old `getImagePosition` assumed
the image filled the widget, so cell picking was slightly off. The world map sets `setWrapX(true)`,
so panning across the antimeridian works via `GL_REPEAT` rather than hitting a wall.

`mapgen/terrainPalette.h` gives the detail view its ground colours, per Koppen zone. These are
deliberately NOT `worldgen::climateZoneColor` — that is a classification key, picked so eighteen
zones can be told apart on one map, and it would paint a rainforest flat cyan. Climate also varies
going up a hill, so the palette takes the temperature at each SAMPLE's height (cell temperature plus
`elevationTemperature` of the difference), turning ground above the treeline to bare rock and ground
below freezing to snow. Snow keys off the annual mean because there is no seasonality in the model,
which is a slightly generous proxy for a permanent snowline.

`LegendViewer` sits in the slot beside the detail view (where the `ChunkViewer` stub used to be;
that widget still builds but is no longer drawn) and explains the current layer and overlay.
`mapgen/legend.{h,cpp}` builds the entries: for climate, coast, rock, deposit and fuel it calls the
same `worldgen::*Color()` functions `WorldViewer` paints with, so those cannot drift; the colours
`WorldViewer` chooses inline for barriers, ruins and cities are defined in `legend.h` and used from
both places for the same reason. Note that `UiData::displayLayer` is an **index into
`uiDisplayLayers`**, not the enum value — use `getEnumFromIndex`, as both viewers do. `App` (`mapgen/mapgen.h`) owns the widgets and the `UiData`, which owns the generator;
widgets communicate only through that shared `UiData`. `App::setSize` hardcodes the layout
(350px control column, viewers tiled to the right). `App::newWorld()` generates the entire world
synchronously at startup. `mapgen/uiData.h` holds the display-layer/overlay/vector enums paired with
their UI label tables — add new debug visualizations there and in `WorldViewer::updateTexture()`.
Rendering goes through `imglib::SimpleImage` into a GL texture drawn by `ImageWidget`.

## Repository state

- **The working tree has been converted LF→CRLF** (git stores LF; the checkout is CRLF), which is
  most of the diff. Use `git diff --ignore-cr-at-eol` to see
  real changes; the substantive ones are the in-progress split of a monolithic `mapgen.cpp` into the
  untracked widget files.
- **Both submodules carry uncommitted local changes** that must be committed to their own repos
  before the pointers here can be bumped. `deps/imglib`: Hunter removed, `PNG::PNG`/`JPEG::JPEG`
  targets, missing `<cstdio>`/`<cstring>`/`<cstdlib>` includes, and two `cmake/Filesystem.cmake`
  fixes — it hardcoded the target name `voxigen`, and it generated its header into a directory named
  after the target, which collides with the `embed` executable on any platform where binaries have
  no suffix. `deps/fastnoise2`: a missing `<algorithm>` include.
- `mapgen/terrainViewer.{h,cpp}` exist but are not in `CMakeLists.txt` and are not built.
- `source/colorMap.cpp` and `source/randomcolor.cpp` are stale duplicates of the maintained copies in
  `source/utils/`; only the `utils/` versions are compiled. `source/biome.cpp` is empty.
- `include/worldgen/generator.h` and `include/worldgen/biome.h` are unported leftovers from the
  parent [voxigen](https://github.com/caseymcc/voxigen) project — they include nonexistent `voxigen/`
  headers and `biome.h` still declares `namespace voxigen`. Nothing includes them; they compile only
  because they are headers listed in the source group.

## Linux/GCC notes

The project builds clean on GCC (verified on 14.2, zero warnings from worldgen/mapgen). It was
MSVC-only until recently, so when adding code be aware of what previously slipped through:

- **Includes are case-sensitive here.** `FastNoise2`'s directory is `FastNoise/`, and mapgen's
  headers are lower-camel (`imguiWidget.h`, not `ImguiWidget.h`). MSVC did not care.
- Templates get parsed, not just instantiated. Dependent names need `typename`
  (`typename fs::Type`), and a non-dependent `value` inside a lambda in an uninstantiated template
  is still an error.
- MSVC's permissive mode hid missing `<limits>`, `<memory>`, `<cstdio>`, `<cstring>`, `<cstdlib>`
  includes throughout; add them explicitly.
- `glm::distance2` needs `GLM_ENABLE_EXPERIMENTAL` plus `<glm/gtx/norm.hpp>`.

`include/generic/io/fs.h` supplies `generic::io::fs<_FileIO>`, the file-access abstraction that
`EquiRectWorldGenerator`'s persistence path (`load`/`save`, `load/saveWorldOverview`,
`load/saveNormalize`) has always referenced but which was never ported over from voxigen. It is a
policy template over stdio semantics — `read`/`write` take `(buffer, elementSize, elementCount,
file)` and return the element count — with a `StdFileIO` backend built on `<cstdio>` +
`<std::filesystem>`. Those persistence methods are templates that nothing currently instantiates
(mapgen only calls `create()`), but they do compile and instantiate cleanly against `StdFileIO`.

## Conventions

Header guards `_worldgen_<name>_h_` / `_mapgen_<name>_h_`, `namespace worldgen` for the library and
`namespace mapgen` for the app, `m_` member prefix, Allman braces, no spaces around `=` or binary
operators. Library types that cross the DLL boundary are tagged `WORLDGEN_EXPORT`
(`include/worldgen/export.h`). There is no clang-format config — match the surrounding file.
