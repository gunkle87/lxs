# RECOGNITION_CHECKPOINT_04.md

Recognition checkpoint after the arithmetic family evaluation.

------------------------------------------------------------------------

Current Recognition Baseline R0
    Weighted total GEPS:
    456,660,260.470426

Families Evaluated So Far
    1. Parity
    2. Shared XOR
    3. Compare and equality
    4. Arithmetic

Status Of Each Family
    Parity:
    ADMIT

    Shared XOR:
    KEEP KERNEL, DEFER RECOGNITION

    Compare and equality:
    KEEP KERNEL, DEFER RECOGNITION

    Arithmetic:
    KEEP KERNEL, DEFER RECOGNITION

Recognizer Complexity Versus Structural Coverage
    Arithmetic:
    - strict inverted-carry two-bit ripple-slice matcher
    - synthetic positive and negative cases passed
    - match_count in accepted suite = 0
    - node_reduction in accepted suite = 0
    - circuits_affected = 0

    Interpretation:
    - the arithmetic recognizer is now properly family-gated and measurable
    - it does not hit accepted-suite structure under the current legality
      boundary
    - suite and local arithmetic benchmarks regress when the recognizer is
      enabled

Observed Conflicts Between Families
    No direct conflict with parity or compare was observed.

    The arithmetic issue is not family conflict. It is lack of practical
    coverage under the current recognizer legality boundary.

Updated Risk Assessment
    Moderate.

    Low:
    - kernel semantics remain unchanged
    - the legacy arithmetic recognizer is now under protocol control

    Moderate:
    - no accepted-suite coverage
    - local arithmetic benchmarks regress
    - current recognizer shape is too narrow to justify activation

Family Completion Record
    status:
    KEEP KERNEL, DEFER RECOGNITION

    files_changed:
    - lxs_compiler.c
    - lxs_test.c
    - Tests\Circuits\ripple_slice2_cinv_primitive.bench
    - Tests\Circuits\ripple_slice2_cinv_negative.bench

    recognizer_scope:
    - exact two-bit inverted-carry ripple slice into RIPPLE_SLICE2_CINV

    legality_boundary:
    - two consecutive full-adder-inverted-carry cells only
    - first carry output must be the second cell carry input
    - exact internal AND, NOR, and XNOR shape
    - required single-consumer and two-consumer use-count boundaries
    - exact width must be 2

    negative_cases_added:
    - ripple_slice2_cinv_negative

    conflict_precedence_rules:
    - none active beyond standard unmatched-gate legality

    equivalence_proof_method:
    - parser-backed primitive fixture
    - negative-space fixture
    - unchanged benchmark outputs with recognizer on or off

    suite_results_vs_R0:
    - all-suite with arithmetic family off:
      453,036,769.245203 GEPS
    - all-suite with arithmetic family on:
      448,044,623.808714 GEPS
    - delta versus family-off:
      -4,992,145.436489 GEPS
    - delta versus R0:
      -8,615,636.661712 GEPS
    - current admitted default, parity only:
      458,023,720.270816 GEPS
    - parity plus arithmetic:
      448,609,521.639397 GEPS
    - delta versus current admitted default:
      -9,414,198.631419 GEPS

    family_local_results:
    - EPFL adder off:
      278,061,811.048072 GEPS
    - EPFL adder on:
      250,485,159.417588 GEPS
    - delta:
      -27,576,651.630484 GEPS
    - EPFL multiplier off:
      395,877,465.789386 GEPS
    - EPFL multiplier on:
      394,517,365.132174 GEPS
    - delta:
      -1,360,100.657212 GEPS

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
    - recognition phase may stop here with one admitted family and three
      deferred recognizers
    - if continued later, the next family should be re-admitted explicitly
      under RECOGNITION_PROTOCOL.md
