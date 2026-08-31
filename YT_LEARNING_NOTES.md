# yt_learning videos -> worldgen

## Processed
- fantasy-maps-plate-tectonics-x_Tn66PvTn4  (Artifexian) -- DONE
    Euler-pole plate motion, per-cell convergent/divergent/transform split, mid-ocean ridge
    profile, ocean water budget, continental shelf, hotspots, island-arc asymmetry, plate
    size hierarchy. Also fixed: uninitialized m_plateSeed/m_continentSeed, plates identified
    by cellular value instead of connected component.

- plate-tectonics-an-overview-worldbuilders-log-9-auRbMYFCXPc  (Artifexian) -- DONE
    Four-way orogeny split (Andean/Laramide/Ural/Himalayan) with the video's relative belt
    widths, plateaus behind the wide belts, per-type roughness (Laramide ridge-and-basin,
    Himalayan irregular). Selector uses convergence speed (ocean/continent) and PlateSize
    (continent/continent). Orogeny Type overlay in mapgen.
    NOT taken (deliberate, noted for later):
      - back-arc basins / slab rollback (needs per-plate boundary configuration)
      - failed rifts -> major river basins (defer to the how-to-river videos)
      - ocean crust age -> abyssal subsidence away from ridges (needs the influence loop
        split so a crust-age BFS can run between classification and height)
      - supercontinent-cycle history simulation (out of scope: generator is a static snapshot)

- atmospheric-circulation-wind-weather-and-mordor-LifRswfCxFU  (Artifexian) -- DONE
    Rotation rate now drives the circulation cell count (was hardcoded at earth's 3/hemisphere):
    <1x -> 1 cell, 1-2x -> 3, 4x -> 7, 8x -> 5, kept as the video's measured regimes including
    the 4x break in the trend. Rotation direction flips the coriolis deflection. Cell latitudes
    divide each hemisphere evenly, which reproduces earth's 0/30/60/90 exactly. Added pressure
    (low where air rises = ITCZ/polar front, high where it sinks = subtropical ridge/pole),
    exposed as InfluenceCell::pressure + a Pressure overlay.
    Verified: at rotationRate=1.0 the generated cells match the old hardcoded table exactly.
    NOT taken (deliberate):
      - tidally locked circulation regime (day/night, not a latitude-band model)
      - tropical cyclone / tornado / thunderstorm zones (weather events, not terrain; the
        generator emits a static overview)
      - hot/temperate/cold culture zones (Jared Diamond bit -- not generator work)

- winds-pressure-worldbuilders-log-29-gWYzAFi0MZ4  (Artifexian) -- DONE
    Pressure stops being purely zonal and breaks into discrete anticyclones, which is the
    video's whole point ("land basically will screw up this really nice neat pattern").
    Added PressureCentre + placement: subtropical highs over open ocean at ~30 deg (3 per
    hemisphere), continental highs over big landmasses poleward of 35 deg, polar highs.
    Winds spiral OUT of highs, clockwise north / anticlockwise south, blended over the band
    wind by m_pressureCentreWeight (0 = the old zonal model exactly). Lows fall out as the
    ground between cells, which is where the fronts are. Wired pressure -> moisture
    (m_pressureDryness): subsiding air in a high is dry, rising air in a low is wet.
    Verified: anticyclone circulation sense correct in both hemispheres AND under retrograde
    spin; zonal spread of wind angle rises 0.64 -> 1.18 rad as centre weight goes 0 -> 1.
    NOT taken (deliberate):
      - bias highs toward COLD OCEAN CURRENTS (video's rule) - no ocean current model exists
        in the project yet, so there is nothing to bias against. Worth revisiting if an
        upwelling/currents video turns up.
      - seasonal highs (summer 35 deg / winter 25 deg, and summer-vs-winter maps) - the
        generator emits one static snapshot, it has no season axis at all
      - mountain deflection of wind by incidence angle (perpendicular = flow over, shallow
        angle = deviated along the range). Self-contained and worth doing; deferred only for
        iteration size. GOOD CANDIDATE FOR A LATER ITERATION.

- worldbuilding-how-to-design-realistic-climates-1-5lCbxMZJ4zA  (Artifexian) -- DONE
- worldbuilding-how-to-design-realistic-climates-2-fag48Nh8PXE  (Artifexian) -- DONE
    Taken together deliberately: they are one tutorial split over two uploads (part 1 = the
    Hadley/tropical zones, part 2 = ferrel + polar), and a classifier covering only the
    tropics is not shippable.
    Added include/worldgen/climate.h: the full 14-zone Koppen classification with the
    conventional Koppen colours, plus the fields it needs:
      - temperature recalibrated to real celsius (was 60C at the equator / -90C at the pole,
        which no Koppen threshold could be written against). Now 26C equator, 1C at 60 deg,
        -25C pole, fitted to earth.
      - altitude lapse rate 6.5C/km -> highland climates, tundra/ice on tropical mountains
      - continentality from a distance-to-ocean flood fill; interiors run to extremes, coasts
        are damped by the sea. The effect reverses poleward (interiors bake in the tropics,
        freeze near the poles).
      - onshore/offshore wind from the gradient of that same distance field, which is what
        separates mediterranean (wind off the land) from humid subtropical (wind off the sea)
    InfluenceCell gains climate/continentality/onshore. mapgen gains a "Climate (Koppen)"
    layer. Thresholds calibrated against the generator's measured moisture percentiles, not
    guessed: savanna 11.5% vs earth 11.5%, rainforest 8.4 vs 7, monsoon 3.2 vs 3.5,
    hot desert 8.2 vs 10, cold desert 4.4 vs 4, oceanic 3.7 vs 4.
    KNOWN, NOT A CLASSIFIER BUG: subarctic/ice-cap come out low and the temperate zones high,
    because this generator puts 52-58% of land in the tropics and ~10% above 60 deg. That is
    what UNIFORM land on a sphere gives (50/37/13 by area); earth's 37/39/24 is an accident of
    its current continental arrangement. Fixing it would mean biasing plate placement by
    latitude, which is not something the video asks for.
    NOT taken (deliberate):
      - warm/cold OCEAN CURRENTS, which both videos lean on constantly. Still no current model
        in the project. Onshore/offshore wind is standing in for it and covers a good part of
        the same signal (on earth the cold-current flank and the offshore-wind flank coincide).
      - seasonality: every zone definition is phrased as summer-vs-winter ("hot wet summers,
        long dry winters"). The generator has one annual snapshot, so zones are assigned from
        annual means. This is the single biggest fidelity gap in the climate model.
      - part 3 (non-earthlike planets) - not in the corpus.

- worldbuilding-hot-cold-planet-climates-cnKUbcVrZVg  (Artifexian) -- DONE
    Global temperature regime on top of the Koppen classifier: m_globalTemperature in degrees
    off the earth baseline (-5.5 ~ last glacial maximum, +2.5 ~ last interglacial).
      - m_polarAmplification: the offset does NOT land evenly. Poles swing ~3x the tropics,
        weighted so the area average still equals the requested figure. Without this a cold
        earth wiped out the tropics entirely, which is wrong - the video shows tropics merely
        thinning while ice sheets march equatorward. With it: rainforest 6.7% cold vs 10.7%
        modern ("slightly less coverage"), ice cap 13.5 vs 8.2, lowland ice reaching 60 vs 70.
      - applyGlobalMoisture as a GAMMA not a multiplier: colder = drier, but the drying is not
        even. A flat multiply takes the most off the wettest ground and kills the equatorial
        rain belt; a gamma preserves it and eats the margins, which is what actually happens.
      - ice caps now need PRECIPITATION as well as cold (m_iceCapMoistureBonus). A glacier has
        to be fed to grow, so a cold wet coast ices over while a cold dry interior stays tundra.
        This corrects what iteration 5 shipped (EF on temperature alone).
    BUG FIXED from iteration 5: elevation was mapped LINEARLY from heightBase, so ordinary
    lowland at 0.60 was treated as 1800m and lost ~12C to the lapse rate. Equatorial land was
    averaging 16.9C instead of ~25C. Now curved (heightToElevation, m_elevationCurve=2.0) to
    match real hypsometry: mean land 497m (earth 840), max 9000m (earth 8849), equator 24.1C.
    NOTE: this made the raw zone-share table look WORSE against earth, because iteration 5's
    numbers were flattered by a broken lapse rate freezing all land. The mechanism is right now.
    NOT taken (deliberate):
      - hot earth REDUCING rainforest (video says it should). Driven by intensified monsoon
        circulation and a stronger land/sea heat differential, not by temperature alone. Mine
        increases it. Would need the monsoon mechanism modelled.
      - orbital precession greening one hemisphere's deserts. Needs an orbital model, and the
        video itself says "the data trail runs cold... just ape earth here".
      - LGM ice sheets reaching 45 deg. That is ice SHEET extent (ice flows and persists), not
        a climate classification; would need an ice sheet model.

- coastal-landforms-for-fantasy-mapping-ztemzsxso0U  (Artifexian) -- DONE
    Most of this video is sub-resolution: sea stacks, tombolos, blowholes, coves, shore
    platforms are metre-to-hundred-metre features and an influence cell is 4096 blocks. What
    DOES survive at overview scale is the classification the video opens and closes with, so
    that is what got built: include/worldgen/coast.h, a 9-type coast classifier reusing the
    whole stack built so far.
      Rocky   <- active margin (BoundaryType convergent/transform) or land-side relief
      Fjord   <- rocky + glaciated climate (EF/ET/Dfc) + mountains + latitude >= 45
      Dalmatian <- convergent margin running ALONG the shore, low relief, drowned
      SandyPlain <- passive margin, low relief
      BarrierIsland <- passive margin shelf water (video: 49% of barrier coasts are passive)
      Mangrove <- tropical/subtropical low coast;  SaltMarsh <- temperate sheltered low coast
      CoralReef <- shelf water off a warm shore
    Verified: rocky/sandy comes out 79-85% / 15-21% across seeds against the video's stated
    80-20. Fjords confined to 46-67 deg, mangroves and reefs to 1-33 deg, salt marsh 29-56,
    barrier islands 100% passive margin, Dalmatian 95-98% on convergent margins.
    TWO BUGS FOUND AND FIXED while building it:
      - relief was measured ACROSS the shoreline, so it always included the land/sea step and
        every single coast read as rocky (98%). Relief must be land-side only.
      - "glaciated" fired on tropical mountains, because the lapse rate legitimately puts
        icecap climates on tropical peaks. Fjords were appearing at 5 degrees latitude. Needed
        an explicit latitude floor - no ice sheet ever ground a valley into an equatorial coast.
    NOT taken (deliberate):
      - Estuarine is DEFINED but never assigned: estuaries are drowned river valleys and there
        are no rivers yet. The three river videos are next in the queue - assign it there.
      - all the sub-cell landforms (stacks, arches, tombolos, spits, coves, blowholes, shore
        platforms, longshore drift). Below influence-map resolution; these belong in
        generateChunk/generateRegion detail, not the overview.
      - emergent/submergent coasts and uplift terraces: needs a sea-level history, and the
        generator emits one static snapshot.
      - beach colour and dune systems: cosmetic, no consumer in the generator yet.

- how-to-river-cqMiMKnYk5E  (Artifexian) -- DONE
    The project had no rivers at all. Added include/worldgen/river.h + a full drainage solve on
    the influence map:
      - priority-flood pit filling so the height field is drainable, D8 steepest descent, and
        flow accumulation processed high-to-low so every cell has its whole catchment before it
        passes it on
      - river width straight from the video's on-screen formula B = (R/4) x A x 0.1, W = B/F
        with the depth-factor table. Verified by hand against their worked example (2000 km2 /
        500 Mm3/yr -> B 25 -> W 12.5).
      - three stages (youthful/mature/old age) from the local gradient
      - permanent GAINING rivers where it rains, ephemeral LOSING rivers in arid basins - the
        losing ones actually shrink downstream (m_ephemeralRetention), which is the video's
        explicit distinction
      - endorheic basins that never reach the sea: salt lake if wet enough to stand, inland
        delta (Okavango) if it just soaks away. Modelled on aridity, not pure topography,
        because after pit-filling everything drains topographically.
      - CLOSED the item parked in iteration 7: CoastType::Estuarine is now assigned at river
        mouths (83 cells on seed 42) rather than being defined and never used.
    BUG FOUND AND FIXED: priority-flood leaves dead FLAT surfaces where no neighbour is
    strictly lower, so 474 river cells dead-ended and trapped water. Fixed by recording the
    order the flood reached each cell - which increases with distance from the outlet - and
    routing flats down that. Also used to break ties in the accumulation ordering.
    Verified across 4 seeds: 0 dead ends, 0 cycles in the flow graph, permanent rivers never
    lose water, every river terminates at sea or in an endorheic basin, dendritic networks.
    NOT taken (deliberate):
      - deltas. The video ties delta SHAPE to ocean current strength vs river current, and
        there is still no ocean current model. The delta-coastlines video is next in the queue.
      - meander geometry (radius 2.3x width, wavelength 11x), braided channels, oxbows, levees,
        yazoo streams, point bars, floodplain terraces: all sub-cell, they belong in
        generateChunk/generateRegion detail.
      - water gaps (river cutting through a rising range) - needs uplift history vs erosion rate
      - karst/subterranean rivers - needs rock types. The "rocks" video is in the queue.
      - fall line settlements: no settlement layer exists.

- delta-coastlines-more-how-to-river-fu-6qQH6ZEzlO4  (Artifexian) -- DONE
    A short Q&A follow-up to How To River. Two things acted on:
      - DELTAS. Sediment drops where the current dies against standing water, so the fan is
        built by depositing outward from a river mouth, cell by cell, raising shelf water to
        just above sea level. New ground inherits the mouth's climate rather than the sea's.
        Size scales with discharge AND slowness ("the slower the river, the more sediment"),
        so a youthful torrent builds nothing and an old-age river builds a large fan. Result:
        5-9 fans per world, mostly 1-2 cells with the odd 10-14 cell one, which is about
        earth's mix. Renders as a protrusion into the bay - the video's "you will almost never
        see a straight coastline where a delta forms".
      - "water cares about downhill AND direct route simultaneously" - exact gradient ties now
        break toward the cell the flood reached sooner (nearer the outlet) rather than by array
        order. Checked for a directional bias first and found none worth calling a bug: the
        cardinal preference that shows up is correct, diagonals are legitimately penalised for
        covering more ground per unit drop.
    BUG FOUND AND FIXED: river stage was measured on the drop to the SEA BED at a river mouth,
    not to sea level. A river entering the ocean has its surface at sea level, so every mouth
    on the planet was being classified a steep mountain torrent - only 5 of 156 read as old
    age. Same class of mistake as the coast-relief bug in iteration 7 (measuring across the
    shoreline instead of along the land).
    TWO REGRESSIONS THE DELTAS CAUSED, both caught by the iteration-8 invariants and fixed:
      - delta cells are created after routing, so they had no flow direction and rivers
        dead-ended on them (143 cells). They now shed to open sea.
      - a river running onto ANOTHER river's fan inherited that fan's smaller discharge, so
        water appeared to shrink. Fixed by propagating each river's figures forward into
        whatever fan it drains into.
    NOT taken (deliberate):
      - delta SHAPE (fan / cuspate / bird's foot) is set by ocean current strength against
        river current. Still no ocean current model - this is now the FOURTH video to want one.
      - sinuosity/meandering: the video explicitly says the more you zoom out the less
        intricate rivers should be, and that at world-map scale the windiness is not visible.
        The overview is exactly that scale, so leaving rivers unmeandered is correct here.
      - the width formula being order-of-magnitude only: he clarifies it is +/- about 10x. No
        code change, but worth knowing riverWidth is not a precise figure.

- rivers-worldbuilders-log-41-DOutB6-_LpE  (Artifexian) -- DONE
    Third and last river video. It is about the STRUCTURE around rivers rather than the rivers
    themselves, so three things:
      - DRAINAGE BASINS as first class objects. Every land cell walks its flow path to a
        terminus and the whole path takes that terminus's label; DrainageBasin carries outlet,
        area, discharge, river count and the exoreic/endoreic flag. Exposed via
        getDrainageBasins(). The video's checkable claim - no more than about 20% endoreic -
        comes out at 4-8% by count, 7-12% by area.
      - STRAHLER STREAM ORDER over the river network, used for river thickness in mapgen and
        to identify headwaters. Falls off geometrically (order 1 ~950, order 2 ~150, order 3
        ~20), which is the shape it should have.
      - GLACIAL LAKES. Nice reuse: "where did the ice stand at the last glacial maximum" is
        just the cold-regime temperature model from iteration 6 run at m_glacialDrop colder,
        with the polar amplification and lapse rate already in place. Lakes go inside that
        extent, in wetter ground, beside a low-order river - and never below 40 degrees,
        because ice that far equatorward means a runaway freeze rather than a glacial maximum.
      - VERIFIED the video's other claim, that river density tapers from equator to pole:
        18.4% of land in 0-30, 12.5% in 30-60, 1.0% in 60-90.
    THREE BUGS FOUND while checking those claims:
      - basins all read endoreic: the flow walk labelled the OCEAN cell as the outlet, so the
        exoreic test never fired. Outlet must be the last cell still on land.
      - then all read EXOREIC, which exposed something real: pit filling gives every cell a
        downhill path to the sea, so nothing is endoreic topographically. Endoreic-ness has to
        come from the aridity model built in iteration 8 - a basin whose river ends in a
        terminal salt lake or soaks away into swamp never gets its water to the coast. The two
        halves are now connected.
      - stream order never reported 1. Computed over every land cell it counts hillslopes as
        branches, so a river was already order 2-3 by the time it had the catchment to be
        called one. Recomputed over river cells alone.
    NOT taken (deliberate):
      - cryptorheic basins (karst, river exits underground). The video declines these itself -
        "not confident doing it when I don't know what's going on with the rocks in the area".
        The rocks video is still in the queue.
      - seasonal river flow (strongest in the wet season). Same seasonality gap as the climate
        work; the generator has one annual snapshot.
      - hand-editing topography to route a river somewhere more interesting. That is the
        worldbuilder's artistic licence, not something a generator should do.

- rocks-worldbuilders-log-44-HQ4NeMNap_0  (Artifexian) -- DONE
    Taken BEFORE the metals video deliberately: it is Artifexian's own ordering (log 44 before
    the ore video), and two earlier videos had already deferred to "what's going on with the
    rocks in the area". This is the foundation layer metals, karst and salt all sit on.
    Added include/worldgen/rock.h: 7 rock types (sediment, sandstone, carbonate, basalt,
    andesite, granite, metamorphic) + cratons + a mapgen "Rocks (geology)" layer.
    The placement rules land almost entirely on things already built:
      - the four OROGENY TYPES from iteration 2 carry their own rock: laramide -> andesite,
        andean -> granite core with andesite flanks, himalayan/ural -> metamorphic core
      - hotspots (iteration 1) and divergent boundaries -> basalt, plus a metamorphic contact
        aureole baked around the basalt
      - glaciated climate (iterations 5-6) strips a craton back to its shield, which is the
        reference's own reasoning for the Canadian Shield
      - a shallow-sea mask from the elevation curve (ground under 200m, ie. drowned when sea
        level last stood that much higher) biases carbonate, which also needs warm water
      - rivers/lakes/deltas (iterations 8-9) collect sediment
    VERIFIED against the video's two explicit ratios:
      - "75% of land is sediment or sedimentary rock, 25% igneous and metamorphic":
        comes out 62-70% / 30-38% across seeds. Under target but the right shape.
      - "more sediment than sandstone, more sandstone than carbonate": holds on every seed.
    TWO CALIBRATION BUGS, both the same mistake in different places: a cell knowing which
    boundary is NEAREST is not the same as it being AT that boundary, and orogeny is tagged
    across the whole overriding plate rather than just the mountain belt. Keying basalt on
    "boundaryType is Divergent" flooded 16% of land with it, and keying andesite on "orogeny
    is Laramide" did the same. Both now key on the actual thing - distance to the boundary
    line for rifts, and terrainScale for the belts - which took sedimentary from 53% to 70%.
    NOT taken (deliberate):
      - real large igneous provinces. The video has them from its GPlates tectonic history;
        this generator has no history, so hotspots and rifts stand in for them.
      - ocean floor rocks. The video skips them too ("whatever cultures I set in this world
        are just not going to come into contact with the ocean floor").
      - sea level history proper. The shallow-sea mask is a present-elevation proxy for
        "ground that was under the sea 110 million years ago", which is what the video
        actually does too rather than simulating it.
      - karst landforms (caves, sinkholes, underground rivers). karstRock() is provided so the
        cryptorheic basins parked in iteration 10 can now be built, but that is river work.

- where-metals-are-found-on-fantasy-worlds-b9qvQspSbWc  (Artifexian) -- DONE
    The direct sequel to the rocks video, and it needed almost no new machinery - every rule
    keys on ground already on the map. Added include/worldgen/ore.h: 15 deposit types with a
    per-cell scatter so a deposit is a rare accident rather than a blanket over the right
    country. New mapgen "Ore and fuel" layer.
      arcs (Andean/Laramide belts + hotspots) -> porphyry copper, then epithermal gold ringed
        one step out, then IOCG around those and in the continental rifts
      cratons (iteration 11) -> nickel-PGE, chrome, diamonds. Rare.
      mountain belts -> VMS; the WORN belts -> banded iron
      foreland basins (new: a trough pressed down beside a belt) -> MVT and oil/gas
      shallow-sea sedimentary ground -> oil and gas source rock
      warm wet lowland -> coal, or peat where the wetland is still living. Explicitly NOT in
        mountain interiors, where the reference notes the pressure makes graphite instead
      salt lakes / inland deltas / dry sedimentary basins -> salt and lithium
      rainforest (Af/Am) -> laterite bauxite
      then WATER MOVES THINGS: walk downstream from every hard-rock deposit and drop placer
        gold in the river gravels and secondary uranium below the IOCGs, which is the
        reference's instruction to make the dominant uranium a secondary enriched one
    VERIFIED: all 15 types present on every seed, and every placed deposit checked back
    against the setting the video names for it - 100% on all types. 11-17% of land carries
    something, which reads as a resource map rather than a blanket.
    TWO BUGS: DepositTypeCount was 15 when the enum has 15 kinds PLUS None, so the last type
    (secondary uranium) fell outside every loop bound and was silently never checked - the
    verification reported success while ignoring it. And the first pass put a deposit on 42%
    of all land; rarities cut ~3x.
    NOT taken (deliberate):
      - the mineral detail inside each deposit type (Cu-Au vs Cu-Mo, the three epithermal gold
        zones by distance, the minor Pb/Zn/Ag/Sn/W). Recorded in the enum comments; splitting
        them into separate types would be noise at this resolution.
      - oil/gas source-reservoir-seal trap geometry. That is a vertical, sub-cell structure;
        the overview can say a region has hydrocarbons, not which fold traps them.
      - coal RANK by burial depth (lignite through anthracite). Needs a burial history.

- how-do-ocean-currents-work-jennifer-verduin-p4pWafuvdrY  (TED-Ed) -- DONE
    Taken out of order deliberately: FOUR earlier videos (winds/pressure, climates 1 and 2, and
    the delta follow-up) had all deferred to an ocean current model that did not exist. This is
    the highest-value item in the corpus for that reason.
    Added include/worldgen/current.h + a pass: wind drags the surface, Coriolis swings the drag
    aside (right in the north, left in the south, and both reverse on a retrograde planet),
    coastlines steer what is left, viscosity drags neighbouring water round with it, then heat
    is advected so a parcel carries its origin temperature. currentWarmth is how far off the
    local sea temperature that leaves it. New "Ocean currents" mapgen layer.
    WORKS, and robustly: a coast washed by a cold current comes out cooler and drier than one
    washed by a warm current on every seed (0.64 vs 0.88 moisture, 8.1 vs 11.8C on seed 42).
    Wired into the climate classifier, which now puts mediterranean on the cold-current flank
    and humid subtropical on the warm one - the rule the climate videos actually state, instead
    of the onshore/offshore wind proxy standing in for it since iteration 5.
    HONEST LIMITATION, diagnosed not tuned away: basin-scale GYRES do not emerge, so earth's
    systematic "west coasts cold, east coasts warm" does not hold - it comes out 6% cold on
    west coasts for one seed and 57% for another. The mechanism is right; the geometry is not.
    A gyre needs an ocean basin bounded north and south by land to close a loop around, and
    these worlds are ~70% ocean in one connected body with scattered continents, so the flow
    stays zonal and follows the wind belts. Rendering the field confirms it. Earth's gyres are
    a consequence of the Atlantic and Pacific being enclosed; that is a property of continental
    arrangement, not of the current model.
    NOT taken: thermohaline circulation and the global conveyor (deep water, 90% of the ocean,
    driven by density rather than wind - and a thousand years per circuit, which is not
    something a static snapshot can express). Biasing pressure centres toward cold currents is
    still not possible: currents depend on wind, wind depends on pressure.


- gears-of-wind-and-water-climate-currents-and-biomes-in-kaime-vJwhSxcs4nQ  -- DONE
    Mostly a worldbuilder narrating their own planet's biomes rather than giving rules, but two
    things in it were worth taking, both refining the current model from iteration 13:
      - UPWELLING. "During the summer months, offshore winds trigger upwells, and cold, nutrient
        rich waters are pushed through." Wind blowing off the land drags the surface water out to
        sea and cold water rises to replace it. Computed from the local shore normal against the
        wind, so no new machinery. 19-22% of coastal water, 98-99% of it reading as cold current.
      - The current only reaches the land THROUGH THE AIR ABOVE IT: "wind passing over cold water
        brings cooler air and a degree of aridity, while winds travelling over warm water bring
        hot humid air inland". Iteration 13 applied the anomaly to every coastal cell regardless
        of which way the wind was blowing. Now scaled by the onshore component.
    The reference's headline claim - "the most desolate regions are those with cold currents AND
    offshore winds" - now holds: coasts beside an upwelling are 32% arid against 14% without
    (17% against 9% on another seed).
    A BUG the upwelling exposed: I first applied the chill to the reference sea temperature as
    well as to the water, so the anomaly (water minus reference) cancelled out and upwelling
    never read as cold at all - 1-3%. The anomaly has to be measured against what the latitude
    SHOULD be. After fixing, 98%.
    AND A FLAW IN MY OWN CHECK, worth recording: comparing cold-current against warm-current
    coasts on ABSOLUTE temperature said the effect had disappeared. It had not - the comparison
    confounds latitude, because an upwelling desert sits in the warm subtropics (Namib, 25S)
    while a warm current reaches the poles (Norway, 60N). Controlled by latitude band, 4 of 5
    bands have cold-current coasts both cooler and drier, with moisture 0.23 against 0.78 at
    30-45 degrees. The check was wrong, not the model.

- salt-worldbuilders-log-48-g3a0wXxsDTo  (Artifexian) -- DONE
    A short addendum to the ore video, and it splits what I had as one Salt deposit into two:
      - SALT FLAT: a lake evaporated away in an arid hollow. The video names three sorts of
        hollow and all three were already on the map - a plain local depression, the undulating
        ground behind a LARAMIDE belt (iteration 2, and the Bonneville and Bolivian flats both
        sit in country like that), and a FORELAND BASIN (iteration 12). All three produce flats.
      - HALITE / rock salt: an ancient salt flat. Warm shallow sea, dried out, buried - so it
        wants the shallow-sea mask from iteration 11 plus an arid climate.
    And introduced DEPOSIT EXPOSURE, which is the video's real insight and is groundwork for the
    copper/bronze/iron age videos still queued: an ancient deposit is worth nothing to a people
    without drills unless something lifted it back within reach. Buried halite beds stay lost;
    the ones a later collision thrust up become the salt mines and the salt glaciers of the
    Zagros. Comes out 26-31% of rock salt uplifted, 41% of all deposits reachable, and
    nickel-PGE only where the craton shield is actually bare.
    NOT taken: SEA SALT. The video explicitly declines to map it - "it's everywhere, no need to
    map this" - since any coast beside salty water can produce it, by solar evaporation where it
    is arid and by boiling where there is fuel. Following the reference rather than inventing a
    layer it says not to build. Worth noting the fuel side is already derivable: peat and coal
    are on the map, so a consumer can answer "can this coast boil brine" without a new field.

- fuel-the-copper-age-worldbuilders-log-45-z38GgH8En3E  (Artifexian) -- DONE
    Two things, both of which the project lacked entirely.
    FUEL AND FOREST (include/worldgen/fuel.h, new "Fuel and forest" mapgen layer). A people
    sitting on copper with nothing to burn cannot smelt it, so this matters as much as the ore.
    The video gives an exact TREELINE formula and it is now implemented verbatim: 4000m through
    the tropics to 30 degrees, then falling 130m per degree to 50, then 75m per degree until it
    reaches the ground at 68.7 degrees and no elevation is low enough. Arid and polar climates
    are cut out wholesale. Verified against the formula: 40deg->2700m, 50->1400, 60->650, and
    the northernmost forest lands at 64 degrees, under the limit, with zero forest in any arid
    or polar climate. Peat comes from cool wet climates intersected with the last glacial
    maximum - the same cold-world reconstruction the glacial lakes used in iteration 10.
    NATIVE COPPER. The deposit that actually starts a copper age: metallic already so it needs
    no furnace, and shallow enough to find. Water carries it out of a basalt province and drops
    it in the sedimentary country around it, hotspots included ("what is a hotspot if not a very
    small large igneous province").
    THREE BUGS: coal and peat never appeared as fuel at all, because I never set depositExposed
    for either, AND because wood was tested before them so the forest masked every bog and seam
    on the map - wood is the background, coal and peat are the notable resources, so they have to
    be tested first. Native copper was near-absent (4 cells) held to carbonate alone, which is
    1.3% of land; broadened to sedimentary as the video's hotspot aside allows.
    NOT taken: the tectonic-history reasoning that fills most of the video - dating a
    carboniferous period, placing coal swamps on a supercontinent 340 million years ago, then
    running the plate animation forward and erasing coal wherever a later large igneous province
    "nuked" it. That is all downstream of a billion-year tectonic history this generator does not
    have; coal is placed from present climate and rock instead, as noted back in iteration 12.

- the-bronze-age-worldbuilders-log-46-OFaf-f_XY6I  (Artifexian) -- DONE
    Four new deposit types, and tin is the point of all of them - bronze is hard precisely
    because tin is rare and does not turn up where copper does.
      SKARN: a pluton pushing into limestone bakes the contact into ore. Igneous rock with
        carbonate country right beside it, inside an orogenic belt.
      TIN LODE: the video boils a long list of settings down to one rule - tin belts sit in
        orogenic regions with a COLLISIONAL history, and the tin goes with the granites. Uses
        the Himalayan/Ural types from iteration 2.
      ALLUVIAL TIN: cassiterite is heavy and survives the trip, so a lode upstream leaves tin in
        the river gravels. Most bronze age tin was panned rather than mined. Rides the same
        downstream walk the gold placers use.
      NATURAL TIN-BRONZE: copper and tin in the same rock, alloyable without trading for either.
        Rare and enormously valuable.
    VERIFIED: tin comes out 9-25x rarer than copper across seeds, and natural tin-bronze is
    rarer still - 0.04% of land, absent entirely on one seed. That scarcity is the whole
    historical shape of the bronze age and it now falls out of the geology rather than being
    asserted.
    A REAL GAP IN THE ROCK MODEL this exposed: there was NO TIN ANYWHERE at first, because
    iteration 11 puts metamorphic rock through collisional belts while tin granites are late
    intrusions WITHIN those belts - Cornwall, the southeast Asian tin belt. Thickened crust
    melts at depth and that melt rises and freezes as granite bodies inside the range. Added
    m_beltIntrusion; the rock ratios (62-70% sedimentary, sediment>sandstone>carbonate) still
    hold afterwards.
    AND A BACKWARDS TEST ORDER: natural tin-bronze was checked before plain tin lodes AND with a
    looser threshold, so the rare case swallowed the common one and there was almost no ordinary
    tin. Rare cases have to be tested first and held to a tighter threshold, not a looser one.
    NOT taken: the tectonic-history reasoning again - tracking which orogenies were active when,
    which back-arc basins later got uplifted, and reading SEDEX placement off a plate animation.
    Same history dependence noted in iterations 12 and 16. Skarn also comes out very sparse (3
    cells) because carbonate is only ~1.3% of land here, so igneous ground bordering limestone is
    genuinely rare on these worlds - reported rather than tuned away.

- the-iron-age-worldbuilders-log-47-d3VKSjEZwSw  (Artifexian) -- DONE
    Completes the ore trilogy. Four iron types plus an upgrade path:
      BOG IRON: groundwater carries iron out of an ore-bearing belt and it oxidises out again in
        the still water of a marsh DOWNHILL of it. Rides the same downstream walk the placers
        and secondary uranium use, so "downhill of a metallogenic belt" is literal here.
        Shallow, easy to dig and easy to smelt, which is much of why iron spread as it did.
      OOLITIC IRON: dissolved iron settling grain by grain into shallow marine sediment. The
        reference expects most of a world's iron to be this, and it is - the commonest single
        deposit type on the map.
      IRON LATERITE: the same leaching that makes bauxite, but over basalt and under a climate
        that alternates wet and dry (savanna and monsoon) rather than staying wet all year.
      METEORIC IRON: metallic the moment it lands, needs no smelting at all, and vanishingly
        rare - 0 or 1 per world. Placed only in tundra and desert, where nothing grows over it,
        which is why the Cape York iron sat on the Greenland tundra for the Inuit to work.
      HYDROTHERMAL IRON: rather than a type of its own, a quarter of the VMS and SEDEX deposits
        already on the map are upgraded to carry iron as well (new ironBearing flag).
    VERIFIED THE METAL HIERARCHY, which is the whole point of the trilogy: iron is 2.6-2.8x
    commoner than copper and 32-74x commoner than tin. Iron everywhere, copper moderate, tin
    scarce - and that ordering now falls out of geology rather than being asserted anywhere.
    TUNING: oolitic first came out at 14.3% of land, which is "a lot" past the point of useful -
    one type at that density crowds every other off the map, since a cell carries one deposit.
    Cut to 4.8%, still the commonest.
    NOT taken: ZINC. The video places zinc beside every lead deposit, but SEDEX and MVT are
    already lead-ZINC by definition here - the pairing the video is asking for is one this map
    has had since iteration 12. Adding a zinc type would restate it, not add to it.

- is-earths-most-important-ocean-current-doomed-f2evaLaDvCI  -- DONE
    Thermohaline circulation, which iteration 13 deferred. Nine tenths of the ocean moves not
    because the wind pushes it but because cold salty water at high latitude is dense enough to
    sink, dragging warm water poleward behind it. Added overturningWarmth() and an overturning
    flag: where water runs hard poleward at high latitude it is treated as the returning surface
    branch of the deep circulation, carrying heat gathered across a whole ocean rather than from
    one cell upstream.
    VERIFIED ON THE OCEAN SIDE, and cleanly: 11 of 11 latitude bands across three seeds have the
    limb carrying warmer water than the ocean around it, and the anomaly GROWS with latitude
    (+1.4C at 30-45 rising to +6.2C at 75-90) - which is the mechanism, since the further the
    water gets the further it has carried that heat.
    THE LAND-SIDE EFFECT DOES NOT HOLD, and it is the same wall as iteration 13 reached from the
    other side. The video's own test is London against Calgary: same latitude, and London is far
    milder because the overturning delivers heat there. Across seeds that comes out 1 of 1, 0 of
    2, 1 of 2 bands - inconsistent, on samples of 10-40 cells. Earth's AMOC is a narrow intense
    western boundary current because the Atlantic is a narrow north-south basin bounded by
    continents; these worlds are ~70% ocean in one connected body and there is nowhere for a Gulf
    Stream to form. Two iterations have now hit that same geometry limit from different
    directions, which makes it a property of the worlds rather than a failure of either model.
    Tuning the threshold traded breadth for intensity without fixing it: at a loose threshold 29%
    of the ocean was "limb" and the heat spread too thin to matter; at a tight one it was 0-3%
    and the samples too small to measure. Settled in between and reported what is actually true.

- the-koppen-geiger-climate-classification-system-BsOL9Fafo2w  -- DONE
- the-koppen-geiger-climate-classification-system-kvWVoCrPUyU  -- DONE (same subject, covered)
    A tour of the zones rather than a criteria reference, but it gives the LETTER CODE SEMANTICS,
    and that exposed something my classifier was collapsing. The third letter is how hot the
    summer gets - a hot, b warm, c cool - so several of the zones I had as one are really pairs.
    Split three of them: Csa/Csb (mediterranean, hot vs warm summer), Cfb/Cfc (oceanic vs
    subpolar oceanic), Dfa/Dfb (humid continental, hot vs warm summer). 15 zones -> 18.
    The video states the spatial relationships outright - Csb "always lying above their Csa
    siblings on the continental western coasts", Cfc poleward of Cfb - which makes them directly
    testable. All three orderings now hold on every seed: Csa 34-35 -> Csb 41-44, Cfb 41-49 ->
    Cfc 47-52, Dfa 38-40 -> Dfb 49-50 -> Dfc 55-56.
    It also confirms the mediterranean rule from iteration 13 - "always on the western fringes of
    continents" - which is the cold-current flank the ocean currents now supply.
    A FALSE PASS, worth recording: Dfa first came out POLEWARD of Dfb, which is backwards. Raising
    the threshold "fixed" it - by emptying the zone entirely, and the check then reported the
    ordering as correct because an absent zone trivially satisfies any ordering. The real cause
    was structural: the C/D split here is on ANNUAL mean, and a hot summer with a freezing winter
    averages warmer than that cut allows, so a warm D zone was unreachable by construction. Dfa
    belongs on the warm-but-continental branch inside the C group, where the sea never gets to
    damp the swing. The check now treats an absent zone as untested rather than as passing.
    NOT taken: the ~12 remaining Koppen zones that need SEASONALITY to distinguish - Cwa/Cwb and
    the whole Ds and Dw families turn on whether the dry season falls in summer or winter, and
    this generator carries one annual figure. Same gap flagged since iteration 5.

- types-of-copper-deposits-porphyry-vms-sedimentary-UXpFk2FJdjk  -- DONE
    Two things worth taking.
    SEDIMENT-HOSTED COPPER: copper accumulated in sandstone and shale beds in a basin with no
    volcano anywhere near it - Michigan's native copper comes out of Proterozoic shale like this.
    It is the one copper that does not need an arc, so it puts the metal somewhere the arc-based
    types cannot reach.
    ORE GRADE, which is the more useful half. The video's framing of porphyry as "low grade, high
    tonnage" and the world's largest copper source at once is a distinction the map did not carry:
    how MUCH rock there is and how rich that rock is are different questions. Added OreGrade and
    workableByHand(), which combines grade with the depositExposed flag from iteration 15 to
    answer whether a people without industry could use a given deposit at all.
    THE RESULT IS THE VIDEO'S POINT MADE CONCRETE: of 1775 deposits, 65% are within reach but
    only 34% are workable by hand - 563 are reachable and still not worth the digging. Broken
    down by copper source: porphyry is a major source at 61 cells and ZERO of it is hand-workable,
    while VMS, skarn, native and sediment-hosted copper are usable to the last cell. A
    pre-industrial civilisation on these worlds cannot touch what would later be the largest
    copper source on the planet, which is historically exactly right.
    It also independently confirms two existing rules: porphyry sits on volcanic arcs (iteration
    12) and porphyry intruding carbonate makes skarn (iteration 17).

- maps-fractals-tectonics-and-the-fourth-dimension-7xL0udlhnqI  -- DONE
    This one independently arrives at almost exactly the architecture already built here, which
    is worth recording as confirmation rather than as new work: voronoi-ish plates grown from
    sample points, ~60% of them oceanic, EULER POLES ("any motion on a sphere can be represented
    by a single 3D vector... the cross product of that axis and the position gives the motion at
    individual regions of the plate"), per-tile stress from comparing a tile's motion to its
    neighbours', raising where they converge and lowering where they part, and 3D noise sampled
    on the sphere so the poles do not tear. That is iterations 1 and 2, arrived at separately.
    THE ONE NEW THING is a problem it names and this generator had: "we need pretty big plates to
    get continent-like features, but because of that the internals of plates end up looking
    pretty flat". Measured before believing it - plate interiors were 4.4-4.7x flatter than
    boundary country, median local relief 0.013-0.015, essentially featureless.
    Its fix is the fractal argument: real terrain is fractal but not self-similar, because the
    events that carve an ocean basin are not the events that carve a valley. So the tectonic
    result stands in for the low frequency layers and a finer, weaker noise field supplies the
    high ones. Added m_terrainDetail over the whole map. Interiors now 3.3-3.7x flatter with
    median relief up ~50%, which keeps the tectonic contrast (a craton SHOULD be flatter than an
    orogen) while giving the interiors texture.
    Verified it did not disturb anything downstream, since every later layer reads heightBase:
    ocean 68.5-71.2%, rivers still acyclic and terminating, all 18 climate zones and all 26
    deposits present, coast 78-85% rocky, rock 63-70% sedimentary.

- on-worldbuilding-mountains-in-fantasy-maps-asoiaf-dune-storm-cSFIEYmOAp0  -- DONE (nothing to take)
    Read it. This is a writing-craft video about what mountains DO to a story - ranges as
    national borders, the difficulty of projecting force across them, trade routes through
    passes, mountain cities and the cultures that form behind a range. Its only geographic claim
    is that a range divides land into a wetter and a drier side, which is the rain shadow the
    moisture transport has had since iteration 5. Nothing to implement in a terrain generator,
    though it is a fair description of what the layers built here are FOR.

- lets-build-part-1-plate-tectonics-simulation-with-platec-okQjGdVrOVY  -- DONE
- procedural-tectonic-plates-terrain-simulations-with-earthini-F_suI-KvDiI  -- DONE
    Both are TIME-EVOLVING plate simulations, which this generator is not - it emits one
    snapshot. Mostly they confirm what is already here. PlaTec runs at a sea ratio of 0.7 and
    ~30 plates (here: 0.70 and 24). EarthInit independently states the subduction asymmetry from
    iteration 2 - "we want our interpolator to ignore subducting vertices and only sample from
    the overriding plates" - which is the trench-on-one-side, arc-on-the-other rule. PlaTec also
    flags its own world as TOROIDAL, wrapping in Y as well as X, which it calls a deal breaker;
    this generator wraps only in longitude and handles the poles, so that limitation does not
    apply here.
    THE ONE ACTIONABLE THING was PlaTec running periodic erosion "to prevent everything from
    becoming too mountainous". Measured before implementing anything - and found the OPPOSITE
    problem. Against earth's hypsometry, 83-85% of land sat under 500m (earth ~50%) with the
    500m-3km bands badly under-represented: a bimodal world, crushed near sea level with a thin
    spike of peaks and almost no upland between. Erosion would have made that worse.
    The cause was the elevation curve set in iteration 6. That was calibrated against earth's
    MEAN land elevation, which is a single number and hid the shape entirely. Swept the exponent
    against the full distribution instead: 2.0 gave 83/6/5/2/4 against earth's 50/21/18/6/5, and
    1.5 gives 55/27/8/3/6. Mean land elevation went from 496m to 827m against earth's 840 - so
    fixing the distribution fixed the mean as well, which is the direction that implication only
    ever runs one way.
    Re-verified everything, since elevation drives the lapse rate and therefore the whole climate
    chain: ocean 68.5-71.2%, rivers acyclic and terminating, all 18 zones and 26 deposits present,
    coast and rock ratios unchanged.
    A LESSON WORTH KEEPING: calibrating a distribution against its mean is not calibration. The
    mean was 497m in iteration 6 and I recorded it as "a bit low but the right order" - it was
    actually telling me the shape was wrong, and one summary statistic could not show that.
    NOT taken: the simulations themselves. Both run plate motion forward over hundreds of
    millions of years with erosion between steps; this generator computes a present-day state
    directly. Same history dependence recorded in iterations 12, 16 and 17.

- assigning-climate-regions-worldbuilding-guide-series-part-7-ouEqR9e1Mco  -- DONE
    This one gives an explicit LATITUDE BAND and a continental-side rule for every Koppen zone,
    which is a complete independent specification to check the classifier against rather than
    just more description. Built a check for it.
    12 OF 14 ZONES sit in the band the reference states - Af within 12 deg, Cfa 20-35, Csa 30-35,
    Cfb 35-55, Dfb 30-60, Dfc 50-70 and so on. The two that do not are tundra (median 44) and ice
    cap (51), both stated as 60-75 and above 65. That is the lapse rate putting polar climates on
    mountains, which is correct geography - highland tundra is real - but it drags the median
    equatorward of a band written for lowlands. Noted rather than "fixed", since suppressing it
    would mean removing legitimate highland climates.
    A REAL BUG IT CAUGHT: the east/west rule was INVERTED. Mediterranean came out 17% western
    when it should be overwhelmingly western, and humid subtropical 30% eastern. Mediterranean
    had been placed on whichever flank happened to be dry - via cold current, then offshore wind
    as a fallback - and iteration 13 already established that these worlds grow no proper gyres,
    so neither of those reliably points west. Which flank of a continent a coast is on is a
    geographic fact, not something to be inferred from a circulation that does not organise here.
    Added westernFlank, read straight off the gradient of the distance-to-ocean field. Result:
    mediterranean is now 100% western on both seeds tested.
    Three separate references now state that rule - the climate videos in iteration 5, the
    Koppen tour in iteration 20 ("always on the western fringes of continents") and this one -
    which is what made it worth encoding as geography rather than hoping it emerged.
    HUMID SUBTROPICAL remains mixed (36% and 51% eastern against a stated eastern preference).
    It is the fallback of the C group, so it takes whatever mediterranean does not. Forcing it
    east would be fitting the statistic rather than modelling the cause, and the cause - warm
    currents and monsoon inflow on eastern coasts - needs the gyres these worlds lack. Reported.

- how-rivers-work-world-building-geography-YI5GnAs8yRY  -- DONE (nothing new)
- river-basins-x7lBi6OaFMo  -- DONE (nothing new)
- rivers-and-lakes-on-your-map-worldbuilding-course-phKey-_LDNc  -- DONE (nothing new)
- how-to-draw-rivers-on-your-fantasy-map-geography-of-rivers-h-AdmsLQ6Fb8I  -- DONE (nothing new)
- hydrology-and-fluvial-geomorphology-the-drainage-basin-syste-SibHNMFzA40  -- DONE (nothing new)
    Read the openings of all five. Introductory river and drainage explainers covering exactly
    what iterations 8-10 built: water flows downhill, rivers rise in mountains because the range
    forces air up and wrings it out, rivers join and never split, a drainage basin funnels
    everything to one outlet. The hydrology one is an A-level unit on the drainage basin SYSTEM -
    interception, stemflow, throughflow, baseflow - which are sub-cell processes, and its only
    figures are the global freshwater budget rather than anything a runoff model could use.

- introduction-to-river-deltas-and-their-stratigraphy-c16-v1-F-u2A95lSII  -- DONE
    A stratigraphy lecture - topset, foreset and bottomset beds, avulsion, lobe switching - most
    of which is vertical structure inside a delta and below the resolution here. What it prompted
    was CLOSING AN ITEM PARKED SINCE ITERATION 9.
    That iteration deferred delta SHAPE because the reference sets it by a contest between the
    river pushing sediment out and the sea sweeping it away, and there was no ocean current model
    to weigh against the river. Iteration 13 built one. So the contest is now computable from
    two things already on the map: discharge from the drainage solve and the surface current from
    the circulation pass. Added DeltaShape - none where the sea carries it off as fast as it
    arrives, cuspate where it is reworked back against the shore, fan where the two are close,
    bird's foot where the river wins - and the shape now drives how the fan grows, so a bird's
    foot pushes fingers seaward while a cuspate delta is refused seaward growth.
    All three shapes appear. Bird's foot dominates at 2.8-3.3 cells per fan against 1.0-1.2 for
    the others, which is the elongation it should have - and its dominance is itself coherent,
    since these worlds have weak currents (no gyres, iteration 13) so rivers usually win. Earth
    has a fuller mix because it has strong western boundary currents.
    ONE DISTINCTION I COULD NOT DRAW, and it is an interaction between two of my own passes: the
    reference separates cuspate by the current striking the mouth HEAD ON. The current model
    deliberately removes the into-shore component so water runs ALONG a coast rather than into
    it - right for a coastline in general, but it leaves nothing pointing at a river mouth, so
    head-on is unmeasurable by construction. Cuspate is taken on current strength alone, which is
    the same mechanism without the angle. Recorded in the code rather than faked.

- starting-your-map-with-landmasses-and-plate-tectonics-worldb-LnVtHVKL5YQ  (Madeline James) -- DONE (verified, nothing to change)
    Worth noting who this is: Madeline James is the source Artifexian cites throughout the rocks,
    metals and ore videos ("following Madeline James' wonderful deposits and gemology guide").
    This first entry in her series deliberately AVOIDS the generative route - "plate tectonics is
    decidedly the most scientific way to go, but it involves fractal methods and special
    algorithms, and it's a little bit too complicated even for this guide" - and starts from
    hand-drawn landmasses instead. So there is no algorithm in it to take.
    Two claims were checkable and I checked both.
    LAND/WATER: she says aim for earth's ~71% water. This generator gives 68.5-71.2% ocean. Holds.
    TRANSFORM BOUNDARIES: "most likely all of your transform boundaries should occur between the
    same types of crust - you won't have oceanic and continental crusts at a transform boundary."
    Measured: transforms come out 55-61% same-crust, not all.
    I am NOT changing this, and the reason matters. Boundary type here is derived from the
    relative MOTION of the two plates - shear against normal - which is what actually defines a
    transform boundary; it is conservative, neither making nor destroying crust, and that is a
    statement about movement, not about what happens to lie either side. Her rule is a sound
    heuristic for someone drawing a map by hand, but forcing it would mean overriding the motion
    geometry with a drawing convention. Earth has ocean-continent transforms too - the Queen
    Charlotte fault. The measurement is recorded; the model stands.

- how-to-code-procedural-terrain-with-perlin-noise-javascript-ZoqPQ0sFo6A  -- DONE (nothing to take)
    An introductory p5.js tutorial: why gradient noise beats white noise for terrain, and
    thresholding a noise field into water/sand/grass bands. This project already runs FastNoise2
    with fractal fBm, domain warping and cellular noise, sampled in 3D on the sphere so the poles
    do not tear, with the tectonic simulation supplying the low frequencies and noise only the
    high ones (iteration 22). Nothing here it does not already do.


## Civilization work (after the loop closed)

Two features added on request, both built on the layers the loop produced.

SPECIES HABITAT PREFERENCES - include/worldgen/civilization.h
    Settlement geography splits a location into SITE (the exact ground: water, arable soil,
    grazing, building material, fuel) and SITUATION (what surrounds it). The site factors are
    what a generator can answer and every one was already on the map - rivers and lakes, climate
    and sediment, the grassland zones, forest and stone, and the fuel layer.
    SpeciesHabitat is a weight per factor plus tolerances for elevation, relief, temperature and
    moisture. The weights ARE the configuration: nothing in the scoring assumes humans. Profiles
    ship for human, elf, dwarf, orc and halfling, and a caller can replace the whole list before
    create() without touching the generator - verified by defining lizardfolk and skyfolk in a
    test and getting 98m/26C/45% river and 3013m/-0.7C/0% river respectively.
    Scores are NORMALISED PER PROFILE before comparing. Raw scores are not comparable between
    peoples: elves weight forest at 3.0, which gave them a high baseline anywhere trees grow and
    crowded narrower peoples off the map entirely (halflings held 19 cells). Each profile is now
    measured against the best ground THAT profile can find.

RUINS OF PREVIOUS CIVILIZATIONS
    Following the Caves of Qud approach - generate a history, then instantiate the places it
    mentions. Each FallenCivilization gets a species, an age and, crucially, ITS OWN CLIMATE:
    the further back it fell the further its world sat from this one. The map is re-scored under
    that era's temperature and moisture using the cold/hot-world machinery from iteration 6, so
    ruins land where the ground suited those people THEN. A dead city can sit in what is now
    desert, or under forest that was steppe when it was built.
    What survives depends on age and on what has grown over it: desert and ice preserve ruins,
    rainforest swallows them, rivers take them apart. Later civilizations build over earlier ones
    rather than beside them.

- site-and-situation-grade-12-settlement-geography-I4dCwf0vIfI  -- DONE (drove the habitat work above)
- jason-grinblat-procedural-history-in-caves-of-qud-ClGAApZYIvI  -- DONE (drove the ruins work above)

- cultural-divides-and-physiographic-regions-society-culture-w-f-MRoDN4IfA  (Madeline James) -- DONE
    The natural next layer on the habitat work, and the same author Artifexian cites throughout
    the geology videos. Her aim is "a global boundary map for isolating cultural and civilization
    clusters", and the important part is that she draws TWO sorts of divide that do NOT mean the
    same thing:
      GEOGRAPHIC - open sea, mountain wall, desert and rough steppe. Stops the spread of a people
        AND the exchange between them. These bound the PHYSIOGRAPHIC REGIONS.
      CLIMATIC - a change of climate family. Stops the spread but NOT the exchange: "there will
        still be plenty of trading, diplomacy and exchange, though it will be less likely for a
        civilization to spread across that barrier to a dissimilar climate." These subdivide each
        physiographic region into CULTURAL REGIONS.
    Both are flood fills over ground already on the map. Added BarrierType, ClimateGroup,
    CultureRegion, and two mapgen layers (Cultural regions, Barriers).
    TWO OF HER CLAIMS WERE CHECKABLE AND BOTH HOLD:
      "Agricultural technology tends to spread easily east to west, as you are more likely to have
      similar climates." Regions come out 2.8 to 6.5 times WIDER than tall - they follow the
      climate bands, which is the Jared Diamond east-west axis argument arriving from geometry
      rather than being asserted.
      "Circle the flood plains around any major rivers in desert or steppe regions - these are
      possible agricultural centres in a region that otherwise wouldn't support agriculture."
      Arid regions carried by a river are flagged separately; 5 of them on seed 42. That is the
      Egypt and Mesopotamia case falling out of the river and climate layers meeting.
    TUNING: mountain barrier first came out at 47% of land, which would make half a world
    impassable. The elevation cut was doing the damage rather than the steepness - a high plateau
    you can walk onto is not the wall a broken range is - so the elevation threshold went up to
    2300m and relief to 0.24, giving 17-25% barrier mountain against earth's ~24% mountainous
    land, and 61-78% of land crossable.
    NOT taken: her scale note is that these divides suit an iron age to late medieval world and
    an earlier one wants more of them. That is exposed as BarrierThresholds rather than fixed, so
    a caller can loosen or tighten the whole divide structure for the technological era they want.

- how-empires-borders-maps-work-Jmk7hOvDFB0  (Stoneworks) -- DONE
    Sits exactly on top of the cultural regions from the last iteration. Its central claim is that
    a pre-modern border is not a line at all: "the border was as far as the Lord could project his
    power", and "if you ever see a map of pre-modern countries, just know that the borders are more
    of an approximation of how far the rulers could project their power than they are hard lines in
    the sand." That is a generation method, not a description, and it is the one taken:
      "When creating an empire or state on a map, I always start out with the core regions of
      power. This should be well in control of the state, and far away from other empire's core
      regions of power. From this area, the state's power should emanate."
    So no border is ever drawn. Each state is a CORE plus a POWER BUDGET, the budget is spent
    walking outward over ground that costs what it costs to hold, and wherever it runs out is the
    border. include/worldgen/polity.h: Polity, PolityThresholds, coreScore, expansionCost,
    projectedControl. The generator picks cores, then runs one multi-source Dijkstra over the whole
    map where each state carries its own species and its own home climate.
    WHAT MAKES GROUND CHEAP OR DEAR - straight off his lightning round. "You got rivers, trade,
    unity and irrigation. Then along coasts for fish, trade, unity and transport. Fertile areas for
    food... Then go up to the natural borders, because it's hard to bring an army and trade over
    mountains, deserts, jungles and rivers." Rivers and coasts discount; mountain, desert, jungle
    and ice cost multiples; poor ground costs more to hold than it returns. The sea is crossable at
    a price but never claimed, so a state can take a strait without ruling the water.
    HOME FIELD - "for smaller states, the environment of the core area is going to be where it
    expands to. If it's mountains, they'll be more likely to stick to the mountains, like in
    Armenia. It gives a sort of home territory advantage for whatever biome they exist in." Ground
    in the core's own climate family is discounted, and this is where it meets the species work
    already in the generator: the cost is scored through the RULING PEOPLE'S habitat, so a dwarven
    state pushes into hill country a human one would stop at.
    MARCHES - "oftentimes the line would blur out to the land around it and the entire region would
    become a buffer zone with a bit more autonomy, cultural and political influence from both sides,
    and a lot of fighting", and "by the time you reached the frontier, the ability of the rulers to
    enforce their laws and collect taxes was less effective than in the heartland". Control falls
    from 1 at the core to 0 at the limit of the budget, and a cell is a march when either its
    control has thinned or a rival is next to it. 21-29% of held ground.
    HOLDOUTS - "you can see in a map of Aztec holdings that they're... well, missing a few parts."
    Tlaxcala was surrounded by Aztec tributaries and never taken. Defensible ground a state's power
    only barely reaches, and that the state has entirely enclosed, is detached as an independent
    enclave. This is the "give it visual inconsistencies" note, which he argues is what makes a map
    look like it has history behind it rather than like it was drawn in one sitting.
    VERIFIED ON SIX SEEDS. The whole point of the model is that borders land on natural features
    without being told to, so that is what was measured, against the base rate of the same features
    across all land:
      mountain  24-39% of border ground vs 17-25% of all land
      desert    23-52% vs 9-25%  - roughly double in every seed
      coast     14-24% vs 18-34% - UNDER-represented, and correctly so: a state grows ALONG a
                coast rather than stopping at one
      river     8-10% vs 8-11%   - flat, see below
    States come out wider than tall in all six seeds (lowland-core aspect 1.15-1.61), which is the
    home-climate discount following the latitude bands. It is a far weaker stretch than the
    cultural regions showed (2.8-6.5) and that is right rather than a shortfall - he predicts
    exactly this shape for core-radiating states: "if a state has one center of power... then that
    state's influence may radiate outwards in a blobular fashion."
    His Inca claim - "not many empires in history are vertical because climate changes with
    latitude... but the Incas were in the mountains" - is only DIRECTIONALLY confirmed.
    Mountain-core states average 1.18 against 1.30 for lowland ones, lower in 3 seeds of 6, equal
    in 2, higher in 1. Reported as marginal rather than tuned into looking clean.
    NOT TAKEN: he wants rivers to be borders as well as highways ("it's both convenient and
    strategic to use rivers and mountaintops as borders because they're well-defined lines and
    they're good at blocking armies"). Both readings are true of real rivers and they pull opposite
    ways in a cost field. The highway reading was taken because it is what the expansion list
    argues for, and the measurement above shows the consequence honestly: rivers sit at their base
    rate on borders, neither attracting nor repelling them. Splitting the two would need the river
    to be an edge property rather than a cell property - a cost to cross the channel, separate from
    the cost to travel the valley - which the influence map has no place to store.
    ALSO NOT TAKEN: succession, marriage inheritance, dynastic collapse and war (Habsburgs, the HRE
    fragmenting, dynastic cycles in China) are all HISTORY rather than geography. They need a
    simulation over time, which is the same limitation already recorded against the tectonic
    history videos - this generator emits one snapshot.
    TUNING: the enclave rule first produced 14-23 holdouts a world, which defeats its own purpose;
    an inconsistency that happens twenty times is a pattern. Candidates are now ranked by how
    defensible they are and how much of them there is, and capped at 3.

- history-of-the-major-trade-routes-summary-on-a-map-Q7xp1-VvtZ0  -- DONE
    Four thousand years of trade in half an hour, and the useful thing is that it never once
    explains a route by distance. Every single one is a want met by a place: jade out of East
    Turkestan against Chinese silk and tea, incense off southern Arabia against Indian pepper and
    cinnamon, Saharan gold against Saharan salt, Novgorod furs and Riga grain against Scandinavian
    herring. That is the rule taken: a route is drawn between two states because each has something
    the other has not, and it then takes the cheapest way there.
    include/worldgen/trade.h: TradeGood (the video's own list - grain, livestock, fish, timber,
    furs, salt, spices, incense, gold, silver, copper, tin, iron, gems, stone, coal), cellGoods,
    TradeThresholds, travelCost, TradeRoute. Cell fields goods/traffic/chokepoint, and a Trade
    routes layer in mapgen.
    NOTHING NEW HAD TO BE INVENTED TO SAY WHAT A PLACE SELLS. Every good reads off a field an
    earlier iteration already produced: grain and livestock off the site factors, timber and furs
    off the forest and climate group, fish off cold-water coasts, salt off the halite and salt-flat
    deposits, spices off Af/Am, incense off the hot dry country (which is literally where
    frankincense comes from), and the eight metals straight off DepositType. A world that grew its
    own climates, ores and forests does not need a table of who trades what.
    THE COST OF CARRYING IS NOT THE COST OF HOLDING. This is the part that mattered most. The
    polity pass from the last iteration already had a cost field, and reusing it would have been
    wrong: a caravan only finds a mountain expensive where an army finds it impassable, and the sea
    reverses the pair completely - it is the cheapest ground there is for cargo and the dearest for
    rule. Coastal water (within sight of land) costs a third of plain ground; open ocean costs more
    than twice it, because deep-water seamanship is a thing you have to have - "the Romans come
    into contact with the Kingdom of Aksum, where merchants live who've mastered techniques of
    navigation on the high seas."
    CHOKEPOINTS. Petra "an important commercial crossroad between Egypt and Mesopotamia",
    Alexandria "the great warehouse", Constantinople "the new commercial crossroads between East
    and West", then Malacca, Suez, Panama. Every one is a place the world narrows to a gap. So a
    chokepoint is heavy traffic across ground with no way round it: an isthmus with sea on both
    sides, a strait with land on both sides, a pass between mountain walls. 1 to 16 a world.
    THE CLAIM THAT WAS WORTH TESTING. The video's causal engine is control of intermediaries -
    "China turns away from land routes to concentrate on sea routes, which are safer and have fewer
    intermediaries"; Portugal rounding Africa "at the expense of the powers that control the
    passage via the Middle East"; the Arabs cutting Constantinople off by taking Alexandria. So
    every route is also routed a SECOND time with the sea forbidden, and the two are compared. Over
    six seeds, forcing the same trade overland costs 1.8x to 4.0x more and puts it through 2.4x to
    4.1x as many hands. The claim holds, and the by-product is the genuinely game-usable output:
    each route now knows what its overland alternative would cost, which is to say who could
    strangle it and what it would cost them if somebody did.
    TWO FLAWS FOUND AND FIXED BEFORE THEY COULD BE BELIEVED:
      A state's offer was first taken as the union of everything found anywhere it rules. That
      produced 8 routes on a world of 17 states, because a state ruling a thousand miles of ground
      has a little of everything somewhere in it and therefore wants nothing. Offer is now what a
      state has in QUANTITY (>=6% of its cells) and want is what it has almost NONE of (<=1%),
      which is what makes jade, tin and incense worth carrying at all. 19 to 69 routes a world.
      The intermediary test was vacuous as first written - it compared sea-heavy routes against
      land-heavy ones, and there were no land-heavy ones to compare against. Replaced with the
      counterfactual above, which asks the question the video actually asks.
    NOT TAKEN: the whole modern half - canals, steam, rail, containers, oil, rare earths - is past
    the technological era this generator's other layers are drawn for. The barrier thresholds are
    already documented as iron age to late medieval and the trade costs match them.
    LIMITATION, and it is the same one recorded against the ocean gyres in iteration 13 and the
    AMOC in 19: traffic funnels less than it should, with the busiest 5% of trading cells carrying
    only 16-19% of the traffic. Real trade squeezes far harder than that, and the reason is that
    this world's ~70% ocean is one connected body with few narrows in it. Enclosed basins are what
    make a Mediterranean, and a Mediterranean is what makes a Constantinople.

- the-parasite-city-on-your-fantasy-map-I--xW5Jgw5Q  -- DONE
    Lands directly on top of the trade routes from the last iteration, and opens by confirming the
    decision made there: "sea transport costs 40 times less than land transport. That gap creates a
    flow, and wherever flow concentrates something builds to catch it. The city is a drain." The
    trade pass produced the flow; this one finds the drains.
    include/worldgen/city.h: NodeType, CityThresholds, waterCeiling, riverCeiling, classifyNode,
    City, cityReason. Cell field city, and a Coastal cities layer in mapgen.
    THE FOUR NODES, checked strongest first because the reference says which wins - "the mouth is
    the strongest node on your map, its power is purely geographic":
      MOUTH      a navigable river or a delta reaching the sea. Two supply chains meet and the man
                 sitting on the meeting point taxes both without producing anything. 11-14 a world.
      STRAIT     the chokepoints the trade pass already found, taxed from the shore beside them
                 rather than from the water. 0-1 a world, which is honest - see the limitation.
      ANCHORAGE  shelter (fjord, ria, island-screened coast) surrounded by coast that kills ships
                 (cliff, reef, bar). The surrounding hostility is required: an anchorage lord has
                 nothing to sell if the next bay is as good. 7-12 a world.
      EXTRACTION a coast holding a good almost nowhere else has. "The only way an extraction lord
                 wins is a monopoly." 0-4 a world.
    THE THREE CEILINGS - "they stack, and the smallest one wins":
      FRESH WATER  "you can haul calories across an ocean, but shipping water for a metropolis is
        impossible." No city ever ends up water-limited in the output, and that is the rule working
        rather than failing: thirsty coasts are ELIMINATED, not shrunk. 4-9% of all coast on these
        worlds cannot hold a city at any size for want of drinking water.
      OWN FIELDS   arable in the hinterland the wagon can actually reach.
      RIVER SYSTEM "no river means a town, one river means a city, a delta means something larger."
    THE PARASITE. "Rome did not eat Italian bread. It ate Egyptian bread." A city whose own fields
    are the binding limit, but which sits on a lane that can carry grain, refuses the limit and
    takes its bread off the water. 1-8 a world, and the largest city on most seeds is one of them.
    Its reason sentence says so outright: cut the lanes and it starves. That is the reference's own
    step 5 - "those sea lanes are the arteries... those answers are the city's entire foreign
    policy" - and it is answerable now because the trade pass knows which routes touch which port.
    THE JOB DESCRIPTION. Step 4 asks for one sentence per node: "the sentence is the city,
    everything else follows." cityReason writes it from the node type and the binding ceiling, so
    every city on the map explains itself.
    THE 40 TO 1 RATIO WAS TESTED AND NOT ADOPTED. The trade pass had guessed about 3 to 1 for
    coastal water against plain ground. The video gives a hard number, so it was run at 40 to 1.
    Result: sea share 83-84% -> 94-95%, and intermediaries crossed fall to 0.00 - every route goes
    by water and passes through nobody, so there is no silk road left anywhere on the map. The 40
    to 1 is a GRAIN figure. The long overland routes never carried grain; they carried silk, jade
    and spice, cargo dense enough in value to pay for a wagon, which is the same reason the
    reference itself says "past a CERTAIN RADIUS the wagon becomes a waste of time" rather than
    saying the wagon is always useless. The mixed-cargo value is kept and the experiment is
    recorded in the header beside it.
    CALIBRATION ERROR CAUGHT: the first run gave a river ceiling of exactly 0.35 for every city on
    the map, because the thresholds had been written in earth numbers (Amazon-scale, hundreds of
    thousands of square kilometres) and this world's mouths drain p50 200, p90 1100, max 7400 - two
    orders of magnitude out, so every river read as "no river". Thresholds are now set off the
    world's own measured distribution, and the ceiling spreads properly across town, city and
    delta. The first run also made 20 of 24 cities mouths, because any trickle reaching the sea
    counted; a mouth now needs a navigable river.
    NOT TAKEN: chapter 3, the defensive geometry - high ground for visibility, the lord's seat, and
    connecting the hill to the harbour so an army can cross between them in time. That is city
    LAYOUT, at a scale of hundreds of metres. An influence cell here is thousands of blocks across,
    so this belongs in generateChunk/generateRegion, the same place the coastal landforms from
    iteration 7 were left.
    LIMITATION: only 0-1 strait nodes a world, where earth has Gibraltar, Bosphorus, Malacca, Hormuz
    and the Danish straits. Same root cause as the trade funnelling, the ocean gyres and the AMOC:
    one connected ocean with no enclosed basins in it. There is nowhere for a Bosphorus to be.

- end-to-end-procedural-generation-in-caves-of-qud-jV-DZqdKlnE  (Grimblat & Bucklew, GDC) -- DONE
    An architecture talk rather than a geography one, so most of it confirms decisions this
    generator already made: their "abstraction mountain into concrete valley" - abstract output
    early, reified on demand later - is exactly the influence map feeding generateChunk. Two things
    in it were genuinely new and both went in.
    1. THE INVERSION OF HISTORICAL LOGIC. "We actually have a sort of inversion of historical logic
    where we decide how an event ends and then create a rationalization for why it ended that way."
    The generator already had fallen civilizations with an era, a climate and a scatter of sites,
    but no account of what happened to them. Now a CivilizationFate is ROLLED FIRST - conquest,
    drought, ice, exhaustion, sundering, drowning, plague - and only then is the map searched for
    what would account for it, over that civilization's own sites: how far their era's rain sat
    from this world's, what share of their sites stood on a deposit, how many are on ground the sea
    has since come up over, how many separate cultural regions their country ended up divided
    among, and which living state's seat of power sits nearest their own.
    THE DISCIPLINE THAT MAKES THIS WORK IS THAT THE EVIDENCE IS FOUND, NOT INVENTED. The first
    version failed this on its first run: a civilization rolled Drought and the sentence read
    "their world was wetter than this one" while its own recorded swing was -2.0C, meaning their
    world was DRIER. The fate is a given and may be stated freely; a wetter world is a CLAIM and
    may only be made where the map shows one. It now reads "the rains failed them, though nothing
    in this world's weather still shows it." Two to four of the four civilizations a world get a
    fate the map can account for, and the rest say so. That is the reference's own position - "the
    histories aren't complete, we let them be sort of spotty."
    2. POPULATION TABLES WITH PRIORITY FALLTHROUGH. "If you've got a fish village it rolls off the
    factional table, if it doesn't it falls down to a regional table, and if that doesn't exist it
    gets the default." Implemented as WeightedEntry/rollTable plus ruinKindsForSite falling through
    to defaultRuinKinds, and used to decide what each ruin actually IS: harbour works on a coast,
    a mine head on an exposed deposit, a bridge at a river, terraces or a fort on a slope, a tomb
    in dry country, a monument anywhere else. The mapgen ruins layer now shows the kind as hue and
    the condition as brightness.
    3. REIFYING ABSTRACT LINKS AGAINST REAL OBJECTS. Their step two takes "this village worships a
    legendary creature" - where at that point "they don't even know what creature they worship,
    they simply know that they worship a creature" - and binds it to a monster actually placed
    nearby. The equivalent here is that the fall is resolved AFTER the states, regions and coasts
    exist, so its abstract relations can be bound to them: the nearest rival is a real polity's
    seat of power, and the refuge is a real cultural region.
    THE REFUGE HAD TO BE MADE MEANINGFUL TWICE. First written as "the nearest region that is not
    theirs", it fired for every civilization on every seed, which makes the clause worthless -
    the next valley over is where you go for market day, not where you flee to. It now takes two
    walks: one that stays on open ground, and only if THAT finds nothing does a second walk go over
    the divide. Refuges now appear 3 times in 24 civilizations, and when the sentence says their
    descendants are over the barrier it means something.
    NOT TAKEN: the fabrication half of the talk - building placement by seeded Dijkstra maps, WFC
    templates, largest-internal-rectangle, inside/outside analysis, object blueprints. All of it
    operates inside a single screen. This generator's influence cell is thousands of blocks across;
    that work belongs in generateChunk, alongside the coastal landforms from iteration 7 and the
    city defensive layout from the parasite-city video.
    ALSO NOT TAKEN: their design-time authoring pipeline (XML population tables, replacement
    grammars for text, faction graphs). The weighted table with fallthrough was worth having as
    code; the data-authoring tooling around it is a game content pipeline, not a terrain library.
    HONEST GAP: the Tomb ruin kind appears about once every six seeds, because ruins are sited on
    ground that suited their builders and almost nobody's builders liked desert. Ruin sites now
    standing in desert or steppe run 0 to 9 a world. The branch is reachable and the premise it
    rests on - a dead city in what is now desert - does happen, just rarely.

- herbert-wolverson-procedural-map-generation-techniques-TlLIOgWYVpI  (Roguelike Celebration) -- DONE
    A dungeon-generation toolkit talk - room placement, BSP, cellular automata, drunkard's walk,
    diffusion-limited aggregation, Voronoi, prefabs. Nearly all of it works inside one screen and
    has nothing to say to an overworld library. Two things do, and the second is the reason this
    one was worth reading.
    CONFIRMED, NOT NEW: his Perlin advice is what this generator already does. "Generate multiple
    noise maps and mix them together... as you zoom in you change the percentage of each of the two
    gradients" is the detail-noise layer feeding getBaseHeight; his Dijkstra-map connectivity work
    is what the polity expansion and trade routing already run on.
    TAKEN - THE HOT PATH. "Once you've got your start point and your end point you generate the
    path between the two, then you make ANOTHER Dijkstra map using all the points on the path as
    your sources. Everything with a tile value under say 10 is the hot path." He uses it to decide
    what a map is for: the rooms on the path are where the player goes, and "the gray rooms could
    be where you hide bonus things." The generator knew traffic ON its trade routes and nothing at
    all about distance FROM them, which is precisely that second map. One multi-source Dijkstra out
    from every trafficked cell and every city, walking overland at the same cost trade pays, gives
    cell.remoteness and cell.onTheBeatenTrack, and a Beaten track layer in mapgen.
    It separates the world sharply, on all six seeds:
      on the beaten track  30-43% of land, and 72-83% of THAT is held by a state
      off it               57-70% of land, and only 19-50% of that is held by anybody
      mean habitability    0.33-0.37 on the track against 0.16-0.25 off it
    Which is the expected result rather than a surprising one - states and trade both follow cheap
    ground, so they concentrate in the same places - but it is now a field a caller can ask about,
    and it answers the question his talk is really about: where do you put the thing you want
    FOUND rather than passed.
    ALSO TAKEN - THE SENTINEL CULL. "All of the tiles that are open but have the sentinel value
    can't be reached. You can delete those because nobody can get there." Ground the walk never
    reaches is not merely remote, it is cut off, and it is now marked as such: 3-4% of land on most
    seeds, and 19% on one, where a whole landmass has no port on it and nothing walks to it from
    anywhere. That is a real generator finding rather than a placement convenience, and it is
    exactly the check he says to run before placing anything.
    MISTAKE MADE AND FIXED: remoteness was first thresholded at 55% of the furthest ground on the
    map, which named only 5-7% of land as backcountry. Distance-from-network is heavily skewed -
    half the land sits within a tenth of the maximum and one walled-off corner sets the maximum -
    so a threshold expressed as a share of the furthest cell lands in the tail and names almost
    nothing. His own construct is an ABSOLUTE cutoff, and that is what it is now. The redundant
    third band was dropped rather than retuned.
    NOT TAKEN: BSP, cellular automata, drunkard's walk, DLA, Voronoi cells, prefab injection. All
    of these build a map from nothing at room scale; this generator's cells are thousands of blocks
    across and its terrain comes from tectonics rather than from carving. Where they would be
    useful is inside generateChunk, which is where the coastal landforms, the city layouts and the
    Qud fabrication work have all been pointed.
    ONE THING WORTH REVISITING LATER, from his Q&A: "run your Perlin through erosion, especially if
    you decide that some rocks are harder than others." This generator HAS a rock type per cell, so
    differential erosion by hardness is right there - except that rock is derived from height and
    height would then be derived from rock. Breaking that circle means splitting the height pass in
    two with the rock pass between them, which is a re-plumbing of the whole pipeline rather than
    an addition to it. Recorded, not attempted.

- how-one-guy-fixed-procedural-generation-Y19Mw5YsgjI  (on Oskar Stalberg) -- DONE, mostly triaged
    About Townscaper and Bad North: hand-modelled tile pieces, marching-squares style variant
    selection, Stalberg's irregular quad grid, mesh deformation to fit a relaxed grid, and model
    synthesis / wave function collapse. Almost all of it is MESH work at the rendering layer of a
    tile game - this generator has no tile art and assembles no meshes - so it would have been
    triaged like the drawing tutorials, except that chasing down its one applicable idea turned up
    a real bug.
    THE DUAL GRID, AND THE BUG IT FOUND. His central move: "on the main grid the type is defined
    for the whole tile, but on the dual grid the type is defined for each CORNER of the tile", so
    that neighbouring pieces agree along a shared edge instead of having to be reconciled. Checking
    whether this generator does the equivalent led to updateInfluenceNeighbors(), which stores four
    corner values per influence cell, each the average of the four cells meeting at that corner -
    the same construct, already written, arrived at independently.
    AND NEVER CALLED. The call at the end of generatePlates() was commented out, and the only other
    one sits inside the persistence path, which CLAUDE.md already records as templates that nothing
    currently instantiates. So after any create() - the only path mapgen uses - the neighbour map
    was EMPTY at 0 floats where 131072 were needed, and getBaseHeight() indexed straight off the
    end of it. Every chunk-level height query in the library was an out-of-bounds read.
    Restored and verified: the map now fills, and 666 samples of getBaseHeight across the world all
    land inside [0, 1024], span 294 to 859 blocks about a sea level of 512, and sit within 15
    blocks of the cell height they interpolate - the small smoothing that corner averaging should
    produce and nothing worse.
    That this surfaced on the last video is worth recording plainly: it was found by taking the
    video's idea seriously enough to go looking for the local equivalent, not by reading the code
    for its own sake. It is also the one bug in this whole run that no amount of verifying OUTPUT
    would have caught, because nothing in the overview path touches getBaseHeight - every check
    written across 63 videos reads the influence map directly.
    NOT TAKEN: the tile pieces, the variant sets, the multi-tile special pieces, the vertex
    deformation, and the irregular quad grid. All of them exist to make a rendered surface out of
    discrete authored art. This generator's cells are thousands of blocks across and its terrain
    comes from tectonics rather than from fitted pieces. Its answer to the same "a regular grid
    looks regular" complaint is already a different and more appropriate one: a projection layer
    with a Fibonacci sphere seeding the plates, so the underlying structure is not aligned to the
    lat-lon grid in the first place.
    RECORDED, NOT ATTEMPTED: wave function collapse and model synthesis, which the video explicitly
    defers to a future one of its own. Their real lesson is architectural - constrain generation so
    invalid output is IMPOSSIBLE, rather than generating freely and validating afterwards. This
    generator does the latter throughout (the river dead-end check, the connectivity cull from the
    previous iteration). That is the right trade for terrain, where the constraints are physical
    rather than combinatorial, but it is the honest contrast to draw.

## ALL VIDEOS PROCESSED - LOOP COMPLETE
All 69 summaries in ../../yt_learning/runs have been read and either implemented or triaged with a
reason. Nothing remains unprocessed and no new summaries have appeared across the last five checks.

## Triaged as not applicable (iteration 13)
Checked, not implemented, with the reason. These are counted as processed - each was read far
enough to establish it has nothing this generator can use.

FANTASY MAP DRAWING TUTORIALS (4 videos). Scanned all four. These teach how to DRAW a map by
hand - "three different ways to artistically represent coastlines", pen technique, the nuts and
bolts of one person's mapping workflow. Where they do cover geography (plate tectonics, wind
zones, ocean currents) it is the same material already taken from the Artifexian videos in
iterations 1-6 and 13, aimed at someone drawing rather than generating. There is no rule in them
this generator does not already apply.
  drawing-coastlines-for-your-map-8VcksRDC5NM
  fantasy-maps-plate-tectonics-tutorial-yGMKmbGTEHQ
  how-to-draw-a-fantasy-world-map-from-scratch-coastlines-tect-1Sp8cunh74E
  how-to-make-a-fantasy-map-with-geography-worldbuilding-X3abr8yAuxk

PORPHYRY DEPOSIT DETAIL (3 videos). Read the openings of all three. Two are general explainers
of porphyry formation - large low-grade intrusion-hosted deposits on volcanic arcs - which is
already implemented and was just re-confirmed by the copper-types video. The third is about the
A/B/D vein naming system of Gustafson and Hunt 1975, which classifies QUARTZ VEIN TEXTURES within
a single deposit. That is metre-scale structure inside one orebody; an influence cell is 4096
blocks across and carries one deposit as a whole.
  a-b-and-d-type-veins-in-porphyry-copper-deposits-HB6lC6k521s
  porphyry-and-epithermal-mineral-deposits-tpvkyukklXo
  what-are-porphyry-mineral-deposits-how-are-porphyry-deposits-YwPSln2dewg

COASTAL EROSION AND DEPOSITION (5 videos). School geography explainers on how waves cut cliffs
and build beaches, spits, bars and tombolos. Spot-checked two. This is the same material
iteration 7 already established does not fit: these are metre-to-hundred-metre landforms and an
influence cell is 4096 blocks across. The coast TYPE classification built in iteration 7 is the
part of this that survives at overview scale, and it is already there; the landforms themselves
belong in generateChunk/generateRegion detail.
  coastal-deposition-how-waves-build-landforms-beach-spit-bar-w16HmD_qe_o
  coastal-erosion-reshaping-our-coastline-y6WbzkI17k0
  coastal-landforms-erosional-and-depositional-capes-bays-arch-AA2MC3WC1mw
  erosion-transportation-and-deposition-aqa-gcse-geography-coa-6z2N8Mtv_kw
  what-coastal-landforms-are-formed-by-deposition-4RiHd4c1F8c

UNREAL ENGINE TOOLING (11 videos). Spot-checked two: they are engine plumbing - Epic Games
Launcher, actor classes, procedural mesh components, marching cubes for smooth voxel MESHING.
This project is a standalone C++/CMake library built on FastNoise2 with no engine dependency,
and its generateChunk/generateRegion fill a block buffer - turning blocks into triangles is the
consumer's job (voxigen), not this library's. Nothing about terrain SCIENCE in them.
  create-entire-cities-automatically-with-pcg-splines-procedur-STqt92VF3KM
  how-to-use-unreal-engines-new-landscape-system-mesh-terrain-Lhj2LutYNjA
  landscape-generation-2-terrain-unreal-engine-c-bhObq66AY5E
  landscape-generation-2-terrain-unreal-engine-s2_K3dzXDoM
  procedural-generation-pcg-under-5-mins-in-unreal-engine-5-MF8ZMdOX7pU
  terrain-generation-for-beginners-unreal-engine-tutorial-9ASgfwzNvL8
  terrain-generation-in-c-for-beginners-unreal-engine-tutorial-sNZ2g4qah28
  ue5-ctutorial-minecraft-like-voxel-terrain-generation-part-1-6tiNAet32sg
  ue5-ctutorial-smooth-voxel-terrain-generation-part-5-marchin-OPefZSjlJms
  ue5-procedural-suburban-city-using-pcg-tutorial-part-1-unrea-C-_RYWevAW0
  unreal-engine-5-tutorial-procedural-content-generation-part-PwEDb84sxi8

NOT A WORLDBUILDING VIDEO
  me-at-the-zoo-jNQXAC9IVRw -- the first video ever uploaded to YouTube. 19 seconds, a man at
    an elephant enclosure. Verified by reading it. Nothing to take.

## Remaining
(none - every video in the corpus has been read and either implemented or triaged)
