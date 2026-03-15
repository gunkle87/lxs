CORPUS PROVENANCE

Purpose
This file is the compact source-of-truth for benchmark roots, default-suite
membership, known exclusions, and the canonical settings used in current
weighted-GEPS comparisons.

Benchmark roots

1. Default runnable suite
- root:
  - `Benchmarks\Benches`
- role:
  - canonical default regression and weighted-GEPS benchmark path
- current suite directories:
  - `ISCAS85`
  - `ISCAS89`
  - `ITC99`
  - `EPFL`

2. Extended research corpus
- root:
  - `BenchSuites`
- role:
  - extended corpus for recognition census, reporting-only scans, and research
    sweeps
- current suite directories:
  - `ISCAS85`
  - `ISCAS89`
  - `ITC99`
  - `EPFL`

3. Staged source corpus
- root:
  - `Benchmarks\Sources`
- role:
  - source-only staging area for corpora that are not yet part of the live
    BENCH benchmark path
- current staged families:
  - `EPFL\benchmarks`
  - `IWLS2005\IWLS_benchmarks_2005_V_1.0`

Default-suite membership

Current default suite
- root:
  - `Benchmarks\Benches`
- members:
  - `ISCAS85`
  - `ISCAS89`
  - `ITC99`
  - `EPFL`

Default-suite intent
- keep the accepted regression path stable
- use only runnable LXS BENCH-dialect artifacts
- reserve wider corpus expansion for `BenchSuites` or staged sources until a
  path is trusted and benchmark-stable

Known exclusions

Excluded from default weighted-GEPS runs
- `BenchSuites\**`
  - reason:
    - extended corpus only
- `Benchmarks\Sources\**`
  - reason:
    - staged source inputs, not the live BENCH benchmark path
- `Benchmarks\Sources\IWLS2005\**`
  - reason:
    - staged only until a compatible source/parser path exists

Current practical exclusion / limitation
- full EPFL source expansion from staged BLIF sources
  - reason:
    - blocked by the current standalone `net2bench` BLIF acceptance boundary
  - implication:
    - the repo uses the already trusted converted EPFL BENCH subset, not a full
      live source-to-BENCH conversion flow

Canonical default-suite settings

Primary weighted-GEPS comparison settings
- suite root:
  - `Benchmarks\Benches`
- samples:
  - `5`
- sample-select:
  - `median`
- iterations:
  - `1`
- cycles:
  - `10000`

Secondary shorter validation settings
- use only for exploratory screening, not headline default admissions:
  - `samples=3`
  - `sample-select=median`
  - `iterations=1`
  - `cycles=10000`

Artifact-local qualification settings
- use only for artifact or fixture qualification, not default-path admission:
  - `samples=3` or `5`
  - `sample-select=median`
  - `iterations=1`
  - `cycles=50000` or `100000`

Usage rule
- when a checkpoint claims a default-path win, it should name:
  - the suite root
  - the exact settings
  - whether the comparison is:
    - historical recorded
    - same-toolchain rerun
    - fixture or artifact only

Primary references
- [Benchmarks/README.md](/c:/DEV/LXS/Benchmarks/README.md)
- [BenchSuites/README.md](/c:/DEV/LXS/BenchSuites/README.md)
- [MEASUREMENT_RIGOR_APPENDIX.md](/c:/DEV/LXS/docs/recognition/MEASUREMENT_RIGOR_APPENDIX.md)
