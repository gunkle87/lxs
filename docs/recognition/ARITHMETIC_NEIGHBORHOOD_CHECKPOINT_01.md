ARITHMETIC NEIGHBORHOOD CHECKPOINT 01

Purpose
This checkpoint records the first explicit neighborhood-scale arithmetic
qualification pass after the compare family was closed for the current
architecture.

Candidate
- RIPPLE_ADD4_NEIGHBORHOOD

Status
- KEEP KERNEL
- KEEP REWRITE MODE
- DO NOT REPLACE EXISTING RIPPLE_SLICE2 PACKED PATH

Why
- The explicit 4-bit neighborhood kernel is correct.
- Controlled offline substitution on EPFL adder is profitable.
- The new neighborhood does not beat the current trusted packed
  RIPPLE_SLICE2 arithmetic rewrite, so it is not the new default arithmetic
  rewrite path.

Implementation Surface
- explicit kernel:
  - RIPPLE_ADD4
- parser support:
  - 9 inputs
  - 5 outputs
- runtime support:
  - one multi-output arithmetic neighborhood step
- offline rewrite mode:
  - vector-adder neighborhood

Validation
- dedicated explicit fixture:
  - Tests/Circuits/ripple_add4_explicit.bench
- primitive reference fixture:
  - Tests/Circuits/ripple_add4_primitive.bench
- parser-backed test:
  - passes
- controlled rewrite target:
  - Benchmarks/Benches/EPFL/adder.bench

Isolated Kernel Qualification
- primitive ripple_add4:
  - 142748042.227216 GEPS
- explicit RIPPLE_ADD4:
  - 274506572.021790 GEPS

Controlled Offline Substitution
- original EPFL adder:
  - 284160680.868892 GEPS
- anchor:
  - 536625964.037333 GEPS
- current packed slice2 path:
  - 597256911.713746 GEPS
- RIPPLE_ADD4 neighborhood path:
  - 568913025.072882 GEPS

Interpretation
- RIPPLE_ADD4_NEIGHBORHOOD is a real and profitable arithmetic neighborhood.
- It materially improves over the original adder.
- It also improves over the HA/FA anchor.
- It does not improve over the current trusted packed RIPPLE_SLICE2 path on
  the same benchmark.

Decision
- The kernel is valid and worth keeping.
- The rewrite mode is valid and worth keeping as an alternate arithmetic
  neighborhood path.
- The current arithmetic default remains the existing packed RIPPLE_SLICE2
  substitution flow.

Next Allowed Move
- define the next larger bounded arithmetic neighborhood candidate
- likely:
  - RIPPLE_ADD8_NEIGHBORHOOD
- or stop here and treat RIPPLE_ADD4 as the current arithmetic neighborhood
  ceiling for the present execution model
