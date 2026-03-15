import ctypes
import os
from ctypes import POINTER, byref, c_char_p, c_uint32, c_uint64, c_void_p
from dataclasses import dataclass
from pathlib import Path


class LxsApiError(RuntimeError):
    pass


class LxsApiResult:
    OK = 0


@dataclass
class PlanCounts:
    input_count: int
    output_count: int
    net_count: int
    level_count: int
    chunk_count: int
    macro_count: int
    multi_macro_count: int
    standard_mux_count: int
    standard_add_count: int
    standard_cmp_count: int
    standard_alu_count: int
    functional_region_count: int
    register_count: int
    rom_count: int
    ram_count: int
    regfile_count: int


@dataclass
class Probes:
    input_apply: int
    chunk_exec: int
    gate_eval: int
    dff_exec: int
    tick_count: int
    state_commit_count: int
    input_toggle: int
    state_change_commit: int
    contention_count: int
    unknown_state_materialize_count: int
    highz_materialize_count: int
    multi_driver_resolve_count: int
    tri_no_drive_count: int
    pup_z_source_count: int
    pdn_z_source_count: int


@dataclass
class RegisterInfo:
    width_bits: int
    mode: int


@dataclass
class RamInfo:
    addr_width: int
    data_width: int
    depth: int


@dataclass
class RegfileInfo:
    addr_width: int
    data_width: int
    depth: int


class _PlanCountsC(ctypes.Structure):
    _fields_ = [
        ("input_count", c_uint32),
        ("output_count", c_uint32),
        ("net_count", c_uint32),
        ("level_count", c_uint32),
        ("chunk_count", c_uint32),
        ("macro_count", c_uint32),
        ("multi_macro_count", c_uint32),
        ("standard_mux_count", c_uint32),
        ("standard_add_count", c_uint32),
        ("standard_cmp_count", c_uint32),
        ("standard_alu_count", c_uint32),
        ("functional_region_count", c_uint32),
        ("register_count", c_uint32),
        ("rom_count", c_uint32),
        ("ram_count", c_uint32),
        ("regfile_count", c_uint32),
    ]


class _ProbesC(ctypes.Structure):
    _fields_ = [
        ("input_apply", c_uint64),
        ("chunk_exec", c_uint64),
        ("gate_eval", c_uint64),
        ("dff_exec", c_uint64),
        ("tick_count", c_uint64),
        ("state_commit_count", c_uint64),
        ("input_toggle", c_uint64),
        ("state_change_commit", c_uint64),
        ("contention_count", c_uint64),
        ("unknown_state_materialize_count", c_uint64),
        ("highz_materialize_count", c_uint64),
        ("multi_driver_resolve_count", c_uint64),
        ("tri_no_drive_count", c_uint64),
        ("pup_z_source_count", c_uint64),
        ("pdn_z_source_count", c_uint64),
    ]


class _RegisterInfoC(ctypes.Structure):
    _fields_ = [
        ("width_bits", c_uint32),
        ("mode", c_uint32),
    ]


class _RamInfoC(ctypes.Structure):
    _fields_ = [
        ("addr_width", c_uint32),
        ("data_width", c_uint32),
        ("depth", c_uint32),
    ]


class _RegfileInfoC(ctypes.Structure):
    _fields_ = [
        ("addr_width", c_uint32),
        ("data_width", c_uint32),
        ("depth", c_uint32),
    ]


class LxsApi:
    def __init__(self, dll_path: str | Path | None = None):
        if dll_path is None:
            repo_root = Path(__file__).resolve().parents[2]
            dll_path = repo_root / "build" / "bin" / "lxs_api.dll"
        self.dll_path = Path(dll_path)
        if os.name == "nt":
            os.add_dll_directory(str(self.dll_path.parent))
        self.lib = ctypes.CDLL(str(self.dll_path))
        self._bind()

    def _bind(self):
        lib = self.lib

        lib.lxs_api_result_string.argtypes = [ctypes.c_int]
        lib.lxs_api_result_string.restype = c_char_p
        lib.lxs_api_get_last_error.argtypes = []
        lib.lxs_api_get_last_error.restype = c_char_p
        lib.lxs_api_diag_get_last_error_code.argtypes = []
        lib.lxs_api_diag_get_last_error_code.restype = ctypes.c_int
        lib.lxs_api_diag_clear_last_error.argtypes = []
        lib.lxs_api_diag_clear_last_error.restype = None

        lib.lxs_api_netlist_load_bench.argtypes = [c_char_p, POINTER(c_void_p)]
        lib.lxs_api_netlist_load_bench.restype = ctypes.c_int
        lib.lxs_api_netlist_free.argtypes = [c_void_p]
        lib.lxs_api_netlist_free.restype = None

        lib.lxs_api_plan_compile.argtypes = [c_void_p, POINTER(c_void_p)]
        lib.lxs_api_plan_compile.restype = ctypes.c_int
        lib.lxs_api_plan_free.argtypes = [c_void_p]
        lib.lxs_api_plan_free.restype = None
        lib.lxs_api_plan_get_counts.argtypes = [c_void_p, POINTER(_PlanCountsC)]
        lib.lxs_api_plan_get_counts.restype = ctypes.c_int
        lib.lxs_api_plan_get_input_net_id.argtypes = [c_void_p, c_uint32, POINTER(c_uint32)]
        lib.lxs_api_plan_get_input_net_id.restype = ctypes.c_int
        lib.lxs_api_plan_get_output_net_id.argtypes = [c_void_p, c_uint32, POINTER(c_uint32)]
        lib.lxs_api_plan_get_output_net_id.restype = ctypes.c_int
        lib.lxs_api_plan_get_input_name.argtypes = [c_void_p, c_uint32, POINTER(c_char_p)]
        lib.lxs_api_plan_get_input_name.restype = ctypes.c_int
        lib.lxs_api_plan_get_output_name.argtypes = [c_void_p, c_uint32, POINTER(c_char_p)]
        lib.lxs_api_plan_get_output_name.restype = ctypes.c_int
        lib.lxs_api_plan_get_net_name.argtypes = [c_void_p, c_uint32, POINTER(c_char_p)]
        lib.lxs_api_plan_get_net_name.restype = ctypes.c_int
        lib.lxs_api_plan_find_net.argtypes = [c_void_p, c_char_p, POINTER(c_uint32)]
        lib.lxs_api_plan_find_net.restype = ctypes.c_int
        lib.lxs_api_plan_get_register_info.argtypes = [c_void_p, c_uint32, POINTER(_RegisterInfoC)]
        lib.lxs_api_plan_get_register_info.restype = ctypes.c_int
        lib.lxs_api_plan_get_ram_info.argtypes = [c_void_p, c_uint32, POINTER(_RamInfoC)]
        lib.lxs_api_plan_get_ram_info.restype = ctypes.c_int
        lib.lxs_api_plan_get_regfile_info.argtypes = [c_void_p, c_uint32, POINTER(_RegfileInfoC)]
        lib.lxs_api_plan_get_regfile_info.restype = ctypes.c_int

        lib.lxs_api_engine_create.argtypes = [c_void_p, POINTER(c_void_p)]
        lib.lxs_api_engine_create.restype = ctypes.c_int
        lib.lxs_api_engine_free.argtypes = [c_void_p]
        lib.lxs_api_engine_free.restype = None
        lib.lxs_api_engine_reset.argtypes = [c_void_p]
        lib.lxs_api_engine_reset.restype = ctypes.c_int
        lib.lxs_api_engine_apply_inputs.argtypes = [c_void_p, POINTER(c_uint64), POINTER(c_uint64), c_uint32]
        lib.lxs_api_engine_apply_inputs.restype = ctypes.c_int
        lib.lxs_api_engine_tick.argtypes = [c_void_p]
        lib.lxs_api_engine_tick.restype = ctypes.c_int
        lib.lxs_api_engine_tick_many.argtypes = [c_void_p, c_uint32]
        lib.lxs_api_engine_tick_many.restype = ctypes.c_int
        lib.lxs_api_engine_clear_probes.argtypes = [c_void_p]
        lib.lxs_api_engine_clear_probes.restype = ctypes.c_int
        lib.lxs_api_engine_read_outputs.argtypes = [c_void_p, POINTER(c_uint64), POINTER(c_uint64), c_uint32]
        lib.lxs_api_engine_read_outputs.restype = ctypes.c_int
        lib.lxs_api_engine_read_net.argtypes = [c_void_p, c_uint32, POINTER(c_uint64), POINTER(c_uint64)]
        lib.lxs_api_engine_read_net.restype = ctypes.c_int
        lib.lxs_api_engine_read_register.argtypes = [c_void_p, c_uint32, POINTER(c_uint64), POINTER(c_uint64), c_uint32]
        lib.lxs_api_engine_read_register.restype = ctypes.c_int
        lib.lxs_api_engine_read_ram_word.argtypes = [c_void_p, c_uint32, c_uint32, POINTER(c_uint64), POINTER(c_uint64), c_uint32]
        lib.lxs_api_engine_read_ram_word.restype = ctypes.c_int
        lib.lxs_api_engine_read_regfile_word.argtypes = [c_void_p, c_uint32, c_uint32, POINTER(c_uint64), POINTER(c_uint64), c_uint32]
        lib.lxs_api_engine_read_regfile_word.restype = ctypes.c_int
        lib.lxs_api_engine_read_probes.argtypes = [c_void_p, POINTER(_ProbesC)]
        lib.lxs_api_engine_read_probes.restype = ctypes.c_int
        lib.lxs_api_engine_get_input_count.argtypes = [c_void_p, POINTER(c_uint32)]
        lib.lxs_api_engine_get_input_count.restype = ctypes.c_int
        lib.lxs_api_engine_get_output_count.argtypes = [c_void_p, POINTER(c_uint32)]
        lib.lxs_api_engine_get_output_count.restype = ctypes.c_int

    def _check(self, result: int, context: str) -> None:
        if result != LxsApiResult.OK:
            message = self.lib.lxs_api_get_last_error()
            text = message.decode("utf-8") if message else "unknown error"
            raise LxsApiError(f"{context}: {text}")

    def load_bench(self, path: str | Path):
        netlist = c_void_p()
        self._check(
            self.lib.lxs_api_netlist_load_bench(str(path).encode("utf-8"), byref(netlist)),
            "load_bench",
        )
        return netlist

    def free_netlist(self, netlist):
        self.lib.lxs_api_netlist_free(netlist)

    def compile_plan(self, netlist):
        plan = c_void_p()
        self._check(self.lib.lxs_api_plan_compile(netlist, byref(plan)), "compile_plan")
        return plan

    def free_plan(self, plan):
        self.lib.lxs_api_plan_free(plan)

    def create_engine(self, plan):
        engine = c_void_p()
        self._check(self.lib.lxs_api_engine_create(plan, byref(engine)), "create_engine")
        return engine

    def free_engine(self, engine):
        self.lib.lxs_api_engine_free(engine)

    def get_plan_counts(self, plan) -> PlanCounts:
        counts = _PlanCountsC()
        self._check(self.lib.lxs_api_plan_get_counts(plan, byref(counts)), "get_plan_counts")
        return PlanCounts(*[getattr(counts, field) for field, _ in counts._fields_])

    def get_input_name(self, plan, index: int) -> str:
        out = c_char_p()
        self._check(self.lib.lxs_api_plan_get_input_name(plan, index, byref(out)), "get_input_name")
        return out.value.decode("utf-8")

    def get_output_name(self, plan, index: int) -> str:
        out = c_char_p()
        self._check(self.lib.lxs_api_plan_get_output_name(plan, index, byref(out)), "get_output_name")
        return out.value.decode("utf-8")

    def find_net(self, plan, name: str) -> int:
        net_id = c_uint32()
        self._check(self.lib.lxs_api_plan_find_net(plan, name.encode("utf-8"), byref(net_id)), "find_net")
        return int(net_id.value)

    def get_register_info(self, plan, index: int) -> RegisterInfo:
        info = _RegisterInfoC()
        self._check(self.lib.lxs_api_plan_get_register_info(plan, index, byref(info)), "get_register_info")
        return RegisterInfo(info.width_bits, info.mode)

    def get_ram_info(self, plan, index: int) -> RamInfo:
        info = _RamInfoC()
        self._check(self.lib.lxs_api_plan_get_ram_info(plan, index, byref(info)), "get_ram_info")
        return RamInfo(info.addr_width, info.data_width, info.depth)

    def get_regfile_info(self, plan, index: int) -> RegfileInfo:
        info = _RegfileInfoC()
        self._check(self.lib.lxs_api_plan_get_regfile_info(plan, index, byref(info)), "get_regfile_info")
        return RegfileInfo(info.addr_width, info.data_width, info.depth)

    def apply_inputs(self, engine, values: list[int], masks: list[int] | None = None) -> None:
        if masks is None:
            masks = [0] * len(values)
        value_array = (c_uint64 * len(values))(*values)
        mask_array = (c_uint64 * len(masks))(*masks)
        self._check(self.lib.lxs_api_engine_apply_inputs(engine, value_array, mask_array, len(values)), "apply_inputs")

    def tick(self, engine, count: int = 1) -> None:
        if count == 1:
            self._check(self.lib.lxs_api_engine_tick(engine), "tick")
        else:
            self._check(self.lib.lxs_api_engine_tick_many(engine, count), "tick_many")

    def clear_probes(self, engine) -> None:
        self._check(self.lib.lxs_api_engine_clear_probes(engine), "clear_probes")

    def read_outputs(self, engine, count: int) -> tuple[list[int], list[int]]:
        value_array = (c_uint64 * count)()
        mask_array = (c_uint64 * count)()
        self._check(self.lib.lxs_api_engine_read_outputs(engine, value_array, mask_array, count), "read_outputs")
        return list(value_array), list(mask_array)

    def read_net(self, engine, net_id: int) -> tuple[int, int]:
        value = c_uint64()
        mask = c_uint64()
        self._check(self.lib.lxs_api_engine_read_net(engine, net_id, byref(value), byref(mask)), "read_net")
        return int(value.value), int(mask.value)

    def read_register(self, engine, register_index: int, width_bits: int) -> tuple[list[int], list[int]]:
        value_array = (c_uint64 * width_bits)()
        mask_array = (c_uint64 * width_bits)()
        self._check(
            self.lib.lxs_api_engine_read_register(engine, register_index, value_array, mask_array, width_bits),
            "read_register",
        )
        return list(value_array), list(mask_array)

    def read_ram_word(self, engine, ram_index: int, address: int, data_width: int) -> tuple[list[int], list[int]]:
        value_array = (c_uint64 * data_width)()
        mask_array = (c_uint64 * data_width)()
        self._check(
            self.lib.lxs_api_engine_read_ram_word(engine, ram_index, address, value_array, mask_array, data_width),
            "read_ram_word",
        )
        return list(value_array), list(mask_array)

    def read_regfile_word(self, engine, regfile_index: int, address: int, data_width: int) -> tuple[list[int], list[int]]:
        value_array = (c_uint64 * data_width)()
        mask_array = (c_uint64 * data_width)()
        self._check(
            self.lib.lxs_api_engine_read_regfile_word(engine, regfile_index, address, value_array, mask_array, data_width),
            "read_regfile_word",
        )
        return list(value_array), list(mask_array)

    def read_probes(self, engine) -> Probes:
        probes = _ProbesC()
        self._check(self.lib.lxs_api_engine_read_probes(engine, byref(probes)), "read_probes")
        return Probes(*[getattr(probes, field) for field, _ in probes._fields_])
