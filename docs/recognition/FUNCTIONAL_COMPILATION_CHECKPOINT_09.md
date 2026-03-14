FUNCTIONAL COMPILATION CHECKPOINT 09

Candidate
- name: CACHE_CHAIN_CONE7
- class: structural-combinational
- status: ADMIT AS PHASE 0 PREFERRED

Purpose
This checkpoint records the default-suite replacement qualification for the
chain-local cache-aware Phase 0 candidate after report-only coverage
qualification.

Question
Can one additional strictly local fused step outperform the current
CACHE_LOCAL_CONE6 prototype without reopening wide-root or expression-heavy
failure modes?

Guard rails retained
- rooted
- single-output
- fixed depth
- fixed node budget
- strict fanout abort
- no wider search radius

Candidate summary
- op_count fixed at 4
- temp_count must not exceed 3
- root arity remains 2 or 3
- one internal temp may feed the root directly
- internal dependency graph must remain chain-like
- execution form remains microprogram-first only

Correctness
- full test suite:
  - passed
- explicit chain micro vs primitive fixture:
  - passed across all 64 input combinations
- report-only chain recognition:
  - passed

Local execution qualification
- primitive baseline:
  - 59,735,368.854513 GEPS
- explicit microprogram:
  - 27,995,127.407714 GEPS

Interpretation
- The chain-local explicit kernel is still slower than the primitive baseline
  on the isolated synthetic fixture.
- As with CACHE_LOCAL_CONE6, the decision must be made from suite behavior,
  not from the explicit synthetic alone.

Report-only coverage
- match_count: 29
- gate_equiv: 117
- step_share: 0.0102437301306959
- work_share: 0.000761441141250586
- circuits_affected: 8
- timed_out: 0
- errors: 0

Default-suite replacement qualification
- overall_off: 382,607,162.284136 GEPS
- overall_on: 404,759,690.237028 GEPS
- overall_delta: +22,152,527.952892 GEPS

Per-suite delta
- ISCAS85:
  - +18,315,025.167088
- ISCAS89:
  - +11,945,749.300656
- ITC99:
  - -4,775,663.950043
- EPFL:
  - +40,700,550.755422

Per-benchmark highlights
- notable gains:
  - ISCAS89\s27: +76,204,175.347079
  - EPFL\multiplier: +72,612,561.394976
  - ITC99\b04: +52,555,015.234024
  - EPFL\router: +29,897,601.977358
- notable losses:
  - ISCAS89\s526: -61,864,046.205987
  - EPFL\arbiter: -32,590,583.698943
  - EPFL\ctrl: -23,913,865.494733
  - ITC99\b22: -11,145,970.405100

Comparison against prior Phase 0 candidates
- GENERIC_CONE5_ROOTED
  - overall_delta: +31,492,654.038877 GEPS
  - report-only work_share: 0.000624772218462019
  - status: KEEP KERNEL, DEFER DEFAULT REPLACEMENT
- WIDE_ROOT_CONE6
  - overall_delta: -11,831,295.726704 GEPS
  - report-only work_share: 0.000800489404904462
  - status: KEEP KERNEL, DEFER DEFAULT REPLACEMENT
- CACHE_LOCAL_CONE6
  - overall_delta: +10,263,382.571474 GEPS
  - report-only work_share: 0.000761441141250586
  - status: ADMIT AS PHASE 0 DEFAULT PROTOTYPE
- CACHE_CHAIN_CONE7
  - overall_delta: +22,152,527.952892 GEPS
  - report-only work_share: 0.000761441141250586
  - status: ADMIT AS PHASE 0 PREFERRED

Interpretation
- CACHE_CHAIN_CONE7 preserves the same bounded report-only coverage as
  CACHE_LOCAL_CONE6.
- It materially improves default-suite replacement over CACHE_LOCAL_CONE6.
- It remains far better aligned with the cache-aware Phase 0 direction than
  WIDE_ROOT_CONE6.
- GENERIC_CONE5_ROOTED still has the largest raw suite delta, but it is a
  looser historical prototype and not the best architectural baseline to keep
  extending.

Decision
- ADMIT AS PHASE 0 PREFERRED

Why this is not a full default admission
- absorbed work share remains small at suite scale
- isolated explicit execution still loses on the synthetic fixture
- benchmark-local regressions remain real and must be watched

What is admitted
- CACHE_CHAIN_CONE7 is now the preferred bounded functional prototype for
  continuing Phase 0 work.
- CACHE_LOCAL_CONE6 remains a valid alternate baseline.
- GENERIC_CONE5_ROOTED remains a valid historical prototype.
- WIDE_ROOT_CONE6 remains deferred.

Next sensible move
- keep the same cache-aware guard rails
- widen expression only if it preserves chain-local temp reuse and bounded
  locality
- use CACHE_CHAIN_CONE7 as the baseline for the next Phase 0 candidate
