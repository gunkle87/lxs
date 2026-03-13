FUNCTIONAL COMPILATION PHASE PLAN

Purpose
This document defines the next architecture phase after the recognition and
bounded neighborhood phases reached their practical limit on small and medium
local replacements.

Phase Conclusion That Motivates This Shift
- recognition produced one admitted live family:
  - parity
- several kernels and neighborhood rewrites proved valuable in controlled form
- most deferred recognizers were bounded and measurable, but not profitable
  enough to justify live compiler admission
- larger bounded arithmetic neighborhoods proved more useful than small local
  motifs, but still often lost to boundary overhead or stronger packed paths

Interpretation
The next useful step is not broader recognizer hunting.
The next useful step is not another round of tiny stitched macro additions.

The next useful step is:

functional compilation of bounded regions

This means compiling a closed rooted logic region into one dedicated execution
object that:
- consumes only explicit boundary inputs
- emits only explicit boundary outputs
- hides its internal intermediate structure completely
- executes as a single compiled region kernel

Core Phase Goal
Increase useful absorbed work share and reduce internal net traffic by
compiling larger closed logic regions into dedicated execution forms instead of
assembling performance from many small macros or medium stitched
neighborhoods.

What Functional Compilation Means Here
Functional compilation does not mean symbolic explosion or unbounded logic
inlining.

It means:
- rooted, bounded region discovery
- explicit legality and closure
- fixed compiled execution shape
- direct evaluation of the region as one unit

The target is a region that is:
- large enough to amortize boundary cost
- small enough to remain bounded, fail-fast, and cache-aware

The new target is not:
- wider graph search
- looser legality
- global reconvergence search
- unconstrained expression growth

The new target is:
- larger closed regions
- same rootedness
- same boundedness
- same fail-fast rules
- fewer intermediate writes and handoffs

What Changes From The Current Neighborhood Model
Current weak point
- even profitable bounded neighborhoods often remain too local
- many still leave enough surrounding structure that boundary overhead wins
- the strongest current wins still come from trusted offline substitution and
  packed arithmetic paths rather than live recognition

Phase shift
- move from neighborhood kernels as larger macros
- to compiled functional regions as dedicated execution objects
- preserve the same legality discipline
- preserve telemetry
- preserve offline qualification before recognition

Functional Region Rules
- rooted at a fixed output or fixed boundary output set
- fixed maximum node budget
- fixed maximum traversal depth
- fixed maximum boundary input count
- strict abort on internal fanout leakage
- strict abort on branch growth
- strict abort on overlap ambiguity
- strict abort on boundary instability
- one compiled region replaces one closed bounded subgraph
- semantics must be explicit and closed
- no runtime allocation in the tick path
- no pointer-chasing object graphs
- no per-region dynamic interpretation at runtime

Runtime Shape
Each compiled region should become a dedicated execution object with:
- type
- input_count
- output_count
- stable input indices
- stable output indices
- gate-equivalent accounting
- fixed kernel implementation

The runtime should not reconstruct region structure dynamically.

Compiler Responsibilities
The compiler for this phase should:
- discover candidate bounded regions
- prove closure and legality
- lower them into fixed execution objects
- remove the absorbed internal primitive structure from normal execution
- preserve benchmark accounting comparability

Execution Strategy
Functional compilation should use a hybrid execution model.

Small regions
- direct boolean or fixed-expression evaluation
- intended for very small bounded cones

Larger bounded regions
- compiled gate microprogram style evaluation
- fixed temporary schedule
- fixed temporary storage

The first implementation should not attempt dynamic code generation or
unbounded symbolic flattening.

Profitability Pre-Filter
Before a candidate region becomes a compiled execution object, it should pass a
simple profitability pre-filter such as:

estimated_benefit = absorbed_gate_equiv - boundary_cost

If the estimate is not positive, the region should be skipped.

This filter is intentionally simple and may evolve later, but the phase should
not generate large numbers of kernels whose likely value is already poor.

Why This Phase Is Worth Doing
- recognition alone is not delivering enough suite-scale leverage
- local and medium macros taught us that boundary cost is a central problem
- arithmetic has already shown that larger trusted rewrites can materially win
- the next plausible payoff comes from erasing more internal structure per
  replacement, not by adding more local motifs

Phase Split
This phase is intentionally divided in two.

Phase 0
- prove the functional compilation machinery on small generic single-output
  cones
- validate bounded discovery, legality, telemetry, and runtime execution shape
- do not begin with arithmetic-specific regions

Phase 1
- apply the proven machinery to arithmetic compiled regions
- use arithmetic as the first serious payoff family

Why this split exists
- arithmetic remains the best payoff family
- but it is not the simplest first infrastructure pilot
- a small generic cone pilot reduces risk before arithmetic complexity enters
  the system

PHASE 0

Purpose
Validate the functional compilation machinery itself.

Phase 0 Region Limits
- root: single output gate
- max_depth: 3
- max_node_budget: 8
- max_boundary_input_budget: 5
- output_count: 1
- max_internal_fanout: 1

Phase 0 Region Class
- generic bounded combinational cones
- no sequential boundaries
- no multi-output regions
- no arithmetic-specific assumptions

Phase 0 Execution Forms
- tiny expression evaluation for very small cones
- fixed microprogram evaluation for slightly larger cones

Phase 0 Goals
- prove rooted bounded discovery
- prove legality separation
- prove fail-fast abort behavior
- prove telemetry and accounting
- measure whether compiled cones beat primitive execution often enough to
  justify the layer

Phase 0 Success Criteria
- bounded discovery stays predictable
- runtime execution is stable and correct
- at least one prototype cone class shows measurable benefit
- no search-path pathology appears

Phase 0 Stopping Conditions
- compile-time behavior trends toward graph search
- legality becomes hard to explain
- tiny compiled cones show no measurable benefit at all
- telemetry cannot distinguish strictness from expense

PHASE 1

Purpose
Move from proven functional-compilation machinery into the highest-payoff
family.

Pilot Family
Arithmetic

Why arithmetic remains first for payoff
- strongest controlled evidence of larger-structure payoff
- existing offline arithmetic rewrite flow already provides a trustworthy
  qualification harness
- arithmetic remains the best candidate for proving that functional
  compilation can outperform the current packed slice2 path

Phase 1 Objective
Move from bounded arithmetic neighborhoods to bounded compiled arithmetic
regions.

The first arithmetic pilot should target:
- local adder-tree or row-reduction regions in multiplier structure
- not isolated adders
- not loose wide search
- not whole multipliers

Recommended arithmetic region shape
- absorb multiple adjacent reduction and propagation steps
- export only a narrow carry or row boundary
- remain small enough for strict legality and straightforward runtime support

Examples of acceptable arithmetic directions
- bounded two-row reduction-plus-propagate region
- bounded reduction-plus-finalize window
- bounded local carry-save plus short propagate region

Examples of unacceptable arithmetic directions
- whole-adder-tree flattening
- full multiplier compilation
- open-ended carry propagation
- any region whose legality depends on global search

Non-Goals
- do not restart broad recognizer expansion first
- do not add unrelated kernel families first
- do not widen search radius to rescue weak families
- do not change sequential semantics in this phase
- do not turn the compiler into a general graph-rewrite engine

Qualification Order
1. candidate definition
2. explicit kernel implementation
3. dedicated tests
4. controlled offline substitution
5. isolated benchmark qualification
6. comparison against current best packed path
7. checkpoint and decision

Decision Outcomes
- ADMIT AS ALTERNATE REGION
- ADMIT AS PREFERRED REGION
- HOLD FOR LATER
- REJECT

Recommended First Practical Step
- define the first Phase 0 bounded generic cone candidate
- qualify the machinery there first
- then move into arithmetic compiled regions only if the prototype behaves well

Current Expected Benefit
This phase is not expected to guarantee a broad suite jump immediately.

It is expected to answer a sharper question:

Can bounded compiled regions beat stitched neighborhood execution often enough
to justify a new execution layer?

That is the right next question for LXS.
