STANDARD MACRO LIBRARY CHECKPOINT 04

Title
- Standard Macro Library Phase
- Checkpoint 04
- `REG_EN_RST8/16/32/64` controlled reset register family

Comparison class
- fixture-only

Corpus
- `Tests\Circuits\reg_en_rst8_reference.bench`
- `Tests\Circuits\reg_en_rst8_explicit.bench`
- `Tests\Circuits\reg_en_rst16_reference.bench`
- `Tests\Circuits\reg_en_rst16_explicit.bench`

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
  - `reg_en_rst8_reference`: correctness-qualified
  - `reg_en_rst16_reference`: correctness-qualified
- candidate:
  - `reg_en_rst8_explicit`: correctness-qualified
  - `reg_en_rst16_explicit`: correctness-qualified
- delta:
  - not expressed as GEPS
  - these fixtures are pure sequential descriptors with `Gate Eval = 0`, so
    fixture GEPS is not the right admission metric

Reset contract
- synchronous
- active-high
- reset value is all-zero
- reset takes precedence over enable
- when reset is not asserted, enable semantics follow the admitted `REG_EN_*`
  family behavior
- unknown reset participates in normal 4-state control propagation rather than
  being silently coerced

Correctness qualification
- `lxs_test.exe`: passed
- `lxs_compare.exe` reference vs explicit:
  - `REG_EN_RST8`: equivalent across `128` trials
  - `REG_EN_RST16`: equivalent across `128` trials
- parser-backed sequential equivalence helper:
  - `REG_EN_RST8`: passed across `64` cycles
  - `REG_EN_RST16`: passed across `64` cycles

Interpretation
- The controlled reset register family is now live as fixed built-in library
  names. `REG_EN_RST8/16/32/64` compile to a dedicated register backend mode
  and are qualified against a trusted generic `REGISTER_EN_RST` form. This is a
  correctness and usability admission, not a throughput admission.

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
  - `REG_EN_RST8`
  - `REG_EN_RST16`
  - `REG_EN_RST32`
  - `REG_EN_RST64`

Next step
- move to the next Tier 1 library family:
  - `ADD8`
  - `ADD16`
  - `ADD32`
  - `ADD64`
