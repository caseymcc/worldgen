# The Detail Pass

*Verified research · procedural worldgen*

How to turn one cell of a coarse overview map into playable ground — and route the river the overview
promised. **The downscaling half of this question has good published answers. The kingdom-scale half
does not, at least not in anything that survived verification.**

| Sources fetched | Claims extracted | Put to a vote | Confirmed | Refuted | After merge |
|---|---|---|---|---|---|
| 23 | 114 | 25 | 23 | 2 | 18 |

---

## Part one — From one cell to a landscape

Five stages, in order, each with a published method behind it. The sequence matters: conditioning
before routing, routing before carving, and seam agreement decided before any of it runs.

### 01 · Amplify the cell into a detail heightmap

The strongest implementable pattern represents terrain as a sum of overlapping local primitives
blended through a normalized partition of unity. Adjacent patches agree because the weights never
vanish and the blend is continuous — not because edges were matched afterwards.

> **Guérin, Digne, Galin, Peytavie — *Sparse representation of terrains for procedural modeling*,
> Eurographics 2016 / CGF 35(2):177–187**
>
> ```
> f(p) = Σ fᵢ(p)·aᵢ(p) ⁄ Σ aᵢ(p)    aᵢ(p) = g(‖p−cᵢ‖ / R),  g(r) = (1−r²)³ for r<1, else 0
> ```
>
> *"The distribution guarantees that the weighting function never vanishes onto the domain of the
> terrain."*
>
> **Confirmed** · 3–0

Disc supports sit on a grid whose step is half the radius, so every point is covered by several
primitives. The paper amplifies a 1 km/cell Australian DEM to 4 m/cell — **256× linear**, in four
successive ×4 steps — and reports a 32×32 → 8192×8192 amplification in 27 seconds with a 256-atom
learned dictionary. The radius `R` is the one knob: small R preserves the input's shape, large R lets
the exemplar dominate.

> ⚠️ **Qualification carried from verification.** The words *seam*, *tile* and *edge matching* appear
> nowhere in that paper. It decomposes one terrain, not independently streamed chunks. Using it for
> on-demand generation means evaluating every primitive whose support overlaps the chunk — a halo of
> width R around it — which the paper does not discuss.

The example-based alternative merges patches in two stages: a max-flow/min-cut graph cut chooses the
seam through the overlap region using elevation differences as edge weights, then a discrete Poisson
solve over that overlap zeroes the gradient across the seam under Dirichlet boundary conditions — one
small solve per placed patch rather than one huge one.

> **Zhou, Sun, Turk, Rehg — *Terrain Synthesis from Digital Elevation Models*, IEEE TVCG
> 13(4):834–848, 2007**
>
> *"We plan to provide the ability to specify a desired elevation at a specific position. This could
> be accomplished by constraining the Poisson solver."*
>
> **Confirmed** · 3–0 · ⚠️ constraint unimplemented

> ❌ **The central unmet need.** No verified method guarantees that an amplified tile reproduces the
> overview value it came from. Zhou et al. list prescribed-elevation constraints as future work;
> Guérin's radius R is a soft fidelity dial; the 2017 cGAN's constraint satisfaction is soft. The one
> mechanism that would have given exactness by construction — paired low-res/high-res dictionaries
> sharing a coefficient matrix — was **refuted 0–3**. Conservation must be designed in separately:
> amplify the *residual* over an interpolated coarse base, or project the result back onto the
> constraint.

**For this generator.** The residual approach is the natural fit and the base already exists:
`updateInfluenceNeighbors()` stores four corner values per influence cell, each the average of the
cells meeting there. Bilinear over those corners gives a coarse base that is continuous across cell
boundaries by construction; amplification then only has to add a zero-mean residual, and coarse-value
conservation is exact rather than approximate.

### 02 · Condition the surface so water can leave it

Amplification injects local minima. Before any flow can be computed the detail heightmap needs the
same two-stage treatment production hydrology uses — and they are two stages, not one, because
filling a depression leaves a flat where flow direction is undefined.

> **Barnes, Lehman & Mulla — *Priority-flood: an optimal depression-filling and watershed-labeling
> algorithm for DEMs*, Computers & Geosciences 62:117–127, 2014**
>
> *"Ensures that all filled depressions have surfaces with a height difference of at least ε between
> any two consecutive cells as one moves away from or towards the depression's outlet… every cell
> will drain to the edge of the DEM."*
>
> **Confirmed** · 3–0 · merged from 3 claims

Priority-Flood+ε runs in O(*n*) on integer elevations and O(*n* log *n*) on floating point. Note that
"optimal" in the title means asymptotically optimal *running time*, not minimal-modification fill
quality. The companion paper handles flats by superimposing a gradient away from higher terrain with
a gradient toward lower terrain, producing a drainage field that converges rather than sheets.

**For this generator.** This is already implemented — at overview scale. The river pass runs
priority-flood with flood-resolution ordering to route across flats, a fix made after 474 river cells
dead-ended. The same code applies unchanged one level down; only the grid changes.

### 03 · Route the channel between mandated endpoints

The overview fixes where the river enters the cell and where it leaves. That is a shortest-path
problem with two fixed endpoints over a terrain-derived cost field — and it has already been solved
in graphics, for roads.

> **Galin, Peytavie, Maréchal, Guérin — *Procedural Generation of Roads*, CGF 29(2),
> Eurographics 2010**
>
> ```
> minimise C(ρ) = ∫ c(p, p′, p″) dt   subject to  ρ(0) = a,  ρ(T) = b
> ```
>
> *"We use the cost function c(p) plus an admissible heuristic cost estimate function e(p)… the cost
> of a straight-line road to the goal b so that it never overestimates the actual minimal cost. The
> heuristic function e is monotonic, therefore A\* itself is admissible."*
>
> **Confirmed** · 3–0

Anisotropy comes from measuring slope *in the direction of travel* rather than as a scalar — which is
exactly what a channel needs, since water cares about downhill and nothing else. Impassable terrain
gets infinite cost.

The second half of that paper fixes a defect worth knowing about before writing any grid pathfinder:
4- and 8-connectivity cap the achievable angle resolution at 45°, producing visibly faceted paths.
The fix is a **k-neighbourhood coprime segment mask** — all segments from the origin to (i, j) in
[−k, k]² with gcd(i, j) = 1, which avoids redundant checking.

| k | Angle resolution | Segments | |
|---|---|---|---|
| 1 | 45.00° | 8 | equivalent to 8-connectivity |
| 2 | 26.57° | 16 | |
| 3 | 18.43° | 32 | |
| 4 | 14.04° | 48 | |
| 5 | 11.31° | 80 | author's recommendation |
| 6 | 9.46° | 96 | |

Timings grow empirically as O(k²). Verification independently reproduced both columns: the mask
counts follow by inclusion–exclusion over shared prime factors, and the angles are exactly
arctan(1/k).

> ⚠️ **This transfer is inference, not a published result.** Galin et al. never route water — rivers
> appear only as obstacles and bridge targets. The mathematics is identical (a fixed-endpoint
> minimum-line-integral path over a heightmap-derived cost field) and verifiers judged the transfer
> sound, but no paper states it.

**For this generator.** The cost-field machinery is already built twice over: `expansionCost()` for
state power projection and `travelCost()` for trade, both A\* / Dijkstra over the influence grid. A
third cost field for channel routing is the same shape — what it adds is the coprime segment mask,
which neither existing pass uses.

### 04 · Size the channel from discharge

This is the mechanism that makes the same overview river a stream in one cell and a major river in
another. It is pure continuity: a normalized cross-section of unit area, scaled by discharge over
velocity.

> **Peytavie, Dupont, Guérin, Cortial, Benes, Gain, Galin — *Procedural Riverscapes*, Pacific
> Graphics 2019 / CGF 38(7):35–46**
>
> ```
> a = φ ⁄ ‖u‖        (m³/s) ⁄ (m/s) = m²        i.e.  Q = A_wetted · v
> ```
>
> *"Rosgen templates also define cross-sections C(t) taken orthogonal to the river direction and of
> unit area… for every template cross section C(t), the scaling factor is defined as the area
> a = φ/‖u‖."*
>
> **Confirmed** · 3–0

> ⚠️ **The paper never defines ‖u‖.** No Manning or Chézy relation appears anywhere in it, so the
> velocity model is the implementer's to choose — and the area scaling is only as good as that
> choice. Hold velocity constant and wetted area scales as Q^1.0; Leopold–Maddock downstream
> hydraulic geometry gives w ∝ Q^0.5 and d ∝ Q^0.4, so A ∝ Q^0.9, which requires velocity itself to
> scale roughly as Q^0.1.

Riverscapes is genuinely a coarse-to-fine amplification pipeline, driven by slope, drainage area and
stream power derived from the input map. But its amplification is confined to the river corridor —
the terrain is carved by a riverbed function whose support is the riverbed domain only, not general
heightmap super-resolution — and its headline resolution ratio is internally inconsistent, with the
results section citing a 100 m/pixel input against a method section stating a 1–30 m/pixel operating
range. That finding carried a split 2–1 vote for exactly those reasons.

**For this generator.** Both inputs already exist per influence cell: `riverDischarge` in millions of
m³/year and `drainageArea` in km². The existing `riverWidth()` uses a video-derived rule of thumb
rather than hydraulic geometry — worth reconciling against Leopold–Maddock exponents before the
detail layer starts consuming it, since the detail pass will magnify any error in it into visibly
wrong channel geometry.

### 05 · Make neighbouring chunks agree

Tiled hydrology with cross-tile reconciliation is published, proven at trillion-cell scale, and
structurally exactly the seam problem — with one scope limit that turns out not to matter here.

> **Barnes — *Parallel Priority-Flood depression filling for trillion-cell DEMs*, C&G 96:56–68, 2016
> · and *Parallel non-divergent flow accumulation*, EM&S 92:202–212, 2017**
>
> *"A new, linearly-scaling algorithm which parallelizes the Priority-Flood depression-filling
> algorithm by subdividing a DEM into tiles."*
>
> **Confirmed** · 3–0

The structure is three stages: solve each tile independently; send only each tile's *perimeter*
elevations, labels and spillover graph to a central producer; make labels globally unique, connect
adjoining tile edges into one global graph, and broadcast corrections back. Two trillion cells filled
in 4.8 hours on 48 cores; flow accumulation over the same in 24 minutes.

> ⚠️ **Scope limit.** Both require a central producer that has seen every tile's perimeter before it
> can broadcast anything. Under this scheme you cannot route one chunk's river from a seed alone.
> What transfers is the *pattern* — solve per tile, exchange perimeter state only, reconcile on a
> graph over tile edges.

**For this generator — the load-bearing insight.** That global reconciliation pass is already done.
The overview map *is* Barnes' pre-solved global flow graph: `flowDirection`, `drainageArea` and
`riverDischarge` were computed by a world-wide priority-flood, D8 and accumulation pass at overview
resolution. A detail chunk therefore never negotiates with its neighbours — it reads its boundary
conditions off the overview, which both neighbours also read. Agreement is inherited from a solve
that already happened, which is the one thing that makes on-demand generation possible here where it
is not possible in Barnes' formulation.

---

## Part two — Civilizations, and a large gap

Only settlement-scale material survived verification. It is good material — but it is not the
kingdom-scale layer the question asked for.

### Settlement siting: weighted interest with a hard veto

> **Emilien, Bernhardt, Peytavie, Cani, Galin — *Procedural Generation of Villages on Arbitrary
> Terrains*, The Visual Computer 28:809–818, 2012**
>
> ```
> I(B) = 0 if any fᵢ(B) = −1,  else  max(0, Σ wᵢ·fᵢ(B))     fᵢ ∈ [−1, 1]
> ```
>
> *"A negative value is given if the location is undesired (−1 if impossible)… a set of n weighing
> factors {wᵢ} is predefined for each couple (V, B)."*
>
> **Confirmed** · 3–0

Weights live in a "building encyclopedia" keyed by (village type, building type). Placement is
**probabilistic acceptance** proportional to I(B), not argmax — the paper checks constructibility,
computes interest, then runs a stochastic aggregation test.

Seven criteria are enumerated: sociability, worship, accessibility, slope, water, fortification, and
geographical domination. The last has a closed form and is the only verified example anywhere in this
round of gameplay-relevant data being derived from physical geography:

```
f(x) = Σ over p with ‖x−p‖ < r  of  (h(x) − h(p)) ⁄ (1 + ‖x−p‖²)
```

An inverse-square-weighted local prominence, glossed by the authors as "either an indicator of social
superiority or as necessity for defense" — implementable as a windowed convolution over the
heightmap. Static criteria are precomputed as 2D maps; sociability and accessibility are updated as
buildings land.

**For this generator.** This is the same shape as the existing `habitability()` — a weighted sum of
independent site factors with per-species weights, which is Emilien's (V, B) encyclopedia by another
name. Two things it does that the current code does not: a *hard veto* at fᵢ = −1 rather than a soft
threshold, and probabilistic acceptance rather than taking the best cell. The domination term is a
genuine addition; nothing in the generator currently measures local prominence.

### Roads from the same machinery, branching from a re-use weight

No topology rule appears anywhere in the method. Branching structure emerges from making existing
road cheap to travel.

> **Emilien et al. 2012, §4 · building on Galin et al. CGF 2010**
>
> ```
> C′(R) = w_ex · C(R)  if R lies on existing road,  else  C(R)      with  w_ex ≪ 1
> ```
>
> *"To prevent the method from generating a fully ramified road network… the less costly connection
> to the existing network should be looked for (road re-use coming for free)."*
>
> **Confirmed** · 3–0

A separate deliberate step creates cycles by extending a new road to a nearby node within a cone —
cycles, not ramification, consistent with the same mechanism.

### Caves of Qud as an integration exemplar

The best-verified example of coupling settlement generation to history, culture and NPCs — but a
settlement-scale case study presenting no single new algorithm, and its macro geography is authored,
not generated.

> **Bucklew & Grinblat — *End-to-End Procedural Generation in Caves of Qud*, GDC 2019 (slides and
> recording public)**
>
> **Confirmed** · 3–0 · ⚠️ scope: village, not world

**Already harvested.** This talk was processed in the video loop: the inversion of historical logic
(roll the ending, then find the rationalization), population tables with priority fallthrough, and
reifying abstract relations against real map objects are all now in the generator's `history.h` and
fallen-civilization pass.

### What nobody answered

> ❌ **Gap in the verified evidence.** Of 23 verified claims, **zero** address territory or border
> growth from settlement cores, procedural history simulation at civilization scale, trade route
> economics, or the derivation of chokepoints, arable land and strategic resources from physical
> geography. Nothing on Dwarf Fortress world history, Ultima Ratio Regum, Songs of Conquest, or Civ
> map generation survived. Several requested downscaling sub-topics fared the same way: wave function
> collapse with boundary constraints, dual-grid seam schemes, coordinate-deterministic seeding, and
> Genevaux's terrain-from-rivers produced no surviving claim.
>
> This is a gap in the evidence base, not evidence that the work does not exist. The round
> concentrated its verification budget on two corpora — the LIRIS graphics group (Galin, Guérin,
> Peytavie) and Barnes' hydrology papers — and spent it there.

---

## Part three — What not to build

Two claims were killed outright and two more are contraindicated. These are the expensive mistakes,
listed so they are not made twice.

### ✕ Do not burn the overview river into the detail heightmap

Subtracting elevation along the overview's river polyline is the obvious way to force a detail tile
to honour it. It is a documented failure mode in peer-reviewed hydrology.

> **Wu et al. — *High-efficient extraction of drainage networks from DEMs constrained by enhanced
> flow enforcement*, Geomorphology, 2019**
>
> *"The existing flow enforcement methods (e.g., the elevation-based stream-burning method) have
> certain limitations, as they may cause unreal longitudinal profiles, lead to unintended topological
> errors, and even misinterpret the overall drainage patterns."*
>
> **Contraindicated** · 3–0

Corroborated independently by Lindsay (2016, ESPL 41:658), which attributes the topological errors to
scale mismatch between vector hydrography and the raster DEM, and identifies erroneous stream piracy
where multiple stream links rasterize into a single cell. WhiteboxTools ships `TopologicalBreachBurn`
precisely because naive burning fails.

**The mechanism transfers directly.** Burning a coarse overview polyline into a fine tile *is* the
vector/raster scale-mismatch case, at its worst. Route the channel (stage 03) and carve a profile
along the routed path instead.

### ✕ Two claims were refuted 0–3

- **Paired-dictionary coarse-value conservation.** The appealing idea that you can sparse-code the
  coarse map against a downsampled low-res dictionary and re-evaluate the identical coefficients
  against a paired high-res dictionary — giving exactness by construction — is *not* in the Guérin
  2016 paper and was rejected unanimously. Do not build on it.
- **φ = 0.42·A^0.69.** A specific drainage-area-to-discharge power law, attributed to Riverscapes
  with a non-conservativeness rationale, did not survive. The cross-section scaling a = φ/‖u‖
  survived independently, but any drainage-area-to-discharge exponent needs fresh sourcing before
  use.

### § RichDEM is GPL-3.0

The reference C++ implementation of everything in stage 02 is real, embeddable and empirically
header-only for the parts that matter — and licensed in a way that is disqualifying for a
closed-source game.

Verification compiled `richdem/depressions/Barnes2014.hpp` and `richdem/flowmet/d8_flowdirs.hpp` with
nothing but `g++ -std=c++17 -I include` — no link step, no GDAL, no OpenMP, no CMake. But master is
not wholly header-only, and `-DRICHDEM_LOGGING` produces an undefined reference. The licence was
confirmed three ways: LICENSE.txt is GPLv3 verbatim, the GitHub SPDX id is GPL-3.0, and there is no
dual-licensing or permissive-core exception.

> ⚠️ **Practical route.** Prototype against RichDEM to get behaviour right, then reimplement
> Priority-Flood from the published pseudocode. The algorithm is short and the papers give it in
> full; it is the implementation that is encumbered, not the method.

---

## Part four — Open questions

What the round could not settle, in the order it would block implementation.

- **Coarse-value conservation.** Residual amplification over an interpolated base, a constrained
  Poisson solve (Zhou's unimplemented extension), or post-hoc projection onto the cell mean — and
  what each does at chunk boundaries. The residual route looks strongest here because the corner dual
  grid already supplies a continuous base, but nothing verified says so.
- **Locally-computable seam routing.** Given that burning is contraindicated and Barnes needs a
  global solve, what scheme routes an overview river through one chunk so it hits both mandated
  boundary points, flows monotonically downhill, and agrees with a neighbour neither chunk has
  generated? The A\* formulation supplies the routing but not the guarantee.
- **The velocity model.** What pairs with a = φ/‖u‖ so channel width and depth follow
  Leopold–Maddock, and how do floodplain and valley width come off the same discharge? No verified
  source answers this.
- **The whole kingdom layer.** Border growth from cores, trade economics, civilization history,
  strategic-value derivation. Worth a second research round scoped narrowly at shipped systems rather
  than graphics papers, since that is where the budget did not reach.
- **Staleness watch.** Only the learned-amplification finding is time-sensitive: the 2017 cGAN has
  been superseded by diffusion approaches (Lochner CGF 2023, Huftier CGF 2025, TerraFusion
  arXiv:2505.04050), none verified this round. The hydrology and least-cost results are mature; the
  coprime segment-mask result is a number-theoretic property that cannot go stale.

---

## Part five — Sources that carried findings

Primary sources behind the verified claims above. Twenty-three were fetched in total; these are the
ones whose claims survived.

| Stage | Source | Link |
|---|---|---|
| 01 | Guérin et al., *Sparse representation of terrains*, EG 2016 | [liris.cnrs.fr](https://perso.liris.cnrs.fr/eguerin/download/eg2016.pdf) · [code](https://github.com/eric-guerin/terrain-amplification) |
| 01 | Zhou et al., *Terrain Synthesis from DEMs*, TVCG 2007 | [howardzzh.com](http://www.howardzzh.com/research/terrain/) |
| 01 | Guérin et al., *Interactive Example-Based Terrain Authoring with cGANs*, SIGGRAPH Asia 2017 | [doi.org](https://doi.org/10.1145/3130800.3130804) |
| 02 | Barnes, Lehman & Mulla, Priority-Flood + flats resolution, C&G 2014 | [arXiv](https://arxiv.org/abs/1511.04463) · [arXiv](https://arxiv.org/abs/1511.04433) |
| 02 | RichDEM reference implementation (GPL-3.0) | [github.com](https://github.com/r-barnes/richdem) |
| 03 | Galin et al., *Procedural Generation of Roads*, CGF 2010 | [liris.cnrs.fr](https://perso.liris.cnrs.fr/eric.galin/Articles/2010-roads.pdf) |
| 03 | Wu et al., flow enforcement without elevation modification, Geomorphology 2019 | [sciencedirect](https://www.sciencedirect.com/science/article/pii/S0169555X19301771) |
| 03 | Lindsay, *The practice of DEM stream burning revisited*, ESPL 2016 | [doi.org](https://doi.org/10.1002/esp.3888) |
| 04 | Peytavie et al., *Procedural Riverscapes*, PG 2019 | [liris.cnrs.fr](https://perso.liris.cnrs.fr/eric.galin/Articles/2019-riverscapes.pdf) |
| 04 | Leopold & Maddock, hydraulic geometry of stream channels | [semanticscholar](https://www.semanticscholar.org/paper/The-hydraulic-geometry-of-stream-channels-and-some-Leopold-Maddock/ffaed02dacd712b44bd9e7f7d43b717197ed7922) |
| 05 | Barnes, parallel Priority-Flood and flow accumulation, 2016 / 2017 | [arXiv](https://arxiv.org/abs/1606.06204) · [arXiv](https://arxiv.org/abs/1608.04431) |
| civ | Emilien et al., *Procedural Generation of Villages on Arbitrary Terrains*, TVC 2012 | [liris.cnrs.fr](https://perso.liris.cnrs.fr/egalin/Articles/2012-villages.pdf) |
| civ | Bucklew & Grinblat, *End-to-End Procedural Generation in Caves of Qud*, GDC 2019 | [gdcvault](https://gdcvault.com/browse/gdc-19/play/1026313) |

---

## Colophon

Produced by a fan-out research harness: 5 search angles → 23 sources fetched → 114 falsifiable claims
extracted → 25 put to three-vote adversarial verification, needing 2 of 3 refutations to kill a claim
→ 18 findings after merging semantic duplicates. 105 agents, 1,018 tool calls, 20 minutes wall clock.

Evidence base is heavily concentrated in two groups — the LIRIS graphics lab (Galin, Guérin,
Peytavie) and Barnes' hydrology work. Game-industry sources are represented by a single GDC talk.
Several findings above rest on *application inference* rather than source statements: routing rivers
with the road solver, using partition-of-unity blending for chunk seams, and treating the overview
grid as Barnes' pre-solved global flow graph. Each was judged sound by verifiers; none is a published
result for the downscaling use case.

---

*Converted from the artifact "The Detail Pass"
(<https://claude.ai/code/artifact/d804ef2e-19b4-4dee-bfc2-9c06dd105cc2>) on 2026-08-31. The artifact
remains the live version; this file is a snapshot.*
