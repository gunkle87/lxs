FUNCTIONAL COMPILATION CHECKPOINT 10

Candidate
- name: CACHE_MERGE_CONE8
- class: structural-combinational
- status: HOLD FOR REPLACEMENT QUALIFICATION

Purpose
This checkpoint records the report-only suite qualification for the first
merge-local cache-aware functional candidate after CACHE_CHAIN_CONE7 was
admitted as the preferred Phase 0 prototype.

Question
Can one bounded root-adjacent side merge preserve the coverage and boundedness
of CACHE_CHAIN_CONE7 while creating a better opportunity for replacement
profitability?

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

Report-only default-suite coverage
- match_count: 29
- gate_equiv: 117
- step_share: 0.0102437301306959
- work_share: 0.000761441141250586
- circuits_affected: 8
- timed_out: 0
- errors: 0

Comparison against prior cache-aware candidates
- CACHE_LOCAL_CONE6
  - match_count: 29
  - gate_equiv: 117
  - step_share: 0.0102437301306959
  - work_share: 0.000761441141250586
  - circuits_affected: 8
- CACHE_CHAIN_CONE7
  - match_count: 29
  - gate_equiv: 117
  - step_share: 0.0102437301306959
  - work_share: 0.000761441141250586
  - circuits_affected: 8
- CACHE_MERGE_CONE8
  - match_count: 29
  - gate_equiv: 117
  - step_share: 0.0102437301306959
  - work_share: 0.000761441141250586
  - circuits_affected: 8

Interpretation
- Candidate 04 preserves the same nontrivial default-suite visibility as the
  prior cache-aware candidates.
- It does not broaden report-only coverage beyond the current Phase 0
  preferred baseline.
- That is still acceptable for this step because the candidate goal is not
  wider reach; it is better replacement quality from one additional bounded
  root-adjacent merge.

Decision
- HOLD FOR REPLACEMENT QUALIFICATION

Next sensible move
- run default-suite replacement qualification for CACHE_MERGE_CONE8
- compare directly against:
  - CACHE_CHAIN_CONE7
  - CACHE_LOCAL_CONE6
  - GENERIC_CONE5_ROOTED
  - WIDE_ROOT_CONE6
