ENGINE HOT-PATH CHECKPOINT 01

Purpose
This checkpoint records the engine-side arithmetic hot-loop campaign for the
packed baseline.

Baseline
- protected starting point:
  - commit `fc4ea34`
- baseline packed artifact measurements gathered at current head:
  - `adder_macro_packed`: `373,179,511.294432` GEPS
  - `multiplier_macro_packed`: `837,663,529.822608` GEPS
  - `c6288_macro_packed`: `434,619,345.520082` GEPS

Targeted bottleneck class
- engine-side arithmetic multi-macro execution
- specifically repeated input-rail fetches inside `RIPPLE_SLICE2` and related
  arithmetic kernels in [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)

Candidate family

Candidate A
- cache `RIPPLE_SLICE2` input rails locally before evaluation

Candidate B
- Candidate A plus cached `RIPPLE_ADD4`

Candidate C
- Candidate B plus cached `HALF_ADDER` and `FULL_ADDER`

Artifact batch results
- baseline:
  - adder: `454,361,154.228209`
  - multiplier: `801,331,842.211797`
  - c6288: `555,898,677.501060`
- Candidate A:
  - adder: `528,357,373.369248`
  - multiplier: `815,619,354.161972`
  - c6288: `524,099,096.817196`
- Candidate B:
  - adder: `538,575,352.408780`
  - multiplier: `765,599,211.861561`
  - c6288: `526,465,968.670380`
- Candidate C:
  - adder: `492,764,957.045160`
  - multiplier: `795,057,095.360669`
  - c6288: `530,388,968.455590`

Interpretation
- Candidate A was the only credible local winner
- `RIPPLE_ADD4` caching and broader arithmetic caching did not hold up
- the first useful signal was:
  - cache `RIPPLE_SLICE2`
  - keep everything else unchanged

Thresholded follow-up
Because Candidate A hurt smaller arithmetic cases, the cached `RIPPLE_SLICE2`
path was retried with activation thresholds based on arithmetic batch size.

Artifact-level threshold results
- threshold `128`:
  - adder: `556,160,985.542802`
  - multiplier: `779,816,103.393140`
  - c6288: `570,623,009.582579`
- threshold `256`:
  - adder: `536,596,106.881640`
  - multiplier: `793,479,365.713178`
  - c6288: `552,395,967.174914`
- threshold `512`:
  - adder: `582,861,865.630764`
  - multiplier: `751,285,979.325441`
  - c6288: `694,970,933.037325`

Suite-level validation

Serial baseline
- [lxs_bench_hot0.exe](/c:/DEV/LXS/build/bin/lxs_bench_hot0.exe)
- total GEPS:
  - `426,217,507.246840`

Serial candidate threshold `256`
- [lxs_bench_thr256.exe](/c:/DEV/LXS/build/bin/lxs_bench_thr256.exe)
- total GEPS:
  - `426,229,095.839523`

Interpretation
- threshold `256` was the best suite-level version of the engine candidate
- but the delta was only about `+11,589` GEPS on a `426M` GEPS suite run
- that is too small to count as a credible new default win

Decision
- REJECT engine-side `RIPPLE_SLICE2` caching as a new default change
- KEEP the packed baseline unchanged

Why it was rejected
- it produced local wins on packed artifacts
- it did not produce a trustworthy suite-level admission signal
- the best thresholded version only tied the suite baseline within noise
