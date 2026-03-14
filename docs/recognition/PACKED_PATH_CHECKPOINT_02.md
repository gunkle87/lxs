PACKED PATH CHECKPOINT 02

Purpose
This checkpoint records the second packed-path optimization pass, starting from
the multiplier side.

Objective
1. profile the packed multiplier against its strongest alternate
2. isolate one concrete multiplier packed-path bottleneck
3. implement one narrow rewrite change
4. re-measure before doing anything broader

Baseline profile

Packed multiplier before optimization
- packed:
  - `574,796,345.768477` GEPS
  - `chunks`: `98`
  - `primitive_steps`: `98`
  - `macro_steps`: `2079`
  - `total_gate_equiv`: `24193`
  - `unary_gates`: `128`
  - `buf`: `128`
  - `tiny_chunks`: `97`
  - `single_chunk_levels`: `96`

Strongest alternate on the same current line
- neighborhood:
  - `451,462,775.470927` GEPS
  - `chunks`: `2`
  - `macro_steps`: `3848`
  - `total_gate_equiv`: `24267`
  - `buf`: `0`

Interpretation
- packed multiplier was still the arithmetic winner
- the concrete asymmetry was not macro coverage
- the concrete asymmetry was boundary staging:
  - packed still emitted a trailing `BUF` output layer
  - that layer materialized `128` unary gates and `97` tiny primitive chunks
  - the neighborhood alternate had no such output staging layer

Targeted optimization
- changed packed multiplier rewrite generation so the final accumulation row
  writes directly to product outputs where no further internal use exists
- kept the change narrow:
  - generator-only
  - no runtime change
  - no recognition change
  - no structural widening

Correctness
- regenerated packed multiplier artifact:
  - equivalent to original multiplier across `512` trials
- full test suite:
  - passed

Packed multiplier after optimization
- packed:
  - `612,931,047.465202` GEPS
  - `chunks`: `65`
  - `primitive_steps`: `65`
  - `macro_steps`: `2079`
  - `total_gate_equiv`: `24128`
  - `unary_gates`: `63`
  - `buf`: `63`
  - `tiny_chunks`: `64`
  - `single_chunk_levels`: `63`

Original circuit reference
- original EPFL multiplier:
  - `249,690,896.520770` GEPS

Interpretation
- the multiplier packed-path bottleneck was real
- the narrow boundary change removed roughly half of the trailing output staging
- macro coverage stayed unchanged
- packed multiplier throughput improved by about `38.13M` GEPS on the same
  measurement command
- packed remains the strongest arithmetic path in this comparison set

Net result
- the second packed-path optimization pass succeeded
- the packed-path direction remains validated
- the change stayed narrow and measurable

Decision
- KEEP OPTIMIZING PACKED PATH

Next sensible move
- only pursue another packed-path pass if a similarly concrete bottleneck can be
  isolated first
- avoid broader structural changes until another narrow multiplier or shared
  packed-path bottleneck is identified
