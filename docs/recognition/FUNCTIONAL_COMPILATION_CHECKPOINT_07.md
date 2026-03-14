FUNCTIONAL COMPILATION CHECKPOINT 07

Candidate
- name: CACHE_LOCAL_CONE6
- class: structural-combinational
- status: ADMIT AS PHASE 0 DEFAULT PROTOTYPE

Purpose
This checkpoint records the default-suite replacement qualification for the
cache-aware bounded Phase 0 candidate after report-only qualification.

Question
Can a rooted, single-output, cache-local bounded cone outperform the earlier
functional candidates on the default suite while preserving the same Phase 0
guard rails?

Guard rails retained
- rooted
- single-output
- fixed depth
- fixed node budget
- strict fanout abort
- no wider search radius

Candidate summary
- root input count limited to 2 or 3
- total op_count fixed at 3
- temp_count must not exceed 2
- non-root ops unary or binary only
- each internal temp may be consumed only once
- execution form remains microprogram-first

Correctness
- full test suite:
  - passed
- explicit micro vs primitive fixture:
  - passed across all 32 input combinations
- report-only recognition hit on cache-local fixture:
  - passed
- wide-root negative case under cache-local matcher:
  - passed

Local execution qualification
- primitive baseline:
  - 44,333,614.340397 GEPS
- explicit microprogram:
  - 33,894,093.181293 GEPS

Interpretation
- The cache-local explicit kernel is still slower than the primitive baseline
  on the isolated synthetic fixture.
- That makes default-suite replacement qualification essential before drawing
  any usefulness conclusion.

Report-only coverage
- match_count: 29
- gate_equiv: 117
- step_share: 0.0102437301306959
- work_share: 0.000761441141250586
- circuits_affected: 8
- timed_out: 0
- errors: 0

Default-suite replacement qualification
- overall_off: 380,779,341.175246 GEPS
- overall_on: 391,042,723.746720 GEPS
- overall_delta: +10,263,382.571474 GEPS

Per-suite delta
- ISCAS85:
  - -614,209.308420
- ISCAS89:
  - +22,314,602.581102
- ITC99:
  - +15,314,697.564688
- EPFL:
  - +7,182,753.187468

Per-benchmark highlights
- notable gains:
  - ISCAS85\c432: +93,331,365.134891
  - ISCAS85\c17: +82,489,791.390165
  - ISCAS89\s382: +67,495,800.820184
  - EPFL\adder: +62,628,464.012179
- notable losses:
  - EPFL\ctrl: -18,726,292.721031
  - EPFL\router: -16,764,501.564706
  - ISCAS85\c880: -15,995,670.227468
  - ISCAS85\c499: -11,021,493.600872

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

Interpretation
- CACHE_LOCAL_CONE6 does not beat GENERIC_CONE5_ROOTED on raw suite delta.
- It does beat WIDE_ROOT_CONE6 and is the first richer bounded candidate to
  recover a positive default-suite replacement result after widening beyond the
  original rooted cone.
- Its guard rails are tighter and its intent is more architecturally aligned
  with the next cache-aware direction than GENERIC_CONE5_ROOTED.
- That makes it the right Phase 0 default prototype to carry forward even
  though it is not the numerically strongest suite delta seen so far.

Decision
- ADMIT AS PHASE 0 DEFAULT PROTOTYPE

Why this is not a full default admission
- absorbed work share remains small at suite scale
- local isolated execution still loses on the explicit synthetic fixture
- some benchmark-local regressions remain sharp

What is admitted
- CACHE_LOCAL_CONE6 is now the preferred bounded functional prototype for
  continuing Phase 0 work.
- GENERIC_CONE5_ROOTED remains a valid historical prototype.
- WIDE_ROOT_CONE6 remains deferred.

Next sensible move
- keep the same cache-aware guard rails
- widen expression only when it preserves locality and bounded temp reuse
- use CACHE_LOCAL_CONE6 as the baseline for the next Phase 0 candidate rather
  than widening WIDE_ROOT_CONE6 further
