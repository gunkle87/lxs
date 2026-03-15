MEASUREMENT RIGOR APPENDIX

Purpose
This appendix defines how performance claims should be described so checkpoint
results stay comparable and auditable.

Core rule
- structural improvement is supporting evidence only
- admission requires stable runtime evidence

Benchmark labeling rules

1. Always label the comparison class
- `recorded historical baseline`
- `fresh same-toolchain rerun`
- `packed artifact batch`
- `default suite`

2. Always label the corpus
- for example:
  - `Benchmarks\Benches`
  - one generated packed artifact
  - one explicit fixture

3. Always label the settings
- samples
- sample-select
- iterations
- cycles

4. Always say whether a value is:
- direct GEPS from one benchmark row
- weighted GEPS computed from total gate-eval and total time across the suite

Checkpoint discipline

1. Artifact wins are not admissions
- packed artifact improvements are useful screening evidence
- they do not justify a default-path admission on their own

2. Default-suite wins need confirmation
- if a candidate looks positive on the default suite:
  - rerun it at least once with the same settings before admission
- if the result flips sign or collapses materially:
  - do not admit it

Hard rerun rule
- any default-path admission requires at least one confirmation rerun with the
  same corpus and settings
- if the initial weighted-GEPS gain is less than either:
  - `10,000,000` weighted GEPS
  - or `2.0%`
  then one additional confirmation rerun is required before admission
- if the confirmation reruns disagree on sign:
  - do not admit the change
- if the confirmation reruns remain positive but differ materially:
  - keep the change in `HOLD FOR FURTHER QUALIFICATION` until a tighter batch is
    run

3. Prefer same-toolchain comparisons
- when comparing current work to a recent stable:
  - rebuild both from source under the same toolchain if practical

4. Historical baselines must be marked clearly
- if a number comes from an older recorded checkpoint rather than a fresh rerun:
  - say `recorded`
  - do not present it as equal in rigor to a same-toolchain rerun

Suggested admission wording

Good
- `admitted after packed-artifact screen and confirmed default-suite rerun`
- `recorded historical baseline`
- `fresh same-toolchain rerun against previous protected stable`

Bad
- `faster than baseline`
  - without saying which baseline
- `improved throughput`
  - without saying on what corpus and settings

Minimum evidence tiers

Tier 1
- fixture or artifact-only
- use for:
  - local plausibility
  - correctness-preserving exploration
- not enough for default admission

Tier 2
- one default-suite run
- use for:
  - provisional candidate screening
- still not enough if the gain is small or noisy

Tier 3
- default-suite confirmation rerun under the same settings
- use for:
  - default admission decisions

Current practical standard
- for default-path wins:
  - use default suite
  - prefer `samples=5`, `sample-select=median`, `iterations=1`, `cycles=10000`
  - confirm positive candidates before admission

Open improvement
- keep refining noise thresholds if future suite variance changes materially,
  but the current hard rerun rule is now the minimum admission floor
