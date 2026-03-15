# STANDARD_MACRO_LIBRARY_PLAN.md

Defines the next architecture phase for explicit built-in macro blocks intended
for direct end-user CPU and system construction.

------------------------------------------------------------------------

## 1. Purpose
    This phase creates a fixed standard macro library owned by the engine.

    These macros are:
    - explicit
    - built-in
    - semantic
    - black-box from the user point of view

    They are not:
    - recognizers
    - inferred replacements
    - heuristic graph rewrites
    - user-defined arbitrary inline macros

    The goal is to let users build large digital systems from a much smaller
    number of high-value primitives.

## 2. Why This Phase Exists
    The previous phases produced a clear answer:
    - recognition found limited but real value
    - packed and runtime work produced larger practical wins
    - the engine now has enough structure and discipline to support larger
      semantic blocks directly

    The next useful step is not more recognizer hunting.

    The next useful step is a controlled standard library of built-in macro
    blocks that users can instantiate directly.

## 3. Core Goal
    Reduce the primitive count required to model real CPUs and similar systems
    by introducing a limited but feature-rich library of built-in functional
    blocks executed by the engine as first-class primitives.

    The success condition is not:
    - maximum number of macro types

    The success condition is:
    - minimum user-visible primitive count for realistic CPU construction
    - without semantic ambiguity
    - without recognition complexity
    - without turning the engine into a loose macro language

## 4. Core Principles
    1. Explicit only
       Every standard library macro must be user-instantiated directly.
       No recognizer or heuristic path may silently substitute one.

    2. Engine-owned semantics
       Macro semantics are defined by the engine and versioned with the engine.
       Users do not supply arbitrary behavioral definitions in this phase.

    3. Library before language
       This phase builds a curated fixed library first.
       User-defined reusable macros remain a future possibility, not current
       scope.

    4. Semantic blocks, not dressed-up gates
       The library should prioritize CPU construction leverage:
       - arithmetic blocks
       - selection blocks
       - storage blocks
       - memory blocks

    5. Validation before expansion
       No macro family should expand rapidly before:
       - semantic contract is stable
       - reference lowering exists
       - correctness tests exist
       - benchmark behavior is measured honestly

    6. One family at a time
       This phase should still advance in bounded families, not as a
       simultaneous explosion of unrelated macro kinds.

## 5. What Counts As A Standard Macro
    A standard macro in this phase is a built-in block with:
    - fixed name
    - fixed port contract
    - fixed width family or fixed width variants
    - fixed timing semantics
    - fixed 4-state behavior expectations
    - dedicated compiler lowering and runtime execution path

    Each macro must be able to exist in two forms:

    1. Runtime form
       A dedicated descriptor and execution path used by the engine.

    2. Reference form
       A trusted lowered structural form used only for correctness validation.

## 6. Phase Scope
    This phase covers:
    - built-in standard macro naming
    - semantic contracts
    - compiler/runtime representation
    - explicit library admission discipline
    - correctness validation
    - benchmark qualification

    This phase does not cover:
    - user-defined arbitrary macros
    - automatic macro inference
    - macro scripting language design
    - HDL-like behavioral authoring
    - broad memory compiler features

## 7. Library Design Target
    The library should enable construction of:
    - simple datapaths
    - ALU-centered CPUs
    - register-heavy control logic
    - instruction ROM plus data RAM systems
    - small register-file-based machines

    The library should not try to solve:
    - every digital design problem
    - arbitrary DSP fabrics
    - configurable FPGA-style routing
    - unlimited memory-model variation

## 8. First-Wave Library
    The first wave should stay intentionally tight.

    TIER 1

    Selection
    - `MUX2_8`
    - `MUX2_16`
    - `MUX2_32`
    - `MUX2_64`
    - `MUX4_8`
    - `MUX4_16`
    - `MUX4_32`
    - `MUX4_64`

    Registers
    - `REG8`
    - `REG16`
    - `REG32`
    - `REG64`
    - `REG_EN8`
    - `REG_EN16`
    - `REG_EN32`
    - `REG_EN64`
    - `REG_EN_RST8`
    - `REG_EN_RST16`
    - `REG_EN_RST32`
    - `REG_EN_RST64`

    Arithmetic
    - `ADD8`
    - `ADD16`
    - `ADD32`
    - `ADD64`
    - `ALU8`
    - `ALU16`
    - `ALU32`
    - `ALU64`

    Memory
    - `ROM16`
    - `RAM8`
    - `RAM16`
    - `RAM24`

    TIER 2

    Arithmetic / control support
    - `CMP8`
    - `CMP16`
    - `CMP32`
    - `CMP64`
    - `COUNTER8`
    - `COUNTER16`
    - `COUNTER32`
    - `COUNTER64`
    - `COUNTER_EN8`
    - `COUNTER_EN16`
    - `COUNTER_EN32`
    - `COUNTER_EN64`

    Storage support
    - `REGFILE8_2R1W`
    - `REGFILE16_2R1W`
    - `REGFILE32_2R1W`
    - optional later:
      - `REGFILE64_2R1W`

    Optional later additions after proof
    - `SHIFTER8`
    - `SHIFTER16`
    - `SHIFTER32`
    - `SHIFTER64`

## 9. Naming Rules
    Naming should remain fixed and boring.

    Recommended pattern:
    - `<FAMILY><WIDTH>`
    - `<FAMILY>_<WIDTH>` only if separator clarity is required
    - multi-port storage may include access shape:
      - `REGFILE32_2R1W`

    Avoid:
    - marketing names
    - ambiguous aliases
    - too many near-duplicate variants

## 10. Semantic Contract Requirements
    Every standard macro family must define:

    1. Port contract
       - inputs
       - outputs
       - width
       - control signals

    2. Timing contract
       - combinational
       - sequential
       - read timing
       - write timing
       - state update timing

    3. 4-state behavior
       - how `X` propagates
       - how unknown control signals behave
       - whether outputs become unknown under ambiguous control

    4. Reset / enable precedence if applicable
       Example:
       - reset over enable
       - enable over data

    5. Read/write conflict behavior for memories
       Example:
       - sync read or async read
       - read-during-write behavior
       - out-of-range or unknown-address behavior

## 11. Family-Specific Semantic Expectations
    ALU
    - define supported operations explicitly
    - first admitted ALU contract:
      - inputs:
        - `a[W]`
        - `b[W]`
        - `op0`
        - `op1`
        - `op2`
      - outputs:
        - `y[W]`
        - `cout`
        - `eq`
        - `lt`
        - `gt`
      - op encoding:
        - `000` = add
        - `001` = sub
        - `010` = and
        - `011` = or
        - `100` = xor
        - `101` = pass `a`
        - `110` = pass `b`
        - `111` = zero
      - `eq/lt/gt` are always the unsigned compare flags for `a` versus `b`
      - `cout` is meaningful for add/sub and forced to zero for the other ops
    - avoid feature creep in the first wave

    RAM
    - admitted RAM contract:
      - fixed families:
        - `RAM8`
        - `RAM16`
        - `RAM24`
      - data output width is fixed by family name
      - address width is inferred from the explicit read/write address input lists
      - read address width must equal write address width
      - initialization words must match:
        - `2^addr_width`
      - read behavior is combinational over current committed storage contents
      - write behavior is synchronous and becomes visible only after commit
      - write-enable is active-high
      - same-cycle read-during-write returns the pre-commit stored value
      - unknown address and control behavior follows the existing RAM descriptor semantics

    ROM
    - first admitted ROM contract:
      - fixed family:
        - `ROM16`
      - data output width is fixed at 16 bits
      - address width is inferred from the explicit address input list
      - initialization words must match:
        - `2^addr_width`
      - read behavior is combinational over the compiled ROM image
      - ROM contents are fixed at compile/load time
      - ROM does not participate in writeback or commit machinery
      - unknown address bits propagate through the existing ROM read semantics

    Registers
    - define:
      - clock model
      - enable behavior
      - reset behavior
      - power-on or initial state expectations if any
    - first admitted reset contract:
      - synchronous
      - active-high
      - reset value is all-zero
      - reset takes precedence over enable
      - when reset is not asserted, enable semantics follow the existing
        register-enable contract
      - unknown reset participates in normal 4-state control propagation rather
        than being silently coerced to 0 or 1

    Comparators
    - first admitted comparator contract:
      - unsigned only
      - outputs are ordered:
        - `eq`
        - `lt`
        - `gt`
      - no signed interpretation in the first admitted family
      - no flag packing or encoded status word in the first admitted family

    Register file
    - first admitted register-file contract:
      - fixed families:
        - `REGFILE8_2R1W`
        - `REGFILE16_2R1W`
        - `REGFILE32_2R1W`
      - two read ports and one write port
      - outputs are ordered:
        - `read_a[W]`
        - `read_b[W]`
      - data width is fixed by family name
      - depth is inferred from the explicit address input lists
      - read-a, read-b, and write address widths must match
      - read behavior is combinational over current committed storage contents
      - write behavior is synchronous and becomes visible only after commit
      - write-enable is active-high
      - same-cycle read-during-write returns the pre-commit stored value on both read ports
      - 4-state behavior follows the admitted memory-family conventions

    Counters
    - first admitted counter contract:
      - fixed families:
        - `COUNTER8`
        - `COUNTER16`
        - `COUNTER32`
        - `COUNTER64`
        - `COUNTER_EN8`
        - `COUNTER_EN16`
        - `COUNTER_EN32`
        - `COUNTER_EN64`
      - `COUNTER*` increments once per tick
      - `COUNTER_EN*` increments once per tick only when enable is asserted
      - enable is active-high
      - count wraps modulo `2^W`
      - updated state becomes visible after commit
      - unknown enable follows the existing counter-control 4-state behavior

## 12. Runtime Representation
    The compiler should emit dedicated macro descriptors, not expanded runtime
    primitive graphs.

    A standard macro runtime descriptor should include at least:
    - macro type
    - width
    - control mode if applicable
    - stable input offsets
    - stable output offsets
    - state or memory backing references if sequential
    - gate-equivalent accounting

    The engine should execute each macro family through a dedicated kernel.

## 13. Reference Lowering Requirement
    Every standard macro must have a trusted reference form.

    The reference form is not the user-facing execution path.
    It exists so we can:
    - compare outputs against a lowered primitive form
    - write deterministic fixture tests
    - qualify correctness before optimizing runtime behavior

    No family may be admitted without a reference lowering path or equivalent
    structural oracle.

## 14. Admission Sequence
    Every macro family must pass these gates in order.

    1. Family spec
       Define the block, width variants, and intended use.

    2. Semantic contract
       Define exact timing and 4-state behavior.

    3. Reference lowering
       Create a trusted lowered form or equivalent reference path.

    4. Explicit fixture tests
       Add positive and negative tests at the parser/compiler/runtime level.

    5. Equivalence proof
       Show runtime form matches the reference form.

    6. Benchmark qualification
       Measure:
       - explicit fixture behavior
       - relevant benchmark or workload behavior
       - CPU-construction leverage if applicable

    7. Admission decision
       End with one of the allowed outcomes.

## 15. Allowed Decision Outcomes
    ADMIT
    HOLD FOR LATER
    REJECT
    KEEP AS ALTERNATE

## 16. Checkpoint Requirement
    After each family admission attempt, write a checkpoint that includes:
    - family name
    - width variants covered
    - semantic contract summary
    - files changed
    - reference lowering method
    - correctness evidence
    - benchmark evidence
    - maintenance risk
    - decision
    - next allowed move

    Future checkpoint docs should use:
    - [CHECKPOINT_TEMPLATE.md](/c:/DEV/LXS/docs/recognition/CHECKPOINT_TEMPLATE.md)
    - [ADMISSION_UPDATE_PROTOCOL.md](/c:/DEV/LXS/docs/recognition/ADMISSION_UPDATE_PROTOCOL.md)

## 17. Validation Strategy
    Each family should be validated at three levels.

    1. Fixture correctness
       Small explicit test circuits using the built-in macro directly.

    2. Lowered equivalence
       Compare against the trusted lowered primitive form.

    3. System leverage
       Show that the macro meaningfully reduces primitive count or user-visible
       construction complexity for CPU-style designs.

    Throughput wins are welcome, but this phase is not judged only by GEPS.
    The library is also a usability and modeling-power phase.

## 18. Non-Goals
    This phase must not:
    - silently infer these macros
    - expose arbitrary user-defined behavior
    - become a hardware description language inside BENCH
    - explode into dozens of lightly different variants
    - blur sequential semantics
    - relax correctness in the name of convenience

## 19. Future User-Defined Macro Position
    User-defined reusable macros remain a valid future direction.

    They are explicitly deferred in this phase because:
    - they raise language-design complexity sharply
    - they complicate validation and compatibility
    - they are easier to do well after the built-in library semantics are proven

    If revisited later, that work should likely start as:
    - offline-authored reusable macro libraries
    - validated and importable
    - not arbitrary inline behavioral definitions in the first pass

## 20. Recommended Build Order
    1. Selection family
       Start with `MUX2_*` and `MUX4_*`

    2. Register family
       Add `REG_*`, then `REG_EN_*`, then `REG_EN_RST_*`

    3. Arithmetic family
       Add `ADD_*`, then `ALU_*`

    4. Memory family
       Add `ROM16`, then `RAM8/16/24`

    5. Tier 2 support
       Add counters, compare blocks, and register files

    This order gives early CPU-building value without taking on all hard
    semantics at once.

## 21. Success Criteria
    This phase is successful when:
    - the first-wave library is explicit and stable
    - semantics are unambiguous
    - reference lowerings exist
    - users can express CPU-scale structures with dramatically fewer
      primitives
    - the runtime remains honest and maintainable

## 22. Stopping Conditions
    Stop or pause a family if:
    - semantic contract becomes fuzzy
    - reference lowering cannot be trusted
    - memory behavior is underdefined
    - variant count starts growing faster than user value
    - implementation complexity grows faster than the leverage provided

## 23. Immediate Next Step
    Begin with the first Tier 1 family:
    - `MUX2_*`
    - `MUX4_*`

    These are:
    - high leverage
    - easy to reason about
    - broadly useful
    - a safe way to validate the standard library architecture before moving
      into registers, arithmetic, and memories
