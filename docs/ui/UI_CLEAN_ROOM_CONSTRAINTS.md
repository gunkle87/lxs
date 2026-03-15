# UI Clean-Room Constraints

## Non-Negotiable Constraints

These rules are mandatory for every UI run.

## 1. Repository Isolation

- `C:\DEV\LXS` is read-only
- `C:\DEV\LXS_UI` is the only writeable implementation target
- No edits to LXS code
- No edits to LXS docs
- No edits to LXS tests
- No edits to LXS build scripts
- No generated UI artifacts may be written into the LXS repo

## 2. Allowed Inputs From LXS

Only these are allowed as design/interface references:

- [UI.md](/c:/DEV/LXS/UI.md)
- [LXS_PUBLIC_API_PLAN.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_PLAN.md)
- [LXS_PUBLIC_API_CHECKPOINT_01.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_01.md)
- [LXS_PUBLIC_API_CHECKPOINT_02.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_02.md)
- [LXS_PUBLIC_API_CHECKPOINT_03.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_03.md)
- [LXS_PUBLIC_API_CHECKPOINT_04.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_04.md)
- [LXS_PUBLIC_API_CHECKPOINT_05.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_05.md)
- [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h)
- [lxs_api.dll](/c:/DEV/LXS/build/bin/lxs_api.dll)

Do not read `src/core/*.c` as implementation guidance for the UI.
Do not lift code from the LXS repo into the UI repo.

## 3. Framework Constraint

- Use Python
- Use PySide6
- Use `ctypes` to call `lxs_api.dll`

No fallback toolkit.
No alternate binding library without explicit approval.

## 4. Geometry Constraint

- No centered lanes
- No centered single-pin placement
- Only 25% and 75% lanes
- Single leftover terminal uses upper lane
- Component width fixed at 1 cell
- Component depth derived from pin count

## 5. Connection Constraint

- Crossings are non-semantic
- Only explicit nodes connect nets
- Nodes belong to one owner trace context
- No floating traces
- No arbitrary node placement in empty space
- No "helpful" auto-merge behavior
- Trace layout is autorouted
- Moving a connected component or explicit node must trigger reroute
- Direct freeform dragging of trace segments is not part of v0 unless later
  explicitly authorized

## 6. Scope Discipline

Each run may implement only the features listed in its directive.

If a feature would be nice but is not listed:
- do not add it

If a better framework or architecture seems tempting:
- do not switch

If manual trace shaping seems easier than autorouting:
- do not switch

If an ambiguity appears:
- choose the most conservative interpretation that preserves the written rules
- or stop and report if the ambiguity affects architecture

## 7. Style Constraint

- ASCII only unless a file already requires otherwise
- Python type hints required on public functions and dataclasses
- Prefer dataclasses for model structures
- Keep modules small and purpose-specific
- No giant "god object" modules
- No rendering logic in board model classes
- No direct LXS API access from renderers
- No global mutable singleton state except app bootstrap if unavoidable

## 8. Validation Constraint

Before commit/push, every run must:

- execute the run-specific tests
- execute any existing UI repo test suite relevant to touched code
- verify the UI repo launches or runs the intended smoke path
- verify `git status` in `C:\DEV\LXS` is unchanged

## 9. Blocker Handling

If blocked:

- stop
- report exactly what blocked progress
- report what files were changed in `C:\DEV\LXS_UI`
- do not attempt speculative fixes outside the directive scope
- do not touch `C:\DEV\LXS`

## 10. Push Requirement

Every successful implementation run must:

1. commit in `C:\DEV\LXS_UI`
2. push
3. report only after the push completes

If push fails:
- stop and report
- do not continue to later runs
