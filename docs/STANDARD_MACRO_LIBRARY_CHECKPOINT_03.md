STANDARD MACRO LIBRARY CHECKPOINT 03

Title
- Standard Macro Library Phase
- Checkpoint 03
- `REG_EN8/16/32/64` controlled register family

Comparison class
- fixture-only

Corpus
- `Tests\Circuits\reg_en8_reference.bench`
- `Tests\Circuits\reg_en8_explicit.bench`
- `Tests\Circuits\reg_en16_reference.bench`
- `Tests\Circuits\reg_en16_explicit.bench`

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
  - `reg_en8_reference`: correctness-qualified
  - `reg_en16_reference`: correctness-qualified
- candidate:
  - `reg_en8_explicit`: correctness-qualified
  - `reg_en16_explicit`: correctness-qualified
- delta:
  - not expressed as GEPS
  - these fixtures are pure sequential descriptors with `Gate Eval = 0`, so
    fixture GEPS is not the right admission metric

Correctness qualification
- `lxs_test.exe`: passed
- `lxs_compare.exe` reference vs explicit:
  - `REG_EN8`: equivalent across `128` trials
  - `REG_EN16`: equivalent across `128` trials
- parser-backed sequential equivalence helper:
  - `REG_EN8`: passed across `64` cycles
  - `REG_EN16`: passed across `64` cycles

Interpretation
- The controlled register family is now live as fixed built-in library names.
  `REG_EN8/16/32/64` compile to the existing trusted register-enable backend and
  preserve current sequential semantics. This is a correctness and usability
  admission, not a throughput admission.

Current decision state
- Live now:
  - `REG8`
  - `REG16`
  - `REG32`
  - `REG64`
  - `REG_EN8`
  - `REG_EN16`
  - `REG_EN32`
  - `REG_EN64`
- Deferred:
  - `REG_EN_RST8`
  - `REG_EN_RST16`
  - `REG_EN_RST32`
  - `REG_EN_RST64`
  - reset semantics are not yet admitted as a fixed standard-library contract

Next step
- define the reset contract explicitly, then qualify:
  - `REG_EN_RST8`
  - `REG_EN_RST16`
  - `REG_EN_RST32`
  - `REG_EN_RST64`
