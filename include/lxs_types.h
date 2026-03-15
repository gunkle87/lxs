#ifndef LXS_TYPES_H
#define LXS_TYPES_H

#include <stdint.h>

#ifndef LXS_TEST_PROBES
#define LXS_TEST_PROBES 0
#endif

#define LXS_FUNCTIONAL_REGION_MAX_INPUTS 16U
#define LXS_FUNCTIONAL_REGION_MAX_OUTPUTS 8U
#define LXS_FUNCTIONAL_REGION_MAX_OPS 40U
#define LXS_FUNCTIONAL_REGION_MAX_TEMPS 40U
#define LXS_STANDARD_MACRO_MAX_WIDTH 64U
#define LXS_STANDARD_MACRO_MAX_OUTPUTS 80U
#define LXS_STANDARD_MACRO_MAX_INPUTS 320U

typedef enum lxs_gate_type
	{
	LXS_GATE_AND = 0,
	LXS_GATE_OR,
	LXS_GATE_XOR,
	LXS_GATE_TRI,
	LXS_GATE_NOT,
	LXS_GATE_NAND,
	LXS_GATE_NOR,
	LXS_GATE_XNOR,
	LXS_GATE_BUF,
	LXS_GATE_DFF,
	LXS_GATE_TYPE_COUNT
	} lxs_gate_type;

typedef enum lxs_macro_type
	{
	LXS_MACRO_MUX2 = 0,
	LXS_MACRO_XOR2,
	LXS_MACRO_XNOR2,
	LXS_MACRO_PARITY4,
	LXS_MACRO_PARITY8,
	LXS_MACRO_CARRY_INV2,
	LXS_MACRO_SUM_CINV2,
	LXS_MACRO_TYPE_COUNT
	} lxs_macro_type;

typedef enum lxs_multi_macro_type
	{
	LXS_MULTI_MACRO_HALF_ADDER = 0,
	LXS_MULTI_MACRO_FULL_ADDER,
	LXS_MULTI_MACRO_RIPPLE_SLICE2,
	LXS_MULTI_MACRO_RIPPLE_ADD4,
	LXS_MULTI_MACRO_CARRY_SAVE_ROW4,
	LXS_MULTI_MACRO_REDUCE_PROPAGATE4,
	LXS_MULTI_MACRO_XOR_FAN8,
	LXS_MULTI_MACRO_AND_FAN8,
	LXS_MULTI_MACRO_COMPARE_AND4,
	LXS_MULTI_MACRO_COMPARE_OR4,
	LXS_MULTI_MACRO_XNOR_BANK4,
	LXS_MULTI_MACRO_GUARD_CHAIN4,
	LXS_MULTI_MACRO_FULL_ADDER_CINV,
	LXS_MULTI_MACRO_RIPPLE_SLICE2_CINV,
	LXS_MULTI_MACRO_TYPE_COUNT
	} lxs_multi_macro_type;

typedef enum lxs_source_multi_macro_type
	{
	LXS_SOURCE_MULTI_MACRO_HALF_ADDER = 0,
	LXS_SOURCE_MULTI_MACRO_FULL_ADDER,
	LXS_SOURCE_MULTI_MACRO_RIPPLE_SLICE2,
	LXS_SOURCE_MULTI_MACRO_RIPPLE_ADD4,
	LXS_SOURCE_MULTI_MACRO_CARRY_SAVE_ROW4,
	LXS_SOURCE_MULTI_MACRO_REDUCE_PROPAGATE4,
	LXS_SOURCE_MULTI_MACRO_XOR_FAN8,
	LXS_SOURCE_MULTI_MACRO_AND_FAN8,
	LXS_SOURCE_MULTI_MACRO_XNOR_BANK4,
	LXS_SOURCE_MULTI_MACRO_GUARD_CHAIN4,
	LXS_SOURCE_MULTI_MACRO_TYPE_COUNT
	} lxs_source_multi_macro_type;

typedef enum lxs_source_macro_type
	{
	LXS_SOURCE_MACRO_PARITY4 = 0,
	LXS_SOURCE_MACRO_PARITY8,
	LXS_SOURCE_MACRO_TYPE_COUNT
	} lxs_source_macro_type;

typedef enum lxs_recognition_family
	{
	LXS_RECOGNITION_FAMILY_PARITY = 0,
	LXS_RECOGNITION_FAMILY_SHARED_XOR,
	LXS_RECOGNITION_FAMILY_SHARED_AND,
	LXS_RECOGNITION_FAMILY_COMPARE,
	LXS_RECOGNITION_FAMILY_REGISTER_EN,
	LXS_RECOGNITION_FAMILY_ARITHMETIC,
	LXS_RECOGNITION_FAMILY_CONTROL,
	LXS_RECOGNITION_FAMILY_FUNCTIONAL,
	LXS_RECOGNITION_FAMILY_COUNT
	} lxs_recognition_family;

typedef enum lxs_recognition_mode
	{
	LXS_RECOGNITION_MODE_REPLACE = 0,
	LXS_RECOGNITION_MODE_REPORT_ONLY
	} lxs_recognition_mode;

typedef enum lxs_recognition_pattern_kind
	{
	LXS_RECOGNITION_PATTERN_NONE = 0,
	LXS_RECOGNITION_PATTERN_REDUCTION_TREE,
	LXS_RECOGNITION_PATTERN_SHARED_FANOUT,
	LXS_RECOGNITION_PATTERN_BANK
	} lxs_recognition_pattern_kind;

typedef enum lxs_recognition_abort_reason
	{
	LXS_RECOGNITION_ABORT_SHAPE = 0,
	LXS_RECOGNITION_ABORT_FANOUT,
	LXS_RECOGNITION_ABORT_BRANCH,
	LXS_RECOGNITION_ABORT_DEPTH,
	LXS_RECOGNITION_ABORT_NODE_BUDGET,
	LXS_RECOGNITION_ABORT_OVERLAP,
	LXS_RECOGNITION_ABORT_REASON_COUNT
	} lxs_recognition_abort_reason;

typedef struct lxs_recognition_pattern lxs_recognition_pattern;
struct lxs_recognition_pattern
	{
	uint32_t family;
	uint32_t kind;
	uint32_t root_gate_type;
	uint32_t internal_gate_type;
	uint32_t leaf_count;
	uint32_t gate_count;
	uint32_t macro_type;
	uint32_t multi_macro_type;
	uint8_t is_multi_macro;
	};

typedef struct lxs_recognition_legality lxs_recognition_legality;
struct lxs_recognition_legality
	{
	uint8_t require_single_use_internal;
	uint8_t require_output_boundary;
	uint8_t forbid_matched_overlap;
	uint8_t reserved;
	uint32_t max_nodes;
	uint32_t max_leaves;
	};

typedef struct lxs_recognition_precedence lxs_recognition_precedence;
struct lxs_recognition_precedence
	{
	uint32_t family;
	uint32_t blocks_mask;
	};

typedef enum lxs_register_mode
	{
	LXS_REGISTER_MODE_PLAIN = 0,
	LXS_REGISTER_MODE_ENABLE,
	LXS_REGISTER_MODE_ENABLE_RESET,
	LXS_REGISTER_MODE_HOLD,
	LXS_REGISTER_MODE_COUNTER_EN,
	LXS_REGISTER_MODE_COUNTER_UPDOWN
	} lxs_register_mode;

typedef struct lxs_gate_ir lxs_gate_ir;
struct lxs_gate_ir
	{
	uint32_t type;
	uint32_t input_count;
	uint32_t inputs[8];
	uint32_t output;
	uint32_t level;
	};

typedef struct lxs_netlist lxs_netlist;
typedef struct lxs_source_macro lxs_source_macro;
typedef struct lxs_source_multi_macro lxs_source_multi_macro;
typedef struct lxs_source_standard_mux lxs_source_standard_mux;
typedef struct lxs_source_standard_add lxs_source_standard_add;
typedef struct lxs_source_standard_cmp lxs_source_standard_cmp;
typedef struct lxs_source_standard_alu lxs_source_standard_alu;
typedef struct lxs_source_functional_region lxs_source_functional_region;
typedef struct lxs_source_register lxs_source_register;
typedef struct lxs_source_rom lxs_source_rom;
typedef struct lxs_source_ram lxs_source_ram;
typedef struct lxs_source_regfile lxs_source_regfile;
struct lxs_netlist
	{
	char **net_names;
	uint32_t net_count;
	uint32_t net_cap;

	uint32_t *inputs;
	uint32_t input_count;
	uint32_t input_cap;

	uint32_t *outputs;
	uint32_t output_count;
	uint32_t output_cap;

	lxs_gate_ir *gates;
	uint32_t gate_count;
	uint32_t gate_cap;

	lxs_source_macro *source_macros;
	uint32_t source_macro_count;
	uint32_t source_macro_cap;

	lxs_source_multi_macro *source_multi_macros;
	uint32_t source_multi_macro_count;
	uint32_t source_multi_macro_cap;

	lxs_source_standard_mux *source_standard_muxes;
	uint32_t source_standard_mux_count;
	uint32_t source_standard_mux_cap;
	uint32_t *source_standard_mux_data_net_ids;
	uint32_t source_standard_mux_data_count;
	uint32_t source_standard_mux_data_cap;
	uint32_t *source_standard_mux_select_net_ids;
	uint32_t source_standard_mux_select_count;
	uint32_t source_standard_mux_select_cap;
	uint32_t *source_standard_mux_output_net_ids;
	uint32_t source_standard_mux_output_count;
	uint32_t source_standard_mux_output_cap;
	uint32_t *source_standard_mux_gate_indices;
	uint32_t source_standard_mux_gate_index_count;
	uint32_t source_standard_mux_gate_index_cap;

	lxs_source_standard_add *source_standard_adders;
	uint32_t source_standard_add_count;
	uint32_t source_standard_add_cap;
	uint32_t *source_standard_add_input_net_ids;
	uint32_t source_standard_add_input_count;
	uint32_t source_standard_add_input_cap;
	uint32_t *source_standard_add_output_net_ids;
	uint32_t source_standard_add_output_count;
	uint32_t source_standard_add_output_cap;
	uint32_t *source_standard_add_gate_indices;
	uint32_t source_standard_add_gate_index_count;
	uint32_t source_standard_add_gate_index_cap;

	lxs_source_standard_cmp *source_standard_cmps;
	uint32_t source_standard_cmp_count;
	uint32_t source_standard_cmp_cap;
	uint32_t *source_standard_cmp_input_net_ids;
	uint32_t source_standard_cmp_input_count;
	uint32_t source_standard_cmp_input_cap;
	uint32_t *source_standard_cmp_output_net_ids;
	uint32_t source_standard_cmp_output_count;
	uint32_t source_standard_cmp_output_cap;
	uint32_t *source_standard_cmp_gate_indices;
	uint32_t source_standard_cmp_gate_index_count;
	uint32_t source_standard_cmp_gate_index_cap;

	lxs_source_standard_alu *source_standard_alus;
	uint32_t source_standard_alu_count;
	uint32_t source_standard_alu_cap;
	uint32_t *source_standard_alu_input_net_ids;
	uint32_t source_standard_alu_input_count;
	uint32_t source_standard_alu_input_cap;
	uint32_t *source_standard_alu_output_net_ids;
	uint32_t source_standard_alu_output_count;
	uint32_t source_standard_alu_output_cap;
	uint32_t *source_standard_alu_gate_indices;
	uint32_t source_standard_alu_gate_index_count;
	uint32_t source_standard_alu_gate_index_cap;

	lxs_source_functional_region *source_functional_regions;
	uint32_t source_functional_region_count;
	uint32_t source_functional_region_cap;

	lxs_source_register *source_registers;
	uint32_t source_register_count;
	uint32_t source_register_cap;
	uint32_t *source_register_input_net_ids;
	uint32_t source_register_input_count;
	uint32_t source_register_input_cap;
	uint32_t *source_register_output_net_ids;
	uint32_t source_register_output_count;
	uint32_t source_register_output_cap;

	lxs_source_rom *source_roms;
	uint32_t source_rom_count;
	uint32_t source_rom_cap;
	uint32_t *source_rom_addr_net_ids;
	uint32_t source_rom_addr_count;
	uint32_t source_rom_addr_cap;
	uint32_t *source_rom_output_net_ids;
	uint32_t source_rom_output_count;
	uint32_t source_rom_output_cap;
	uint64_t *source_rom_init_value;
	uint32_t source_rom_init_count;
	uint32_t source_rom_init_cap;
	uint64_t *source_rom_init_mask;
	uint32_t source_rom_mask_count;
	uint32_t source_rom_mask_cap;

	lxs_source_ram *source_rams;
	uint32_t source_ram_count;
	uint32_t source_ram_cap;
	uint32_t *source_ram_read_addr_net_ids;
	uint32_t source_ram_read_addr_count;
	uint32_t source_ram_read_addr_cap;
	uint32_t *source_ram_write_addr_net_ids;
	uint32_t source_ram_write_addr_count;
	uint32_t source_ram_write_addr_cap;
	uint32_t *source_ram_data_input_net_ids;
	uint32_t source_ram_data_input_count;
	uint32_t source_ram_data_input_cap;
	uint32_t *source_ram_output_net_ids;
	uint32_t source_ram_output_count;
	uint32_t source_ram_output_cap;
	uint64_t *source_ram_init_value;
	uint32_t source_ram_init_count;
	uint32_t source_ram_init_cap;
	uint64_t *source_ram_init_mask;
	uint32_t source_ram_mask_count;
	uint32_t source_ram_mask_cap;

	lxs_source_regfile *source_regfiles;
	uint32_t source_regfile_count;
	uint32_t source_regfile_cap;
	uint32_t *source_regfile_read_a_addr_net_ids;
	uint32_t source_regfile_read_a_addr_count;
	uint32_t source_regfile_read_a_addr_cap;
	uint32_t *source_regfile_read_b_addr_net_ids;
	uint32_t source_regfile_read_b_addr_count;
	uint32_t source_regfile_read_b_addr_cap;
	uint32_t *source_regfile_write_addr_net_ids;
	uint32_t source_regfile_write_addr_count;
	uint32_t source_regfile_write_addr_cap;
	uint32_t *source_regfile_data_input_net_ids;
	uint32_t source_regfile_data_input_count;
	uint32_t source_regfile_data_input_cap;
	uint32_t *source_regfile_output_net_ids;
	uint32_t source_regfile_output_count;
	uint32_t source_regfile_output_cap;
	uint64_t *source_regfile_init_value;
	uint32_t source_regfile_init_count;
	uint32_t source_regfile_init_cap;
	uint64_t *source_regfile_init_mask;
	uint32_t source_regfile_mask_count;
	uint32_t source_regfile_mask_cap;
	};

struct lxs_source_macro
	{
	uint32_t type;
	uint32_t inputs[8];
	uint32_t output;
	uint32_t gate_indices[7];
	uint32_t input_count;
	uint32_t gate_count;
	};

struct lxs_source_multi_macro
	{
	uint32_t type;
	uint32_t inputs[12];
	uint32_t outputs[8];
	uint32_t gate_indices[40];
	uint32_t input_count;
	uint32_t output_count;
	uint32_t gate_count;
	};

typedef enum lxs_standard_mux_kind
	{
	LXS_STANDARD_MUX_KIND_2 = 0,
	LXS_STANDARD_MUX_KIND_4
	} lxs_standard_mux_kind;

typedef enum lxs_standard_add_kind
	{
	LXS_STANDARD_ADD_KIND_RIPPLE = 0
	} lxs_standard_add_kind;

typedef enum lxs_standard_cmp_kind
	{
	LXS_STANDARD_CMP_KIND_UNSIGNED = 0
	} lxs_standard_cmp_kind;

typedef enum lxs_standard_alu_kind
	{
	LXS_STANDARD_ALU_KIND_CORE = 0
	} lxs_standard_alu_kind;

struct lxs_source_standard_mux
	{
	uint32_t kind;
	uint32_t width_bits;
	uint32_t data_start;
	uint32_t select_start;
	uint32_t output_start;
	uint32_t gate_index_start;
	uint32_t gate_count;
	};

struct lxs_source_standard_add
	{
	uint32_t kind;
	uint32_t width_bits;
	uint32_t input_start;
	uint32_t output_start;
	uint32_t gate_index_start;
	uint32_t gate_count;
	};

struct lxs_source_standard_cmp
	{
	uint32_t kind;
	uint32_t width_bits;
	uint32_t input_start;
	uint32_t output_start;
	uint32_t gate_index_start;
	uint32_t gate_count;
	};

struct lxs_source_standard_alu
	{
	uint32_t kind;
	uint32_t width_bits;
	uint32_t input_start;
	uint32_t output_start;
	uint32_t gate_index_start;
	uint32_t gate_count;
	};

struct lxs_source_functional_region
	{
	uint32_t exec_kind;
	uint32_t gate_indices[LXS_FUNCTIONAL_REGION_MAX_OPS];
	uint32_t gate_count;
	uint32_t input_count;
	uint32_t output_count;
	uint8_t temp_count;
	uint8_t op_count;
	uint8_t node_budget;
	uint8_t max_depth;
	uint32_t inputs[LXS_FUNCTIONAL_REGION_MAX_INPUTS];
	uint32_t outputs[LXS_FUNCTIONAL_REGION_MAX_OUTPUTS];
	uint8_t op_type[LXS_FUNCTIONAL_REGION_MAX_OPS];
	uint8_t op_dst_count[LXS_FUNCTIONAL_REGION_MAX_OPS];
	uint8_t op_dst_kind[LXS_FUNCTIONAL_REGION_MAX_OPS][2];
	uint8_t op_dst[LXS_FUNCTIONAL_REGION_MAX_OPS][2];
	uint8_t op_arity[LXS_FUNCTIONAL_REGION_MAX_OPS];
	uint8_t op_src[LXS_FUNCTIONAL_REGION_MAX_OPS][4];
	};

struct lxs_source_register
	{
	uint32_t width_bits;
	uint32_t input_start;
	uint32_t output_start;
	uint32_t control_net;
	uint32_t aux_control_net;
	uint32_t mode;
	uint8_t control_invert;
	};

struct lxs_source_rom
	{
	uint32_t addr_width;
	uint32_t data_width;
	uint32_t depth;
	uint32_t addr_start;
	uint32_t output_start;
	uint32_t data_start;
	};

struct lxs_source_ram
	{
	uint32_t addr_width;
	uint32_t data_width;
	uint32_t depth;
	uint32_t read_addr_start;
	uint32_t write_addr_start;
	uint32_t data_input_start;
	uint32_t output_start;
	uint32_t data_start;
	uint32_t write_enable_net;
	};

struct lxs_source_regfile
	{
	uint32_t addr_width;
	uint32_t data_width;
	uint32_t depth;
	uint32_t read_a_addr_start;
	uint32_t read_b_addr_start;
	uint32_t write_addr_start;
	uint32_t data_input_start;
	uint32_t output_start;
	uint32_t data_start;
	uint32_t write_enable_net;
	};

typedef struct lxs_chunk_plan lxs_chunk_plan;
struct lxs_chunk_plan
	{
	uint32_t level;
	uint32_t type;
	uint32_t start;
	uint32_t count;
	uint32_t gate_equiv_count;
	};

typedef struct lxs_level_plan lxs_level_plan;
struct lxs_level_plan
	{
	uint32_t chunk_start;
	uint32_t chunk_count;
	uint32_t macro_start;
	uint32_t macro_count;
	uint32_t multi_macro_start;
	uint32_t multi_macro_count;
	uint32_t standard_mux_start;
	uint32_t standard_mux_count;
	uint32_t standard_add_start;
	uint32_t standard_add_count;
	uint32_t standard_cmp_start;
	uint32_t standard_cmp_count;
	uint32_t standard_alu_start;
	uint32_t standard_alu_count;
	uint32_t functional_region_start;
	uint32_t functional_region_count;
	};

typedef struct lxs_macro_plan lxs_macro_plan;
struct lxs_macro_plan
	{
	uint32_t type;
	uint32_t level;
	uint32_t inputs[8];
	uint32_t output;
	uint32_t gate_equiv_count;
	};

typedef struct lxs_multi_macro_plan lxs_multi_macro_plan;
struct lxs_multi_macro_plan
	{
	uint32_t type;
	uint32_t level;
	uint32_t inputs[12];
	uint32_t outputs[8];
	uint32_t input_count;
	uint32_t output_count;
	uint32_t gate_equiv_count;
	uint32_t param0;
	};

typedef struct lxs_standard_mux_plan lxs_standard_mux_plan;
struct lxs_standard_mux_plan
	{
	uint32_t kind;
	uint32_t level;
	uint32_t width_bits;
	uint32_t data_start;
	uint32_t select_start;
	uint32_t output_start;
	uint32_t gate_equiv_count;
	};

typedef struct lxs_standard_add_plan lxs_standard_add_plan;
struct lxs_standard_add_plan
	{
	uint32_t kind;
	uint32_t level;
	uint32_t width_bits;
	uint32_t input_start;
	uint32_t output_start;
	uint32_t gate_equiv_count;
	};

typedef struct lxs_standard_cmp_plan lxs_standard_cmp_plan;
struct lxs_standard_cmp_plan
	{
	uint32_t kind;
	uint32_t level;
	uint32_t width_bits;
	uint32_t input_start;
	uint32_t output_start;
	uint32_t gate_equiv_count;
	};

typedef struct lxs_standard_alu_plan lxs_standard_alu_plan;
struct lxs_standard_alu_plan
	{
	uint32_t kind;
	uint32_t level;
	uint32_t width_bits;
	uint32_t input_start;
	uint32_t output_start;
	uint32_t gate_equiv_count;
	};

typedef enum lxs_functional_region_exec_kind
	{
	LXS_FUNCTIONAL_REGION_EXEC_EXPR = 0,
	LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM
	} lxs_functional_region_exec_kind;

typedef enum lxs_functional_region_op_type
	{
	LXS_FUNCTIONAL_REGION_OP_BUF = 0,
	LXS_FUNCTIONAL_REGION_OP_NOT,
	LXS_FUNCTIONAL_REGION_OP_AND,
	LXS_FUNCTIONAL_REGION_OP_OR,
	LXS_FUNCTIONAL_REGION_OP_XOR,
	LXS_FUNCTIONAL_REGION_OP_NAND,
	LXS_FUNCTIONAL_REGION_OP_NOR,
	LXS_FUNCTIONAL_REGION_OP_XNOR,
	LXS_FUNCTIONAL_REGION_OP_MAJ3,
	LXS_FUNCTIONAL_REGION_OP_HA2,
	LXS_FUNCTIONAL_REGION_OP_FA3
	} lxs_functional_region_op_type;

typedef enum lxs_functional_region_dst_kind
	{
	LXS_FUNCTIONAL_REGION_DST_TEMP = 0,
	LXS_FUNCTIONAL_REGION_DST_OUTPUT
	} lxs_functional_region_dst_kind;

typedef struct lxs_functional_region_op lxs_functional_region_op;
struct lxs_functional_region_op
	{
	uint8_t type;
	uint8_t dst_count;
	uint8_t arity;
	uint8_t src[4];
	uint8_t dst_kind[2];
	uint8_t dst[2];
	};

typedef struct lxs_functional_region_plan lxs_functional_region_plan;
struct lxs_functional_region_plan
	{
	uint32_t level;
	uint32_t exec_kind;
	uint32_t input_count;
	uint32_t output_count;
	uint32_t gate_equiv_count;
	uint32_t inputs[LXS_FUNCTIONAL_REGION_MAX_INPUTS];
	uint32_t outputs[LXS_FUNCTIONAL_REGION_MAX_OUTPUTS];
	uint8_t temp_count;
	uint8_t op_count;
	uint8_t node_budget;
	uint8_t max_depth;
	lxs_functional_region_op ops[LXS_FUNCTIONAL_REGION_MAX_OPS];
	};

typedef struct lxs_io_plan lxs_io_plan;
struct lxs_io_plan
	{
	uint32_t count;
	uint32_t *net_ids;
	uint32_t contiguous_base;
	uint8_t is_contiguous;
	};

typedef struct lxs_state_plan lxs_state_plan;
struct lxs_state_plan
	{
	uint32_t count;
	uint32_t *d_inputs;
	uint32_t *q_outputs;
	uint32_t d_contiguous_base;
	uint32_t q_contiguous_base;
	uint8_t d_is_contiguous;
	uint8_t q_is_contiguous;
	};

typedef struct lxs_register_plan lxs_register_plan;
struct lxs_register_plan
	{
	uint32_t width_bits;
	uint32_t input_start;
	uint32_t output_start;
	uint32_t storage_offset;
	uint32_t control_net;
	uint32_t aux_control_net;
	uint32_t mode;
	uint8_t control_invert;
	};

typedef struct lxs_rom_plan lxs_rom_plan;
struct lxs_rom_plan
	{
	uint32_t addr_width;
	uint32_t data_width;
	uint32_t depth;
	uint32_t addr_input_start;
	uint32_t output_start;
	uint32_t data_offset;
	};

typedef struct lxs_ram_plan lxs_ram_plan;
struct lxs_ram_plan
	{
	uint32_t addr_width;
	uint32_t data_width;
	uint32_t depth;
	uint32_t read_addr_start;
	uint32_t write_addr_start;
	uint32_t data_input_start;
	uint32_t output_start;
	uint32_t storage_offset;
	uint32_t stage_offset;
	uint32_t write_enable_net;
	};

typedef struct lxs_regfile_plan lxs_regfile_plan;
struct lxs_regfile_plan
	{
	uint32_t addr_width;
	uint32_t data_width;
	uint32_t depth;
	uint32_t read_a_addr_start;
	uint32_t read_b_addr_start;
	uint32_t write_addr_start;
	uint32_t data_input_start;
	uint32_t output_start;
	uint32_t storage_offset;
	uint32_t stage_offset;
	uint32_t write_enable_net;
	};

typedef struct lxs_plan lxs_plan;
struct lxs_plan
	{
	uint32_t net_count;
	uint32_t gate_count;
	uint32_t comb_gate_count;
	uint32_t level_count;
	uint32_t span_count;
	uint32_t max_span_count;
	uint32_t macro_count;
	uint32_t multi_macro_count;
	uint32_t standard_mux_count;
	uint32_t standard_add_count;
	uint32_t standard_cmp_count;
	uint32_t standard_alu_count;
	uint32_t functional_region_count;
	uint32_t recognition_mask;
	uint32_t recognition_mode;
	uint32_t recognition_match_count[LXS_RECOGNITION_FAMILY_COUNT];
	uint32_t recognition_node_reduction[LXS_RECOGNITION_FAMILY_COUNT];
	uint64_t recognition_candidate_roots[LXS_RECOGNITION_FAMILY_COUNT];
	uint64_t recognition_nodes_visited[LXS_RECOGNITION_FAMILY_COUNT];
	uint32_t recognition_max_depth[LXS_RECOGNITION_FAMILY_COUNT];
	uint64_t recognition_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT];
	uint64_t recognition_time_us[LXS_RECOGNITION_FAMILY_COUNT];

	lxs_gate_ir *comb_gates;
	lxs_chunk_plan *chunks;
	lxs_macro_plan *macros;
	lxs_multi_macro_plan *multi_macros;
	lxs_standard_mux_plan *standard_muxes;
	lxs_standard_add_plan *standard_adders;
	lxs_standard_cmp_plan *standard_cmps;
	lxs_standard_alu_plan *standard_alus;
	lxs_functional_region_plan *functional_regions;
	lxs_level_plan *levels;

	lxs_io_plan inputs;
	lxs_io_plan outputs;
	lxs_state_plan state;

	uint32_t register_count;
	uint32_t register_bit_count;
	uint32_t register_input_net_count;
	lxs_register_plan *registers;
	uint32_t *register_input_net_ids;
	uint32_t *register_output_net_ids;
	uint64_t *register_init_value;
	uint64_t *register_init_mask;

	uint32_t rom_count;
	uint32_t rom_addr_net_count;
	uint32_t rom_output_net_count;
	uint32_t rom_bit_count;
	lxs_rom_plan *roms;
	uint32_t *rom_addr_net_ids;
	uint32_t *rom_output_net_ids;
	uint64_t *rom_init_value;
	uint64_t *rom_init_mask;

	uint32_t ram_count;
	uint32_t ram_storage_bit_count;
	uint32_t ram_stage_bit_count;
	uint32_t ram_read_addr_net_count;
	uint32_t ram_write_addr_net_count;
	uint32_t ram_data_input_net_count;
	uint32_t ram_output_net_count;
	lxs_ram_plan *rams;
	uint32_t *ram_read_addr_net_ids;
	uint32_t *ram_write_addr_net_ids;
	uint32_t *ram_data_input_net_ids;
	uint32_t *ram_output_net_ids;
	uint64_t *ram_init_value;
	uint64_t *ram_init_mask;

	uint32_t regfile_count;
	uint32_t regfile_storage_bit_count;
	uint32_t regfile_stage_bit_count;
	uint32_t regfile_read_a_addr_net_count;
	uint32_t regfile_read_b_addr_net_count;
	uint32_t regfile_write_addr_net_count;
	uint32_t regfile_data_input_net_count;
	uint32_t regfile_output_net_count;
	lxs_regfile_plan *regfiles;
	uint32_t *regfile_read_a_addr_net_ids;
	uint32_t *regfile_read_b_addr_net_ids;
	uint32_t *regfile_write_addr_net_ids;
	uint32_t *regfile_data_input_net_ids;
	uint32_t *regfile_output_net_ids;
	uint64_t *regfile_init_value;
	uint64_t *regfile_init_mask;

	uint32_t standard_mux_data_net_count;
	uint32_t standard_mux_select_net_count;
	uint32_t standard_mux_output_net_count;
	uint32_t *standard_mux_data_net_ids;
	uint32_t *standard_mux_select_net_ids;
	uint32_t *standard_mux_output_net_ids;
	uint32_t standard_add_input_net_count;
	uint32_t standard_add_output_net_count;
	uint32_t *standard_add_input_net_ids;
	uint32_t *standard_add_output_net_ids;
	uint32_t standard_cmp_input_net_count;
	uint32_t standard_cmp_output_net_count;
	uint32_t *standard_cmp_input_net_ids;
	uint32_t *standard_cmp_output_net_ids;
	uint32_t standard_alu_input_net_count;
	uint32_t standard_alu_output_net_count;
	uint32_t *standard_alu_input_net_ids;
	uint32_t *standard_alu_output_net_ids;
	};

typedef struct lxs_probes lxs_probes;
struct lxs_probes
	{
	uint64_t input_apply;
	uint64_t chunk_exec;
	uint64_t gate_eval;
	uint64_t dff_exec;
	uint64_t tick_count;
	uint64_t state_commit_count;

#if LXS_TEST_PROBES
	/* TEST_ONLY semantic diagnostics. Remove when no longer asserted by tests. */
	uint64_t input_toggle;
	uint64_t state_change_commit;
	uint64_t contention_count;
	uint64_t unknown_state_materialize_count;
	uint64_t highz_materialize_count;
	uint64_t multi_driver_resolve_count;
	uint64_t tri_no_drive_count;
	uint64_t pup_z_source_count;
	uint64_t pdn_z_source_count;
#endif
	};

typedef struct lxs_engine_ctx lxs_engine_ctx;
struct lxs_engine_ctx
	{
	uint64_t *net_value;
	uint64_t *net_mask;
	uint64_t *next_state_value;
	uint64_t *next_state_mask;
	uint64_t *output_value;
	uint64_t *output_mask;
	uint64_t *register_value;
	uint64_t *register_mask;
	uint64_t *next_register_value;
	uint64_t *next_register_mask;
	uint64_t *ram_value;
	uint64_t *ram_mask;
	uint64_t *ram_stage_value;
	uint64_t *ram_stage_mask;
	uint32_t *ram_pending_addr;
	uint8_t *ram_pending_write;
	uint64_t *regfile_value;
	uint64_t *regfile_mask;
	uint64_t *regfile_stage_value;
	uint64_t *regfile_stage_mask;
	uint32_t *regfile_pending_addr;
	uint8_t *regfile_pending_write;

#if LXS_TEST_PROBES
	uint64_t *input_shadow_value;
	uint64_t *input_shadow_mask;
#endif

	lxs_probes probes;
	};

lxs_netlist* lxs_load_iscas(const char *path);
void lxs_free_netlist(lxs_netlist *nl);

lxs_plan* lxs_compile_to_plan(lxs_netlist *nl);
void lxs_free_plan(lxs_plan *plan);

int lxs_init_engine(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_reset_engine(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_free_engine(lxs_engine_ctx *ctx);

void lxs_apply_inputs(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const uint64_t *values,
	const uint64_t *masks);

void lxs_begin_tick(lxs_engine_ctx *ctx);
void lxs_execute_levels(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_capture_outputs(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_capture_next_state(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_commit_state(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_execute_plan(lxs_engine_ctx *ctx, const lxs_plan *plan);

void lxs_read_outputs(
	const lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	uint64_t *values,
	uint64_t *masks);

lxs_probes lxs_get_probes(const lxs_engine_ctx *ctx);

#endif
