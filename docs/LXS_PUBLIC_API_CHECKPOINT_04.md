# LXS Public API Checkpoint 04

## Family
- Public C API

## Scope
- Admit the first diagnostics and probe polish layer with:
  - explicit last-error code access
  - explicit last-error clear
  - probe reset
  - expanded public probe snapshot fields

## Contract
- Every API call updates the public diagnostic state through:
  - result code return
  - thread-local last-error code
  - thread-local last-error message
- Clearing diagnostics resets:
  - last-error code to `LXS_API_OK`
  - last-error message to empty
- Clearing probes resets the engine probe counters without resetting engine state
- Extended probe fields are always present in the public struct:
  - when internal test-probe fields are unavailable, they read as zero

## Implementation
- Extended [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h) with:
  - `lxs_api_diag_get_last_error_code`
  - `lxs_api_diag_clear_last_error`
  - `lxs_api_engine_clear_probes`
- Expanded [lxs_api.h](/c:/DEV/LXS/include/lxs_api.h) `lxs_api_probes` to include stable slots for extended semantic counters
- Extended [lxs_api.c](/c:/DEV/LXS/src/core/lxs_api.c) to track:
  - thread-local last-error code
  - thread-local last-error message
- Extended [lxs_api.c](/c:/DEV/LXS/src/core/lxs_api.c) probe export so public callers can read a richer probe snapshot without depending on internal compile-time layout details

## Validation
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- Added a diagnostics smoke path in [lxs_test.c](/c:/DEV/LXS/src/apps/lxs_test.c) that verifies:
  - invalid lookup sets `LXS_API_ERR_BOUNDS`
  - last-error message captures failing lookup detail
  - clearing diagnostics restores the neutral state
  - clearing probes zeroes tick and commit counters without tearing down the engine

## Decision
- Admit the first diagnostics and probe polish layer

## Notes
- This completes Step 5 of:
  - [LXS_PUBLIC_API_PLAN.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_PLAN.md)
- The public C API now covers:
  - lifecycle
  - execution
  - structure and naming
  - live state
  - probes
  - diagnostics
- The next step is Step 6:
  - first Python proof harness
