**Summary**

Phase 2 should treat `net2bench` as an existing strict standalone C converter whose main job is still offline conversion into deterministic generic BENCH.

The purpose of phase 2 is to improve usability and carefully expand accepted coverage without changing the tool’s nature. The tool should remain a strict converter with a small trusted surface. It should not drift into synthesis, hierarchy elaboration, behavioral lowering, or engine-specific rewriting.

**Phase 2 goals**

- Improve diagnostics so failures are easier to understand and fix.
- Add report output for CI, corpus runs, and build integration.
- Expand primitive recognition where the source meaning is exact and deterministic.
- Add a small number of source-format conveniences that do not require synthesis logic.
- Strengthen tests and corpus discipline so every new accepted construct is explicit.

**Phase 2 non-goals**

- No behavioral Verilog support.
- No `always`, `case`, `if`, or procedural lowering.
- No hierarchy flattening beyond the existing flat-only contract.
- No `.subckt` or module instantiation support.
- No macro recognition.
- No optimization framework.
- No broad constant propagation or synthesis-style expression rewriting.

**Recommended scope**

Phase 2 should be limited to four work areas:

1. Better diagnostics
2. Report and stats output
3. Broader exact primitive recognition
4. A short whitelist of extra accepted source constructs

That is enough to make the tool much more usable without changing its identity.

**Diagnostics proposal**

Add source-aware diagnostics with consistent structure.

Recommended behavior:

- Every hard error should include:
  - input file path
  - line number
  - source format
  - short error code
  - one plain-English message
- Where possible, include the source token or directive that caused the failure.
- For `.names` fallback failures, explain whether the failure came from:
  - unsupported constant function
  - malformed cube width
  - unsupported row form
  - exact unsupported construct
- For Verilog failures, distinguish:
  - unsupported syntax
  - unsupported semantic construct
  - malformed declaration
  - malformed gate instance
  - behavioral construct rejection

Recommended output shape:

```text
error[N2B_VLOG_UNSUPPORTED]: tests/inputs/foo.v:12: unsupported Verilog construct in phase 2 scope: always
```

Warnings should remain rare and narrow. Good warning candidates:

- ignored `.model` text
- duplicate identical declarations
- declared-but-unused wires
- identifier remapping occurred and `--name-map` was not requested

**Report output proposal**

Add machine-readable reporting with a stable format.

Recommended new CLI flag:

- `--report <path>`

Recommended report format for phase 2:

- plain text summary for human use is fine
- machine-readable CSV or line-oriented text is safer than JSON if you want to keep the dependency footprint minimal
- if JSON is acceptable, emit it manually without pulling in a library

Recommended report fields:

- input path
- output path
- detected input format
- success or failure
- node count
- net count
- input count
- output count
- helper temp count
- alias count after normalization
- primitive counts by op
- warnings count
- error code if failed

This helps CI and build scripts without widening parsing scope.

**Coverage expansion proposal**

Phase 2 should expand only exact, non-lossy cases.

Recommended BLIF additions:

- better recognition for `NAND` and `NOR` when the truth table is exact and fully recognized
- recognition for trivial single-cube and single-product patterns that currently fall back to SOP
- optional inverter reuse during SOP lowering so repeated `NOT(x)` helpers do not multiply unnecessarily
- stronger `.names` validation messages
- still reject `.gate`, `.subckt`, `.latch`, `.mlatch`, `.exdc`, hierarchy, and constants unless separately approved

Recommended Verilog additions:

- allow harmless formatting variations and repeated declarations when identical
- allow explicit `buf` and `not` instances in more real-world formatting shapes
- allow direct output declarations with packed vectors in more cases
- allow simple continuous assignments of the form:
  - `assign y = x;`
  - `assign y = ~x;`
  - optionally `assign y = x & z;`, `|`, `^` only if you explicitly decide to parse these as structural operator sugar and only for single binary operations
- still reject:
  - behavioral code
  - concatenation
  - part-selects unless explicitly approved
  - module instantiations
  - escaped identifiers unless explicitly approved
  - generate constructs
  - delays and strengths

My recommendation: keep expression support very narrow. If you add any operator-form `assign`, stop at one binary operator and unary invert. Do not recurse into expression trees in phase 2 unless that is separately approved.

**Primitive recognition proposal**

This is the safest technical expansion.

Recommended additions:

- BLIF:
  - exact `NAND`
  - exact `NOR`
  - more robust `XOR2` and `XNOR2` detection with row-order independence
  - inverter reuse in SOP lowering
- Verilog:
  - preserve current primitive set
  - improve validation of fan-in rules
  - optional lowering of wide `XOR` and `XNOR` input lists into deterministic binary chains if encountered as primitive instances

This gives better output quality without changing accepted semantic scope much.

**Convenience features proposal**

Only add conveniences that do not require synthesis.

Good candidates:

- `--report <path>`
- `--fail-on-warning`
- `--diagnostic-format plain|csv`
- `--stats-only`
- better `--validate-only` output
- optional summary on stderr for failures
- deterministic warning ordering
- optional `--explain-failure` mode that prints one extra line of context

Avoid convenience features that secretly expand semantics.

**Determinism policy for phase 2**

Keep the phase 1 determinism contract unchanged.

Additional phase 2 rules:

- diagnostics must be stable in wording and order for the same input
- report field order must be fixed
- primitive recognition must not depend on source row ordering where the function is identical
- inverter reuse, if added, must use deterministic first-seen allocation
- if multiple reusable helpers are equivalent, choose by canonical source net name then first creation order

**Corpus and test proposal**

Phase 2 should be test-driven by manifest expansion, not ad hoc examples.

Add to the corpus manifest for each new accepted case:

- exact input path
- expected pass/fail
- exact BENCH output path
- exact report output path if used
- exact warning expectation if warnings are part of behavior

Recommended new test categories:

- diagnostics golden tests
- report output golden tests
- primitive-recognition tests
- inverter-reuse determinism tests
- duplicate identical declaration tests
- improved failure classification tests
- repeated-run byte-stability tests
- near-boundary syntax tests that must still fail

This is the main guardrail against stealth broadening.

**Implementation plan**

Phase 2 implementation order should be:

1. Define diagnostic codes and output format
2. Add source location tracking in both parsers
3. Implement `--report`
4. Add diagnostic and report golden tests
5. Add broader BLIF primitive recognition
6. Add deterministic inverter reuse
7. Add the smallest approved Verilog convenience set
8. Expand corpus and rerun determinism checks

Do not start with more accepted syntax. Start with diagnostics and reporting.

**Files that should exist for phase 2**

Recommended additions:

- `docs/PHASE2_CONTRACT.md`
- `docs/DIAGNOSTICS.md`
- `docs/REPORT_FORMAT.md`
- `tests/golden/diagnostics/`
- `tests/golden/reports/`
- `tests/inputs/blif_phase2/`
- `tests/inputs/verilog_phase2/`

Likely touched modules:

- parser modules for source spans
- validator for richer failure classes
- report module
- BLIF primitive recognition logic
- normalization if inverter reuse is added

**Main risks**

- adding expression parsing to `assign` can quietly turn into synthesis
- broadening BLIF truth-table interpretation can create semantic surprises if not kept exact
- helper reuse can break byte-stability if tie-breaking is not locked
- richer diagnostics can become inconsistent if error codes are not defined first
- “convenience” support for more syntax can outgrow the corpus if every acceptance is not explicitly listed

**Final recommendation**

Phase 2 should be approved as a disciplined quality-and-coverage pass with these priorities:

1. Strong diagnostics
2. Stable report output
3. Better exact primitive recognition
4. A very short whitelist of extra accepted constructs backed by corpus tests

The key rule for the agent should be simple: every new accepted construct must be exact, deterministic, corpus-listed, and small enough that `net2bench` still clearly reads as a converter, not a synthesis tool.

If you want, I can turn this into the same kind of implementation contract as phase 1, with locked acceptance rules, CLI additions, report schema, and done criteria.