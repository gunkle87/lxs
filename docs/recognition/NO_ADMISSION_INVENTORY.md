NO-ADMISSION INVENTORY

Purpose
This file consolidates the major exploratory branches that were evaluated and
explicitly not admitted as default-path wins.

Reading rule
- inclusion here means:
  - the idea was real enough to evaluate
  - but it did not earn default-path admission
- some entries remain useful as:
  - alternates
  - explicit-only forms
  - offline rewrite modes
- they are not default-path admissions unless another document later says so

Recognition-phase no-admission outcomes

1. Shared XOR recognition
- status:
  - KEEP KERNEL, DEFER RECOGNITION
- reference:
  - [RECOGNITION_CHECKPOINT_02.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_CHECKPOINT_02.md)

2. Compare/equality recognition
- status:
  - KEEP KERNEL, DEFER RECOGNITION
- references:
  - [RECOGNITION_PHASE2_CHECKPOINT_03.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_PHASE2_CHECKPOINT_03.md)
  - [RECOGNITION_PHASE2_COMPARE_CLOSE.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_PHASE2_COMPARE_CLOSE.md)

3. Arithmetic recognition
- status:
  - KEEP KERNEL, DEFER RECOGNITION
- references:
  - [RECOGNITION_CHECKPOINT_04.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_CHECKPOINT_04.md)
  - [RECOGNITION_SUMMARY.md](/c:/DEV/LXS/docs/recognition/RECOGNITION_SUMMARY.md)

Functional compilation no-admission outcomes

4. Wide rooted cone prototype
- status:
  - KEEP KERNEL, DEFER DEFAULT REPLACEMENT
- reference:
  - [FUNCTIONAL_COMPILATION_CHECKPOINT_05.md](/c:/DEV/LXS/docs/recognition/FUNCTIONAL_COMPILATION_CHECKPOINT_05.md)

5. Early generic phase-0 replacement path
- status:
  - KEEP KERNEL, DEFER DEFAULT REPLACEMENT
- references:
  - [FUNCTIONAL_COMPILATION_CHECKPOINT_03.md](/c:/DEV/LXS/docs/recognition/FUNCTIONAL_COMPILATION_CHECKPOINT_03.md)
  - [FUNCTIONAL_COMPILATION_CHECKPOINT_07.md](/c:/DEV/LXS/docs/recognition/FUNCTIONAL_COMPILATION_CHECKPOINT_07.md)

6. Arithmetic compiled-region branch
- status:
  - valid alternate / not promoted
- references:
  - [FUNCTIONAL_COMPILATION_PHASE1_CHECKPOINT_02.md](/c:/DEV/LXS/docs/recognition/FUNCTIONAL_COMPILATION_PHASE1_CHECKPOINT_02.md)
  - [FUNCTIONAL_COMPILATION_PHASE1_CHECKPOINT_03.md](/c:/DEV/LXS/docs/recognition/FUNCTIONAL_COMPILATION_PHASE1_CHECKPOINT_03.md)
  - [FUNCTIONAL_COMPILATION_PHASE1_CHECKPOINT_04.md](/c:/DEV/LXS/docs/recognition/FUNCTIONAL_COMPILATION_PHASE1_CHECKPOINT_04.md)
  - [FUNCTIONAL_COMPILATION_PHASE1_CLOSE.md](/c:/DEV/LXS/docs/recognition/FUNCTIONAL_COMPILATION_PHASE1_CLOSE.md)

Packed-path and runtime no-admission outcomes

7. Packed multiplier boundary-remap pass
- status:
  - REJECT CHANGE
- references:
  - [PACKED_PATH_CHECKPOINT_03.md](/c:/DEV/LXS/docs/recognition/PACKED_PATH_CHECKPOINT_03.md)
  - [PACKED_PATH_CHECKPOINT_09.md](/c:/DEV/LXS/docs/recognition/PACKED_PATH_CHECKPOINT_09.md)

8. Packed multiplier op-mix / granularity reduction pass
- status:
  - REJECT CHANGE
- references:
  - [PACKED_PATH_CHECKPOINT_04.md](/c:/DEV/LXS/docs/recognition/PACKED_PATH_CHECKPOINT_04.md)
  - [PACKED_PATH_CHECKPOINT_05.md](/c:/DEV/LXS/docs/recognition/PACKED_PATH_CHECKPOINT_05.md)

9. Engine hot-path arithmetic caching pass
- status:
  - REJECT engine-side `RIPPLE_SLICE2` caching as default change
- reference:
  - [ENGINE_HOT_PATH_CHECKPOINT_01.md](/c:/DEV/LXS/docs/recognition/ENGINE_HOT_PATH_CHECKPOINT_01.md)

10. Runtime-attribution compiler boundary reshaping pass
- status:
  - no-admission
- reference:
  - [RUNTIME_ATTRIBUTION_CHECKPOINT_03.md](/c:/DEV/LXS/docs/recognition/RUNTIME_ATTRIBUTION_CHECKPOINT_03.md)

Inventory note
- this file is intentionally conservative
- it tracks the major rejected or deferred branches that are easy to
  accidentally over-credit later
- if a future pass revives one of these ideas successfully, that new admission
  should reference this file explicitly
