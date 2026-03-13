# RECOGNITION_PHASE2_PLAN.md

Recognition Phase 2 planning document.

This document defines the next recognition phase after Recognition Snapshot R1.
It exists to prevent drift back into ad hoc recognizer work and to keep future
recognition changes aligned with measured absorbed-work goals.

------------------------------------------------------------------------

## 1. Phase 2 Entry Point

Recognition Snapshot R1 established the current state:

- overall recognition is beneficial
- parity is the only admitted recognizer doing measurable live work
- parity is plan-dominant, not work-dominant
- all other families remain deferred, explicit-only, or offline-only

R1 conclusion:

The recognition vocabulary is useful, but still narrow.

This means Phase 2 must not focus on adding more hand-coded family recognizers
one by one. The next goal is to increase structural coverage and absorbed-work
share without losing measurement discipline.

------------------------------------------------------------------------

## 2. Phase 2 Objective

Phase 2 objective:

Increase recognized absorbed work share materially while preserving:

- kernel correctness
- legality boundaries
- suite stability
- benchmark comparability
- recognizer maintainability

The target is not more recognizers by count.
The target is broader useful coverage.

------------------------------------------------------------------------

## 3. Phase 2 Non-Goals

Phase 2 is not allowed to become:

- a benchmark-specific pattern hunt
- a new kernel-design phase
- a parser redesign phase
- a scheduling-semantics redesign phase
- a broad heuristic matcher with unclear legality

Kernel semantics remain fixed.
The current BENCH-only engine path remains fixed.
Recognition remains strictly structural and legality-driven.

------------------------------------------------------------------------

## 4. What R1 Proved

R1 proved:

- recognition value is real
- parity can improve the suite under strict protocol control
- the present family-by-family hardcoded recognizer model is too narrow to
  produce dominant absorbed-work coverage
- plan compression and throughput do not correlate automatically
- absorbed-work share is now the critical metric, not match count alone

This is the basis for Phase 2.

------------------------------------------------------------------------

## 5. Phase 2 Architectural Direction

Phase 2 should move recognition from family-specific hardcoded structural
checks toward a bounded template-driven graph recognizer.

The design target is:

- graph-based pattern definitions
- legality rules kept separate from structural pattern description
- conflict precedence rules defined explicitly
- recognizer-on, recognizer-off, and reporting-only modes
- bounded search size and bounded width
- direct mapping from pattern to existing macro kernel

The recognition system must remain:

- deterministic
- bounded
- measurable
- disableable by family

Phase 2 rule:

Widen expression, not search radius.

Matcher expressiveness may increase, but only under:
- fixed root or output anchoring
- fixed max node budget
- fixed traversal depth
- strict abort on fanout growth
- strict abort on branch growth
- strict abort on overlap ambiguity

------------------------------------------------------------------------

## 6. Required Phase 2 Capabilities

Phase 2 should add the following capabilities.

### 6.1 Pattern Form

Patterns should be represented as data structures rather than only as handwritten
recognizer code.

Minimum requirement:

- pattern node types
- input and output roles
- exact edge connectivity
- bounded size
- macro target

External file loading is desirable later, but not required for the first Phase 2
implementation.

### 6.2 Legality Separation

Legality must be kept separate from pattern shape.

Examples:

- fanout rules
- boundary rules
- output-only constraints
- overlapping-family exclusions
- negative space definitions

Pattern structure should answer:
- what shape is this?

Legality should answer:
- when is replacement allowed?

### 6.3 Conflict Precedence

The recognizer must support explicit precedence between families.

Examples:

- parity before compare/equality where both could touch XOR-derived structure
- arithmetic before fanout if the arithmetic slice consumes the same local nodes
- deferred families excluded from competition entirely

Precedence must be explicit, not accidental by source order alone.

### 6.4 Reporting-Only Mode

Recognition must support a reporting-only pass.

Reporting-only mode should:

- detect matches
- count them
- estimate node reduction
- estimate gate-equivalent absorbed work
- perform no replacement

This is required to separate:

- structural availability
- from replacement profitability

### 6.5 Coverage Metrics

Phase 2 reporting must include:

- macro_step_share
- absorbed_work_share
- family_absorbed_work_share
- family_step_share
- recognition_delta
- per-suite delta
- per-benchmark delta

Coverage should also include:

- percentage of total gate-equivalent work touched by each family
- percentage of plan steps replaced by each family

Phase 2 matcher telemetry must also include:

- candidate_roots
- nodes_visited
- max_depth_reached
- abort_reason
- time_per_family

Without this telemetry, zero-match outcomes are not interpretable.

### 6.6 Stress-Pack Gate

Before any full BenchSuites census, each deferred-family matcher must clear a
small heavy-bench stress pack within a fixed time budget.

Minimum stress pack:
- EPFL multiplier
- ISCAS89 s38417
- ISCAS89 s38584
- one late ITC99 benchmark

If a matcher cannot fail fast on this set, it is not corpus-ready.

------------------------------------------------------------------------

## 7. Phase 2 Work Order

Phase 2 should proceed in this order.

### Step 1
Build template-capable recognition infrastructure.

Deliverables:

- in-memory pattern representation
- legality representation
- precedence representation
- reporting-only mode

No new family should be admitted at this step.

### Step 2
Port the admitted parity recognizer into the new infrastructure.

Purpose:

- prove the new recognition path can reproduce the one currently admitted family
- compare old recognizer and new recognizer on:
  - match count
  - node reduction
  - absorbed work share
  - suite delta

Phase 2 does not continue until parity parity is preserved under the new path.

### Step 3
Port one deferred family into reporting-only mode first.

Recommended order:

1. compare/equality
2. shared XOR
3. arithmetic

Reason:

- compare/equality already has a valid kernel and a known structural foothold
- it conflicts with parity, which makes it a good precedence test
- it is the clearest current example of a family blocked by matcher
  expressiveness rather than kernel value

Before Step 3 may proceed, the compare/equality matcher must first pass the
stress-pack gate with telemetry enabled.

### Step 4
Enable one deferred family for true replacement only after reporting-only metrics
show meaningful structural coverage and legality confidence.

### Step 5
Re-run full-suite qualification and issue a new recognition snapshot.

------------------------------------------------------------------------

## 8. Recommended First Deferred Family

The recommended first deferred family for Phase 2 is:

compare/equality

Why:

- it already has a valid kernel
- it already has an explicit-only foothold
- wide-gate support removed part of the earlier ingestion barrier
- it is a strong test of precedence with parity
- it is structurally meaningful without needing broad global search

Shared XOR is second.
Arithmetic is third.
Control remains deferred until correctness is fully resolved.

------------------------------------------------------------------------

## 9. Phase 2 Success Criteria

Phase 2 is successful if all are true:

- the new recognition infrastructure reproduces admitted parity behavior
- reporting-only mode works and produces stable coverage metrics
- at least one deferred family becomes measurable under the new system without
  false positives
- absorbed_work_share increases materially over R1
- recognizer complexity remains proportional to achieved coverage
- legality and conflict handling are explicit and documented

Material progress should be judged more by:

- absorbed_work_share
- family coverage breadth
- suite stability

than by raw number of recognizers implemented.

------------------------------------------------------------------------

## 10. Phase 2 Stopping Conditions

Stop Phase 2 immediately if:

- pattern complexity grows faster than coverage
- legality rules become family-specific special cases instead of reusable rules
- precedence becomes ambiguous
- reporting-only mode cannot predict useful replacement behavior
- parity cannot be reproduced faithfully under the new infrastructure
- a deferred-family matcher cannot fail fast on the stress pack
- matcher telemetry does not explain zero-match corpus outcomes

If any of these occur, Phase 2 must pause and issue a planning checkpoint before
continuing.

------------------------------------------------------------------------

## 11. Phase 2 Deliverables

Expected deliverables:

- new recognition infrastructure document updates
- reporting-only mode
- parity port into the new recognizer path
- at least one deferred-family reporting-only pass
- Recognition Snapshot R2
- family-level decision records for any promoted or still-deferred recognizers

------------------------------------------------------------------------

## 12. Practical Interpretation

Phase 1 recognition answered:

- can any recognizers help?

Phase 2 should answer:

- can recognition become broad enough to matter at suite scale without losing
  legality and maintainability?

That is the next real question for the repo.
