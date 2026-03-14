PACKED PATH CHECKPOINT 07

Purpose
This checkpoint records the next packed-path subphase after checkpoint 06,
focused on level-local arithmetic macro ordering and engine dispatch behavior.

Objective
1. inspect packed arithmetic level-local ordering and engine dispatch behavior
2. test multiple candidates in one pass
3. keep only a candidate that improves throughput honestly
4. reject anything invalid or mixed enough to weaken the packed path

Starting point
- current protected baseline was checkpoint 06:
  - compiler-side arithmetic unit-level scheduling
  - no packed artifact change
  - packed multiplier `level_count` reduced from `562` to `436`

Hypothesis
- after checkpoint 06, the next headroom might come from:
  - level-local ordering of arithmetic multi macros
  - runtime dispatch overhead inside `lxs_execute_multi_macros`
- not from generator reshaping

Candidate A
- compiler-only
- reorder arithmetic multi macros within a level by macro type before output
  order
- intent:
  - group arithmetic types contiguously
  - improve dispatch predictability

Candidate A outcome
- same-batch measurement looked promising on packed artifacts:
  - adder improved
  - multiplier improved
- but full tests failed
- failing fixtures:
  - `ripple_slice2_explicit`
  - `ripple_add4_explicit`

Interpretation of Candidate A failure
- the reordered arithmetic multi macros were not universally independent within a
  level
- explicit arithmetic macro tests proved this ordering can violate real
  dependencies
- conclusion:
  - level-local arithmetic type grouping is not a safe compiler policy in the
    current plan model

Decision on Candidate A
- REJECT AS INVALID

Candidate B
- engine-only
- keep compiler ordering unchanged
- add fast-path dispatch entry for:
  - `RIPPLE_SLICE2`
  - `RIPPLE_ADD4`
- intent:
  - reduce dispatch overhead in the packed arithmetic hot path
  - keep artifact and compiler ordering unchanged

Candidate B correctness
- full tests passed

Same-batch comparison for Candidate B

Packed adder
- baseline:
  - `550,137,127.405502` GEPS
- Candidate B:
  - `444,699,162.082429` GEPS

Packed multiplier
- baseline:
  - `708,681,959.503848` GEPS
- Candidate B:
  - `735,632,049.944469` GEPS

Interpretation of Candidate B
- the engine-only dispatch change was real:
  - multiplier improved by about `26.95M` GEPS
- but it hurt the packed adder by about `105.44M` GEPS
- conclusion:
  - this dispatch specialization is too mixed to keep
  - it is not a net packed-path winner

Decision on Candidate B
- REJECT AS MIXED / NOT WORTH KEEPING

Net result of checkpoint 07
- no new packed-path change was admitted
- checkpoint 06 remains the current packed-path winner
- this pass still added useful constraints:
  - arithmetic macro reordering within a level is not automatically safe
  - engine dispatch specialization for packed arithmetic must be judged across
    both adder and multiplier, not just the heavier multiplier

Current kept state
- compiler-side arithmetic unit-level scheduling from checkpoint 06
- no new engine dispatch change
- no new compiler ordering change

Next sensible move
- if packed-path work continues, target a bottleneck that:
  - preserves correctness without assuming same-level arithmetic independence
  - helps both packed adder and packed multiplier
  - correlates with measured execution cost, not just branch-count intuition
