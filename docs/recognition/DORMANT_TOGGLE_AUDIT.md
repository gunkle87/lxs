DORMANT TOGGLE AUDIT

Purpose
This file records compile-time and env-driven paths that look like cleanup
candidates after the recent recognition, packed-path, and runtime-attribution
phases.

Scope
- do not remove anything in this pass
- identify likely pruning candidates only
- distinguish:
  - keep
  - cleanup candidate
  - already gone from live code

Audit rule
- a toggle is a cleanup candidate when:
  - it exists mainly to preserve rejected candidate matrices
  - the admitted default is now settled
  - keeping the extra switch primarily increases maintenance ambiguity

Current live toggles and paths

1. `LXS_MIXED_LEVEL_EXEC_MODE`
- location:
  - [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
- current default:
  - `2`
- role:
  - compile-time selection among mixed-level execution-order modes
- audit read:
  - modes `1` and `3` were useful during candidate evaluation
  - checkpoint 02 admitted mode `2` as the runtime-attribution default
- status:
  - cleanup candidate
- suggested future action:
  - consider collapsing to the admitted mode `2` path after the next stability
    checkpoint, or moving alternate modes behind a dedicated experimental build
    guard

2. `LXS_LOGIC_CHUNK_FAST_MODE`
- location:
  - [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
- current default:
  - `1`
- role:
  - compile-time selection for logic chunk fast-path breadth
- audit read:
  - the admitted winner is the narrow 2-input `AND` fast path behavior
  - broader `AND/OR/XOR` behavior was explicitly rejected
- status:
  - cleanup candidate
- suggested future action:
  - consider narrowing this to the admitted behavior only, or replacing the
    current mode family with a more explicit constant naming the admitted path

3. `LXS_LOGIC_CHUNK_FAST_MIN_COUNT`
- location:
  - [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
- current default:
  - `0`
- role:
  - compile-time threshold for chunk fast-path eligibility
- audit read:
  - thresholded `AND` variants were rejected during runtime-attribution
  - the admitted path is the unthresholded variant
- status:
  - cleanup candidate
- suggested future action:
  - consider removing the threshold hook if no current checkpoint depends on it

4. `LXS_RECOGNITION_MASK`
- location:
  - [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c)
  - recognition helper scripts in `tools\`
- role:
  - env-driven family selection for recognition scans and controlled runs
- audit read:
  - still active and justified
  - required for recognition census and differential evaluation workflows
- status:
  - keep

5. `LXS_RECOGNITION_MODE`
- location:
  - [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c)
  - recognition helper scripts in `tools\`
- role:
  - env-driven selection between `replace` and `report_only`
- audit read:
  - still active and justified
  - core part of the recognition protocol and reporting discipline
- status:
  - keep

6. `LXS_TEST_PROBES`
- location:
  - [lxs_types.h](/c:/DEV/LXS/include/lxs_types.h)
  - [lxs_engine.c](/c:/DEV/LXS/src/core/lxs_engine.c)
  - [docs/PROBE_PROTOCOL.md](/c:/DEV/LXS/docs/PROBE_PROTOCOL.md)
- role:
  - compile-time test/probe instrumentation gate
- audit read:
  - explicitly documented
  - expected to be `0` in performance builds
- status:
  - keep

7. `LXS_REVISION`
- location:
  - [lxs_bench.c](/c:/DEV/LXS/src/apps/lxs_bench.c)
- role:
  - metadata stamp for recorded benchmark output
- audit read:
  - not a hot-path behavior switch
  - low risk either way
- status:
  - keep for now

Historical or rejected toggles already absent from live code

8. `LXS_ARITH_REMAP_POLICY`
9. `LXS_ARITH_REMAP_THRESHOLD`
- live-code status:
  - not present in current compiler or engine source
- audit read:
  - these appear only in rejected packed-path checkpoint history
- status:
  - already gone from live code
- suggested future action:
  - none in code
  - just avoid reintroducing them casually without a new checkpointed case

Candidate cleanup summary
- highest-confidence cleanup candidates:
  - `LXS_MIXED_LEVEL_EXEC_MODE`
  - `LXS_LOGIC_CHUNK_FAST_MODE`
  - `LXS_LOGIC_CHUNK_FAST_MIN_COUNT`

Why these are the right cleanup candidates
- each one preserves a rejected candidate matrix in live compile-time surface
- each one now has a clearly admitted default
- pruning them later would reduce ambiguity in:
  - performance claims
  - build identity
  - hot-path maintenance

Suggested follow-up
- do a dedicated cleanup pass after the next stable benchmark checkpoint
- in that pass:
  - remove or narrow only the high-confidence candidates above
  - keep recognition protocol env controls and probe gating intact
- target list for that cleanup pass:
  - `LXS_MIXED_LEVEL_EXEC_MODE`
  - `LXS_LOGIC_CHUNK_FAST_MODE`
  - `LXS_LOGIC_CHUNK_FAST_MIN_COUNT`
