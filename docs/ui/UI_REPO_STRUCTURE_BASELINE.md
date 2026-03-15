# UI Repo Structure Baseline

## Purpose

Define the intended file structure for `C:\DEV\LXS_UI` and constrain agents from
inventing alternate layouts unless a blocker requires escalation.

## Canonical Root Layout

The UI repo should converge toward:

- `ui/`
- `tests/`
- `docs/`
- `scripts/`
- `assets/`
  - optional in v0
  - must remain empty or absent unless later explicitly authorized

No other top-level code directories are allowed unless a blocker requires
escalation.

## Canonical `ui/` Layout

- `ui/app.py`
- `ui/main_window.py`
- `ui/board_view.py`
- `ui/camera.py`
- `ui/input_controller.py`
- `ui/tool_controller.py`
- `ui/menu_controller.py`
- `ui/clipboard.py`

- `ui/model/`
  - `board_state.py`
  - `component_instance.py`
  - `trace_instance.py`
  - `node_instance.py`
  - `selection_state.py`
  - `tool_state.py`
  - `platform_state.py`
  - `geometry.py`
  - `primitive_definition.py`

- `ui/render/`
  - `renderer.py`
  - `board_renderer.py`
  - `component_renderer.py`
  - `trace_renderer.py`
  - `node_renderer.py`
  - `theme.py`

- `ui/tools/`
  - `select_tool.py`
  - `place_tool.py`
  - `trace_tool.py`
  - `node_tool.py`
  - `erase_tool.py`

- `ui/services/`
  - `primitive_registry.py`
  - `project_io.py`
  - `engine_bridge.py`
  - `command_stack.py`
  - `lxs_api_wrapper.py`

- `ui/widgets/`
  - `menu_bar.py`
  - `primitive_dropdown.py`
  - `status_bar.py`
  - `inspector_panel.py`

## Canonical `tests/` Layout

- `tests/test_model_*.py`
- `tests/test_registry_*.py`
- `tests/test_camera_*.py`
- `tests/test_render_*.py`
- `tests/test_routing_*.py`
- `tests/test_commands_*.py`
- `tests/test_project_io_*.py`
- `tests/test_engine_bridge_*.py`

## Canonical `docs/` Layout

- `docs/UI_PHASE_STATUS.md`
- `docs/checkpoints/`
  - `RUN_01.md`
  - `RUN_02.md`
  - `RUN_03.md`
  - `RUN_04.md`
  - `RUN_05.md`
  - `RUN_06.md`
  - `RUN_07.md`
  - `RUN_08.md`
  - `RUN_09.md`
  - `RUN_10.md`
- `docs/architecture.md`
- `docs/run.md`

## Veer Constraint

Agents must not invent a materially different file structure unless:

1. the canonical structure creates a real blocker
2. the blocker is reported explicitly
3. the user approves the change

Examples of forbidden veer:

- replacing `ui/model/` with a single giant `models.py`
- replacing `ui/render/` with rendering code inside widgets
- merging engine bridge code into UI widgets
- introducing a second parallel services layout

## Allowed Small Deviations

These are allowed without escalation:

- adding one small helper module inside an existing canonical folder
- splitting a file that has become too large
- adding a test helper module in `tests/`
- adding a small docs helper file under `docs/`

These are not allowed if they create a second competing structure.

## Run-Level Allowed File Families

Implementation agents should limit themselves to the file families implied by
the current directive.

Examples:

- bootstrap runs:
  - `ui/app.py`
  - `ui/main_window.py`
  - `ui/widgets/*`
  - bootstrap docs/scripts/tests
- model runs:
  - `ui/model/*`
  - `ui/services/primitive_registry.py`
  - model tests
- rendering runs:
  - `ui/render/*`
  - `ui/board_view.py`
  - camera tests/render tests
- engine bridge runs:
  - `ui/services/engine_bridge.py`
  - `ui/services/lxs_api_wrapper.py`
  - bridge tests

If a run needs files outside its implied families, the agent must explain why in
the run checkpoint.
