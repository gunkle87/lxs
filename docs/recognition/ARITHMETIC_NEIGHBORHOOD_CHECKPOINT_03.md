ARITHMETIC NEIGHBORHOOD CHECKPOINT 03

Purpose
This checkpoint records the first larger arithmetic neighborhood concept after
the RIPPLE_ADD4 neighborhood phase:

CARRY_SAVE_ROW4_NEIGHBORHOOD

Candidate
- name: CARRY_SAVE_ROW4_NEIGHBORHOOD
- class: arithmetic
- type: bounded local reduction neighborhood
- semantic contract:
  - four independent 3:2 column compressions
  - 12 inputs
  - 8 outputs
  - outputs are:
    - local sums for the current four columns
    - local carry terms for the next four columns

Why It Was Tried
- move beyond serial ripple widening
- absorb more multiplier-local arithmetic structure
- reduce local boundary count without extending a fragile carry chain

Explicit Qualification
- primitive reference:
  - Tests/Circuits/carry_save_row4_primitive.bench
- explicit kernel:
  - Tests/Circuits/carry_save_row4_explicit.bench

Isolated Result
- primitive: 291,050,921.827114 GEPS
- explicit: 263,155,813.594416 GEPS

Interpretation
- the standalone carry-save row kernel is not itself a local A/B winner
- by itself it does not amortize the macro boundary well enough

Controlled Offline Substitution
- target:
  - EPFL multiplier
- generated artifact:
  - Benchmarks/Generated/multiplier_macro_neighborhood.bench
- equivalence:
  - original vs neighborhood rewrite passed across 512 compare trials

Multiplier Results
- original multiplier: 457,963,779.706425 GEPS
- current packed slice2 multiplier path: 931,823,549.795413 GEPS
- carry-save neighborhood multiplier path: 710,491,481.321794 GEPS

Interpretation
- the bounded carry-save neighborhood is a real profitable arithmetic
  neighborhood at the benchmark level
- it materially improves over the original multiplier
- it does not exceed the current packed slice2 arithmetic rewrite path

Decision
- keep explicit kernel
- keep neighborhood rewrite mode
- do not replace the current packed slice2 path as the default arithmetic
  multiplier rewrite winner

Status
HOLD AS ALTERNATE NEIGHBORHOOD

Reason
- profitable and stable enough to keep
- not yet a new default winner
- useful as proof that larger local reduction neighborhoods can work even when
  the isolated kernel A/B is weaker

Next Allowed Move
- continue exploring larger arithmetic neighborhoods that:
  - compress local reduction structure
  - stay rooted, bounded, and fail-fast
  - beat the current packed slice2 path rather than only the original circuit
