# LXS Public API Checkpoint 01

## Family
- Public C API

## Scope
- Admit the first public C API shim layer with:
  - netlist load/free
  - plan compile/free
  - engine create/reset/free
  - ordered input application
  - single tick and multi-tick execution
  - ordered output reads
  - probe reads
  - plan and engine count accessors

## Contract
- Public consumers use opaque handles only:
  - `lxs_api_netlist`
  - `lxs_api_plan`
  - `lxs_api_engine`
- Public callers do not depend on internal structs:
  - `lxs_netlist`
  - `lxs_plan`
  - `lxs_engine_ctx`
- Result reporting is via explicit `lxs_api_result` codes plus:
  - `lxs_api_get_last_error()`
- Input and output access is ordered-index based in this first admission
- Probe access is exposed through a stable public `lxs_api_probes` struct
- Engine handles borrow the compiled plan for lifetime:
  - the plan must remain alive until the engine is freed

## Implementation
- Added public API header:
  - [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h)
- Added wrapper implementation:
  - [lxs_api.c](/c:/DEV/LXS/src/core/lxs_api.c)
- Kept the existing engine/compiler implementation as the execution truth
- Added the shim to the standard build through:
  - [build_lxs.ps1](/c:/DEV/LXS/tools/build_lxs.ps1)
- Added a direct smoke test in:
  - [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c)

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- Public API smoke path:
  - loads `mux2_8_explicit.bench`
  - compiles through the public API
  - creates an engine through the public API
  - applies `64` randomized input vectors
  - compares public-API outputs against the internal engine path on every iteration
  - validates probe tick counts for:
    - `tick`
    - `reset`
    - `tick_many`

## Decision
- Admit the first public C API shim

## Notes
- This is the minimal lifecycle-and-IO admission from Step 2 of:
  - [LXS_PUBLIC_API_PLAN.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_PLAN.md)
- Deeper inspection is still deferred:
  - named net lookup
  - arbitrary net reads
  - register reads
  - RAM reads
  - regfile reads
  - structural name enumeration
