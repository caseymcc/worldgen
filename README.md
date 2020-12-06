[![discord](https://img.shields.io/discord/495955797872869376.svg?logo=discord "Discord")](https://discord.gg/BfceAsX)

# worldgen
World generator is a procedural world genereator using the [FastNoise2 library](https://github.com/Auburn/FastNoise2). It builds the world by generating tectonic plates and randomly assigning plate directions. From there weights from simple plate tectonics are used to adjust the height of the regions along with a noise height map. Weather is created based on weather cells and a bit of noise. These weather cells are use to generate wind speed, direction along with a moisture map. All of this combined with a global tempature map are used to detemine temperature.

- Documentation (WIP) - https://github.com/caseymcc/worldgen/wiki

## Lots of thanks go to
- [RedBlobGames](https://www.redblobgames.com/x/1843-planet-generation/)
- [Experilous](http://experilous.com/1/blog/post/procedural-planet-generation)

## Current Featues
- Simple Plate tectonics
- Weather Cells
- Temperture
- Rain shadow

![screenshot](https://github.com/caseymcc/worldgen/raw/master/resources/screenshot.png)

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
