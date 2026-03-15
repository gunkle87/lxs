from pathlib import Path

from lxs_api import LxsApi

ALL_ONES = (1 << 64) - 1


def main() -> None:
    repo_root = Path(__file__).resolve().parents[2]
    api = LxsApi(repo_root / "build" / "bin" / "lxs_api.dll")

    mux_path = repo_root / "Tests" / "Circuits" / "mux2_8_explicit.bench"
    reg_path = repo_root / "Tests" / "Circuits" / "reg16_explicit.bench"
    ram_path = repo_root / "Tests" / "Circuits" / "ram8_explicit.bench"
    regfile_path = repo_root / "Tests" / "Circuits" / "regfile8_explicit.bench"

    # MUX smoke: compile, name lookup, step, output/net read
    mux_netlist = api.load_bench(mux_path)
    mux_plan = api.compile_plan(mux_netlist)
    mux_engine = api.create_engine(mux_plan)
    try:
        counts = api.get_plan_counts(mux_plan)
        assert counts.input_count == 17
        assert counts.output_count == 8
        assert api.get_input_name(mux_plan, 0) == "a0"
        assert api.get_output_name(mux_plan, 0) == "q0"
        q0_id = api.find_net(mux_plan, "q0")

        values = [0] * 17
        # Select b bus.
        values[8:16] = [ALL_ONES if bit in (0, 2, 4, 6) else 0 for bit in range(8)]
        values[16] = ALL_ONES
        api.apply_inputs(mux_engine, values)
        api.tick(mux_engine)
        out_values, out_masks = api.read_outputs(mux_engine, counts.output_count)
        assert out_masks == [0] * 8
        assert out_values[0] == ALL_ONES
        assert out_values[1] == 0
        net_value, net_mask = api.read_net(mux_engine, q0_id)
        assert net_mask == 0
        assert net_value == ALL_ONES
    finally:
        api.free_engine(mux_engine)
        api.free_plan(mux_plan)
        api.free_netlist(mux_netlist)

    # Register smoke: one tick should capture D into Q.
    reg_netlist = api.load_bench(reg_path)
    reg_plan = api.compile_plan(reg_netlist)
    reg_engine = api.create_engine(reg_plan)
    try:
        info = api.get_register_info(reg_plan, 0)
        assert info.width_bits == 16
        values = [ALL_ONES if bit in (0, 3, 5, 8, 13) else 0 for bit in range(info.width_bits)]
        api.apply_inputs(reg_engine, values)
        api.tick(reg_engine)
        reg_values, reg_masks = api.read_register(reg_engine, 0, info.width_bits)
        assert reg_masks == [0] * info.width_bits
        assert reg_values == values
    finally:
        api.free_engine(reg_engine)
        api.free_plan(reg_plan)
        api.free_netlist(reg_netlist)

    # RAM smoke: confirm initial contents and write visibility after commit.
    ram_netlist = api.load_bench(ram_path)
    ram_plan = api.compile_plan(ram_netlist)
    ram_engine = api.create_engine(ram_plan)
    try:
        info = api.get_ram_info(ram_plan, 0)
        assert info.depth == 4
        word0, mask0 = api.read_ram_word(ram_engine, 0, 0, info.data_width)
        word1, mask1 = api.read_ram_word(ram_engine, 0, 1, info.data_width)
        assert mask0 == [0] * info.data_width
        assert mask1 == [0] * info.data_width
        assert word0[1] == ALL_ONES  # 0x12
        assert word1[2] == ALL_ONES  # 0x34

        # ra=2, wa=2, d=0xA5, we=1
        values = [0] * 13
        values[0] = 0
        values[1] = ALL_ONES
        values[2] = 0
        values[3] = ALL_ONES
        values[4:12] = [ALL_ONES if bit in (0, 2, 5, 7) else 0 for bit in range(8)]
        values[12] = ALL_ONES
        api.apply_inputs(ram_engine, values)
        api.tick(ram_engine)
        word2, _ = api.read_ram_word(ram_engine, 0, 2, info.data_width)
        assert word2[0] == ALL_ONES
        assert word2[1] == 0
        assert word2[2] == ALL_ONES
        assert word2[5] == ALL_ONES
        assert word2[7] == ALL_ONES
    finally:
        api.free_engine(ram_engine)
        api.free_plan(ram_plan)
        api.free_netlist(ram_netlist)

    # Regfile smoke: initial values and one committed write.
    regfile_netlist = api.load_bench(regfile_path)
    regfile_plan = api.compile_plan(regfile_netlist)
    regfile_engine = api.create_engine(regfile_plan)
    try:
        info = api.get_regfile_info(regfile_plan, 0)
        assert info.depth == 2
        word0, _ = api.read_regfile_word(regfile_engine, 0, 0, info.data_width)
        word1, _ = api.read_regfile_word(regfile_engine, 0, 1, info.data_width)
        assert word0[1] == ALL_ONES  # 0x12
        assert word1[0] == ALL_ONES  # 0xA5

        # ra=0, rb=1, wa=1, d=0x3C, we=1
        values = [0] * 12
        values[0] = 0
        values[1] = ALL_ONES
        values[2] = ALL_ONES
        values[3:11] = [ALL_ONES if bit in (2, 3, 4, 5) else 0 for bit in range(8)]
        values[11] = ALL_ONES
        api.apply_inputs(regfile_engine, values)
        api.tick(regfile_engine)
        word1_after, _ = api.read_regfile_word(regfile_engine, 0, 1, info.data_width)
        assert word1_after[2] == ALL_ONES
        assert word1_after[3] == ALL_ONES
        assert word1_after[4] == ALL_ONES
        assert word1_after[5] == ALL_ONES

        probes = api.read_probes(regfile_engine)
        assert probes.tick_count == 1
        api.clear_probes(regfile_engine)
        probes_cleared = api.read_probes(regfile_engine)
        assert probes_cleared.tick_count == 0
    finally:
        api.free_engine(regfile_engine)
        api.free_plan(regfile_plan)
        api.free_netlist(regfile_netlist)

    print("LXS Python API smoke passed.")


if __name__ == "__main__":
    main()
