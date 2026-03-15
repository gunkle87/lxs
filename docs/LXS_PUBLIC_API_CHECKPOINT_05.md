# LXS Public API Checkpoint 05

## Family
- Public C API
- Python proof harness

## Scope
- Admit the first Python proof harness on top of the public C API
- Produce a loadable Windows DLL build of the API layer
- Validate Python-side lifecycle, structure, state, probes, and diagnostics access through `ctypes`

## Contract
- Python integration binds only to the public C API
- Python does not depend on:
  - internal structs
  - internal headers
  - executable entrypoints
- The proof harness uses the exported DLL:
  - [lxs_api.dll](/c:/DEV/LXS/build/bin/lxs_api.dll)
- The DLL build is self-contained enough for local Python proof use in the checked-in environment

## Implementation
- Extended [build_lxs.ps1](/c:/DEV/LXS/tools/build_lxs.ps1) to build:
  - `lxs_api.dll`
- Extended [build_lxs.ps1](/c:/DEV/LXS/tools/build_lxs.ps1) to copy the required Windows MinGW runtime dependency:
  - `libwinpthread-1.dll`
- Added reusable Python wrapper:
  - [lxs_api.py](/c:/DEV/LXS/tools/python/lxs_api.py)
- Added Python proof harness:
  - [lxs_api_smoke.py](/c:/DEV/LXS/tools/python/lxs_api_smoke.py)

## Validation
- Rebuild completed successfully with:
  - `lxs_api.dll`
  - `lxs_bench.exe`
  - `lxs_compare.exe`
  - `lxs_test.exe`
- [lxs_test.exe](/c:/DEV/LXS/build/bin/lxs_test.exe): passed
- Python proof harness:
  - `python tools\python\lxs_api_smoke.py`
  - passed

## Python Proof Coverage
- MUX smoke:
  - load
  - compile
  - engine create
  - input apply
  - tick
  - output read
  - net read
  - structural name lookup
- Register smoke:
  - register metadata
  - whole-register read after tick
- RAM smoke:
  - RAM metadata
  - initial word reads
  - committed write visibility
- Regfile smoke:
  - regfile metadata
  - initial word reads
  - committed write visibility
  - probe read
  - probe clear

## Decision
- Admit the first Python proof harness

## Notes
- This completes Step 6 of:
  - [LXS_PUBLIC_API_PLAN.md](/c:/DEV/LXS/docs/LXS_PUBLIC_API_PLAN.md)
- The public API phase now has an end-to-end proof from:
  - C ABI
  - exported DLL
  - Python `ctypes`
  - live engine interaction
- The next natural phase is no longer API bootstrapping.
  It is either:
  - a richer Python integration layer
  - or the first actual Python-side tooling/UI work
