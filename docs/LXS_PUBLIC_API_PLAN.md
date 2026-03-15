# LXS Public API Plan

Defines the first formal external API boundary for LXS.

This phase is about exposing the engine safely and completely to outside tools.
It is not about adding a new simulation model.

------------------------------------------------------------------------

## 1. Purpose

LXS already has a usable internal C surface:

- load netlist
- compile plan
- initialize engine
- apply inputs
- execute ticks
- read outputs
- inspect probes
- reset and free state

That surface is enough for in-repo apps, but it is not yet a true external API.

This phase creates that boundary.

The result should support:

- Python tooling
- Python UI
- future Java integration if needed
- stable external inspection of logic state
- stable external inspection of plan/runtime metadata

## 2. Why This Phase Exists

The standard macro library is now large enough that LXS is becoming a real
engine platform, not just a benchmark core.

At this point we need:

- a stable way to drive the engine
- a stable way to inspect state
- a stable way to expose probes and diagnostics
- a binding-friendly contract that does not depend on internal struct layout

Without this, every tool or UI would need to reach directly into internal
headers and runtime structures, which would make future refactors expensive and
unsafe.

## 3. Core Goal

Create a stable public C API for LXS that:

- fully controls the simulation lifecycle
- fully exposes the state needed for tooling and UI
- does not leak internal implementation details as the required public contract
- is easy to bind from Python first

Success means:

- a Python client can load a circuit, compile it, step it, inspect outputs,
  internal nets, registers, RAM, regfiles, macros, and probes
- without depending on internal engine struct layouts

## 4. Core Principles

1. C ABI first
   The public API must be a stable C boundary.
   Python and any later Java layer should sit on top of that.

2. Opaque handles externally
   Public consumers should not depend on internal structs like:
   - `lxs_netlist`
   - `lxs_plan`
   - `lxs_engine_ctx`

3. Internal truth stays internal
   Existing internal engine/compiler functions remain the implementation base.
   The public API is a wrapper layer, not a rewrite.

4. Full inspection matters
   The API must expose enough engine state for:
   - UI rendering
   - tooling
   - debugging
   - snapshotting
   - benchmark attribution

5. Stable names, stable lifecycle
   The public API should feel deliberate and boring.
   No cleverness.

6. Python first
   Bindings should be designed with Python in mind first.
   Future Java support is allowed, but not required for the first admission.

## 5. Non-Goals

This phase does not include:

- HTTP or service APIs
- distributed simulation
- remote execution protocol
- user-defined plugin language
- Java binding implementation
- replacing the internal execution model

## 6. Public API Design Target

The public API should expose these object families:

- library/runtime initialization
- netlist objects
- compiled plan objects
- engine/session objects
- state accessors
- structural metadata accessors
- probe/diagnostic accessors
- error/diagnostic reporting

### Recommended handle types

- `lxs_api_netlist`
- `lxs_api_plan`
- `lxs_api_engine`

These should be opaque outside the API implementation.

## 7. Required Capabilities

The public API must support:

### Lifecycle

- create/load netlist
- free netlist
- compile plan from netlist
- free plan
- create engine from plan
- reset engine
- free engine

### Execution control

- apply inputs by ordered input index
- apply inputs by named net lookup helper
- execute one tick
- execute many ticks
- read outputs

### State visibility

- read output values/masks
- read arbitrary net values/masks
- read register contents
- read RAM contents
- read regfile contents
- read current sequential state snapshots

### Structural visibility

- enumerate input nets
- enumerate output nets
- enumerate named nets
- inspect plan counts:
  - levels
  - chunks
  - macros
  - multi-macros
  - standard macros
  - functional regions
  - registers
  - ROMs
  - RAMs
  - regfiles

### Probe visibility

- read `lxs_probes`
- expose benchmark-relevant counters
- expose state-commit counts
- expose future attribution metrics through a stable access path

### Diagnostics

- last error code
- last error message
- explicit invalid-handle / invalid-index / invalid-state errors

## 8. Public API Shape

The external API should be split into families:

- `lxs_api_netlist_*`
- `lxs_api_plan_*`
- `lxs_api_engine_*`
- `lxs_api_state_*`
- `lxs_api_probe_*`
- `lxs_api_diag_*`

### Example lifecycle

1. `lxs_api_netlist_load_bench(path, &netlist)`
2. `lxs_api_plan_compile(netlist, &plan)`
3. `lxs_api_engine_create(plan, &engine)`
4. `lxs_api_engine_apply_inputs(engine, values, masks, count)`
5. `lxs_api_engine_tick(engine)`
6. `lxs_api_engine_read_outputs(engine, values, masks, count)`
7. `lxs_api_engine_free(engine)`
8. `lxs_api_plan_free(plan)`
9. `lxs_api_netlist_free(netlist)`

## 9. Data Access Rules

The API should prefer coarse-grained access over chatty per-bit calls.

Good:

- read all outputs into caller-owned buffers
- read one whole register by index
- read one whole RAM word by word index
- read one probe struct at once

Avoid:

- single-bit accessor loops as the primary intended path
- forcing Python to make thousands of FFI calls per frame

That means the first version should expose:

- contiguous value/mask fills into caller-provided buffers
- descriptor metadata so the caller knows how large each object is

## 10. Naming and Discovery

To support a UI and tooling layer, the API must expose mapping between names and
stable IDs.

The first version should include:

- input count + input net IDs + input names
- output count + output net IDs + output names
- net count + net ID to name lookup
- name to net ID lookup helper

This is the minimum needed for:

- waveform-like displays
- probe selection
- UI circuit navigation
- scripting by signal name

## 11. State Semantics

The public API must not blur execution semantics.

What the API should expose clearly:

- current committed state
- current combinational/output state after the last tick
- probe counters since reset

What it should not do in v1:

- expose fragile internal transient arrays whose meaning is implementation-only
- expose half-committed state as if it were stable public truth

If a future debugger needs that, it should be added deliberately as a debug
family, not leaked accidentally.

## 12. Error Model

The API should use explicit result codes.

Recommended:

- `LXS_API_OK`
- `LXS_API_ERR_INVALID_ARG`
- `LXS_API_ERR_INVALID_HANDLE`
- `LXS_API_ERR_LOAD_FAILED`
- `LXS_API_ERR_COMPILE_FAILED`
- `LXS_API_ERR_INIT_FAILED`
- `LXS_API_ERR_BOUNDS`
- `LXS_API_ERR_UNSUPPORTED`
- `LXS_API_ERR_INTERNAL`

And one message path:

- thread-local or API-context last error string

## 13. File and Layer Layout

Recommended new files:

- `include/lxs_api.h`
- `src/core/lxs_api.c`

Optional later:

- `include/lxs_api_types.h`

The wrapper layer should call into existing internals already declared in
[lxs_types.h](/c:/DEV/LXS/include/lxs_types.h).

## 14. Python Binding Strategy

Python is the first integration target.

Recommended sequence:

1. Stable C API first
2. Thin Python binding second

Preferred first binding approach:

- `ctypes` or `cffi` for early proof

Preferred later if needed:

- a dedicated extension module once the API is stable

The important rule is:

- do not bind directly against internal structs
- bind only to the public API layer

## 15. Java Position

Java is explicitly deferred, not rejected.

If needed later, Java should also bind to the same public C API boundary.

That keeps:

- one engine truth
- one exported ABI
- multiple front-end languages on top

## 16. Admission Sequence

This phase should be admitted in bounded steps.

### Step 1

Public API plan and contract

### Step 2

Create minimal C API wrapper with:

- load
- compile
- create engine
- apply inputs
- tick
- read outputs
- free/reset

### Step 3

Add structural inspection:

- counts
- names
- net lookup
- plan metadata

### Step 4

Add state inspection:

- nets
- registers
- RAM
- regfiles

### Step 5

Add probes and diagnostics

### Step 6

Add first Python proof harness

## 17. Validation Requirements

Every admitted public API layer step must have:

- direct C-level tests
- no semantic drift relative to internal engine behavior
- explicit ownership/lifetime tests
- Python binding smoke tests once bindings begin

The first API phase is correctness and access first, not speed first.

## 18. Risks To Avoid

1. Exposing internal structs as public ABI
2. Exposing too little state for the UI/tooling vision
3. Per-bit FFI chatter that makes Python look slow
4. Binding to unstable internal memory layout
5. Mixing debug-only state into the public core without naming it clearly

## 19. Immediate Next Step

Implement the first public API shim:

- add [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h)
- add [lxs_api.c](/c:/DEV/LXS/src/core/lxs_api.c)
- expose:
  - load/free netlist
  - compile/free plan
  - create/reset/free engine
  - apply inputs
  - tick
  - read outputs
  - read probes

That is the clean first admission boundary before deeper state access.
