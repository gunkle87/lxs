RUNTIME ATTRIBUTION CHECKPOINT 03

Purpose
This checkpoint records a compiler-side candidate campaign against the same
runtime bottleneck class:
- primitive-boundary execution plus level and phase transition overhead around
  arithmetic-heavy packed regions

Why this pass was attempted
- checkpoint 02 already improved mixed-level execution order in the engine
- the next plausible lever was compiler-side boundary formation around packed
  arithmetic

Candidate family A
- shared-level admission for primitive-fed arithmetic source multi macros
- goal:
  - collapse a leading primitive arithmetic boundary when the consuming
    arithmetic macro does not depend on other arithmetic macro outputs

Candidate family B
- trailing unary boundary collapse
- goal:
  - move `BUF` and `NOT` gates fed directly by arithmetic source multi outputs
    onto the same level as their arithmetic producers

Combined candidate family
- apply both A and B together

Matrix tested
- `p0`
  - baseline
- `p1`
  - family A only
- `p2`
  - family B only
- `p3`
  - family A + family B

Packed artifact qualification
Settings
- samples: 3
- sample_select: median
- iterations: 1
- cycles: 50000

Packed adder
- `p0`: `794,168,006.363875` GEPS
- `p1`: `789,265,010.709213` GEPS
- `p2`: `805,005,349.949221` GEPS
- `p3`: `810,676,137.026800` GEPS

Packed multiplier
- `p0`: `972,620,381.537525` GEPS
- `p1`: `968,954,735.255412` GEPS
- `p2`: `980,451,994.541507` GEPS
- `p3`: `982,900,833.341308` GEPS

Packed c6288
- `p0`: `798,853,111.034850` GEPS
- `p1`: `798,073,546.762255` GEPS
- `p2`: `801,668,092.685557` GEPS
- `p3`: `810,570,605.722670` GEPS

What looked promising
- family B and the combined candidate improved the packed artifacts
- the combined candidate especially improved:
  - packed adder
  - packed multiplier
  - packed c6288

Structural effect
Packed multiplier struct-profile
- baseline `p0`
  - `primitive_only_levels = 3`
  - `phase_transitions = 123`
  - `arith_adj_prim_gate_equiv = 4099`
- combined `p3`
  - `primitive_only_levels = 1`
  - `phase_transitions = 122`
  - `arith_adj_prim_gate_equiv = 2`

Interpretation
- the compiler-side candidates were structurally meaningful
- they did reduce arithmetic-adjacent primitive residue
- but that structural improvement did not carry through to the default suite

Default-suite qualification
Settings
- samples: 3
- sample_select: median
- iterations: 1
- cycles: 10000

Results
- `p0`: `518,011,817.644023` GEPS
- `p1`: `520,087,375.282350` GEPS
- `p2`: `517,107,364.327675` GEPS
- `p3`: `515,863,859.735446` GEPS

Same-toolchain comparison note
- an earlier single-family compiler-side pass looked positive on one suite run
- a stricter same-toolchain rerun did not confirm that win
- this checkpoint therefore records only the final validated result:
  - no compiler-side admission from this campaign

Decision
- REJECT `p1`
- REJECT `p2`
- REJECT `p3`
- KEEP baseline `p0`

Why the pass was rejected
- the packed artifact gains were real
- but the default-suite gains were not
- the combined candidate was actually worse than baseline at suite scale
- this is another case where structural compression alone was not a reliable
  admission signal

Net result
- no new compiler-side runtime-attribution change is admitted from this pass
- the kept baseline remains:
  - checkpoint 02 engine-side mixed-level execution order
  - no additional compiler-side boundary reshaping

Current decision state
- runtime-attribution phase remains open
- this candidate branch is closed as no-admission
