STANDARD MACRO LIBRARY CHECKPOINT 02

Title
- Standard Macro Library Phase
- Checkpoint 02
- `REG8/16/32/64` fixed-name family

Comparison class
- fixture-only

Corpus
- `Tests\Circuits\reg8_reference.bench`
- `Tests\Circuits\reg8_explicit.bench`
- `Tests\Circuits\reg16_reference.bench`
- `Tests\Circuits\reg16_explicit.bench`

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
  - `reg8_reference`: correctness-qualified
  - `reg16_reference`: correctness-qualified
- candidate:
  - `reg8_explicit`: correctness-qualified
  - `reg16_explicit`: correctness-qualified
- delta:
  - not expressed as GEPS
  - these fixtures are pure sequential descriptors with `Gate Eval = 0`, so
    the reported fixture GEPS is not a meaningful performance admission signal

Correctness qualification
- `lxs_test.exe`: passed
- `lxs_compare.exe` reference vs explicit:
  - `REG8`: equivalent across `128` trials
  - `REG16`: equivalent across `128` trials
- parser-backed sequential equivalence helper:
  - `REG8`: passed across `64` cycles
  - `REG16`: passed across `64` cycles

Interpretation
- The plain register family is now available as fixed built-in library names
  without introducing a second sequential backend. `REG8/16/32/64` are admitted
  as explicit library aliases over the existing trusted register descriptor and
  runtime semantics. This is a correctness and usability admission, not a new
  throughput win.

Current decision state
- Live now:
  - `REG8`
  - `REG16`
  - `REG32`
  - `REG64`
- Not yet admitted:
  - `REG_EN8/16/32/64`
  - `REG_EN_RST8/16/32/64`

Next step
- extend the same family discipline to:
  - `REG_EN8`
  - `REG_EN16`
  - `REG_EN32`
  - `REG_EN64`
