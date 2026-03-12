# SEQUENTIAL_PROTOCOL.md

Defines rules governing sequential-state representation, storage, and update
discipline.

------------------------------------------------------------------------

## 1. Purpose
    Sequential features must be introduced through compiled descriptors and
    stable runtime storage, not through ad hoc object graphs or per-access
    allocation.

    The sequential model must preserve:
    -   deterministic execution
    -   explicit update boundaries
    -   stable index-based addressing
    -   no tick-path allocation

## 2. Sequential Families
    Sequential state is grouped into three families only:

    1. Bit State
       -   DFF
       -   other bit-level primitives only if later proven necessary

    2. Register State
       -   fixed-width registers
       -   arbitrary packed-width registers

    3. Memory State
       -   ROM
       -   RAM

    DFF remains the only primitive sequential element unless a new primitive
    is proven to be more efficient than compiled register or memory state.

## 3. Descriptor Rule
    Every sequential resource must compile into a descriptor.

    Descriptors must use stable integer indices or byte offsets.
    Runtime state must never depend on pointer-chasing object graphs.

    Descriptor examples:
    -   bit-state descriptor
    -   register descriptor
    -   ROM descriptor
    -   RAM descriptor

## 4. Storage Rule
    Sequential storage must be contiguous and allocated at load or compile time.

    Required properties:
    -   no runtime malloc in the tick path
    -   stable backing storage for the entire lifetime of the plan
    -   one contiguous array per block or one shared arena with stable offsets
    -   no per-access temporary heap objects

## 5. Arena Model
    Sequential backing storage should be organized into typed arenas:
    -   bit-state arena
    -   register byte arena
    -   memory byte arena

    Descriptors should point into arenas by offset rather than storing many
    raw heap pointers.

    This preserves:
    -   serialization friendliness
    -   reload stability
    -   predictable runtime access

## 6. Update Discipline
    All mutable sequential families must follow staged writeback.

    Required sequence:
    -   read current state
    -   compute next-state writes
    -   stage writes into next-state or write queues
    -   commit in an explicit update phase

    Required properties:
    -   no visible partial update during combinational evaluation
    -   no stale-value read after commit ordering has been defined
    -   commit remains deterministic

## 7. DFF Rule
    DFF remains the base bit-state primitive.

    DFF requirements:
    -   captured separately from combinational evaluation
    -   committed only in the update phase
    -   referenced through stable indices
    -   no dynamic scheduling or event semantics

## 8. Register Rule
    Registers must compile into descriptors plus packed storage.

    Register descriptors should include:
    -   width
    -   arena offset
    -   optional staging offset
    -   any compile-time flags needed for enable or hold behavior

    Register behavior must still follow:
    -   staged writes
    -   explicit commit
    -   stable index-based lookup

## 9. Memory Rule
    RAM and ROM blocks must compile into descriptors plus contiguous backing
    storage.

    Memory descriptors should include:
    -   kind
    -   addr_width
    -   data_width
    -   depth
    -   arena offset or base offset

    Logical depth is:
        depth = 1 << addr_width

## 10. RAM Rule
    RAM is mutable sequential state.

    RAM requirements:
    -   backing storage allocated at load or compile time
    -   writes staged
    -   commits performed only in update phase
    -   reads use stable index calculation

    Practical guard rail:
    -   mutable RAM should reject or cap impractical widths
    -   24-bit address space maximum is a reasonable practical cap
    -   this corresponds to 16 Mi locations

## 11. ROM Rule
    ROM is fixed compiled state, not mutable runtime memory.

    ROM requirements:
    -   contents loaded at compile or load time
    -   reads are pure indexed lookups
    -   ROM never participates in writeback or commit machinery

    Two storage forms are acceptable:
    -   small inline ROM embedded directly in compiled design data
    -   larger ROM backed by contiguous allocated storage

    16-bit address ROM is a common case, but ROM does not need an artificial
    16-bit ceiling if descriptor storage already supports wider depths.

## 12. Forbidden Patterns
    The following are not allowed in sequential implementation:
    -   runtime malloc in tick path
    -   per-state-object heap nodes
    -   pointer-chasing update graphs
    -   direct in-place RAM mutation during combinational execution
    -   hidden commit work inside read paths
    -   introducing a new primitive sequential element when compiled register
        or memory state is sufficient

## 13. Determinism Rule
    Sequential state must preserve deterministic behavior.

    Required guarantees:
    -   explicit update boundary
    -   stable address calculation
    -   identical results for identical inputs and prior state
    -   no allocation-dependent runtime behavior

## 14. Initial Implementation Scope
    Initial implementation should proceed in this order:

    1. Descriptor spec only
    2. Register descriptor plus packed storage
    3. ROM descriptor plus fixed backing storage
    4. RAM descriptor plus staged writeback
    5. Optional register-enable and hold forms

    Do not widen the family set until the base descriptor model is stable.

## 15. Release Guard Rail
    Sequential expansion must not degrade the existing engine discipline.

    Required guard rails:
    -   no tick-path allocation
    -   no object-graph scheduler
    -   no hidden update semantics
    -   no primitive proliferation without proof

## 16. Starting Principle
    Compile each sequential structure into:
    -   descriptor
    -   typed backing storage
    -   explicit staged update path

    Execution then becomes:
    -   indexed reads
    -   staged writes
    -   deterministic commit
