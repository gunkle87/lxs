ARITHMETIC NEIGHBORHOOD CHECKPOINT 04

Candidate
- name: ROW_PAIR_REDUCE_PROPAGATE4
- kernel form: REDUCE_PROPAGATE4
- class: arithmetic
- status: KEEP AS ALTERNATE NEIGHBORHOOD

Purpose
This checkpoint records the first implementation and controlled multiplier
qualification pass for Candidate 02.

What was implemented
- explicit six-output REDUCE_PROPAGATE4 kernel
- parser support for:
  - REDUCE_PROPAGATE4(i0..i11) -> o0..o5
- dedicated explicit and reference fixtures
- bounded multiplier rewrite mode:
  - neighborhood2

Files
- [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)
- [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c)
- [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
- [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)
- [Tests/Circuits/reduce_propagate4_explicit.bench](/c:/DEV/LXS/Tests/Circuits/reduce_propagate4_explicit.bench)
- [Tests/Circuits/reduce_propagate4_primitive.bench](/c:/DEV/LXS/Tests/Circuits/reduce_propagate4_primitive.bench)
- [tools/gen_vector_multiplier_macro.ps1](/c:/DEV/LXS/tools/gen_vector_multiplier_macro.ps1)
- [tools/LxsRewriteLib.ps1](/c:/DEV/LXS/tools/LxsRewriteLib.ps1)
- [tools/lxs_rewrite.ps1](/c:/DEV/LXS/tools/lxs_rewrite.ps1)

Correctness
- parser-backed explicit fixture test: passed
- reference vs explicit compare:
  - equivalent
- EPFL multiplier vs generated neighborhood2 artifact:
  - equivalent across 512 trials

Generated artifact
- [multiplier_macro_neighborhood2.bench](/c:/DEV/LXS/Benchmarks/Generated/multiplier_macro_neighborhood2.bench)

Controlled EPFL multiplier results
- original:
  - 403,166,240.361612 GEPS
- carry-save row neighborhood:
  - 613,899,524.182323 GEPS
- current packed slice2 path:
  - 786,855,322.060523 GEPS
- candidate 02 neighborhood2 path:
  - 593,890,283.502518 GEPS

Interpretation
- Candidate 02 is a real profitable bounded neighborhood relative to the
  original multiplier.
- It does not beat the current carry-save row neighborhood.
- It does not beat the current packed slice2 multiplier path.
- So it is not the new default arithmetic rewrite winner.

Important note
- The standalone bench runner does not currently emit usable stdout on the
  isolated REDUCE_PROPAGATE4 explicit fixture, even though:
  - the fixture loads under the parser-backed test harness
  - the explicit/reference compare passes
  - the generated multiplier rewrite using the same kernel loads, compares, and
    benchmarks correctly
- This is treated as runner-path noise, not as a kernel correctness failure.

Decision
- keep the REDUCE_PROPAGATE4 kernel
- keep neighborhood2 as an alternate bounded multiplier rewrite mode
- do not promote it over the current packed slice2 path

Next sensible move
- either plan the next larger arithmetic neighborhood candidate
- or stop climbing neighborhood width and start comparing which bounded
  arithmetic neighborhood shapes deserve long-term maintenance
