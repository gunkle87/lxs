CHECKPOINT TEMPLATE

Purpose
Use this short template for future checkpoint notes so every performance claim
states the same minimum facts.

Template

Title
- checkpoint name and phase

Comparison class
- one of:
  - historical recorded
  - same-toolchain rerun
  - default-suite screening
  - default-suite admission
  - artifact-only
  - fixture-only

Corpus
- suite root:
  - example:
    - `Benchmarks\Benches`
- specific artifacts or suites if narrower than the full default suite

Settings
- samples:
- sample-select:
- iterations:
- cycles:

Admission status
- one of:
  - ADMIT
  - KEEP ALTERNATE
  - KEEP KERNEL, DEFER
  - REJECT CHANGE
  - HOLD FOR FURTHER QUALIFICATION

Rerun confirmation status
- one of:
  - not required
  - required and completed
  - required and pending

Minimum result block
- baseline:
- candidate:
- delta:

Interpretation
- one short paragraph explaining:
  - what changed
  - why the result is trustworthy or not yet trustworthy

Current decision state
- one short block stating:
  - what is live now
  - what is explicitly not admitted

Usage notes
- if the checkpoint is artifact-only, say so plainly
- if the baseline is historical rather than rerun, say so plainly
- if a confirmation rerun was required for admission, record it explicitly
- if the checkpoint admits a new default-path win, also update:
  - [ADMISSION_UPDATE_PROTOCOL.md](/c:/DEV/LXS/docs/recognition/ADMISSION_UPDATE_PROTOCOL.md)
