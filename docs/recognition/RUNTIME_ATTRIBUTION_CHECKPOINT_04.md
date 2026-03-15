RUNTIME ATTRIBUTION CHECKPOINT 04

Purpose
This checkpoint records the first admitted primitive-chunk typing optimization
from the runtime-attribution phase.

Why this pass was attempted
- checkpoint 03 closed the compiler-side boundary-reshaping branch as
  no-admission
- the remaining boundary-cost signal pointed at primitive chunk typing around
  arithmetic regions, especially the large arithmetic-adjacent `AND` slab in
  packed multiplier

Bottleneck read
- tiny unary cleanup was not the dominant cost
- the real boundary hot spot was the large 2-input `AND` chunk adjacent to
  arithmetic-heavy regions

Candidate family
- engine-side fast path for 2-input logic chunks

Matrix tested
- `q0`
  - baseline
- `q1`
  - fast path for 2-input `AND` chunks
- `q2`
  - fast path for 2-input `AND`, `OR`, and `XOR` chunks

Second matrix tested
- `r0`
  - baseline
- `r1`
  - unthresholded 2-input `AND` fast path
- `r2`
  - 2-input `AND` fast path only when `chunk->count >= 64`
- `r3`
  - 2-input `AND` fast path only when `chunk->count >= 256`

Interpretation
- the thresholded variants did not improve the suite
- the unthresholded `AND` path was the only candidate that remained positive
  through stronger validation

Packed artifact qualification
Settings
- samples: 3
- sample_select: median
- iterations: 1
- cycles: 50000

Baseline vs admitted candidate
- packed adder
  - baseline: `829,831,471.376541` GEPS
  - `AND` fast path: `818,677,598.788331` GEPS
- packed multiplier
  - baseline: `987,900,754.330924` GEPS
  - `AND` fast path: `1,008,345,934.881994` GEPS
- packed c6288
  - baseline: `788,818,808.344797` GEPS
  - `AND` fast path: `833,422,016.281271` GEPS

Meaning
- the admitted path is not a universal packed-artifact win
- it is a targeted boundary-chunk win that helps the arithmetic-heavy cases
  that actually dominate this bottleneck class

Default-suite qualification
Initial comparison
- settings:
  - samples: 3
  - sample_select: median
  - iterations: 1
  - cycles: 10000
- baseline: `521,088,889.836854` GEPS
- `AND` fast path: `525,287,599.711748` GEPS
- delta: `+4,198,709.874894`

Confirmation comparison
- settings:
  - samples: 5
  - sample_select: median
  - iterations: 1
  - cycles: 10000
- baseline: `525,463,667.361252` GEPS
- `AND` fast path: `539,171,199.310494` GEPS
- delta: `+13,707,531.949242`

Final admitted-tree rebuild rerun
- settings:
  - samples: 5
  - sample_select: median
  - iterations: 1
  - cycles: 10000
- admitted tree total GEPS after final rebuild:
  - `536,864,218.516751`

Decision
- ADMIT 2-input `AND` chunk fast path
- REJECT broader `AND/OR/XOR` fast path
- REJECT thresholded `AND` variants

Why this one earned admission
- it targets the measured boundary chunk type directly
- it survives a stronger suite rerun, not just a short artifact batch
- it is narrow enough to keep the change understandable and defensible

Validation
- rebuilt benchmark binaries for the candidate matrix
- rebuilt test binary on the admitted tree
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe) passed

Current decision state
- runtime-attribution phase remains open
- admitted wins in this phase now include:
  - engine-side mixed-level execution order
  - engine-side 2-input `AND` boundary chunk fast path
