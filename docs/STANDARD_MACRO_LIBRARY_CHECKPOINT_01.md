STANDARD MACRO LIBRARY CHECKPOINT 01

Title
- Standard Macro Library Phase
- Checkpoint 01
- `MUX2_*` and `MUX4_*` explicit family

Comparison class
- fixture-only

Corpus
- `Tests\Circuits\mux2_8_primitive.bench`
- `Tests\Circuits\mux2_8_explicit.bench`
- `Tests\Circuits\mux4_8_primitive.bench`
- `Tests\Circuits\mux4_8_explicit.bench`

Settings
- samples: `5`
- sample-select: `median`
- iterations: `1`
- cycles: `10000`

Admission status
- ADMIT

Rerun confirmation status
- not required

Minimum result block
- baseline:
  - `mux2_8_primitive`: `246,791,699.143243` GEPS
  - `mux4_8_primitive`: `407,264,713.439944` GEPS
- candidate:
  - `mux2_8_explicit`: `331,652,977.158682` GEPS
  - `mux4_8_explicit`: `671,384,518.198505` GEPS
- delta:
  - `MUX2_8`: `+84,861,278.015439` GEPS
  - `MUX4_8`: `+264,119,804.758561` GEPS

Correctness qualification
- `lxs_test.exe`: passed
- `lxs_compare.exe` primitive vs explicit:
  - `MUX2_8`: equivalent across `256` trials
  - `MUX4_8`: equivalent across `256` trials

Interpretation
- The first standard macro family is now real, explicit, and qualified. Both
  `MUX2_*` and `MUX4_*` lower cleanly to trusted primitive references, compile
  into dedicated runtime descriptors, execute as single structured units, and
  outperform the primitive fixtures in isolated A/B measurement. This is a
  library admission checkpoint, not a default-suite optimization claim.

Current decision state
- Live now:
  - `MUX2_8`
  - `MUX2_16`
  - `MUX2_32`
  - `MUX2_64`
  - `MUX4_8`
  - `MUX4_16`
  - `MUX4_32`
  - `MUX4_64`
- Not yet started:
  - register family
  - arithmetic family
  - memory family

Next step
- Start the register family from the existing plan:
  - `REG8`
  - `REG16`
  - `REG32`
  - `REG64`
  - then `REG_EN_*`
  - then `REG_EN_RST_*`
