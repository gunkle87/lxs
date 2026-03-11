# LXS Architecture

This document describes the target architecture for LXS as a real deterministic zero-delay 4-state logic simulator.

The goal is not to preserve the current kernel-only benchmark number. The goal is to build an honest engine that can still approach or exceed `1G GEPS` where:

`GEPS = total gate evaluations / total elapsed seconds`

## 1. Design Target

LXS should be:

- Deterministic
- Zero-delay
- True 4-state
- Cache-aware
- Data-oriented
- Compiled ahead of execution
- Benchmarkable in honest stages

The central rule is simple:

Runtime must execute offsets and spans, not names, maps, hashes, or pointer-chasing relationships.

## 2. What Counts As Success

The final engine should:

- Parse real `.bench` style circuits
- Compile them into a fixed execution plan
- Apply real primary inputs
- Evaluate real combinational logic
- Update sequential state in a separate commit phase
- Produce real outputs
- Report honest throughput using full elapsed runtime

If a benchmark excludes wiring, input movement, state commit, or output extraction, it is not a product metric. It is only a subsystem metric.

## 3. Performance Philosophy

There are three meaningful performance numbers:

1. Kernel roof
   Pure gate math on already prepared contiguous buffers.
2. Wired execution
   Real compiled connectivity, real input/output/state movement, minimal tooling.
3. Product throughput
   Full engine semantics plus the standard benchmark harness.

We should track all three. The product number matters most. The other two explain where time is being lost.

## 4. Core Runtime Model

LXS should be a compiled-offset simulator.

The compiler does the expensive structural work once:

- parse gate definitions
- assign every net a physical storage location
- cut sequential boundaries
- levelize combinational logic
- group compatible gates into spans
- emit fixed execution schedules

The runtime then performs only:

- input load
- combinational span execution
- next-state capture
- state commit
- output extraction

No runtime name lookup should exist in the hot path.

## 5. Logic Representation

Use true 4-state dual-rail storage:

- `value`
- `mask`

Interpretation:

- `value=0, mask=0` -> `0`
- `value=1, mask=0` -> `1`
- `mask=1` -> unknown or high impedance class carried by policy

The exact semantic split between `X` and `Z` can evolve, but the storage and primitive layer must be able to support true 4-state behavior without collapsing to 2-state shortcuts in the final engine.

## 6. Storage Layout

Use structure-of-arrays, not array-of-structures.

Primary banks:

- `pi_value[]`
- `pi_mask[]`
- `comb_value[]`
- `comb_mask[]`
- `state_cur_value[]`
- `state_cur_mask[]`
- `state_next_value[]`
- `state_next_mask[]`
- `po_value[]`
- `po_mask[]`

Rules:

- All banks should be 64-byte aligned.
- All hot loops should stream over contiguous memory.
- Separate value and mask banks should remain parallel and identically indexed.
- Runtime-visible net identity should be an integer offset, not a pointer and not a string.

## 7. Net Allocation Strategy

Every net should receive a compiled physical home.

There are two valid approaches:

1. Shared-location model
   Each logical net owns one physical slot. Producers write to that slot. Consumers read from that slot.
2. Staged-copy model
   Producers write to packed temporary regions. A compiled copy/scatter schedule materializes consumer-facing layout before the next phase.

The first model is simpler and more faithful. The second may win if span packing becomes much better.

The important rule is that the choice must be made at compile time, not improvised at runtime.

## 8. Compiler Pipeline

Recommended compiler stages:

1. Parse
   Read the netlist into a temporary structural IR.
2. Symbol resolution
   Turn names into stable integer net IDs.
3. Sequential cut
   Split DFF outputs from DFF inputs so combinational levelization sees the proper cycle boundary.
4. Combinational DAG build
   Build only the current-cycle combinational dependencies.
5. Levelization
   Assign level numbers to combinational gates only.
6. Net storage assignment
   Assign every source and destination net a physical bank and offset.
7. Span formation
   Group gates by type, level, and compatible storage layout.
8. Schedule emission
   Emit the final runtime plan.

The emitted plan should be immutable after compilation.

## 9. Runtime Plan Format

The runtime plan should contain:

- input load schedule
- level count
- per-level span lists
- next-state capture schedule
- state commit schedule
- output extraction schedule
- metadata for probing and debugging

Each span should be compact and execution-ready. A practical shape is:

- gate type
- count
- source A base offset
- source B base offset if needed
- destination base offset
- source bank IDs
- destination bank ID

If a span cannot be expressed as a simple streaming region, it should not be in the fast path.

## 10. Span Design Rules

Span grouping is where throughput is won or lost.

Good spans:

- same gate type
- same level
- same bank pattern
- contiguous source and destination ranges
- no per-element branching

Bad spans:

- mixed gate types
- strided gathers in the hot loop
- per-gate indirect pointers
- runtime source classification

If a gate cluster cannot meet the good-span rules, move it to a slower secondary path and keep the main path clean.

## 11. Sequential Semantics

DFF handling must be strict.

Rules:

- DFF outputs belong to current state
- DFF inputs are combinational next-state results
- no DFF update occurs during combinational level execution
- all DFF writes occur in a separate commit phase
- commit ordering must be deterministic

Recommended runtime sequence:

1. Load primary inputs
2. Expose current state to combinational logic
3. Execute combinational levels in order
4. Capture DFF inputs into `state_next`
5. Commit `state_next -> state_cur`
6. Extract primary outputs

That is the minimum model for deterministic zero-delay synchronous behavior.

## 12. Input And Output Handling

Primary inputs and outputs are part of the real engine cost and must be included in honest throughput runs.

Input load should be a compiled schedule:

- source input vector index
- destination bank offset

Output extraction should also be compiled:

- source bank offset
- destination output vector index

Do not special-case I/O with string lookups or ad hoc code during simulation.

## 13. True 4-State Strategy

True 4-state correctness is expensive. We should preserve correctness while avoiding per-gate chaos.

Recommended approach:

- Keep a universal dual-rail representation.
- Keep the primitive math branchless at gate level.
- Add chunk-level fast paths only when they are semantically exact.

Example:

- A chunk known to have `mask == 0` for all inputs may run a cheaper known-state path.
- A chunk containing any unknowns falls back to the full dual-rail path.

This is acceptable only if the branch is outside the per-gate inner loop and the result is identical.

## 14. Memory Locality Rules

The hot path should obey these rules:

- unit-stride reads
- unit-stride writes
- no pointer chasing
- no runtime symbol resolution
- no per-gate allocation
- no per-gate function-pointer dispatch
- no metric derivation inside the engine

Dispatch should happen at span granularity, not gate granularity.

## 15. Honest Probe Model

The engine should expose raw facts only.

Permanent useful counters:

- `input_apply`
- `gate_eval`
- `tick_count`
- `dff_exec`
- `state_commit_count`
- `span_exec`

These must be:

- O(1)
- updated on existing execution paths
- free of benchmark-only loops and scans

All rates, averages, and summary metrics belong outside the engine.

## 16. What Must Not Exist In The Hot Path

The following are unacceptable in the final fast path:

- string compares
- hash lookups
- linked structures
- runtime topological decisions
- per-gate branching on type
- per-gate pointer arrays
- scattered random net fetches
- benchmark-only logic

If any of these show up in the main execution loop, throughput will collapse before the engine becomes honest.

## 17. Validation Strategy

Performance work without correctness work is wasted.

We need three validation layers:

1. Primitive tests
   Verify every gate truth table for 4-state behavior.
2. Small circuit tests
   Verify known combinational and sequential reference circuits.
3. End-to-end benchmark validation
   Confirm that parsed benchmark circuits produce stable deterministic results for known stimulus.

The performance target is irrelevant if the engine cannot prove it is simulating correctly.

## 18. Measurement Plan

To avoid lying to ourselves, measure these stages separately:

1. Primitive kernel
2. Span executor on precompiled fake data
3. Real compiled combinational engine
4. Full engine with DFF commit
5. Full engine with input/output handling
6. Benchmark runner

Each stage should have:

- total gate evaluations
- elapsed seconds
- GEPS
- bytes touched if practical
- level count
- span count
- average span size

This will show whether losses are caused by:

- poor span packing
- storage layout
- state commit
- I/O movement
- tiny spans
- cache misses

## 19. Recommended Near-Term Build Order

Build in this order:

1. Honest compiler IR
   Create net IDs, gate records, sequential cuts, and levelization.
2. Physical storage assignment
   Assign compiled bank offsets for every net.
3. Real combinational binding
   Replace the current zero-filled isolated span buffers with real source and destination bindings.
4. Honest DFF path
   Capture next-state and commit into current-state.
5. Primary I/O plans
   Add real input application and output extraction.
6. Correctness suite
   Lock down primitive and small-circuit behavior.
7. Performance tuning
   Optimize layout, span sizes, and dispatch only after the engine is real.

This order matters. It avoids tuning a fake execution model.

## 20. Recommended Data Structures

Keep the core flat and procedural.

Suggested structures:

- `lxs_symbol_table`
  Temporary compiler-only structure for parse and resolution.
- `lxs_gate_ir`
  Structural gate record with net IDs.
- `lxs_net_storage`
  Bank and offset assignment for each logical net.
- `lxs_span_plan`
  Runtime-ready span descriptor.
- `lxs_level_plan`
  Array of span descriptors for one level.
- `lxs_state_plan`
  DFF capture and commit schedules.
- `lxs_io_plan`
  Input load and output extract schedules.
- `lxs_exec_plan`
  Final immutable runtime plan.

Compiler-only structures may use slower conveniences. Runtime structures must stay compact and linear.

## 21. Expected Throughput Reality

The current kernel-style number is not the real baseline for the final simulator.

A realistic path is:

- raw kernel number drops when real wiring is introduced
- some loss is recovered by better compiled layout and larger spans
- further loss comes from true 4-state handling, state movement, and I/O

The right question is not:

"Can we keep the fake number?"

The right question is:

"Can we design the real engine so the unavoidable losses are limited and measurable?"

That is the only way to have a believable path to `1G GEPS`.

## 22. Immediate Next Step

The next implementation step should be:

Create a real compiler IR and physical storage assignment layer, then replace the current isolated span-local arrays with compiled bank offsets and real net binding.

Until that exists, LXS is measuring a kernel, not yet a simulator.
