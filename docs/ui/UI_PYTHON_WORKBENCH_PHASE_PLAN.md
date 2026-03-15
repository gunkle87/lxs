# UI Python Workbench Phase Plan

## Purpose

Define the full v0 UI build as a clean-room Python workbench project that sits
beside LXS and talks to LXS only through the admitted public API.

This plan is intentionally strict.
It is written to minimize drift when handed to another agent.

## Canonical Repositories

- LXS engine repo, read-only reference:
  - `C:\DEV\LXS`
- UI workbench repo, writeable implementation target:
  - `C:\DEV\LXS_UI`

The UI workbench must be built in `C:\DEV\LXS_UI`.
The LXS repo must not be edited.

## Clean-Room Boundary

The UI implementation may read only these LXS artifacts unless the user
explicitly expands scope:

- [UI.md](/c:/DEV/LXS/UI.md)
- [LXS_PUBLIC_API_PLAN.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_PLAN.md)
- [LXS_PUBLIC_API_CHECKPOINT_01.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_01.md)
- [LXS_PUBLIC_API_CHECKPOINT_02.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_02.md)
- [LXS_PUBLIC_API_CHECKPOINT_03.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_03.md)
- [LXS_PUBLIC_API_CHECKPOINT_04.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_04.md)
- [LXS_PUBLIC_API_CHECKPOINT_05.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_CHECKPOINT_05.md)
- [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h)
- [lxs_api.dll](/c:/DEV/LXS/build/bin/lxs_api.dll)

The UI implementation must not copy code from LXS source files.
The UI implementation must not modify LXS files, build scripts, docs, tests, or
generated artifacts.

## Language and Toolkit

- Language: Python only
- Toolkit: PySide6 only
- Binding layer: Python `ctypes` against `lxs_api.dll`

No Java.
No alternate Python GUI toolkit.
If PySide6 is unavailable and cannot be installed cleanly, stop and report.

## Core Architectural Split

The UI project must keep these layers separate:

1. UI editor model
2. Rendering layer
3. Tool/controller layer
4. Engine bridge layer
5. LXS API wrapper layer

No rendering code inside the editor model.
No direct LXS API calls from rendering code.
No UI widget state as a substitute for board state.

## Canonical v0 Rules

These rules are fixed for this phase.

### Board and world

- Sparse, unbounded world coordinates
- Derived bounded build platform for rendering and work area framing
- Platform bounds derived from placed component bounds plus zoom-relative padding
- Middle mouse pans
- Mouse wheel uses stepped zoom only

### Components

- Rectangular blocks only
- Width fixed to 1 cell
- Depth fixed to `ceil(max(inputs, outputs) / 2)`
- Inputs on left
- Outputs on right
- Capital block labels on component face

### Lanes and terminals

- No centered lanes
- No centered single-pin placement
- Only two lane positions per span:
  - upper: 25%
  - lower: 75%
- A single leftover terminal in a span always uses the upper lane

### Traces and nodes

- Orthogonal only
- No diagonal traces
- Trace routing is autorouted in v0
- Moving a connected component or explicit node must trigger reroute
- Crossings imply nothing
- Only explicit nodes create electrical connection
- Nodes belong to exactly one owner trace context
- Other traces crossing a node location are still unrelated unless explicitly
  terminated into that node
- No floating traces

### Rendering

- Procedural only
- No asset pipeline
- No texture dependency
- No heavy animation
- No simulation-driven visual effects beyond simple state indication

## Visual Direction

The visual direction is constrained but minimal:

- board background:
  - dark gray or mid-tone neutral
- grid:
  - muted gray-blue
- component face:
  - light neutral gray
- component edge:
  - darker gray
- pads:
  - pale gray or white
- trace palette:
  - red
  - blue
  - yellow
  - green
  - orange
  - purple
- all traces:
  - black outline
- selection accent:
  - white or cyan

Do not invent a new palette.
Do not add gradients except for minimal procedural depth cues already implied by
the spec.

## File and Directory Target

The UI repo should converge toward:

- `ui/`
  - `app.py`
  - `main_window.py`
  - `board_view.py`
  - `camera.py`
  - `input_controller.py`
  - `tool_controller.py`
  - `menu_controller.py`
  - `clipboard.py`
- `ui/model/`
- `ui/render/`
- `ui/tools/`
- `ui/services/`
- `ui/widgets/`
- `tests/`
- `docs/`
- `scripts/`

## Validation Standard

Every run must include:

- lint/format validation if configured in the UI repo
- automated tests for the new surface introduced in that run
- manual smoke validation where UI interaction is the feature under test
- explicit confirmation that `C:\DEV\LXS` is unchanged

Every run also starts with a continuity preflight:

- verify the latest run checkpoint hash matches `git rev-parse --short HEAD` in `C:\DEV\LXS_UI`
- verify `C:\DEV\LXS_UI` has a pushable remote when push is required by the run
- verify the previous run's launch proof artifact exists
- if any item is missing, stop, report, and do not begin implementation edits

## Interaction Addendum

These interaction rules are also fixed for v0:

- placed components are selectable
- placed components are deletable
- placed components are movable by mouse drag
- explicit nodes are selectable
- explicit nodes are deletable
- explicit nodes are movable by mouse drag
- traces are selectable
- traces are deletable
- traces are not freeform drag objects in v0
- trace geometry is produced by the autorouter and refreshed when connected
  components or nodes move

For this phase, "movable with dragging" is interpreted conservatively as:

- component dragging
- explicit node dragging

It does not authorize arbitrary direct manipulation of trace segments as freeform
geometry.

## Blocker Rule

If a run hits a blocker, the agent must:

1. stop
2. report
3. not attempt a workaround that changes scope
4. not edit the LXS repo
5. not silently switch frameworks, architecture, or constraints

## Delivery Style

The UI phase is broken into 10 large runs.

Each run must:

- stay inside its stated scope
- complete its requirements fully
- run its validations
- commit
- push
- then report

Each audit between runs must inspect only the prior run and confirm:

- clean-room adherence
- no-copy policy adherence
- no edits in `C:\DEV\LXS`
- style adherence
- code validity
- directive adherence

## Completion Definition

This phase is complete when:

- the Python workbench exists in `C:\DEV\LXS_UI`
- it launches
- it supports the defined v0 editing semantics
- it talks to LXS through `lxs_api.dll`
- it can build a board, step the engine, and display basic state
- all 10 runs and 9 audits have been completed without violating the clean-room
  boundary
