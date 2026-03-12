# RECOGNITION_CHECKPOINT_03.md

Recognition checkpoint after the compare and equality family evaluation.

------------------------------------------------------------------------

Current Recognition Baseline R0
    Weighted total GEPS:
    456,660,260.470426

Families Evaluated So Far
    1. Parity
    2. Shared XOR
    3. Compare and equality

Status Of Each Family
    Parity:
    ADMIT

    Shared XOR:
    KEEP KERNEL, DEFER RECOGNITION

    Compare and equality:
    KEEP KERNEL, DEFER RECOGNITION

Recognizer Complexity Versus Structural Coverage
    Compare and equality:
    - exact four-slice NOT(XOR(a, b)) bank matcher
    - grouped deterministically by level
    - no decode widening
    - negative-space case passed
    - match_count in accepted suite = 0
    - node_reduction in accepted suite = 0
    - circuits_affected = 0

    Interpretation:
    - the explicit XNOR bank kernel remains valid
    - the recognizer is too narrow to hit preserved accepted-suite structure
    - when combined with the already-admitted parity recognizer, suite
      performance regresses

Observed Conflicts Between Families
    Compare and equality does not overlap directly with parity matching, because
    parity only consumes pure XOR trees and compare consumes NOT(XOR(...))
    slices.

    Practical conflict remains at the benchmark level:
    - c499 benefits more from parity recognition than from equality-bank
      recognition
    - enabling both together worsens the current admitted default

Updated Risk Assessment
    Moderate.

    Low:
    - kernel correctness remains stable
    - legality and negative-space boundaries are narrow and clean

    Moderate:
    - accepted-suite structural coverage is zero
    - family-local uplift without matches is not interpretable and should not be
      treated as evidence of recognizer value

Family Completion Record
    status:
    KEEP KERNEL, DEFER RECOGNITION

    files_changed:
    - lxs_compiler.c
    - lxs_test.c
    - Tests\Circuits\xnor_bank4_primitive.bench
    - Tests\Circuits\xnor_bank4_negative.bench

    recognizer_scope:
    - exact four-slice NOT(XOR(a, b)) equality banks into XNOR_BANK4

    legality_boundary:
    - grouped roots must be NOT gates with one input
    - each NOT input must be driven by a binary XOR
    - the XOR output must have exactly one gate consumer
    - grouped NOT roots must share a level
    - exact width must be 4

    negative_cases_added:
    - xnor_bank4_negative

    conflict_precedence_rules:
    - parity precedence remains effectively stronger on shared benchmarks
    - compare recognition may not broaden into decode logic

    equivalence_proof_method:
    - parser-backed primitive fixture
    - negative-space fixture
    - unchanged benchmark outputs with recognizer on or off

    suite_results_vs_R0:
    - all-suite with compare family off:
      452,477,372.306817 GEPS
    - all-suite with compare family on:
      455,799,795.573327 GEPS
    - delta versus family-off:
      +3,322,423.266510 GEPS
    - delta versus R0:
      -860,464.897099 GEPS
    - current admitted default, parity only:
      458,023,720.270816 GEPS
    - parity plus compare:
      455,415,388.394926 GEPS
    - delta versus current admitted default:
      -2,608,331.875890 GEPS

    family_local_results:
    - c499 with compare family off:
      350,742,116.800372 GEPS
    - c499 with compare family on:
      407,845,215.137527 GEPS
    - note:
      struct-profile reporting showed zero compare matches in the accepted suite,
      so this uplift is not taken as meaningful recognizer evidence

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
