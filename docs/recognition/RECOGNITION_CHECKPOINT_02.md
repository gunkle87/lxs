# RECOGNITION_CHECKPOINT_02.md

Recognition checkpoint after the shared XOR family evaluation.

------------------------------------------------------------------------

Current Recognition Baseline R0
    Weighted total GEPS:
    456,660,260.470426

Families Evaluated So Far
    1. Parity
    2. Shared XOR

Status Of Each Family
    Parity:
    ADMIT

    Shared XOR:
    KEEP KERNEL, DEFER RECOGNITION

Recognizer Complexity Versus Structural Coverage
    Parity:
    - narrow bounded XOR-tree matcher
    - match_count = 24
    - node_reduction = 48
    - circuits_affected = 1

    Shared XOR:
    - exact width-8 shared-input XOR grouping
    - output-only boundary
    - negative-space case passed
    - match_count in accepted suite = 0
    - node_reduction in accepted suite = 0
    - circuits_affected = 0

    Interpretation:
    - parity remains proportional and admitted
    - shared XOR recognizer is now too narrow to justify activation
    - kernel remains valid, but recognition did not earn admission

Observed Conflicts Between Families
    Shared XOR was explicitly bounded away from parity by legality:
    - no downstream gate consumers allowed on grouped XOR outputs

    This prevented family conflict, but it also eliminated practical coverage
    on the accepted suite.

Updated Risk Assessment
    Moderate.

    Low:
    - parity remains stable
    - family masks and stats path are functioning

    Moderate:
    - recognition families that were previously strong as offline rewrites may
      still collapse to zero useful coverage once legality is enforced

Family Completion Record
    status:
    KEEP KERNEL, DEFER RECOGNITION

    files_changed:
    - lxs_types.h
    - lxs_compiler.c
    - lxs_bench.c
    - lxs_test.c
    - Tests\Circuits\xor_fan8_primitive.bench
    - Tests\Circuits\xor_fan8_negative.bench

    recognizer_scope:
    - exact width-8 shared-input binary XOR grouping into XOR_FAN8

    legality_boundary:
    - all grouped gates must be binary XOR
    - all grouped gates must share one exact input net
    - grouped outputs must have no downstream gate consumers
    - grouped gates must be unmatched at entry
    - exact width must be 8

    negative_cases_added:
    - xor_fan8_negative

    conflict_precedence_rules:
    - shared XOR must not consume outputs that feed later logic
    - this keeps parity precedence intact where XOR trees exist

    equivalence_proof_method:
    - parser-backed primitive fixture
    - negative-space fixture
    - kernel-on versus kernel-off fixture behavior

    suite_results_vs_R0:
    - all-suite with shared XOR family off:
      461,887,625.845443 GEPS
    - all-suite with shared XOR family on:
      457,966,343.745202 GEPS
    - delta versus family-off:
      -3,921,282.100241 GEPS
    - delta versus R0:
      +1,306,083.274776 GEPS

    family_local_results:
    - c432 off:
      698,923,571.235939 GEPS
    - c432 on:
      694,273,916.842537 GEPS
    - delta:
      -4,649,654.393402 GEPS

    node_reduction:
    - 0 in the accepted suite

    match_count:
    - 0 in the accepted suite

    circuits_affected:
    - 0 in the accepted suite

    maintenance_risk:
    - moderate

    decision:
    KEEP KERNEL, DEFER RECOGNITION

    next_allowed_move:
    - revisit RECOGNITION_PROTOCOL.md
    - choose the next family only after accepting this checkpoint
