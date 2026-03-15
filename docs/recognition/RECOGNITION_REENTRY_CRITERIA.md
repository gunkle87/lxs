RECOGNITION RE-ENTRY CRITERIA

Purpose
This document defines when recognition work is worth reopening and how it must
restart if reopened.

Do not reopen recognition just because
- a deferred family still seems interesting
- a kernel exists and looks elegant
- a local artifact or fixture win appears
- a family can be widened only by increasing search radius

Recognition may be reopened only if at least one of these is true

1. Architecture changed materially
- example:
  - a new execution model makes recognized replacements cheaper to exploit
  - mixed boundary overhead has been reduced enough that prior deferred
    families may become profitable

2. Corpus evidence changed materially
- example:
  - a new default or accepted corpus shows strong recurring structure that was
    absent or too sparse before
  - report-only scans show nontrivial structural coverage under a materially
    different benchmark mix

3. Family boundary changed materially
- example:
  - a deferred family gets a new legality boundary or structural form that is
    genuinely different from the rejected path
  - not just a small matcher tweak or another retry at the same shape

4. Replacement economics changed materially
- example:
  - a kernel or packed-path change removes the boundary cost that previously
    made recognition unprofitable

Mandatory restart sequence

1. Reopen the protocol
- start with:
  - [RECOGNITION_PROTOCOL.md](/c:/DEV/LXS/docs/RECOGNITION_PROTOCOL.md)

2. State the re-entry reason explicitly
- name which of the allowed re-entry conditions is satisfied
- name the evidence for it

3. Start with report-only qualification
- no immediate replacement admission
- first prove:
  - boundedness
  - fail-fast behavior
  - meaningful coverage

4. Use the retained tools
- `tools\lxs_recognition_stress.ps1`
- `tools\lxs_recognition_census.ps1`
- `tools\lxs_recognition_delta.ps1`

5. Re-establish benchmark discipline
- use:
  - [CORPUS_PROVENANCE.md](/c:/DEV/LXS/docs/recognition/CORPUS_PROVENANCE.md)
  - [MEASUREMENT_RIGOR_APPENDIX.md](/c:/DEV/LXS/docs/recognition/MEASUREMENT_RIGOR_APPENDIX.md)
  - [CHECKPOINT_TEMPLATE.md](/c:/DEV/LXS/docs/recognition/CHECKPOINT_TEMPLATE.md)

First candidate order if recognition ever reopens

1. Compare/equality
- only if a materially different bounded structural form exists
- not if the plan is just one more expression widening pass

2. Arithmetic
- only if replacement economics change because the surrounding execution model
  changed
- not if the recognizer itself is the only new ingredient

3. Shared XOR
- only if corpus evidence changes materially
- not if accepted-suite coverage is still effectively zero

What must not happen on re-entry
- no broad graph-search behavior
- no widening search radius to compensate for weak structure
- no benchmark-specific heuristics
- no parallel family hunting
- no family admission without a default-suite confirmation rerun

Bottom line
- recognition is not the active optimization frontier right now
- it remains a maintained subsystem with one admitted family and several
  documented deferred families
- reopening it is allowed, but only with a clearly different reason than the
  one already exhausted
