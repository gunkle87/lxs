## Benchmark Layout

`Benchmarks\Benches`

- Runnable LXS BENCH-dialect suites.
- Includes the historical `ISCAS85` and `ISCAS89` sets.
- Includes `ITC99`, which was downloaded directly as `.bench` files and can be swept by `lxs_bench.exe` immediately.
- Includes `EPFL`, which is now versioned as converted `.bench` artifacts.

`Benchmarks\Sources`

- Staged benchmark sources that are useful for the next baseline expansion but are not yet in the current LXS BENCH dialect.
- `EPFL\benchmarks` contains the EPFL benchmark repository.
- `IWLS2005\IWLS_benchmarks_2005_V_1.0` contains the extracted IWLS 2005 benchmark archive.

## Notes

- `ITC99` is directly runnable.
- `EPFL` is committed as converted `.bench` files produced offline before entering the engine repo.
- BLIF and flat structural Verilog inputs must be converted with `blif2bench\\net2bench.exe` before they enter the LXS engine workflow.
- The live engine path is canonical BENCH only.
- `IWLS2005` remains staged until a compatible source/parser path is added.
- `tools\lxs_rewrite.ps1` is the unified offline arithmetic rewrite entry point.
- `tools\lxs_rewrite_report.ps1` regenerates the current trusted rewrite report for original vs anchor vs packed arithmetic benches.
