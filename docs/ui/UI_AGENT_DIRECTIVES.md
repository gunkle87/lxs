# UI Agent Directives

These directives are written to be copied directly into another agent.

Use them in order.
Do not skip audits.

All implementation runs target:
- writeable UI repo: `C:\DEV\LXS_UI`
- read-only engine repo: `C:\DEV\LXS`

All runs inherit the rules in:
- [UI_PYTHON_WORKBENCH_PHASE_PLAN.md](/c:/DEV/LXS/docs/ui/UI_PYTHON_WORKBENCH_PHASE_PLAN.md)
- [UI_CLEAN_ROOM_CONSTRAINTS.md](/c:/DEV/LXS/docs/ui/UI_CLEAN_ROOM_CONSTRAINTS.md)
- [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- [UI_RUN_DOCUMENTATION_PROTOCOL.md](/c:/DEV/LXS/docs/ui/UI_RUN_DOCUMENTATION_PROTOCOL.md)

Implementation and audit sessions must start with:

```cmd
powershell -ExecutionPolicy Bypass -File C:\DEV\LXS_UI\scripts\preflight.ps1
```

A non-zero exit code is a hard stop for implementation sessions.

At the start of every fresh session, the agent must read those four files before
doing any work.

---

## Directive 01

Build the clean-room UI repository bootstrap in `C:\DEV\LXS_UI`.

Hard requirements:
- If `C:\DEV\LXS_UI` does not exist, create it as a separate git repo
- Do not modify `C:\DEV\LXS`
- Use Python and PySide6 only
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Create the initial project skeleton:
  - `ui/`
  - `ui/model/`
  - `ui/render/`
  - `ui/tools/`
  - `ui/services/`
  - `ui/widgets/`
  - `tests/`
  - `docs/`
  - `scripts/`
- Add bootstrap application files sufficient to launch:
  - main window
  - top menu bar with `Program` and `Edit`
  - left-side placeholder tool/component area
  - central board viewport shell
  - status bar
- Add a Python environment/bootstrap note in the UI repo docs
- Add a launch script for local startup
- Add a minimal smoke test or launch check for the bootstrap
- Add required UI repo docs:
  - `docs/UI_PHASE_STATUS.md`
  - `docs/checkpoints/RUN_01.md`

Do not:
- implement board logic yet
- implement engine bridge yet
- invent alternate structure
- use Java
- use another GUI toolkit

Validation required before commit:
- launch the app successfully
- run the bootstrap smoke validation
- verify `git status` in `C:\DEV\LXS` is unchanged
- update:
  - `docs/checkpoints/RUN_01.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 01 bootstrap UI repo shell`

---

## Audit 01

Audit only the work from Directive 01.

Requirements:
- inspect only changes in `C:\DEV\LXS_UI`
- confirm `C:\DEV\LXS` has no modified, staged, or untracked files caused by the implementation run
- confirm no-copy clean-room adherence:
  - no copied code blocks from LXS
  - no copied LXS docs embedded as implementation code
- confirm the implementation used Python and PySide6 only
- confirm the canonical repo structure was followed
- confirm the directive scope was followed
- confirm launch validation was actually performed
- confirm the required checkpoint/status docs were updated
- report any drift, overbuild, or repo-boundary violation

Do not modify code.
Audit and report only.

---

## Directive 02

Appendix for Directive 02 (drift remediation required before starting):
- Before any implementation, verify `docs/checkpoints/RUN_01.md` commit hash matches `C:\DEV\LXS_UI` `HEAD`.
  - If it does not match, update only that checkpoint hash and note the correction in the current run notes.
- If `C:\DEV\LXS_UI` has no configured remote, configure one from the existing project policy and push the current HEAD before starting Directive 02.
- Any Directive 02 launch validation should write a proof artifact file under:
  - `C:\DEV\LXS_UI\artifacts\run_02_launch.txt`
  - include the command and success signal in the file.
- If the previous run remains unpushed, treat this as a hard blocker and do not proceed with implementation changes until resolved.

Implement the authoritative editor model and primitive registry in `C:\DEV\LXS_UI`.

Hard requirements:
- Use dataclasses for the core model structures
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Implement the canonical model objects described by the UI spec:
  - `BoardState`
  - `ComponentInstance`
  - `TerminalRef`
  - `TraceEndpoint`
  - `TraceVertex`
  - `TraceInstance`
  - `NodeInstance`
  - `SelectionState`
  - `ToolState`
  - `PlatformState`
  - support types such as `GridBounds` and `PrimitiveDefinition`
- Implement the fixed geometry rules:
  - width = 1 cell
  - depth = `ceil(max(inputs, outputs) / 2)`
  - no centered lanes
  - only upper/lower lane slots
- Implement a dynamic primitive registry with initial categories:
  - Combinational
  - Input/Output
  - Sequential
  - Memory
  - ALUs
  - MUXes
- Include the minimal primitive set:
  - Input
  - Output
  - NOT
  - AND
  - OR
  - XOR
  - Clock
  - Probe
- Add model-level validation helpers for the listed invariants
- Add unit tests for model geometry and invariant enforcement

Do not:
- implement rendering beyond what is needed for imports/tests
- implement tool interactions yet
- implement engine bridge yet
- add files outside the implied run families unless it becomes a blocker

Validation required before commit:
- model tests pass
- primitive registry tests pass
- `C:\DEV\LXS` remains unchanged
- `run_01` hash and push state from the previous pass must be corrected and confirmed in the final run notes
- launch proof artifact created at `artifacts/run_02_launch.txt`
- update:
  - `docs/checkpoints/RUN_02.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 02 add editor model and primitive registry`

---

## Audit 02

Audit only the work from Directive 02.

Requirements:
- inspect only files changed by Directive 02 in `C:\DEV\LXS_UI`
- confirm no edits in `C:\DEV\LXS`
- confirm no-copy clean-room adherence
- confirm dataclass/model layering discipline
- confirm the canonical repo structure was followed
- confirm no rendering or engine leakage into model layer
- confirm tests cover the implemented invariants
- confirm no centered-lane behavior was introduced
- confirm the required checkpoint/status docs were updated

Do not modify code.
Audit and report only.

---

## Directive 03

Implement the camera, board viewport rendering foundation, and derived platform rendering in `C:\DEV\LXS_UI`.

Hard requirements:
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Implement stepped zoom only
- Implement middle-mouse pan only
- Render:
  - board background
  - visible grid
  - derived bounded platform plate
  - visible platform edge
  - red inset gasket/channel
- Platform bounds must be derived from placed component bounds plus zoom-relative padding
- Platform recomputation must be deferred or periodic, not instant on every edit
- Grid should simplify/fade when zoomed out
- Keep rendering procedural only
- Add viewport/camera tests where practical and a manual render smoke path

Do not:
- implement component placement yet
- implement traces yet
- invent continuous zoom
- invent freeform board coordinates

Validation required before commit:
- app launches
- camera pan/zoom works
- platform plate and gasket render correctly
- `C:\DEV\LXS` remains unchanged
- update:
  - `docs/checkpoints/RUN_03.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 03 add camera and platform rendering`

---

## Audit 03

Audit only the work from Directive 03.

Requirements:
- inspect only files changed by Directive 03 in `C:\DEV\LXS_UI`
- confirm no edits in `C:\DEV\LXS`
- confirm no-copy clean-room adherence
- confirm stepped zoom, not continuous zoom
- confirm the canonical repo structure was followed
- confirm bounded derived platform behavior, not infinite rendered plane
- confirm no premature feature creep into placement/traces/engine bridge
- confirm the required checkpoint/status docs were updated

Do not modify code.
Audit and report only.

---

## Directive 04

Implement component placement, selection, deletion, drag-move, and procedural component rendering in `C:\DEV\LXS_UI`.

Hard requirements:
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Left-click places the selected primitive on snapped grid cells
- Single selection only
- Delete support for selected component
- Mouse drag moves a placed component on snapped grid coordinates
- Components render as simple rectangular blocks with:
  - label
  - top face
  - darker edge cue
  - visible terminals/pads
- Terminal placement must obey the fixed upper/lower lane rules only
- Use the registry categories to populate the primitive selector
- Primitive access may use a simple dropdown for v0
- Add tests for:
  - footprint sizing
  - terminal packing
  - placement snapping
  - selection state

Do not:
- implement traces yet
- center single pins
- invent alternate component silhouettes
- add files outside the implied run families unless it becomes a blocker

Validation required before commit:
- app launches
- placement/select/delete/drag works for the minimal primitive set
- tests pass
- `C:\DEV\LXS` remains unchanged
- update:
  - `docs/checkpoints/RUN_04.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 04 add component placement drag and rendering`

---

## Audit 04

Audit only the work from Directive 04.

Requirements:
- inspect only files changed by Directive 04 in `C:\DEV\LXS_UI`
- confirm no edits in `C:\DEV\LXS`
- confirm no-copy clean-room adherence
- confirm fixed-width variable-depth component rule
- confirm no centered terminal placement
- confirm selection remains single-select
- confirm component drag-move is grid-snapped
- confirm the canonical repo structure was followed
- confirm no traces or engine bridge were prematurely added
- confirm the required checkpoint/status docs were updated

Do not modify code.
Audit and report only.

---

## Directive 05

Implement trace and node model operations, trace drawing workflow, autorouting, and routing preview/commit in `C:\DEV\LXS_UI`.

Hard requirements:
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Orthogonal traces only
- No floating traces
- Trace routing must be autorouted
- Clicking a valid connection point starts trace pathing
- Clicking another valid connection point commits the trace
- Clicking a trace body selects the trace
- Right-click or trace-body interaction creates nodes only under explicit trace ownership rules
- Implement explicit node ownership semantics:
  - one owner trace context
  - no accidental net merge from crossings
- Implement validation so crossings remain non-semantic unless explicitly terminated into the same node
- Implement reroute-on-move behavior so connected traces reroute when a connected
  component moves
- Add tests for:
  - orthogonal path validity
  - endpoint validity
  - node ownership
  - non-semantic crossings
  - reroute on component move

Do not:
- add diagonal routing
- auto-connect crossings
- allow arbitrary node placement in empty space
- replace autorouting with manual trace shaping
- add files outside the implied run families unless it becomes a blocker

Validation required before commit:
- trace creation workflow works
- reroute-on-component-move works
- model/validation tests pass
- `C:\DEV\LXS` remains unchanged
- update:
  - `docs/checkpoints/RUN_05.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 05 add trace and node semantics`

---

## Audit 05

Audit only the work from Directive 05.

Requirements:
- inspect only files changed by Directive 05 in `C:\DEV\LXS_UI`
- confirm no edits in `C:\DEV\LXS`
- confirm no-copy clean-room adherence
- confirm crossings are still non-semantic
- confirm explicit node ownership is enforced
- confirm no freeform or diagonal routing
- confirm autorouting and reroute-on-move are present
- confirm the canonical repo structure was followed
- confirm no hidden auto-merge behavior
- confirm the required checkpoint/status docs were updated

Do not modify code.
Audit and report only.

---

## Directive 06

Implement trace rendering, node rendering, node drag-move, hit-testing priority, and selection z-order behavior in `C:\DEV\LXS_UI`.

Hard requirements:
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Render traces as:
  - orthogonal paths
  - black outer stroke
  - inner color stroke from the fixed palette
- Render nodes explicitly
- Explicit nodes must be draggable on valid snapped positions
- Dragging a node must trigger reroute for attached traces
- Last selected overlapping trace must render on top
- Selected object must receive visible highlight
- Add hit-testing for:
  - components
  - traces
  - nodes
- Keep crossings visually clear without implying connection
- Add tests or deterministic checks for selection/z-order behavior where practical

Do not:
- add animation-heavy polish
- add implicit crossing connection visuals
- change the palette
- add files outside the implied run families unless it becomes a blocker

Validation required before commit:
- render smoke path passes
- overlap selection priority works
- node drag-move and reroute works
- `C:\DEV\LXS` remains unchanged
- update:
  - `docs/checkpoints/RUN_06.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 06 add trace rendering and hit testing`

---

## Audit 06

Audit only the work from Directive 06.

Requirements:
- inspect only files changed by Directive 06 in `C:\DEV\LXS_UI`
- confirm no edits in `C:\DEV\LXS`
- confirm no-copy clean-room adherence
- confirm last-selected overlap z-priority is implemented
- confirm crossings remain non-semantic visually and logically
- confirm node drag-move triggers reroute
- confirm the canonical repo structure was followed
- confirm no palette drift
- confirm the required checkpoint/status docs were updated

Do not modify code.
Audit and report only.

---

## Directive 07

Implement edit operations, command stack, save/load schema, and project I/O in `C:\DEV\LXS_UI`.

Hard requirements:
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Implement:
  - undo
  - redo
  - delete
  - duplicate
  - copy
  - paste
- Implement project save/load for committed board content only
- Do not serialize transient tool preview state
- Add command-stack coverage tests
- Add save/load round-trip tests
- Wire `Program` and `Edit` menu actions to the implemented behavior

Do not:
- add speculative project formats beyond the chosen v0 schema
- serialize transient preview/pathing state
- edit `C:\DEV\LXS`
- add files outside the implied run families unless it becomes a blocker

Validation required before commit:
- command stack works
- save/load round-trip passes
- menu actions are wired
- `C:\DEV\LXS` remains unchanged
- update:
  - `docs/checkpoints/RUN_07.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 07 add project I O and editing commands`

---

## Audit 07

Audit only the work from Directive 07.

Requirements:
- inspect only files changed by Directive 07 in `C:\DEV\LXS_UI`
- confirm no edits in `C:\DEV\LXS`
- confirm no-copy clean-room adherence
- confirm transient tool state is not serialized
- confirm command stack and I/O are scoped to v0 only
- confirm the canonical repo structure was followed
- confirm no unrelated UI polish or engine bridge work was added
- confirm the required checkpoint/status docs were updated

Do not modify code.
Audit and report only.

---

## Directive 08

Implement the engine bridge and Python-side LXS API integration in `C:\DEV\LXS_UI`.

Hard requirements:
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Use only the admitted public API through:
  - `lxs_api.dll`
  - the documented public API contract
- Vendor or adapt a Python wrapper in the UI repo that talks to the DLL through `ctypes`
- Implement an engine bridge that compiles the current board model into an engine-facing representation usable by the LXS API boundary for v0 testing
- Support:
  - loading the admitted DLL
  - compile/export from UI board into the bridge form
  - ticking/stepping the engine
  - reading outputs/probes/basic state needed for v0
- If an API surface is missing and blocks progress:
  - stop
  - report
  - do not modify `C:\DEV\LXS`

Do not:
- call internal LXS structs
- read LXS core source as implementation guidance
- patch LXS
- add files outside the implied run families unless it becomes a blocker

Validation required before commit:
- engine bridge smoke path works from the UI repo
- DLL loading works
- at least one board-to-engine example runs
- `C:\DEV\LXS` remains unchanged
- update:
  - `docs/checkpoints/RUN_08.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 08 add engine bridge and API integration`

---

## Audit 08

Audit only the work from Directive 08.

Requirements:
- inspect only files changed by Directive 08 in `C:\DEV\LXS_UI`
- confirm no edits in `C:\DEV\LXS`
- confirm no-copy clean-room adherence
- confirm the bridge uses only the public API/DLL boundary
- confirm there was no fallback to internal LXS code or repo edits
- confirm the canonical repo structure was followed
- confirm blocker handling, if any, was compliant
- confirm the required checkpoint/status docs were updated

Do not modify code.
Audit and report only.

---

## Directive 09

Implement basic simulation state presentation and inspection in `C:\DEV\LXS_UI`.

Hard requirements:
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Show basic state on pads/components after step/tick
- Provide a lightweight inspector or status panel for selected objects
- Display at minimum:
  - selected component identity
  - selected trace/node identity
  - basic engine state/probe readout relevant to the current board
- Keep state presentation simple and procedural
- Do not add waveform tooling in this run
- Add smoke validation for:
  - state update after tick
  - selection-driven inspection updates

Do not:
- invent heavy visualization systems
- add timeline/waveform UI
- change core editor semantics
- add files outside the implied run families unless it becomes a blocker

Validation required before commit:
- state visibly updates after engine step
- inspector/status updates follow selection
- `C:\DEV\LXS` remains unchanged
- update:
  - `docs/checkpoints/RUN_09.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 09 add state presentation and inspection`

---

## Audit 09

Audit only the work from Directive 09.

Requirements:
- inspect only files changed by Directive 09 in `C:\DEV\LXS_UI`
- confirm no edits in `C:\DEV\LXS`
- confirm no-copy clean-room adherence
- confirm only basic state presentation was added
- confirm no waveform scope creep
- confirm the canonical repo structure was followed
- confirm the UI still respects the established editing and connection rules
- confirm the required checkpoint/status docs were updated

Do not modify code.
Audit and report only.

---

## Directive 10

Perform the final v0 integration pass and close the UI phase in `C:\DEV\LXS_UI`.

Hard requirements:
- Use the canonical repo structure from:
  - [UI_REPO_STRUCTURE_BASELINE.md](/c:/DEV/LXS/docs/ui/UI_REPO_STRUCTURE_BASELINE.md)
- Do not add new features outside integration hardening
- Do:
  - remove obvious dead scaffolding introduced during the UI phase
  - tighten docs inside the UI repo
  - verify menu/tool wiring
  - verify startup/run scripts
  - verify save/load
  - verify engine step path
  - verify selection and editing flows
- Add a UI repo phase-close document summarizing:
  - what v0 supports
  - what is deferred
  - how to run it
- Run the broadest practical validation sweep for the UI repo
- Confirm again that `C:\DEV\LXS` remains unchanged

Do not:
- reopen architecture
- change toolkit
- add major new functionality
- edit `C:\DEV\LXS`
- add files outside the implied run families unless it becomes a blocker

Validation required before commit:
- final UI repo validation sweep passes
- launch/run path works
- docs are updated in the UI repo
- `C:\DEV\LXS` remains unchanged
- update:
  - `docs/checkpoints/RUN_10.md`
  - `docs/UI_PHASE_STATUS.md`

Commit and push in `C:\DEV\LXS_UI` before reporting.
Suggested commit message:
- `Run 10 close v0 UI phase`
