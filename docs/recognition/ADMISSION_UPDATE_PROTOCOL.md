ADMISSION UPDATE PROTOCOL

Purpose
This file defines the minimum documentation updates required whenever a new
default-path win is admitted.

Rule
- every new admitted win must update:
  - [ADMITTED_DEFAULT_PATH.md](/c:/DEV/LXS/docs/recognition/ADMITTED_DEFAULT_PATH.md)
  - [BASELINE_LEDGER.md](/c:/DEV/LXS/docs/recognition/BASELINE_LEDGER.md)

Minimum update sequence

1. Checkpoint first
- write or update the checkpoint that records:
  - comparison class
  - corpus
  - settings
  - admission status
  - rerun confirmation status
- start from:
  - [CHECKPOINT_TEMPLATE.md](/c:/DEV/LXS/docs/recognition/CHECKPOINT_TEMPLATE.md)

2. Update admitted-path ledger
- add the new live runtime-affecting win to:
  - [ADMITTED_DEFAULT_PATH.md](/c:/DEV/LXS/docs/recognition/ADMITTED_DEFAULT_PATH.md)
- include:
  - commit
  - checkpoint
  - effect summary
  - current status

3. Update baseline ledger
- if the admission changes the protected default-path baseline or creates a new
  primary comparison point, update:
  - [BASELINE_LEDGER.md](/c:/DEV/LXS/docs/recognition/BASELINE_LEDGER.md)
- include:
  - commit
  - corpus
  - settings
  - weighted GEPS
  - comparison provenance

4. Update no-admission inventory if needed
- if the new admission supersedes a prior rejected or deferred branch, add a
  note in:
  - [NO_ADMISSION_INVENTORY.md](/c:/DEV/LXS/docs/recognition/NO_ADMISSION_INVENTORY.md)
- this prevents old exploratory work from being over-credited later

5. Keep the checklist current
- if the admission changes audit-response maintenance state, update:
  - [AUDIT_RESPONSE_CHECKLIST.md](/c:/DEV/LXS/docs/recognition/AUDIT_RESPONSE_CHECKLIST.md)

Usage note
- this protocol is intentionally small
- the goal is to prevent future admitted wins from living only in phase notes
  or commit history
- future checkpoint docs should use the shared checkpoint template unless there
  is a documented reason to deviate
