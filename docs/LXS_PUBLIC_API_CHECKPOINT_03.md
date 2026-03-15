# LXS Public API Checkpoint 03

## Family
- Public C API

## Scope
- Admit the first state inspection layer with:
  - single-net reads
  - register metadata and whole-register reads
  - RAM metadata and whole-word reads
  - regfile metadata and whole-word reads

## Contract
- State reads reflect the current committed engine state after the most recent tick
- Net reads use compiled-plan net IDs
- Register reads operate on one whole register descriptor at a time
- RAM and regfile reads operate on one whole stored word at a time
- Word-width arguments must match the descriptor width exactly
- Address arguments must be within the descriptor depth

## Implementation
- Extended [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h) with:
  - `lxs_api_engine_read_net`
  - `lxs_api_plan_get_register_info`
  - `lxs_api_engine_read_register`
  - `lxs_api_plan_get_ram_info`
  - `lxs_api_engine_read_ram_word`
  - `lxs_api_plan_get_regfile_info`
  - `lxs_api_engine_read_regfile_word`
- Extended [lxs_api.c](/c:/DEV/LXS/src/core/lxs_api.c) to read directly from the live runtime storage owned by:
  - `net_value/net_mask`
  - `register_value/register_mask`
  - `ram_value/ram_mask`
  - `regfile_value/regfile_mask`
- Kept the API coarse-grained and binding-friendly:
  - whole object
  - whole word
  - no per-bit API chatter as the primary path

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- Added a dedicated state smoke path in [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c) that validates public API state reads against the internal engine state on:
  - [mux2_8_explicit.bench](/c:/DEV/LXS/Tests/Circuits/mux2_8_explicit.bench)
  - [reg16_explicit.bench](/c:/DEV/LXS/Tests/Circuits/reg16_explicit.bench)
  - [ram8_explicit.bench](/c:/DEV/LXS/Tests/Circuits/ram8_explicit.bench)
  - [regfile8_explicit.bench](/c:/DEV/LXS/Tests/Circuits/regfile8_explicit.bench)
- Validation covers:
  - randomized net-state comparison
  - repeated register-state comparison across ticks
  - repeated RAM-word comparison across ticks and all addresses
  - repeated regfile-word comparison across ticks and all addresses

## Decision
- Admit the first public state inspection layer

## Notes
- This completes Step 4 of:
  - [LXS_PUBLIC_API_PLAN.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_PLAN.md)
- ROM read access is still deferred because it is static compiled data rather than live mutable state
- Probe and diagnostics access remains available from the earlier shim admission
