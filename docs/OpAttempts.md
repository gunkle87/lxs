# LXS Optimization Attempt Log

This file records optimization attempts made so far, whether they were kept or reverted, and the short reason why.

Status meanings:

- `Kept` means the change was retained in the branch at the end of that pass.
- `Reverted` means the change was benchmarked or validated and then removed.
- `Support` means the change was primarily for measurement, visibility, or benchmark honesty rather than direct speed.

Notes:

- The list is split between the archived pre-fresh-core work and the current fresh-core work.
- Reported outcomes are relative to the branch state at the time, not absolute forever-truths.
- Many failed attempts were still useful because they narrowed what the real bottleneck is not.

## Archived Engine (`archive/lxs_v1`)

### Kept / Support

- `Kept` Added real net resolution, real DFF sequencing, real input/output/state handling, and an honest public-path benchmark baseline. This replaced the earlier kernel-only number with a real simulator-style baseline.
- `Kept` Added contiguous `memcpy` gather/scatter handling where spans had contiguous input/output ranges. This was one of the few archived-engine changes that produced a measurable gain.
- `Support` Added direct-vs-packed-vs-scratch raw probes so the engine could report where gates were actually being executed.
- `Kept` Added the first packed-span execution path as a second execution class. The first implementation was mostly neutral, but it established the mechanism.
- `Kept` Added compiler-side gate reorder within `(level, type)` buckets to increase packed eligibility. This materially moved work from scratch to packed execution.
- `Kept` Added a subspan planner that allowed a large enough packed slice while limiting chunk explosion. This recovered throughput compared with the more fragmented version.
- `Kept` Changed the reorder heuristic to prefer the best packed slice a bucket could produce, rather than just minimizing total copy fragmentation.
- `Support` Added repeated benchmark sampling with median selection to reduce noise in suite reporting.
- `Support` Added structural profiling in the bench runner so chunk counts, median chunk sizes, unary/binary splits, and level fragmentation could be inspected.

### Reverted

- `Reverted` Execution-oriented net ID remapping plus a contiguous-span fast path. It slightly regressed the honest baseline.
- `Reverted` Split public phases with reusable scratch banks plus full-path timing. This made the product metric more honest, but the runtime shape itself did not become faster.
- `Reverted` Known-state fast path using probe-guided special handling. It regressed badly.
- `Reverted` Natural-order copy-run scheduling for packed movement. It lost on the full suite.
- `Reverted` Widened packed eligibility for mixed-source spans. It worked functionally but did not materially increase packed share.
- `Reverted` Commutative-input normalization on top of compiler reorder. It did not improve the execution mix.
- `Reverted` Cross-type mixed-chunk fusion for small fragmented levels. Chunk count dropped, but throughput still dropped.
- `Reverted` First bounded supernode / cone compiler pass. It was correct but too heavy and fragmented to beat the normal executor.
- `Reverted` Multiple gather/scatter schedule refinements that tried to make scratch execution cheaper without reducing scratch dependence. They did not pay off.

## Current Fresh Core (`lxs_*.c` in repo root)

### Kept / Support

- `Kept` Replaced the old root implementation with a fresh compiled-dataflow core and archived the earlier engine under `archive/lxs_v1`.
- `Kept` Bucket-sorted chunks plus per-type hot-loop specialization in the early fresh-core phase. This was the first fresh-core pass that clearly moved the weighted total upward.
- `Support` Changed benchmark timing so `Total Time` measured only the public simulation path and excluded synthetic input generation and unrelated harness work.
- `Kept` Added compiler remap plus contiguous streaming fast paths for primary inputs, primary outputs, and DFF capture/commit when provable.
- `Kept` Added aligned local aliases and pointer-walk loops in the current executor hot path. This was one of the best successful fresh-core micro-passes.
- `Support` Added and retained the current benchmark/probe discipline so regressions can be evaluated honestly.
- `Support` Fixed and retained benchmark harness behavior around single-file naming, metadata, and consistent suite measurement.

### Reverted

- `Reverted` Whole-chunk direct-stream metadata and a direct pointer-increment execution path. It regressed hard.
- `Reverted` SoA-style source/destination ID banks in earlier executor-shape experiments. They lost on the suite.
- `Reverted` Execution-order net remap attempts beyond the boundary remap already kept. They were neutral or negative.
- `Reverted` Compact runtime gate record lowering attempts before the later compact-binary experiment. One version crashed, another later version was stable but still slower.
- `Reverted` Hot-loop pointer-hoist and pointer-based level-walk experiments. They looked plausible but consistently regressed the suite.
- `Reverted` No-state fast path for chunks that appeared to avoid state overhead. It did not help.
- `Reverted` Compiler-marked fully streaming chunk metadata with direct streaming execution. It lost badly.
- `Reverted` Packed-region plan type with gathered temporary banks and a dedicated packed executor. Both the broad and narrowed forms regressed.
- `Reverted` Grouped-`input0` binary run executor that tried to exploit the existing bucket sort order. It regressed hard on the suite.
- `Reverted` Compiler-emitted SoA ID banks for large binary chunks with a dedicated SoA executor path. It was correct and test-backed, but slower than the baseline.
- `Reverted` Compact binary gate representation with separate unary fallback. It was stable and correct, but still slower than the normal representation.
- `Reverted` Per-type binary executor split in the current hot path. Removing one layer of dispatch did not help and instead regressed badly.
- `Reverted` Binary bucket heuristic that tried multiple legal orderings and chose the lowest local adjacency cost. It hurt important circuits like `c432`.
- `Reverted` Compiler-selected contiguous-output ordering plus a streamed-write binary executor. The extra layout work and streamed store path still lost on the suite.
- `Reverted` Unary contiguous-output streamed-write path. It was narrow and correct, but still slower than the restored baseline.
- `Reverted` `BUF`-only contiguous copy fast path using `memcpy` on both rails for fully contiguous input/output chunks. It did not beat the normal unary path.
- `Reverted` Two-wide unrolled binary inner loops. The extra instruction footprint regressed the suite.
- `Reverted` Explicit local `src_a`/`src_b`/`dst` ID hoisting inside the binary loops. It also regressed.

## Latest Four-Pass Batch

Starting restored reference before the batch:

- Weighted `TOTAL GEPS`: `560,108,494.779612`

Passes:

- `Reverted` Pass 1: unary contiguous-output streamed writes.
  Outcome: weighted `TOTAL GEPS = 463,562,199.434614`
- `Reverted` Pass 2: `BUF`-only contiguous copy fast path.
  Outcome: weighted `TOTAL GEPS = 498,751,216.208079`
- `Reverted` Pass 3: two-wide unrolled binary loops.
  Outcome: weighted `TOTAL GEPS = 460,867,812.338525`
- `Reverted` Pass 4: explicit local source/destination ID hoisting inside binary loops.
  Outcome: weighted `TOTAL GEPS = 418,161,108.239637`

Ending restored confirmation after reverting all four:

- Weighted `TOTAL GEPS`: `570,355,422.046017`

## Attempts That Were Mostly Measurement / Honesty Wins

- `Support` Full public-path timing instead of kernel-style timing.
- `Support` Median-of-N sampling instead of single-shot reporting.
- `Support` Structural profiling for chunk fragmentation and level shape.
- `Support` Raw probe separation for direct, packed, and scratch execution paths.

## What These Attempts Suggest

- The current executor has responded to a small number of low-level wins, but most speculative hot-loop reshapes regress.
- Compiler-side layout changes help only when they reduce a real dominant cost instead of adding more planning or fragmentation.
- Several representation changes were correct and test-backed, but they did not beat the baseline because they added overhead without reducing enough memory traffic.
- The next meaningful gain likely requires a larger structural change that reduces runtime data movement more decisively than the attempted packed, SoA, or compact-record variants did.

## Methods Tried At Least Once

- Net remapping for execution locality.
- Contiguous gather/scatter fast paths.
- Packed-span execution.
- Packed-region temporary-bank execution.
- SoA ID-bank execution.
- Compact record lowering.
- Cross-type chunk fusion.
- Subspan splitting and one-slice planners.
- Supernode / cone compilation.
- Per-type executor specialization.
- Bucket reordering heuristics.
- Boundary streaming for PI/PO/DFF state.
- Measurement and profiling instrumentation.

## Methods Not Yet Proven Worthwhile

- A representation that materially reduces memory traffic without adding per-chunk overhead back elsewhere.
- A packed or streamed path that activates on a large enough share of gates to beat the baseline.
- A supernode strategy that is cheaper than the current chunk executor after full compile and runtime costs are counted.

## Latest SIMD Attempt

- `Reverted` Narrow AVX2 `AND` chunk executor in [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c).
  Shape: 4-lane `_mm256_i64gather_epi64` loads for `value` and `mask`, vector 4-state `AND` math, scalar scatter to the destination nets, scalar fallback for the tail and all non-`AND` chunks.
  Outcome: weighted `TOTAL GEPS = 403,146,505.657127`
  Result: correct and test-backed, but substantially slower than the scalar baseline because gather/scatter cost dominated the small per-gate arithmetic savings.

Restored confirmation after reverting the SIMD pass:

- Weighted `TOTAL GEPS`: `564,319,688.432847`

## Latest Macro Retest

- `Kept` Canonical `XNOR2` macro-kernel retest across [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h), [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c), [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c), and [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c).
  Shape: recognize `XNOR(a,b)` lowered as `OR(AND(a,b), AND(NOT(a), NOT(b)))`, replace the five primitive gates with one macro record, execute via `LXS_EVAL_XNOR`, and preserve `gate_eval` comparability with `gate_equiv_count = 5`.
  Standardized A/B result against the `MUX2 + XOR2` control using three full all-suite sweeps at `--samples 5 --sample-select median --iterations 1 --cycles 10000`:
  - Control mean weighted `TOTAL GEPS = 419,344,191.248965`
  - XNOR2 mean weighted `TOTAL GEPS = 419,649,533.339037`
  - Delta: `+305,342.090072`
  Result: effectively flat and inside the agreed noise band, so the macro stays.

## Latest Arithmetic Macro

- `Kept` Inverted-carry step macro across [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h), [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c), [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c), and [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c).
  Shape: recognize the recurring arithmetic carry motif `NOR(and_ab, NOR(carry_inv_in, nor_ab))`, replace the two chained `NOR` gates with one macro record, leave the shared `AND(a,b)` and `NOR(a,b)` producers in place for sum-path reuse, and execute the step with `gate_equiv_count = 2`.
  Standardized A/B result against the `MUX2 + XOR2 + XNOR2` control using three full all-suite sweeps at `--samples 5 --sample-select median --iterations 1 --cycles 10000`:
  - Control mean weighted `TOTAL GEPS = 419,649,533.339037`
  - Carry-step mean weighted `TOTAL GEPS = 414,276,220.603257`
  - Delta: `-5,373,312.735780`
  Result: mild regression versus the immediate control, but still above the locked `413M` macro-phase entry marker and worth keeping as arithmetic macro infrastructure.

## Failed Full-Adder Macro

- `Reverted` Inverted-carry 1-bit full-adder macro across [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h), [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c), [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c), and [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c).
  Shape: recognize `sum = XNOR(cin_n, NOR(nor_ab, and_ab))` together with `cout_n = NOR(and_ab, NOR(cin_n, nor_ab))`, consume the shared `AND/NOR` producers, and emit one two-output macro record with `gate_equiv_count = 6`.
  Standardized all-suite result using three full sweeps at `--samples 5 --sample-select median --iterations 1 --cycles 10000`:
  - Mean weighted `TOTAL GEPS = 402,589,305.758347`
  - Median weighted `TOTAL GEPS = 402,837,674.200181`
  - Delta versus current kept arithmetic-macro state (`414,276,220.603257`): `-11,686,914.844910`
  Result: not keepable. It dropped below the locked `413M` macro-phase gate and hit `EPFL\\adder` especially hard, so it was fully reverted.

Restored-code confirmation after reverting the full-adder pass:

- Weighted `TOTAL GEPS`: `417,459,029.750914`

## Latest Sum-Side Arithmetic Macro

- `Kept` Inverted-carry sum-step macro across [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h), [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c), [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c), and [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c).
  Shape: recognize `sum = XNOR(cin_n, NOR(nor_ab, and_ab))`, leave the shared `AND(a,b)` and `NOR(a,b)` producers in place for carry reuse, replace the `xor_ab + sum` tail with one single-output macro record, and execute it with `gate_equiv_count = 2`.
  Standardized all-suite result using three full sweeps at `--samples 5 --sample-select median --iterations 1 --cycles 10000`:
  - Mean weighted `TOTAL GEPS = 418,050,641.782011`
  - Median weighted `TOTAL GEPS = 418,309,166.632657`
  - Delta versus the current kept arithmetic-macro state (`414,276,220.603257`): `+3,774,421.178754`
  Result: keep. It recovers most of the carry-step regression while staying inside the single-output macro model.

## Failed Compiler-Emitted Multi-Output Full Adder

- `Reverted` Compiler-emitted `FULL_ADDER_CINV` multi-output macro on top of the separate multi-output infrastructure.
  Shape: compile `sum = XNOR(cin_n, NOR(nor_ab, and_ab))` together with `cout_n = NOR(and_ab, NOR(cin_n, nor_ab))` into one two-output macro record in the dedicated multi-output executor.
  Standardized all-suite result using three full sweeps at `--samples 5 --sample-select median --iterations 1 --cycles 10000`:
  - Mean weighted `TOTAL GEPS = 387,465,936.753523`
  - Median weighted `TOTAL GEPS = 390,626,576.127678`
  - Delta versus the current kept arithmetic-macro state (`414,276,220.603257`): `-26,810,283.849734`
  Result: not keepable. The compiler-emitted multi-output recognizer was reverted, while the separate multi-output infrastructure and the manual engine-level validation test were kept for future work.

## Latest Adder Chain / Ripple Slice

- `Kept` Compiler-emitted `RIPPLE_SLICE2_CINV` multi-output macro on top of the separate multi-output infrastructure.
  Shape: recognize two consecutive inverted-carry full-adder cells chained by `carry_n`, replace the full 12-gate pair with one dedicated two-bit slice record, and execute both sum bits plus the final `cout_n` in the multi-output executor while leaving the single-output primitive hot path unchanged.
  Validation:
  - Added [ripple_slice2_macro.bench](/c:/DEV/LXS/Tests/Circuits/ripple_slice2_macro.bench)
  - Added dedicated compiled recognition/correctness coverage in [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)
  - Fixed a compiler bug where `comb_driver` was not rebuilt after net remap, which had prevented post-remap structural recognition from firing
  Standardized all-suite result using three full sweeps at `--samples 5 --sample-select median --iterations 1 --cycles 10000`:
  - Run 1 weighted `TOTAL GEPS = 395,651,015.119928`
  - Run 2 weighted `TOTAL GEPS = 395,871,535.921338`
  - Run 3 weighted `TOTAL GEPS = 397,689,361.795634`
  - Mean weighted `TOTAL GEPS = 396,403,970.945633`
  - Delta versus the multi-output-infrastructure baseline (`393,212,823.418927`): `+3,191,147.526706`
  Result: keep. This does not recover the full arithmetic-macro band yet, but it moves the new multi-output path forward and proves the first chain-level recognizer is real, test-backed, and measurably beneficial over the infrastructure-only state.

## Failed 4-Bit Ripple Slice

- `Reverted` Compiler-emitted `RIPPLE_SLICE4_CINV` extension on top of the kept 2-bit chain matcher.
  Shape: prefer one 4-bit inverted-carry ripple slice over two 2-bit slices when four consecutive full-adder cells are structurally present, with a dedicated larger multi-output executor record intended to reduce dispatch overhead.
  Standardized all-suite result using three full sweeps at `--samples 5 --sample-select median --iterations 1 --cycles 10000`:
  - Run 1 weighted `TOTAL GEPS = 388,473,433.822157`
  - Run 2 weighted `TOTAL GEPS = 385,904,785.059407`
  - Run 3 weighted `TOTAL GEPS = 385,627,348.728398`
  - Mean weighted `TOTAL GEPS = 386,668,522.536654`
  - Delta versus the kept 2-bit slice state (`396,403,970.945633`): `-9,735,448.408979`
  Result: not keepable. The larger slice increased batching, but the added per-macro work outweighed the saved dispatches, so the branch was reverted to the 2-bit `RIPPLE_SLICE2_CINV` state.

## Failed 2-Bit Slice Scheduler

- `Reverted` Carry-chain scheduler for the kept `RIPPLE_SLICE2_CINV` path.
  Shape: detect longer inverted-carry chains, keep the existing 2-bit slice executor intact, but retime adjacent emitted 2-bit slices onto the chain tail level when intermediate sums are output-only so the engine sees fewer level barriers without adding a larger macro kernel.
  Standardized all-suite result using three full sweeps at `--samples 5 --sample-select median --iterations 1 --cycles 10000`:
  - Run 1 weighted `TOTAL GEPS = 384,972,285.558015`
  - Run 2 weighted `TOTAL GEPS = 379,293,492.434102`
  - Run 3 weighted `TOTAL GEPS = 385,141,338.764537`
  - Mean weighted `TOTAL GEPS = 383,135,705.585551`
  - Delta versus the kept 2-bit slice state (`396,403,970.945633`): `-13,268,265.360082`
  Result: not keepable. Collapsing slice levels hurt more than the saved dispatch/barrier work helped, so the branch was reverted to the plain unscheduled 2-bit slice matcher.

## Failed Positive-Carry Front-End Macros

- `Reverted` Positive-carry single-output front-end macros aimed at BLIF-emitted adder neighborhoods.
  Shape: add `SUM_POS2` and `CARRY_POS2` recognizers for the common positive-carry sum/carry constructions built from `NOT/AND/OR` and `NOR/NOT/AND/OR`, with dedicated runtime kernels intended to reduce emitted node count around adder bits before the multi-output chain layer.
  Diagnostic outcome before benchmarking:
  - `EPFL\\adder` compile shape remained unchanged at `macro=2, multi=0, comb=1267`
  - so the new recognizers were not actually matching the target arithmetic benchmark in volume
  Sanity sweep result:
  - Weighted `TOTAL GEPS = 386,966,935.421084`
  Result: not keepable. The pass did not increase real coverage on the target adder benchmark and did not justify carrying extra recognizer complexity forward.
