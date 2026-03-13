# RECOGNITION_PROTOCOL.md

Defines rules governing macro recognition qualification, checkpointing, and
admission during the recognition phase.

------------------------------------------------------------------------

## 1. Purpose
    Recognition is a controlled compiler admission exercise.

    Recognition must never blur the line between:
    -   kernel correctness
    -   recognition quality
    -   benchmark behavior

    All three must remain independently measurable.

## 2. Recognition Baseline
    Recognition work must begin from a pinned benchmark reference.

    This pinned reference is Recognition Baseline R0.

    Every family evaluation must report against R0 using:
    -   overall weighted GEPS
    -   per-suite weighted GEPS
    -   per-benchmark deltas
    -   classification deltas if classification exists

## 3. Core Principles
    1. One-family-at-a-time execution
       Only one recognizer family may be active at a time.
       No parallel recognizer development is allowed during a recognition run.

    2. Kernel contracts are immutable
       A recognizer may not broaden, reinterpret, or redesign an existing
       kernel.
       If recognizer work requires kernel semantic change, recognizer work
       stops immediately.

    3. Recognition is bounded
       Recognizers must operate within explicit legality boundaries.
       No heuristic widening is allowed during the active family pass.
       Phase 2 guard rails additionally require:
       -   fixed root or output anchoring
       -   fixed max node budget
       -   fixed traversal depth
       -   strict abort on fanout growth
       -   strict abort on branch growth
       -   strict abort on overlap ambiguity

    4. Recognition is measurable
       Every recognizer must produce objective metrics against R0.

    5. Recognition complexity must remain proportional
       If recognizer complexity grows faster than the structural coverage it
       provides, the family must be stopped.

## 4. Recognition Gate Sequence
    Every family must pass the following gates in order.

    1. Family Spec
       Define the structural motif the recognizer is allowed to detect.

    2. Legality Rules
       Define strict match constraints:
       -   required structure
       -   fanout rules
       -   boundary conditions
       -   semantic guarantees

    3. Negative Space Definition
       Explicitly define shapes that must not match.
       These should include:
       -   near-miss structures
       -   overlapping motifs
       -   conflicting families

    4. Conflict Precedence
       Declare interaction rules between families.
       Examples:
       -   parity versus equality or XNOR
       -   arithmetic versus shared fanout
       -   fanout versus parity

    5. Negative Test Cases
       Add synthetic or benchmark-derived cases that intentionally violate
       recognition legality.

    6. Equivalence Proof
       Demonstrate that the replacement is semantically identical to the
       original structure.

    7. Stress-Pack Qualification
       Before any full corpus census, the recognizer must clear a small
       heavy-bench stress pack within a fixed time budget.

       At minimum, the stress pack should include:
       -   EPFL multiplier
       -   ISCAS89 s38417
       -   ISCAS89 s38584
       -   one late ITC99 benchmark

       A recognizer that cannot fail fast on this stress pack is not ready for
       corpus-scale census.

    8. Benchmark Qualification
       Run the full standardized suite and report metrics relative to R0.

    9. Admission Decision
       End the family with one of the allowed decision outcomes.

## 5. Decision Outcomes
    Allowed outcomes are:

    ADMIT
    REJECT
    HOLD FOR LATER
    KEEP KERNEL, DEFER RECOGNITION
    ADMIT AS EXPLICIT-ONLY

## 6. Checkpoint Requirement
    After every family evaluation, recognition must pause and produce a
    checkpoint report before any new family begins.

    The checkpoint must include:
    -   current Recognition Baseline R0 reference
    -   families evaluated so far
    -   status of each family
    -   recognizer complexity versus structural coverage
    -   observed conflicts between families
    -   updated risk assessment

    No new family may begin until this checkpoint exists.

## 7. Protocol Revisit Rule
    Before beginning each new family, this protocol must be reopened and
    checked against the current run state.

    Required revisit questions:
    -   Is the next family still within scope?
    -   Are its legality boundaries already defined?
    -   Are conflict rules with prior admitted or deferred families clear?
    -   Does the next move still preserve independent measurement of kernel,
        recognizer, and benchmark behavior?

    If the answer to any of these is no, the recognition run must stop and
    resolve the protocol issue before continuing.

## 8. Recognizer Reporting Template
    Each family must produce a standardized completion record containing:

    -   status
    -   files_changed
    -   recognizer_scope
    -   legality_boundary
    -   negative_cases_added
    -   conflict_precedence_rules
    -   equivalence_proof_method
    -   suite_results_vs_R0
    -   family_local_results
    -   node_reduction
    -   match_count
    -   circuits_affected
    -   maintenance_risk
    -   decision
    -   next_allowed_move

    If any field is missing, the family evaluation is incomplete.

## 9. Required Recognition Metrics
    Every family evaluation must report:
    -   match count
    -   node reduction
    -   circuits affected
    -   recognizer-on versus recognizer-off delta
    -   full-suite delta versus R0
    -   matcher telemetry:
        -   candidate_roots
        -   nodes_visited
        -   max_depth_reached
        -   abort_reason
        -   time_per_family

    Recognizers must be disableable independently of their kernels.

## 10. Stopping Conditions
    Immediately halt a family if any of the following occur:
    -   false positives appear
    -   equivalence cannot be proven
    -   suite regressions are material and unexplained
    -   recognizer complexity grows faster than coverage
    -   kernel semantics would need modification

## 11. Deferred Families
    Families previously identified as unstable or unresolved remain deferred
    until explicitly re-admitted to the recognition run.

    Example:
    -   GUARD_CHAIN4 control-chain recognition remains deferred until its
        correctness issues are resolved.

## 12. Scope Control
    Recognition is not allowed to:
    -   introduce benchmark-specific heuristics
    -   expand kernel semantics
    -   create broad global search logic
    -   widen search radius when the problem is matcher expressiveness
    -   change execution scheduling semantics
    -   reinterpret sequential boundaries

    Recognition must remain strictly structural and legality-driven.

## 13. Completion Criteria
    The recognition run is considered successful when:
    -   Recognition Baseline R0 remains stable
    -   admitted recognizers produce measurable suite benefit
    -   rejected families are clearly documented
    -   recognizer complexity remains maintainable
    -   kernel correctness remains unchanged
