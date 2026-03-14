FUNCTIONAL COMPILATION CHECKPOINT 11

Candidate
- name: CACHE_MERGE_CONE8
- class: structural-combinational
- status: ADMIT AS PHASE 0 ALTERNATE

Purpose
This checkpoint records the default-suite replacement qualification for the
merge-local cache-aware Phase 0 candidate after report-only coverage
qualification.

Question
Can one bounded root-adjacent side merge outperform the current
CACHE_CHAIN_CONE7 preferred prototype without reopening wide-root or
expression-heavy failure modes?

Guard rails retained
- rooted
- single-output
- fixed depth
- fixed node budget
- strict fanout abort
- no wider search radius

Candidate summary
- op_count fixed at 5
- temp_count fixed at 4 for matched regions
- one dominant internal chain
- one bounded side leaf feeding the root
- execution remains microprogram-first only

Correctness
- full test suite:
  - passed
- explicit merge micro vs primitive fixture:
  - passed across all 64 input combinations
- report-only merge recognition:
  - passed

Report-only coverage
- match_count: 29
- gate_equiv: 117
- step_share: 0.0102437301306959
- work_share: 0.000761441141250586
- circuits_affected: 8
- timed_out: 0
- errors: 0

Default-suite replacement qualification
- overall_off: 375,331,216.641643 GEPS
- overall_on: 386,073,332.772051 GEPS
- overall_delta: +10,742,116.130408 GEPS

Per-suite delta
- ISCAS85:
  - +50,235,274.713654
- ISCAS89:
  - +35,575,384.552481
- ITC99:
  - +6,845,820.427441
- EPFL:
  - +11,182,268.424891

Per-benchmark highlights
- notable gains:
  - ISCAS85\c432: +110,444,128.410368
  - ITC99\b11: +105,842,858.024686
  - ITC99\b04: +98,211,403.169847
  - ISCAS89\s526: +57,014,077.081793
- notable losses:
  - EPFL\adder: -5,801,639.314570
  - ITC99\b22: -3,671,109.325994
  - EPFL\ctrl: -1,509,266.539409

Comparison against prior Phase 0 candidates
- GENERIC_CONE5_ROOTED
  - overall_delta: +31,492,654.038877 GEPS
  - status: KEEP KERNEL, DEFER DEFAULT REPLACEMENT
- WIDE_ROOT_CONE6
  - overall_delta: -11,831,295.726704 GEPS
  - status: KEEP KERNEL, DEFER DEFAULT REPLACEMENT
- CACHE_LOCAL_CONE6
  - overall_delta: +10,263,382.571474 GEPS
  - status: ADMIT AS PHASE 0 DEFAULT PROTOTYPE
- CACHE_CHAIN_CONE7
  - overall_delta: +22,152,527.952892 GEPS
  - status: ADMIT AS PHASE 0 PREFERRED
- CACHE_MERGE_CONE8
  - overall_delta: +10,742,116.130408 GEPS
  - status: ADMIT AS PHASE 0 ALTERNATE

Interpretation
- CACHE_MERGE_CONE8 preserves the same bounded report-only coverage as the
  current cache-aware family.
- It remains positive at suite scale and therefore clears the basic usefulness
  threshold.
- It does not beat CACHE_CHAIN_CONE7 on overall suite delta.
- So it is a valid alternate cache-aware functional candidate, but not the new
  preferred Phase 0 baseline.

Decision
- ADMIT AS PHASE 0 ALTERNATE

What is admitted
- CACHE_CHAIN_CONE7 remains the preferred bounded functional prototype.
- CACHE_MERGE_CONE8 becomes a valid alternate bounded functional prototype.
- CACHE_LOCAL_CONE6 remains a valid alternate baseline.
- GENERIC_CONE5_ROOTED remains a historical prototype.
- WIDE_ROOT_CONE6 remains deferred.

Next sensible move
- stop widening Phase 0 by adding another close variant unless there is a
  clear new structural hypothesis
- either:
  - close Phase 0 as sufficiently explored
  - or define a materially different Phase 0 hypothesis rather than another
    small local variation
