## BenchSuites

This directory stores extended benchmark corpora outside the default
`Benchmarks\Benches` regression path.

Purpose:

- keep the accepted default benchmark run stable
- provide wider structural coverage for research, census, and reporting-only
  recognition scans
- preserve trusted source-derived BENCH corpora in one place

Current suite layout:

- `ISCAS85`
- `ISCAS89`
- `ITC99`
- `EPFL`

Notes:

- Files in this directory are not part of the default benchmark sweep.
- `ISCAS85`, `ISCAS89`, and `ITC99` are sourced from the trusted BENCH library
  mirror at pld.ttu.ee.
- `EPFL` currently contains the trusted converted BENCH subset already used by
  the repo.
- Full EPFL corpus expansion is blocked for now by the current Phase 1
  `net2bench` BLIF acceptance boundary.
- `tools\batch_net2bench.ps1` is the retained batch conversion helper for any
  corpus that the standalone converter can accept cleanly.
