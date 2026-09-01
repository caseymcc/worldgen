#ifndef _worldgen_worldgen_c_h_
#define _worldgen_worldgen_c_h_

/*
    A boundary with no C++ types crossing it.

    worldgen's own headers put std::string and std::vector in their public signatures, and the two
    standard libraries in common use do not agree about how those are laid out. A consumer built
    with a different toolchain than the library cannot link it - which is not a theoretical problem:
    an Unreal project embedding this had to build the library with the engine's own clang and libc++
    before it would link, and bake a file instead until it could.

    Everything here is C: plain structs, fixed-size arrays, integer handles. A caller copies what it
    wants out and never holds a pointer into the library's own containers. Compilers may disagree
    about std::string; nobody disagrees about float.
*/

#include "worldgen/export.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WG_NAME_MAX 48

typedef struct wgWorld wgWorld;

/* What a caller normally wants to know about a square of ground. Deliberately not the library's own
   cell, most of which is how the country was arrived at - plate ids, air currents, Strahler orders -
   rather than what it is. */
typedef struct
{
    float height;          /* 0 to 1, sea level at 0.5 */
    float elevation;       /* metres above sea level, 0 at or below it */
    float temperature;     /* annual mean, celsius */
    float moisture;        /* 0 to 1 */

    int32_t climate;       /* ClimateZone */
    int32_t coast;         /* CoastType */
    int32_t rock;          /* RockType */
    int32_t deposit;       /* DepositType */
    int32_t fuel;          /* FuelSource */
    int32_t water;         /* WaterBody */

    float riverDischarge;  /* millions of cubic metres a year */
    float riverWidth;      /* metres */
    float drainageArea;    /* square kilometres */

    uint8_t forest;
    uint8_t depositExposed;
    uint8_t road;
    uint8_t onTheBeatenTrack;

    /* the civilization layer */
    int32_t polity;        /* which state holds this ground, -1 for none */
    int32_t culturalRegion;
    int32_t settlement;    /* index into the settlement list, -1 for none */
    int32_t dominantSpecies;
    float habitability;
    float control;         /* how firmly the state holds it */
    float remoteness;      /* 0 on the trade network, 1 as far off it as this world goes */
    float traffic;         /* share of the world's trade passing through */
    int32_t barrier;       /* BarrierType */
    int32_t ruinKind;      /* RuinKind, 0 for none */
} wgCell;

typedef struct
{
    int32_t cellX;
    int32_t cellY;
    int32_t kind;          /* SettlementKind */
    int32_t species;
    int32_t polity;
    uint32_t population;
    float interest;
    char name[WG_NAME_MAX];
    char reason[128];
} wgSettlement;

typedef struct
{
    int32_t cellX;
    int32_t cellY;
    int32_t species;
    int32_t mountainCore;
    uint32_t cells;
    float area;            /* square kilometres */
    uint32_t marchCells;
    float meanControl;
    int32_t enclave;
    char name[WG_NAME_MAX];
} wgPolity;

typedef struct
{
    int32_t cellX;
    int32_t cellY;
    int32_t node;          /* NodeType */
    int32_t polity;
    int32_t parasite;
    float size;
    float water;
    float food;
    float river;
    char reason[192];
} wgCity;

typedef struct
{
    int32_t fromCellX, fromCellY;
    int32_t toCellX, toCellY;
    float cost;
    uint32_t length;
} wgRoad;

/* --- making and unmaking a world --- */

/* sizeX/sizeY/sizeZ are the world in blocks; pass 0 for the library's own defaults. */
WORLDGEN_EXPORT wgWorld *wgWorldCreate(int32_t seed, int32_t sizeX, int32_t sizeY, int32_t sizeZ);
WORLDGEN_EXPORT void wgWorldDestroy(wgWorld *world);

/* --- reading it --- */

WORLDGEN_EXPORT void wgInfluenceSize(const wgWorld *world, int32_t *x, int32_t *y);
WORLDGEN_EXPORT int32_t wgSeed(const wgWorld *world);

/* Every one of these copies into the caller's memory. Pass a null buffer to ask only for the
   count, which is how a caller sizes its own array first. Returns how many were written. */
WORLDGEN_EXPORT int32_t wgCellCount(const wgWorld *world);
WORLDGEN_EXPORT int32_t wgReadCells(const wgWorld *world, wgCell *out, int32_t max);
WORLDGEN_EXPORT int32_t wgReadCell(const wgWorld *world, int32_t x, int32_t y, wgCell *out);

WORLDGEN_EXPORT int32_t wgSettlementCount(const wgWorld *world);
WORLDGEN_EXPORT int32_t wgReadSettlements(const wgWorld *world, wgSettlement *out, int32_t max);

WORLDGEN_EXPORT int32_t wgPolityCount(const wgWorld *world);
WORLDGEN_EXPORT int32_t wgReadPolities(const wgWorld *world, wgPolity *out, int32_t max);

WORLDGEN_EXPORT int32_t wgCityCount(const wgWorld *world);
WORLDGEN_EXPORT int32_t wgReadCities(const wgWorld *world, wgCity *out, int32_t max);

WORLDGEN_EXPORT int32_t wgRoadCount(const wgWorld *world);
WORLDGEN_EXPORT int32_t wgReadRoads(const wgWorld *world, wgRoad *out, int32_t max);

/* Places to found a settlement of one's own, best first and kept apart. Asked for rather than
   stored, because which ground is good depends on who is asking. */
typedef struct
{
    int32_t cellX;
    int32_t cellY;
    float interest;
    int32_t polity;
    float nearestSettlement;
    uint32_t neighbours;
    uint32_t neighbourPopulation;
    char reason[128];
    char character[128];
} wgFoundingSite;

WORLDGEN_EXPORT int32_t wgFindFoundingSites(wgWorld *world, int32_t species, int32_t spacing,
    wgFoundingSite *out, int32_t max);

/* --- keeping it --- */
/*
    Plate tectonics, drainage and stream order are whole-world computations: nothing can answer
    "is there a river here" without having run the entire planet. So a world is remembered rather
    than re-derived - which also sidesteps the fact that the noise underneath is not promised to be
    bit-identical across processors, and a re-derived world is a disagreement waiting to happen
    between two machines that think they share one.

    The file carries a magic, a version, the size of the library's own cell, and a hash of the body.
    A file that says one world and holds another is worse than one that will not open.
*/
WORLDGEN_EXPORT int32_t wgWorldSave(const wgWorld *world, const char *path);
WORLDGEN_EXPORT wgWorld *wgWorldLoad(const char *path);

/* Non-zero if the last call failed, with a human-readable reason. */
WORLDGEN_EXPORT const char *wgLastError(void);

#ifdef __cplusplus
}
#endif

#endif /* _worldgen_worldgen_c_h_ */
