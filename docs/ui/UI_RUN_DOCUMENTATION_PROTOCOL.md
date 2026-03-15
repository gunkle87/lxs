# UI Run Documentation Protocol

## Purpose

Make every implementation run leave behind the same kind of documentation and
phase-state updates.

This protocol is mandatory for every UI implementation run in `C:\DEV\LXS_UI`.

## Required Read Before Every Run

At the start of every fresh session, the implementation agent must read:

- [UI_PYTHON_WORKBENCH_PHASE_PLAN.md](/c:/DEV/LXS/docs/ui/UI_PYTHON_WORKBENCH_PHASE_PLAN.md)
- [UI_CLEAN_ROOM_CONSTRAINTS.md](/c:/DEV/LXS/docs/ui/UI_CLEAN_ROOM_CONSTRAINTS.md)
- [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- [UI_AGENT_DIRECTIVES.md](/c:/DEV/LXS/docs/ui/UI_AGENT_DIRECTIVES.md)

The audit agent must read the same set before auditing.

## Documentation Required Per Implementation Run

Every successful implementation run in `C:\DEV\LXS_UI` must update both:

1. a run checkpoint file
2. the phase status file

## Required Files In The UI Repo

The UI repo must maintain:

- `docs/UI_PHASE_STATUS.md`
- `docs/checkpoints/RUN_01.md`
- `docs/checkpoints/RUN_02.md`
- `docs/checkpoints/RUN_03.md`
- `docs/checkpoints/RUN_04.md`
- `docs/checkpoints/RUN_05.md`
- `docs/checkpoints/RUN_06.md`
- `docs/checkpoints/RUN_07.md`
- `docs/checkpoints/RUN_08.md`
- `docs/checkpoints/RUN_09.md`
- `docs/checkpoints/RUN_10.md`

The run file for the current directive must be created or updated during that
run.

## Pre-Run Continuity Gate (Mandatory)

Before starting any implementation run in `C:\DEV\LXS_UI`, the agent must run:

1. **Repository target check**
   - Confirm `C:\DEV\LXS` is not the active write target.
   - Confirm the current `git status` in `C:\DEV\LXS_UI` is clean for files outside the target run scope.
2. **Checkpoint-hash continuity check**
   - Read the latest completed run checkpoint, and verify `**Commit Hash:**` matches
     `git rev-parse --short HEAD` of `C:\DEV\LXS_UI`.
   - If the hash does not match, update only the checkpoint hash as a blocker correction.
3. **Push precondition check**
   - Confirm `git remote -v` shows a valid push destination.
   - If no usable remote is present and push is required by the directive, stop immediately and report blocker.
   - Do not edit code until the blocker is resolved.
4. **Validation artifact continuity check**
   - Confirm prior run launch proof artifacts and smoke check records exist for the claimed run.
   - If missing, rerun the launch proof before proceeding.

## Required Content In Each Run Checkpoint

Each `RUN_XX.md` file must include:

- run number
- scope completed
- files created
- files modified
- tests executed
- manual smoke validations performed
- blockers encountered, if any
- explicit confirmation that `C:\DEV\LXS` was not modified
- commit hash
- push confirmation

## Required Content In The Phase Status File

`docs/UI_PHASE_STATUS.md` must include:

- current phase name
- run completion table for runs 01 to 10
- latest completed run
- current repo launch command
- current known blockers
- current deferred items

After each successful run, the implementation agent must mark the run as
complete in the phase status file.

## Failure / Blocker Documentation Rule

If a run fails or blocks:

- do not mark the run complete
- update the current run checkpoint with:
  - blocker summary
  - files changed
  - validations attempted
- do not advance the phase status beyond the last fully completed run

## Audit Documentation Rule

Each audit must confirm:

- the expected run checkpoint was created or updated
- the phase status file was updated correctly
- the documentation matches the actual files and validations

## No Silent Completion

A run is not considered complete unless:

- code changes passed validation
- commit happened
- push happened
- run checkpoint was updated
- phase status was updated
