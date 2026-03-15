# LXS Public API Checkpoint 02

## Family
- Public C API

## Scope
- Admit the first structural inspection and naming layer with:
  - input net ID lookup by ordered input index
  - output net ID lookup by ordered output index
  - input name lookup
  - output name lookup
  - net ID to name lookup
  - name to net ID lookup

## Contract
- Structural name access is plan-owned:
  - returned strings remain valid until the API plan is freed
- Name lookup is based on compiled-plan net IDs, not pre-compile netlist IDs
- Input and output index order matches the compiled plan order
- Name lookup is stable across:
  - `lxs_api_plan_get_input_net_id`
  - `lxs_api_plan_get_output_net_id`
  - `lxs_api_plan_get_net_name`
  - `lxs_api_plan_find_net`

## Implementation
- Extended [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h) with structural inspection accessors
- Extended [lxs_api.c](/c:/DEV/LXS/src/core/lxs_api.c) so API plans own a copied name table
- Fixed compiled-name alignment in [lxs_compiler.c](/c:/DEV/LXS/src/core/lxs_compiler.c) by remapping `net_names` alongside net IDs during compile
- Extended the public API smoke test in [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c) to verify:
  - input IDs
  - output IDs
  - input names
  - output names
  - net-name reverse lookup

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- Public API smoke path now verifies compiled structural naming on:
  - [mux2_8_explicit.bench](/c:/DEV/LXS/Tests/Circuits/mux2_8_explicit.bench)
- Validation confirms public naming matches the compiler’s remapped internal plan:
  - `17` ordered inputs
  - `8` ordered outputs
  - direct name-to-ID round-trips

## Decision
- Admit the first structural inspection and naming layer

## Notes
- This completes Step 3 of:
  - [LXS_PUBLIC_API_PLAN.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_PLAN.md)
- Deeper state access remains deferred to the next API step:
  - arbitrary net value/mask reads
  - register reads
  - RAM reads
  - regfile reads
