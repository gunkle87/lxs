ADMITTED DEFAULT PATH

Purpose
This file is the compact ledger of live, runtime-affecting wins that are still
admitted on `main`.

Reading rule
- include only changes that:
  - are live on the protected default path
  - affect runtime behavior or default benchmark throughput
  - were explicitly admitted by checkpoint or protected phase close
- do not include:
  - artifact-only wins
  - rejected branches
  - alternates that were kept but not promoted

Admitted live default-path wins

1. Parity recognition
- commit:
  - `2636584`
- checkpoint:
  - [RECOGNITION_CHECKPOINT_01.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_CHECKPOINT_01.md)
- effect summary:
  - admits bounded XOR-tree parity recognition on the live compiler path
  - recognition mask baseline remains parity only
- current status:
  - live
  - admitted recognition family

2. Packed adder rewrite granularity
- commit:
  - `badbe68`
- checkpoint:
  - [PACKED_PATH_CHECKPOINT_01.md](/c:/DEV/LXS/docs/recognition/PACKED_PATH_CHECKPOINT_01.md)
- effect summary:
  - packed adder rewrite prefers `RIPPLE_ADD4` windows before falling back to
    `RIPPLE_SLICE2`
  - reduces packed adder over-segmentation in the trusted packed artifact path
- current status:
  - live
  - part of the kept packed-path baseline

3. Packed multiplier output staging cleanup
- commit:
  - `7525d33`
- checkpoint:
  - [PACKED_PATH_CHECKPOINT_02.md](/c:/DEV/LXS/docs/recognition/PACKED_PATH_CHECKPOINT_02.md)
- effect summary:
  - removes a large safe subset of trailing packed multiplier output staging
  - cuts boundary `BUF` overhead in the trusted packed artifact path
- current status:
  - live
  - part of the kept packed-path baseline

4. Arithmetic unit-level source multi scheduling
- commit:
  - `485c339`
- checkpoint:
  - [PACKED_PATH_CHECKPOINT_06.md](/c:/DEV/LXS/docs/recognition/PACKED_PATH_CHECKPOINT_06.md)
- effect summary:
  - arithmetic source multi macros use unit-level scheduling based on external
    input driver level
  - fixes artificial level penalties on supported arithmetic macro families
- current status:
  - live
  - part of the kept packed-path baseline

5. Thresholded arithmetic net clustering
- commit:
  - `ae5548b`
- checkpoint:
  - [PACKED_PATH_CHECKPOINT_08.md](/c:/DEV/LXS/docs/recognition/PACKED_PATH_CHECKPOINT_08.md)
- effect summary:
  - compiler clusters arithmetic source multi macro inputs and outputs before
    remaining nets when `source_multi_macro_count >= 128`
  - improves arithmetic locality on large packed designs without changing small
    designs
- current status:
  - live
  - part of the kept packed-path baseline

6. Mixed-level execution order mode 2
- commit:
  - `7e94358`
- checkpoint:
  - [RUNTIME_ATTRIBUTION_CHECKPOINT_02.md](/c:/DEV/LXS/docs/recognition/RUNTIME_ATTRIBUTION_CHECKPOINT_02.md)
- effect summary:
  - on mixed levels:
    - large primitive chunks first
    - then macros, multi macros, and functional regions
    - tiny primitive chunks last
  - reduces mixed arithmetic/primitive boundary and transition overhead
- current status:
  - live
  - admitted runtime-attribution default

7. 2-input `AND` chunk fast path
- commit:
  - `5301be4`
- checkpoint:
  - [RUNTIME_ATTRIBUTION_CHECKPOINT_04.md](/c:/DEV/LXS/docs/recognition/RUNTIME_ATTRIBUTION_CHECKPOINT_04.md)
- effect summary:
  - adds a narrow engine-side fast path for 2-input `AND` logic chunks
  - specifically targets arithmetic-adjacent boundary chunk cost
- current status:
  - live
  - admitted runtime-attribution default

Current protected performance reference
- current protected commit in this ledger:
  - `5301be4`
- weighted GEPS:
  - `533,895,804.508597`
- comparison class:
  - same-toolchain rerun vs `11adfcc`
- reference:
  - [BASELINE_LEDGER.md](/c:/DEV/LXS/docs/recognition/BASELINE_LEDGER.md)

Ledger note
- if a future change supersedes any item here, update this file instead of
  letting phase notes become the only source of truth
