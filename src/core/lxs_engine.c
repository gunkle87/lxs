#include "lxs_types.h"
#include "lxs_primitives.h"
#include <string.h>
#include <malloc.h>

#define LXS_ALIGN_SIZE 64U

#if defined(__clang__) || defined(__GNUC__)
#define LXS_ASSUME_ALIGNED_64(ptr) __builtin_assume_aligned((ptr), 64)
#else
#define LXS_ASSUME_ALIGNED_64(ptr) (ptr)
#endif

#ifndef LXS_MIXED_LEVEL_EXEC_MODE
#define LXS_MIXED_LEVEL_EXEC_MODE 2
#endif

#ifndef LXS_LOGIC_CHUNK_FAST_MODE
#define LXS_LOGIC_CHUNK_FAST_MODE 1
#endif

#ifndef LXS_LOGIC_CHUNK_FAST_MIN_COUNT
#define LXS_LOGIC_CHUNK_FAST_MIN_COUNT 0
#endif

static void* lxs_malloc_aligned(size_t size)
	{
	if (size == 0U)
		{
		return NULL;
		}

#ifdef _WIN32
	return _aligned_malloc(size, LXS_ALIGN_SIZE);
#else
	void *ptr = NULL;
	if (posix_memalign(&ptr, LXS_ALIGN_SIZE, size) != 0)
		{
		return NULL;
		}
	return ptr;
#endif
	}

static void* lxs_calloc_aligned(size_t count, size_t size)
	{
	void *ptr;
	size_t total_size = count * size;

	ptr = lxs_malloc_aligned(total_size);
	if (ptr)
		{
		memset(ptr, 0, total_size);
		}
	return ptr;
	}

static void lxs_free_aligned(void *ptr)
	{
	if (!ptr)
		{
		return;
		}

#ifdef _WIN32
	_aligned_free(ptr);
#else
	free(ptr);
#endif
	}

static uint32_t lxs_read_index_bits(
	const lxs_engine_ctx *ctx,
	const uint32_t *net_ids,
	uint32_t start,
	uint32_t width,
	uint8_t *unknown)
	{
	uint32_t index = 0U;

	*unknown = 0U;
	for (uint32_t bit = 0; bit < width; ++bit)
		{
		uint32_t net_id = net_ids[start + bit];
		if (ctx->net_mask[net_id] != 0ULL)
			{
			*unknown = 1U;
			continue;
			}
		if (ctx->net_value[net_id] != 0ULL)
			{
			index |= (1U << bit);
			}
		}

	return index;
	}

static void lxs_write_unknown_outputs(
	lxs_engine_ctx *ctx,
	const uint32_t *net_ids,
	uint32_t start,
	uint32_t count)
	{
	for (uint32_t bit = 0; bit < count; ++bit)
		{
		uint32_t net_id = net_ids[start + bit];
		ctx->net_value[net_id] = 0ULL;
		ctx->net_mask[net_id] = ~0ULL;
		}
	}

static void lxs_materialize_register_outputs(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->register_count; ++i)
		{
		const lxs_register_plan *reg = &plan->registers[i];
		for (uint32_t bit = 0; bit < reg->width_bits; ++bit)
			{
			uint32_t storage_index = reg->storage_offset + bit;
			uint32_t net_id = plan->register_output_net_ids[reg->output_start + bit];

			ctx->net_value[net_id] = ctx->register_value[storage_index];
			ctx->net_mask[net_id] = ctx->register_mask[storage_index];
			}
		}
	}

static void lxs_execute_roms(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->rom_count; ++i)
		{
		const lxs_rom_plan *rom = &plan->roms[i];
		uint8_t unknown = 0U;
		uint32_t address = lxs_read_index_bits(
			ctx,
			plan->rom_addr_net_ids,
			rom->addr_input_start,
			rom->addr_width,
			&unknown);

		if (unknown || address >= rom->depth)
			{
			lxs_write_unknown_outputs(ctx, plan->rom_output_net_ids, rom->output_start, rom->data_width);
			continue;
			}

		for (uint32_t bit = 0; bit < rom->data_width; ++bit)
			{
			uint32_t net_id = plan->rom_output_net_ids[rom->output_start + bit];
			uint32_t storage_index = rom->data_offset + (address * rom->data_width) + bit;

			ctx->net_value[net_id] = plan->rom_init_value[storage_index];
			ctx->net_mask[net_id] = plan->rom_init_mask[storage_index];
			}
		}
	}

static void lxs_execute_rams(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->ram_count; ++i)
		{
		const lxs_ram_plan *ram = &plan->rams[i];
		uint8_t unknown = 0U;
		uint32_t address = lxs_read_index_bits(
			ctx,
			plan->ram_read_addr_net_ids,
			ram->read_addr_start,
			ram->addr_width,
			&unknown);

		if (unknown || address >= ram->depth)
			{
			lxs_write_unknown_outputs(ctx, plan->ram_output_net_ids, ram->output_start, ram->data_width);
			continue;
			}

		for (uint32_t bit = 0; bit < ram->data_width; ++bit)
			{
			uint32_t net_id = plan->ram_output_net_ids[ram->output_start + bit];
			uint32_t storage_index = ram->storage_offset + (address * ram->data_width) + bit;

			ctx->net_value[net_id] = ctx->ram_value[storage_index];
			ctx->net_mask[net_id] = ctx->ram_mask[storage_index];
			}
		}
	}

static void lxs_capture_counter_step(
	lxs_engine_ctx *ctx,
	const lxs_register_plan *reg,
	uint8_t decrement)
	{
	uint8_t carry = 1U;

	for (uint32_t bit = 0; bit < reg->width_bits; ++bit)
		{
		uint32_t storage_index = reg->storage_offset + bit;
		uint64_t cur_value = ctx->register_value[storage_index];
		uint64_t cur_mask = ctx->register_mask[storage_index];

		if (cur_mask != 0ULL)
			{
			for (uint32_t unknown_bit = bit; unknown_bit < reg->width_bits; ++unknown_bit)
				{
				uint32_t unknown_index = reg->storage_offset + unknown_bit;
				ctx->next_register_value[unknown_index] = 0ULL;
				ctx->next_register_mask[unknown_index] = ~0ULL;
				}
			return;
			}

		if (carry)
			{
			uint8_t cur_one = cur_value != 0ULL ? 1U : 0U;
			uint8_t next_one = decrement ? (cur_one ? 0U : 1U) : (cur_one ? 0U : 1U);

			ctx->next_register_value[storage_index] = next_one ? ~0ULL : 0ULL;
			ctx->next_register_mask[storage_index] = 0ULL;
			carry = decrement ? (cur_one ? 0U : 1U) : (cur_one ? 1U : 0U);
			}
		else
			{
			ctx->next_register_value[storage_index] = cur_value;
			ctx->next_register_mask[storage_index] = cur_mask;
			}
		}
	}

static void lxs_capture_next_registers(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->register_count; ++i)
		{
		const lxs_register_plan *reg = &plan->registers[i];
		uint8_t capture_inputs = 1U;
		uint8_t counter_step = 0U;
		uint8_t decrement = 0U;

		if (reg->mode == LXS_REGISTER_MODE_COUNTER_EN || reg->mode == LXS_REGISTER_MODE_COUNTER_UPDOWN)
			{
			if (reg->control_net == UINT32_MAX)
				{
				counter_step = 1U;
				}
			else
				{
				uint64_t control_value = ctx->net_value[reg->control_net];
				uint64_t control_mask = ctx->net_mask[reg->control_net];

				if (control_mask != 0ULL)
					{
					for (uint32_t bit = 0; bit < reg->width_bits; ++bit)
						{
						uint32_t storage_index = reg->storage_offset + bit;
						ctx->next_register_value[storage_index] = 0ULL;
						ctx->next_register_mask[storage_index] = ~0ULL;
						}
					continue;
					}
				counter_step = control_value != 0ULL ? 1U : 0U;
				}

			if (!counter_step)
				{
				for (uint32_t bit = 0; bit < reg->width_bits; ++bit)
					{
					uint32_t storage_index = reg->storage_offset + bit;
					ctx->next_register_value[storage_index] = ctx->register_value[storage_index];
					ctx->next_register_mask[storage_index] = ctx->register_mask[storage_index];
					}
				continue;
				}

			if (reg->mode == LXS_REGISTER_MODE_COUNTER_UPDOWN)
				{
				if (reg->aux_control_net == UINT32_MAX)
					{
					decrement = 0U;
					}
				else if (ctx->net_mask[reg->aux_control_net] != 0ULL)
					{
					for (uint32_t bit = 0; bit < reg->width_bits; ++bit)
						{
						uint32_t storage_index = reg->storage_offset + bit;
						ctx->next_register_value[storage_index] = 0ULL;
						ctx->next_register_mask[storage_index] = ~0ULL;
						}
					continue;
					}
				else
					{
					decrement = ctx->net_value[reg->aux_control_net] != 0ULL ? 1U : 0U;
					}
				}

			lxs_capture_counter_step(ctx, reg, decrement);
			continue;
			}

		if (reg->control_net != UINT32_MAX)
			{
			uint64_t control_value = ctx->net_value[reg->control_net];
			uint64_t control_mask = ctx->net_mask[reg->control_net];

			if (control_mask != 0ULL)
				{
				capture_inputs = reg->control_invert ? 0U : 1U;
				}
			else
				{
				uint8_t control_one = control_value != 0ULL ? 1U : 0U;
				capture_inputs = reg->control_invert ? (control_one ? 0U : 1U) : control_one;
				}
			}

		for (uint32_t bit = 0; bit < reg->width_bits; ++bit)
			{
			uint32_t storage_index = reg->storage_offset + bit;

			if (capture_inputs)
				{
				uint32_t net_id = plan->register_input_net_ids[reg->input_start + bit];
				ctx->next_register_value[storage_index] = ctx->net_value[net_id];
				ctx->next_register_mask[storage_index] = ctx->net_mask[net_id];
				}
			else
				{
				ctx->next_register_value[storage_index] = ctx->register_value[storage_index];
				ctx->next_register_mask[storage_index] = ctx->register_mask[storage_index];
				}
			}
		}
	}

static void lxs_commit_registers(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->register_count; ++i)
		{
		const lxs_register_plan *reg = &plan->registers[i];
		for (uint32_t bit = 0; bit < reg->width_bits; ++bit)
			{
			uint32_t storage_index = reg->storage_offset + bit;
			uint32_t net_id = plan->register_output_net_ids[reg->output_start + bit];

#if LXS_TEST_PROBES
			if (ctx->register_value[storage_index] != ctx->next_register_value[storage_index] ||
				ctx->register_mask[storage_index] != ctx->next_register_mask[storage_index])
				{
				ctx->probes.state_change_commit++;
				}
#endif

			ctx->register_value[storage_index] = ctx->next_register_value[storage_index];
			ctx->register_mask[storage_index] = ctx->next_register_mask[storage_index];
			ctx->net_value[net_id] = ctx->register_value[storage_index];
			ctx->net_mask[net_id] = ctx->register_mask[storage_index];
			}

		ctx->probes.state_commit_count += reg->width_bits;
		}
	}

static void lxs_capture_rams(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->ram_count; ++i)
		{
		const lxs_ram_plan *ram = &plan->rams[i];
		uint8_t unknown = 0U;
		uint32_t write_enable_net = ram->write_enable_net;
		uint64_t write_enable_value;

		ctx->ram_pending_write[i] = 0U;
		if (write_enable_net == UINT32_MAX)
			{
			write_enable_value = ~0ULL;
			}
		else
			{
			if (ctx->net_mask[write_enable_net] != 0ULL)
				{
				continue;
				}
			write_enable_value = ctx->net_value[write_enable_net];
			}

		if (write_enable_value == 0ULL)
			{
			continue;
			}

		ctx->ram_pending_addr[i] = lxs_read_index_bits(
			ctx,
			plan->ram_write_addr_net_ids,
			ram->write_addr_start,
			ram->addr_width,
			&unknown);
		if (unknown || ctx->ram_pending_addr[i] >= ram->depth)
			{
			continue;
			}

		for (uint32_t bit = 0; bit < ram->data_width; ++bit)
			{
			uint32_t stage_index = ram->stage_offset + bit;
			uint32_t net_id = plan->ram_data_input_net_ids[ram->data_input_start + bit];

			ctx->ram_stage_value[stage_index] = ctx->net_value[net_id];
			ctx->ram_stage_mask[stage_index] = ctx->net_mask[net_id];
			}

		ctx->ram_pending_write[i] = 1U;
		}
	}

static void lxs_commit_rams(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->ram_count; ++i)
		{
		const lxs_ram_plan *ram = &plan->rams[i];
		uint32_t address;

		if (!ctx->ram_pending_write[i])
			{
			continue;
			}

		address = ctx->ram_pending_addr[i];
		for (uint32_t bit = 0; bit < ram->data_width; ++bit)
			{
			uint32_t stage_index = ram->stage_offset + bit;
			uint32_t storage_index = ram->storage_offset + (address * ram->data_width) + bit;

#if LXS_TEST_PROBES
			if (ctx->ram_value[storage_index] != ctx->ram_stage_value[stage_index] ||
				ctx->ram_mask[storage_index] != ctx->ram_stage_mask[stage_index])
				{
				ctx->probes.state_change_commit++;
				}
#endif

			ctx->ram_value[storage_index] = ctx->ram_stage_value[stage_index];
			ctx->ram_mask[storage_index] = ctx->ram_stage_mask[stage_index];
			}

		ctx->ram_pending_write[i] = 0U;
		ctx->probes.state_commit_count += ram->data_width;
		}
	}

static void lxs_execute_logic_chunk(
	lxs_engine_ctx *ctx,
	const lxs_gate_ir *gates,
	uint32_t count,
	uint32_t type)
	{
	uint64_t *restrict net_value = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_value);
	uint64_t *restrict net_mask = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_mask);
	const lxs_gate_ir *gate = gates;
	const lxs_gate_ir *gate_end = gates + count;

#if LXS_LOGIC_CHUNK_FAST_MODE != 0
	if (count > 0U &&
		count >= (uint32_t)LXS_LOGIC_CHUNK_FAST_MIN_COUNT &&
		gates[0].input_count == 2U)
		{
		switch (type)
			{
#if LXS_LOGIC_CHUNK_FAST_MODE == 1 || LXS_LOGIC_CHUNK_FAST_MODE == 2
			case LXS_GATE_AND:
				for (; gate != gate_end; ++gate)
					{
					uint32_t a = gate->inputs[0];
					uint32_t b = gate->inputs[1];
					uint32_t dst = gate->output;
					uint64_t out_value;
					uint64_t out_mask;

					LXS_EVAL_AND(
						net_value[a],
						net_mask[a],
						net_value[b],
						net_mask[b],
						out_value,
						out_mask);
					net_value[dst] = out_value;
					net_mask[dst] = out_mask;
					}
				return;
#endif
#if LXS_LOGIC_CHUNK_FAST_MODE == 2
			case LXS_GATE_OR:
				for (; gate != gate_end; ++gate)
					{
					uint32_t a = gate->inputs[0];
					uint32_t b = gate->inputs[1];
					uint32_t dst = gate->output;
					uint64_t out_value;
					uint64_t out_mask;

					LXS_EVAL_OR(
						net_value[a],
						net_mask[a],
						net_value[b],
						net_mask[b],
						out_value,
						out_mask);
					net_value[dst] = out_value;
					net_mask[dst] = out_mask;
					}
				return;
			case LXS_GATE_XOR:
				for (; gate != gate_end; ++gate)
					{
					uint32_t a = gate->inputs[0];
					uint32_t b = gate->inputs[1];
					uint32_t dst = gate->output;
					uint64_t out_value;
					uint64_t out_mask;

					LXS_EVAL_XOR(
						net_value[a],
						net_mask[a],
						net_value[b],
						net_mask[b],
						out_value,
						out_mask);
					net_value[dst] = out_value;
					net_mask[dst] = out_mask;
					}
				return;
#endif
			default:
				break;
			}
		}
#endif

	switch (type)
		{
		case LXS_GATE_AND:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;
				uint32_t input_count = gate->input_count;

				LXS_EVAL_BUF(
					net_value[gate->inputs[0]],
					net_mask[gate->inputs[0]],
					out_value,
					out_mask);
				for (uint32_t i = 1U; i < input_count; ++i)
					{
					LXS_EVAL_AND(
						out_value,
						out_mask,
						net_value[gate->inputs[i]],
						net_mask[gate->inputs[i]],
						out_value,
						out_mask);
					}
				net_value[gate->output] = out_value;
				net_mask[gate->output] = out_mask;
				}
			break;
		case LXS_GATE_OR:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;
				uint32_t input_count = gate->input_count;

				LXS_EVAL_BUF(
					net_value[gate->inputs[0]],
					net_mask[gate->inputs[0]],
					out_value,
					out_mask);
				for (uint32_t i = 1U; i < input_count; ++i)
					{
					LXS_EVAL_OR(
						out_value,
						out_mask,
						net_value[gate->inputs[i]],
						net_mask[gate->inputs[i]],
						out_value,
						out_mask);
					}
				net_value[gate->output] = out_value;
				net_mask[gate->output] = out_mask;
				}
			break;
		case LXS_GATE_XOR:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;
				uint32_t input_count = gate->input_count;

				LXS_EVAL_BUF(
					net_value[gate->inputs[0]],
					net_mask[gate->inputs[0]],
					out_value,
					out_mask);
				for (uint32_t i = 1U; i < input_count; ++i)
					{
					LXS_EVAL_XOR(
						out_value,
						out_mask,
						net_value[gate->inputs[i]],
						net_mask[gate->inputs[i]],
						out_value,
						out_mask);
					}
				net_value[gate->output] = out_value;
				net_mask[gate->output] = out_mask;
				}
			break;
		case LXS_GATE_TRI:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;

				LXS_EVAL_TRI(
					net_value[gate->inputs[0]],
					net_mask[gate->inputs[0]],
					net_value[gate->inputs[1]],
					net_mask[gate->inputs[1]],
					out_value,
					out_mask);
				net_value[gate->output] = out_value;
				net_mask[gate->output] = out_mask;
				}
			break;
		case LXS_GATE_NAND:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;
				uint32_t input_count = gate->input_count;

				LXS_EVAL_BUF(
					net_value[gate->inputs[0]],
					net_mask[gate->inputs[0]],
					out_value,
					out_mask);
				for (uint32_t i = 1U; i < input_count; ++i)
					{
					LXS_EVAL_AND(
						out_value,
						out_mask,
						net_value[gate->inputs[i]],
						net_mask[gate->inputs[i]],
						out_value,
						out_mask);
					}
				LXS_EVAL_NOT(out_value, out_mask, out_value, out_mask);
				net_value[gate->output] = out_value;
				net_mask[gate->output] = out_mask;
				}
			break;
		case LXS_GATE_NOR:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;
				uint32_t input_count = gate->input_count;

				LXS_EVAL_BUF(
					net_value[gate->inputs[0]],
					net_mask[gate->inputs[0]],
					out_value,
					out_mask);
				for (uint32_t i = 1U; i < input_count; ++i)
					{
					LXS_EVAL_OR(
						out_value,
						out_mask,
						net_value[gate->inputs[i]],
						net_mask[gate->inputs[i]],
						out_value,
						out_mask);
					}
				LXS_EVAL_NOT(out_value, out_mask, out_value, out_mask);
				net_value[gate->output] = out_value;
				net_mask[gate->output] = out_mask;
				}
			break;
		case LXS_GATE_XNOR:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;
				uint32_t input_count = gate->input_count;

				LXS_EVAL_BUF(
					net_value[gate->inputs[0]],
					net_mask[gate->inputs[0]],
					out_value,
					out_mask);
				for (uint32_t i = 1U; i < input_count; ++i)
					{
					LXS_EVAL_XOR(
						out_value,
						out_mask,
						net_value[gate->inputs[i]],
						net_mask[gate->inputs[i]],
						out_value,
						out_mask);
					}
				LXS_EVAL_NOT(out_value, out_mask, out_value, out_mask);
				net_value[gate->output] = out_value;
				net_mask[gate->output] = out_mask;
				}
			break;
		default:
			break;
		}
	}

static void lxs_execute_not_chunk(
	lxs_engine_ctx *ctx,
	const lxs_gate_ir *gates,
	uint32_t count)
	{
	uint64_t *restrict net_value = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_value);
	uint64_t *restrict net_mask = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_mask);
	const lxs_gate_ir *gate = gates;
	const lxs_gate_ir *gate_end = gates + count;

	for (; gate != gate_end; ++gate)
		{
		uint32_t src = gate->inputs[0];
		uint32_t dst = gate->output;
		uint64_t out_value;
		uint64_t out_mask;

		LXS_EVAL_NOT(net_value[src], net_mask[src], out_value, out_mask);
		net_value[dst] = out_value;
		net_mask[dst] = out_mask;
		}
	}

static void lxs_execute_buf_chunk(
	lxs_engine_ctx *ctx,
	const lxs_gate_ir *gates,
	uint32_t count)
	{
	uint64_t *restrict net_value = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_value);
	uint64_t *restrict net_mask = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_mask);
	const lxs_gate_ir *gate = gates;
	const lxs_gate_ir *gate_end = gates + count;

	for (; gate != gate_end; ++gate)
		{
		uint32_t src = gate->inputs[0];
		uint32_t dst = gate->output;
		uint64_t out_value;
		uint64_t out_mask;

		LXS_EVAL_BUF(net_value[src], net_mask[src], out_value, out_mask);
		net_value[dst] = out_value;
		net_mask[dst] = out_mask;
		}
	}

static void lxs_execute_unary_chunk(
	lxs_engine_ctx *ctx,
	const lxs_gate_ir *gates,
	uint32_t count,
	uint32_t type)
	{
	if (type == LXS_GATE_NOT)
		{
		lxs_execute_not_chunk(ctx, gates, count);
		}
	else
		{
		lxs_execute_buf_chunk(ctx, gates, count);
		}
	}

static void lxs_execute_macros(
	lxs_engine_ctx *ctx,
	const lxs_macro_plan *macros,
	uint32_t count)
	{
	uint64_t *restrict net_value = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_value);
	uint64_t *restrict net_mask = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_mask);

	for (uint32_t i = 0; i < count; ++i)
		{
		const lxs_macro_plan *macro = &macros[i];
		uint64_t nsel_value;
		uint64_t nsel_mask;
		uint64_t lo_value;
		uint64_t lo_mask;
		uint64_t hi_value;
		uint64_t hi_mask;
		uint64_t out_value;
		uint64_t out_mask;

		if (macro->type == LXS_MACRO_MUX2)
			{
			LXS_EVAL_NOT(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				nsel_value,
				nsel_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				nsel_value,
				nsel_mask,
				lo_value,
				lo_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				hi_value,
				hi_mask);
			LXS_EVAL_OR(lo_value, lo_mask, hi_value, hi_mask, out_value, out_mask);
			}
		else if (macro->type == LXS_MACRO_XOR2)
			{
			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				out_value,
				out_mask);
			}
		else if (macro->type == LXS_MACRO_PARITY4)
			{
			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				lo_value,
				lo_mask);
			LXS_EVAL_XOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				hi_value,
				hi_mask);
			LXS_EVAL_XOR(
				lo_value,
				lo_mask,
				hi_value,
				hi_mask,
				out_value,
				out_mask);
			}
		else if (macro->type == LXS_MACRO_PARITY8)
			{
			uint64_t lo2_value;
			uint64_t lo2_mask;

			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				lo_value,
				lo_mask);
			LXS_EVAL_XOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				hi_value,
				hi_mask);
			LXS_EVAL_XOR(lo_value, lo_mask, hi_value, hi_mask, lo2_value, lo2_mask);
			LXS_EVAL_XOR(
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				net_value[macro->inputs[5]],
				net_mask[macro->inputs[5]],
				lo_value,
				lo_mask);
			LXS_EVAL_XOR(
				net_value[macro->inputs[6]],
				net_mask[macro->inputs[6]],
				net_value[macro->inputs[7]],
				net_mask[macro->inputs[7]],
				hi_value,
				hi_mask);
			LXS_EVAL_XOR(lo_value, lo_mask, hi_value, hi_mask, out_value, out_mask);
			LXS_EVAL_XOR(lo2_value, lo2_mask, out_value, out_mask, out_value, out_mask);
			}
		else if (macro->type == LXS_MACRO_CARRY_INV2)
			{
			LXS_EVAL_NOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				lo_value,
				lo_mask);
			LXS_EVAL_NOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				lo_value,
				lo_mask,
				out_value,
				out_mask);
			}
		else if (macro->type == LXS_MACRO_SUM_CINV2)
			{
			LXS_EVAL_NOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				lo_value,
				lo_mask);
			LXS_EVAL_XNOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				lo_value,
				lo_mask,
				out_value,
				out_mask);
			}
		else
			{
			LXS_EVAL_XNOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				out_value,
				out_mask);
			}
		net_value[macro->output] = out_value;
		net_mask[macro->output] = out_mask;

		ctx->probes.chunk_exec++;
		ctx->probes.gate_eval += macro->gate_equiv_count;
		}
	}

static void lxs_execute_multi_macros(
	lxs_engine_ctx *ctx,
	const lxs_multi_macro_plan *macros,
	uint32_t count)
	{
	uint64_t *restrict net_value = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_value);
	uint64_t *restrict net_mask = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_mask);

	for (uint32_t i = 0; i < count; ++i)
		{
		const lxs_multi_macro_plan *macro = &macros[i];
		uint64_t and_value;
		uint64_t and_mask;
		uint64_t nor_value;
		uint64_t nor_mask;
		uint64_t xor_value;
		uint64_t xor_mask;
		uint64_t inner_value;
		uint64_t inner_mask;
		uint64_t sum_value;
		uint64_t sum_mask;
		uint64_t carry_value;
		uint64_t carry_mask;
		uint64_t and0_value;
		uint64_t and0_mask;
		uint64_t nor0_value;
		uint64_t nor0_mask;
		uint64_t xor0_value;
		uint64_t xor0_mask;
		uint64_t inner0_value;
		uint64_t inner0_mask;
		uint64_t sum0_value;
		uint64_t sum0_mask;
		uint64_t carry0_value;
		uint64_t carry0_mask;
		uint64_t and1_value;
		uint64_t and1_mask;
		uint64_t nor1_value;
		uint64_t nor1_mask;
		uint64_t xor1_value;
		uint64_t xor1_mask;
		uint64_t inner1_value;
		uint64_t inner1_mask;
		uint64_t sum1_value;
		uint64_t sum1_mask;
		uint64_t carry1_value;
		uint64_t carry1_mask;

		if (macro->type == LXS_MULTI_MACRO_HALF_ADDER)
			{
			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				sum_value,
				sum_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				carry_value,
				carry_mask);

			net_value[macro->outputs[0]] = sum_value;
			net_mask[macro->outputs[0]] = sum_mask;
			net_value[macro->outputs[1]] = carry_value;
			net_mask[macro->outputs[1]] = carry_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_FULL_ADDER)
			{
			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				xor_value,
				xor_mask);
			LXS_EVAL_XOR(
				xor_value,
				xor_mask,
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				sum_value,
				sum_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				and_value,
				and_mask);
			LXS_EVAL_AND(
				xor_value,
				xor_mask,
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				inner_value,
				inner_mask);
			LXS_EVAL_OR(
				and_value,
				and_mask,
				inner_value,
				inner_mask,
				carry_value,
				carry_mask);

			net_value[macro->outputs[0]] = sum_value;
			net_mask[macro->outputs[0]] = sum_mask;
			net_value[macro->outputs[1]] = carry_value;
			net_mask[macro->outputs[1]] = carry_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_RIPPLE_SLICE2)
			{
			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				xor0_value,
				xor0_mask);
			LXS_EVAL_XOR(
				xor0_value,
				xor0_mask,
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				sum0_value,
				sum0_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				and0_value,
				and0_mask);
			LXS_EVAL_AND(
				xor0_value,
				xor0_mask,
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				inner0_value,
				inner0_mask);
			LXS_EVAL_OR(and0_value, and0_mask, inner0_value, inner0_mask, carry0_value, carry0_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				xor1_value,
				xor1_mask);
			LXS_EVAL_XOR(carry0_value, carry0_mask, xor1_value, xor1_mask, sum1_value, sum1_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				and1_value,
				and1_mask);
			LXS_EVAL_AND(carry0_value, carry0_mask, xor1_value, xor1_mask, inner1_value, inner1_mask);
			LXS_EVAL_OR(and1_value, and1_mask, inner1_value, inner1_mask, carry1_value, carry1_mask);

			net_value[macro->outputs[0]] = sum0_value;
			net_mask[macro->outputs[0]] = sum0_mask;
			net_value[macro->outputs[1]] = sum1_value;
			net_mask[macro->outputs[1]] = sum1_mask;
			net_value[macro->outputs[2]] = carry1_value;
			net_mask[macro->outputs[2]] = carry1_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_RIPPLE_ADD4)
			{
			uint64_t xor2_value;
			uint64_t xor2_mask;
			uint64_t and2_value;
			uint64_t and2_mask;
			uint64_t inner2_value;
			uint64_t inner2_mask;
			uint64_t sum2_value;
			uint64_t sum2_mask;
			uint64_t carry2_value;
			uint64_t carry2_mask;
			uint64_t xor3_value;
			uint64_t xor3_mask;
			uint64_t and3_value;
			uint64_t and3_mask;
			uint64_t inner3_value;
			uint64_t inner3_mask;
			uint64_t sum3_value;
			uint64_t sum3_mask;
			uint64_t carry3_value;
			uint64_t carry3_mask;

			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				xor0_value,
				xor0_mask);
			LXS_EVAL_XOR(
				xor0_value,
				xor0_mask,
				net_value[macro->inputs[8]],
				net_mask[macro->inputs[8]],
				sum0_value,
				sum0_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				and0_value,
				and0_mask);
			LXS_EVAL_AND(
				xor0_value,
				xor0_mask,
				net_value[macro->inputs[8]],
				net_mask[macro->inputs[8]],
				inner0_value,
				inner0_mask);
			LXS_EVAL_OR(and0_value, and0_mask, inner0_value, inner0_mask, carry0_value, carry0_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				xor1_value,
				xor1_mask);
			LXS_EVAL_XOR(carry0_value, carry0_mask, xor1_value, xor1_mask, sum1_value, sum1_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				and1_value,
				and1_mask);
			LXS_EVAL_AND(carry0_value, carry0_mask, xor1_value, xor1_mask, inner1_value, inner1_mask);
			LXS_EVAL_OR(and1_value, and1_mask, inner1_value, inner1_mask, carry1_value, carry1_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				net_value[macro->inputs[5]],
				net_mask[macro->inputs[5]],
				xor2_value,
				xor2_mask);
			LXS_EVAL_XOR(carry1_value, carry1_mask, xor2_value, xor2_mask, sum2_value, sum2_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				net_value[macro->inputs[5]],
				net_mask[macro->inputs[5]],
				and2_value,
				and2_mask);
			LXS_EVAL_AND(carry1_value, carry1_mask, xor2_value, xor2_mask, inner2_value, inner2_mask);
			LXS_EVAL_OR(and2_value, and2_mask, inner2_value, inner2_mask, carry2_value, carry2_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[6]],
				net_mask[macro->inputs[6]],
				net_value[macro->inputs[7]],
				net_mask[macro->inputs[7]],
				xor3_value,
				xor3_mask);
			LXS_EVAL_XOR(carry2_value, carry2_mask, xor3_value, xor3_mask, sum3_value, sum3_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[6]],
				net_mask[macro->inputs[6]],
				net_value[macro->inputs[7]],
				net_mask[macro->inputs[7]],
				and3_value,
				and3_mask);
			LXS_EVAL_AND(carry2_value, carry2_mask, xor3_value, xor3_mask, inner3_value, inner3_mask);
			LXS_EVAL_OR(and3_value, and3_mask, inner3_value, inner3_mask, carry3_value, carry3_mask);

			net_value[macro->outputs[0]] = sum0_value;
			net_mask[macro->outputs[0]] = sum0_mask;
			net_value[macro->outputs[1]] = sum1_value;
			net_mask[macro->outputs[1]] = sum1_mask;
			net_value[macro->outputs[2]] = sum2_value;
			net_mask[macro->outputs[2]] = sum2_mask;
			net_value[macro->outputs[3]] = sum3_value;
			net_mask[macro->outputs[3]] = sum3_mask;
			net_value[macro->outputs[4]] = carry3_value;
			net_mask[macro->outputs[4]] = carry3_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_CARRY_SAVE_ROW4)
			{
			uint64_t xor2_value;
			uint64_t xor2_mask;
			uint64_t and2_value;
			uint64_t and2_mask;
			uint64_t inner2_value;
			uint64_t inner2_mask;
			uint64_t sum2_value;
			uint64_t sum2_mask;
			uint64_t carry2_value;
			uint64_t carry2_mask;
			uint64_t xor3_value;
			uint64_t xor3_mask;
			uint64_t and3_value;
			uint64_t and3_mask;
			uint64_t inner3_value;
			uint64_t inner3_mask;
			uint64_t sum3_value;
			uint64_t sum3_mask;
			uint64_t carry3_value;
			uint64_t carry3_mask;

			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				xor0_value,
				xor0_mask);
			LXS_EVAL_XOR(
				xor0_value,
				xor0_mask,
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				sum0_value,
				sum0_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				and0_value,
				and0_mask);
			LXS_EVAL_AND(
				xor0_value,
				xor0_mask,
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				inner0_value,
				inner0_mask);
			LXS_EVAL_OR(and0_value, and0_mask, inner0_value, inner0_mask, carry0_value, carry0_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				xor1_value,
				xor1_mask);
			LXS_EVAL_XOR(
				xor1_value,
				xor1_mask,
				net_value[macro->inputs[5]],
				net_mask[macro->inputs[5]],
				sum1_value,
				sum1_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				and1_value,
				and1_mask);
			LXS_EVAL_AND(
				xor1_value,
				xor1_mask,
				net_value[macro->inputs[5]],
				net_mask[macro->inputs[5]],
				inner1_value,
				inner1_mask);
			LXS_EVAL_OR(and1_value, and1_mask, inner1_value, inner1_mask, carry1_value, carry1_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[6]],
				net_mask[macro->inputs[6]],
				net_value[macro->inputs[7]],
				net_mask[macro->inputs[7]],
				xor2_value,
				xor2_mask);
			LXS_EVAL_XOR(
				xor2_value,
				xor2_mask,
				net_value[macro->inputs[8]],
				net_mask[macro->inputs[8]],
				sum2_value,
				sum2_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[6]],
				net_mask[macro->inputs[6]],
				net_value[macro->inputs[7]],
				net_mask[macro->inputs[7]],
				and2_value,
				and2_mask);
			LXS_EVAL_AND(
				xor2_value,
				xor2_mask,
				net_value[macro->inputs[8]],
				net_mask[macro->inputs[8]],
				inner2_value,
				inner2_mask);
			LXS_EVAL_OR(and2_value, and2_mask, inner2_value, inner2_mask, carry2_value, carry2_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[9]],
				net_mask[macro->inputs[9]],
				net_value[macro->inputs[10]],
				net_mask[macro->inputs[10]],
				xor3_value,
				xor3_mask);
			LXS_EVAL_XOR(
				xor3_value,
				xor3_mask,
				net_value[macro->inputs[11]],
				net_mask[macro->inputs[11]],
				sum3_value,
				sum3_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[9]],
				net_mask[macro->inputs[9]],
				net_value[macro->inputs[10]],
				net_mask[macro->inputs[10]],
				and3_value,
				and3_mask);
			LXS_EVAL_AND(
				xor3_value,
				xor3_mask,
				net_value[macro->inputs[11]],
				net_mask[macro->inputs[11]],
				inner3_value,
				inner3_mask);
			LXS_EVAL_OR(and3_value, and3_mask, inner3_value, inner3_mask, carry3_value, carry3_mask);

			net_value[macro->outputs[0]] = sum0_value;
			net_mask[macro->outputs[0]] = sum0_mask;
			net_value[macro->outputs[1]] = sum1_value;
			net_mask[macro->outputs[1]] = sum1_mask;
			net_value[macro->outputs[2]] = sum2_value;
			net_mask[macro->outputs[2]] = sum2_mask;
			net_value[macro->outputs[3]] = sum3_value;
			net_mask[macro->outputs[3]] = sum3_mask;
			net_value[macro->outputs[4]] = carry0_value;
			net_mask[macro->outputs[4]] = carry0_mask;
			net_value[macro->outputs[5]] = carry1_value;
			net_mask[macro->outputs[5]] = carry1_mask;
			net_value[macro->outputs[6]] = carry2_value;
			net_mask[macro->outputs[6]] = carry2_mask;
			net_value[macro->outputs[7]] = carry3_value;
			net_mask[macro->outputs[7]] = carry3_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_REDUCE_PROPAGATE4)
			{
			uint64_t rp_xor0_value;
			uint64_t rp_xor0_mask;
			uint64_t rp_xor1_value;
			uint64_t rp_xor1_mask;
			uint64_t rp_xor2_value;
			uint64_t rp_xor2_mask;
			uint64_t rp_xor3_value;
			uint64_t rp_xor3_mask;
			uint64_t rp_and0a_value;
			uint64_t rp_and0a_mask;
			uint64_t rp_and0b_value;
			uint64_t rp_and0b_mask;
			uint64_t rp_and1a_value;
			uint64_t rp_and1a_mask;
			uint64_t rp_and1b_value;
			uint64_t rp_and1b_mask;
			uint64_t rp_and2a_value;
			uint64_t rp_and2a_mask;
			uint64_t rp_and2b_value;
			uint64_t rp_and2b_mask;
			uint64_t rp_and3a_value;
			uint64_t rp_and3a_mask;
			uint64_t rp_and3b_value;
			uint64_t rp_and3b_mask;
			uint64_t rp_sum0_value;
			uint64_t rp_sum0_mask;
			uint64_t rp_sum1_value;
			uint64_t rp_sum1_mask;
			uint64_t rp_sum2_value;
			uint64_t rp_sum2_mask;
			uint64_t rp_sum3_value;
			uint64_t rp_sum3_mask;
			uint64_t rp_carry1_value;
			uint64_t rp_carry1_mask;
			uint64_t rp_carry2_value;
			uint64_t rp_carry2_mask;
			uint64_t rp_carry3_value;
			uint64_t rp_carry3_mask;
			uint64_t rp_carry4_value;
			uint64_t rp_carry4_mask;
			uint64_t rp_carry01_value;
			uint64_t rp_carry01_mask;
			uint64_t rp_bit2_value;
			uint64_t rp_bit2_mask;
			uint64_t rp_carry12a_value;
			uint64_t rp_carry12a_mask;
			uint64_t rp_carry12b_value;
			uint64_t rp_carry12b_mask;
			uint64_t rp_carry12_value;
			uint64_t rp_carry12_mask;
			uint64_t rp_bit3_value;
			uint64_t rp_bit3_mask;
			uint64_t rp_carry23a_value;
			uint64_t rp_carry23a_mask;
			uint64_t rp_carry23b_value;
			uint64_t rp_carry23b_mask;
			uint64_t rp_spill0a_value;
			uint64_t rp_spill0a_mask;

			LXS_EVAL_XOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				rp_xor0_value,
				rp_xor0_mask);
			LXS_EVAL_XOR(
				rp_xor0_value,
				rp_xor0_mask,
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				rp_sum0_value,
				rp_sum0_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				rp_and0a_value,
				rp_and0a_mask);
			LXS_EVAL_AND(
				rp_xor0_value,
				rp_xor0_mask,
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				rp_and0b_value,
				rp_and0b_mask);
			LXS_EVAL_OR(rp_and0a_value, rp_and0a_mask, rp_and0b_value, rp_and0b_mask, rp_carry1_value, rp_carry1_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				rp_xor1_value,
				rp_xor1_mask);
			LXS_EVAL_XOR(
				rp_xor1_value,
				rp_xor1_mask,
				net_value[macro->inputs[5]],
				net_mask[macro->inputs[5]],
				rp_sum1_value,
				rp_sum1_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				rp_and1a_value,
				rp_and1a_mask);
			LXS_EVAL_AND(
				rp_xor1_value,
				rp_xor1_mask,
				net_value[macro->inputs[5]],
				net_mask[macro->inputs[5]],
				rp_and1b_value,
				rp_and1b_mask);
			LXS_EVAL_OR(rp_and1a_value, rp_and1a_mask, rp_and1b_value, rp_and1b_mask, rp_carry2_value, rp_carry2_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[6]],
				net_mask[macro->inputs[6]],
				net_value[macro->inputs[7]],
				net_mask[macro->inputs[7]],
				rp_xor2_value,
				rp_xor2_mask);
			LXS_EVAL_XOR(
				rp_xor2_value,
				rp_xor2_mask,
				net_value[macro->inputs[8]],
				net_mask[macro->inputs[8]],
				rp_sum2_value,
				rp_sum2_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[6]],
				net_mask[macro->inputs[6]],
				net_value[macro->inputs[7]],
				net_mask[macro->inputs[7]],
				rp_and2a_value,
				rp_and2a_mask);
			LXS_EVAL_AND(
				rp_xor2_value,
				rp_xor2_mask,
				net_value[macro->inputs[8]],
				net_mask[macro->inputs[8]],
				rp_and2b_value,
				rp_and2b_mask);
			LXS_EVAL_OR(rp_and2a_value, rp_and2a_mask, rp_and2b_value, rp_and2b_mask, rp_carry3_value, rp_carry3_mask);

			LXS_EVAL_XOR(
				net_value[macro->inputs[9]],
				net_mask[macro->inputs[9]],
				net_value[macro->inputs[10]],
				net_mask[macro->inputs[10]],
				rp_xor3_value,
				rp_xor3_mask);
			LXS_EVAL_XOR(
				rp_xor3_value,
				rp_xor3_mask,
				net_value[macro->inputs[11]],
				net_mask[macro->inputs[11]],
				rp_sum3_value,
				rp_sum3_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[9]],
				net_mask[macro->inputs[9]],
				net_value[macro->inputs[10]],
				net_mask[macro->inputs[10]],
				rp_and3a_value,
				rp_and3a_mask);
			LXS_EVAL_AND(
				rp_xor3_value,
				rp_xor3_mask,
				net_value[macro->inputs[11]],
				net_mask[macro->inputs[11]],
				rp_and3b_value,
				rp_and3b_mask);
			LXS_EVAL_OR(rp_and3a_value, rp_and3a_mask, rp_and3b_value, rp_and3b_mask, rp_carry4_value, rp_carry4_mask);

			LXS_EVAL_XOR(rp_sum1_value, rp_sum1_mask, rp_carry1_value, rp_carry1_mask, sum_value, sum_mask);
			net_value[macro->outputs[1]] = sum_value;
			net_mask[macro->outputs[1]] = sum_mask;
			LXS_EVAL_AND(rp_sum1_value, rp_sum1_mask, rp_carry1_value, rp_carry1_mask, rp_carry01_value, rp_carry01_mask);

			LXS_EVAL_XOR(rp_sum2_value, rp_sum2_mask, rp_carry2_value, rp_carry2_mask, rp_bit2_value, rp_bit2_mask);
			LXS_EVAL_AND(rp_sum2_value, rp_sum2_mask, rp_carry2_value, rp_carry2_mask, rp_carry12a_value, rp_carry12a_mask);
			LXS_EVAL_XOR(rp_bit2_value, rp_bit2_mask, rp_carry01_value, rp_carry01_mask, net_value[macro->outputs[2]], net_mask[macro->outputs[2]]);
			LXS_EVAL_AND(rp_bit2_value, rp_bit2_mask, rp_carry01_value, rp_carry01_mask, rp_carry12b_value, rp_carry12b_mask);
			LXS_EVAL_OR(rp_carry12a_value, rp_carry12a_mask, rp_carry12b_value, rp_carry12b_mask, rp_carry12_value, rp_carry12_mask);

			LXS_EVAL_XOR(rp_sum3_value, rp_sum3_mask, rp_carry3_value, rp_carry3_mask, rp_bit3_value, rp_bit3_mask);
			LXS_EVAL_AND(rp_sum3_value, rp_sum3_mask, rp_carry3_value, rp_carry3_mask, rp_carry23a_value, rp_carry23a_mask);
			LXS_EVAL_XOR(rp_bit3_value, rp_bit3_mask, rp_carry12_value, rp_carry12_mask, net_value[macro->outputs[3]], net_mask[macro->outputs[3]]);
			LXS_EVAL_AND(rp_bit3_value, rp_bit3_mask, rp_carry12_value, rp_carry12_mask, rp_carry23b_value, rp_carry23b_mask);
			LXS_EVAL_OR(rp_carry23a_value, rp_carry23a_mask, rp_carry23b_value, rp_carry23b_mask, rp_spill0a_value, rp_spill0a_mask);

			LXS_EVAL_XOR(rp_carry4_value, rp_carry4_mask, rp_spill0a_value, rp_spill0a_mask, net_value[macro->outputs[4]], net_mask[macro->outputs[4]]);
			LXS_EVAL_AND(rp_carry4_value, rp_carry4_mask, rp_spill0a_value, rp_spill0a_mask, net_value[macro->outputs[5]], net_mask[macro->outputs[5]]);

			net_value[macro->outputs[0]] = rp_sum0_value;
			net_mask[macro->outputs[0]] = rp_sum0_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_XOR_FAN8)
			{
			for (uint32_t j = 0; j < 8U; ++j)
				{
				LXS_EVAL_XOR(
					net_value[macro->inputs[0]],
					net_mask[macro->inputs[0]],
					net_value[macro->inputs[j + 1U]],
					net_mask[macro->inputs[j + 1U]],
					sum_value,
					sum_mask);
				net_value[macro->outputs[j]] = sum_value;
				net_mask[macro->outputs[j]] = sum_mask;
				}
			}
		else if (macro->type == LXS_MULTI_MACRO_AND_FAN8)
			{
			for (uint32_t j = 0; j < 8U; ++j)
				{
				LXS_EVAL_AND(
					net_value[macro->inputs[0]],
					net_mask[macro->inputs[0]],
					net_value[macro->inputs[j + 1U]],
					net_mask[macro->inputs[j + 1U]],
					sum_value,
					sum_mask);
				net_value[macro->outputs[j]] = sum_value;
				net_mask[macro->outputs[j]] = sum_mask;
				}
			}
		else if (macro->type == LXS_MULTI_MACRO_COMPARE_AND4)
			{
			uint64_t term_value;
			uint64_t term_mask;
			uint32_t invert_mask = macro->param0 & 0xFU;

			term_value = net_value[macro->inputs[0]];
			term_mask = net_mask[macro->inputs[0]];
			if ((invert_mask & 1U) != 0U)
				{
				LXS_EVAL_NOT(term_value, term_mask, term_value, term_mask);
				}

			for (uint32_t j = 1; j < 4U; ++j)
				{
				uint64_t rhs_value = net_value[macro->inputs[j]];
				uint64_t rhs_mask = net_mask[macro->inputs[j]];

				if ((invert_mask & (1U << j)) != 0U)
					{
					LXS_EVAL_NOT(rhs_value, rhs_mask, rhs_value, rhs_mask);
					}

				LXS_EVAL_AND(
					term_value,
					term_mask,
					rhs_value,
					rhs_mask,
					term_value,
					term_mask);
				}

			net_value[macro->outputs[0]] = term_value;
			net_mask[macro->outputs[0]] = term_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_COMPARE_OR4)
			{
			uint64_t direct_value[4];
			uint64_t direct_mask[4];
			uint64_t invert_value[4];
			uint64_t invert_mask_arr[4];
			uint64_t out_value;
			uint64_t out_mask;

			for (uint32_t j = 0; j < 4U; ++j)
				{
				direct_value[j] = net_value[macro->inputs[j]];
				direct_mask[j] = net_mask[macro->inputs[j]];
				LXS_EVAL_NOT(
					direct_value[j],
					direct_mask[j],
					invert_value[j],
					invert_mask_arr[j]);
				}

			out_value = 0ULL;
			out_mask = 0ULL;
			for (uint32_t term_index = 0; term_index < 4U; ++term_index)
				{
				uint32_t term_bits = (macro->param0 >> (term_index * 4U)) & 0xFU;
				uint64_t term_value;
				uint64_t term_mask;

				for (uint32_t j = 0; j < 4U; ++j)
					{
					uint64_t select = ((term_bits >> j) & 1U) ? ~0ULL : 0ULL;
					uint64_t chosen_value = (direct_value[j] & ~select) | (invert_value[j] & select);
					uint64_t chosen_mask = (direct_mask[j] & ~select) | (invert_mask_arr[j] & select);

					if (j == 0U)
						{
						term_value = chosen_value;
						term_mask = chosen_mask;
						}
					else
						{
						LXS_EVAL_AND(
							term_value,
							term_mask,
							chosen_value,
							chosen_mask,
							term_value,
							term_mask);
						}
					}

				if (term_index == 0U)
					{
					out_value = term_value;
					out_mask = term_mask;
					}
				else
					{
					LXS_EVAL_OR(
						out_value,
						out_mask,
						term_value,
						term_mask,
						out_value,
						out_mask);
					}
				}

			net_value[macro->outputs[0]] = out_value;
			net_mask[macro->outputs[0]] = out_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_XNOR_BANK4)
			{
			for (uint32_t j = 0; j < 4U; ++j)
				{
				LXS_EVAL_XOR(
					net_value[macro->inputs[j * 2U]],
					net_mask[macro->inputs[j * 2U]],
					net_value[macro->inputs[j * 2U + 1U]],
					net_mask[macro->inputs[j * 2U + 1U]],
					sum_value,
					sum_mask);
				LXS_EVAL_NOT(sum_value, sum_mask, sum_value, sum_mask);
				net_value[macro->outputs[j]] = sum_value;
				net_mask[macro->outputs[j]] = sum_mask;
				}
			}
		else if (macro->type == LXS_MULTI_MACRO_GUARD_CHAIN4)
			{
			uint64_t inv_value;
			uint64_t inv_mask;

			LXS_EVAL_AND(
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				sum_value,
				sum_mask);
			net_value[macro->outputs[0]] = sum_value;
			net_mask[macro->outputs[0]] = sum_mask;

			LXS_EVAL_NOT(sum_value, sum_mask, inv_value, inv_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				inv_value,
				inv_mask,
				sum_value,
				sum_mask);
			net_value[macro->outputs[1]] = sum_value;
			net_mask[macro->outputs[1]] = sum_mask;

			LXS_EVAL_NOT(sum_value, sum_mask, inv_value, inv_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				inv_value,
				inv_mask,
				sum_value,
				sum_mask);
			net_value[macro->outputs[2]] = sum_value;
			net_mask[macro->outputs[2]] = sum_mask;

			LXS_EVAL_NOT(sum_value, sum_mask, inv_value, inv_mask);
			LXS_EVAL_AND(
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				inv_value,
				inv_mask,
				sum_value,
				sum_mask);
			net_value[macro->outputs[3]] = sum_value;
			net_mask[macro->outputs[3]] = sum_mask;

			LXS_EVAL_NOT(sum_value, sum_mask, sum_value, sum_mask);
			net_value[macro->outputs[4]] = sum_value;
			net_mask[macro->outputs[4]] = sum_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_FULL_ADDER_CINV)
			{
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				and_value,
				and_mask);
			LXS_EVAL_NOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				nor_value,
				nor_mask);
			LXS_EVAL_NOR(nor_value, nor_mask, and_value, and_mask, xor_value, xor_mask);
			LXS_EVAL_XNOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				xor_value,
				xor_mask,
				sum_value,
				sum_mask);
			LXS_EVAL_NOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				nor_value,
				nor_mask,
				inner_value,
				inner_mask);
			LXS_EVAL_NOR(
				and_value,
				and_mask,
				inner_value,
				inner_mask,
				carry_value,
				carry_mask);

			net_value[macro->outputs[0]] = sum_value;
			net_mask[macro->outputs[0]] = sum_mask;
			net_value[macro->outputs[1]] = carry_value;
			net_mask[macro->outputs[1]] = carry_mask;
			}
		else if (macro->type == LXS_MULTI_MACRO_RIPPLE_SLICE2_CINV)
			{
			LXS_EVAL_AND(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				and0_value,
				and0_mask);
			LXS_EVAL_NOR(
				net_value[macro->inputs[0]],
				net_mask[macro->inputs[0]],
				net_value[macro->inputs[1]],
				net_mask[macro->inputs[1]],
				nor0_value,
				nor0_mask);
			LXS_EVAL_NOR(nor0_value, nor0_mask, and0_value, and0_mask, xor0_value, xor0_mask);
			LXS_EVAL_XNOR(
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				xor0_value,
				xor0_mask,
				sum0_value,
				sum0_mask);
			LXS_EVAL_NOR(
				net_value[macro->inputs[4]],
				net_mask[macro->inputs[4]],
				nor0_value,
				nor0_mask,
				inner0_value,
				inner0_mask);
			LXS_EVAL_NOR(
				and0_value,
				and0_mask,
				inner0_value,
				inner0_mask,
				carry0_value,
				carry0_mask);

			LXS_EVAL_AND(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				and1_value,
				and1_mask);
			LXS_EVAL_NOR(
				net_value[macro->inputs[2]],
				net_mask[macro->inputs[2]],
				net_value[macro->inputs[3]],
				net_mask[macro->inputs[3]],
				nor1_value,
				nor1_mask);
			LXS_EVAL_NOR(nor1_value, nor1_mask, and1_value, and1_mask, xor1_value, xor1_mask);
			LXS_EVAL_XNOR(carry0_value, carry0_mask, xor1_value, xor1_mask, sum1_value, sum1_mask);
			LXS_EVAL_NOR(carry0_value, carry0_mask, nor1_value, nor1_mask, inner1_value, inner1_mask);
			LXS_EVAL_NOR(and1_value, and1_mask, inner1_value, inner1_mask, carry1_value, carry1_mask);

			net_value[macro->outputs[0]] = sum0_value;
			net_mask[macro->outputs[0]] = sum0_mask;
			net_value[macro->outputs[1]] = sum1_value;
			net_mask[macro->outputs[1]] = sum1_mask;
			net_value[macro->outputs[2]] = carry1_value;
			net_mask[macro->outputs[2]] = carry1_mask;
			}
		ctx->probes.chunk_exec++;
		ctx->probes.gate_eval += macro->gate_equiv_count;
		}
	}

static void lxs_execute_standard_muxes(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const lxs_standard_mux_plan *muxes,
	uint32_t count)
	{
	uint64_t *restrict net_value = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_value);
	uint64_t *restrict net_mask = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_mask);

	for (uint32_t i = 0; i < count; ++i)
		{
		const lxs_standard_mux_plan *mux = &muxes[i];
		uint64_t s0_value;
		uint64_t s0_mask;
		uint64_t ns0_value;
		uint64_t ns0_mask;
		uint64_t s1_value = 0ULL;
		uint64_t s1_mask = 0ULL;
		uint64_t ns1_value = 0ULL;
		uint64_t ns1_mask = 0ULL;

		s0_value = net_value[plan->standard_mux_select_net_ids[mux->select_start]];
		s0_mask = net_mask[plan->standard_mux_select_net_ids[mux->select_start]];
		LXS_EVAL_NOT(s0_value, s0_mask, ns0_value, ns0_mask);
		if (mux->kind == LXS_STANDARD_MUX_KIND_4)
			{
			s1_value = net_value[plan->standard_mux_select_net_ids[mux->select_start + 1U]];
			s1_mask = net_mask[plan->standard_mux_select_net_ids[mux->select_start + 1U]];
			LXS_EVAL_NOT(s1_value, s1_mask, ns1_value, ns1_mask);
			}

		for (uint32_t bit = 0; bit < mux->width_bits; ++bit)
			{
			uint64_t lo_value;
			uint64_t lo_mask;
			uint64_t hi_value;
			uint64_t hi_mask;
			uint64_t out_value;
			uint64_t out_mask;
			uint32_t out_net = plan->standard_mux_output_net_ids[mux->output_start + bit];

			if (mux->kind == LXS_STANDARD_MUX_KIND_2)
				{
				uint32_t d0_net = plan->standard_mux_data_net_ids[mux->data_start + bit];
				uint32_t d1_net = plan->standard_mux_data_net_ids[mux->data_start + mux->width_bits + bit];

				LXS_EVAL_AND(
					net_value[d0_net],
					net_mask[d0_net],
					ns0_value,
					ns0_mask,
					lo_value,
					lo_mask);
				LXS_EVAL_AND(
					net_value[d1_net],
					net_mask[d1_net],
					s0_value,
					s0_mask,
					hi_value,
					hi_mask);
				LXS_EVAL_OR(lo_value, lo_mask, hi_value, hi_mask, out_value, out_mask);
				}
			else
				{
				uint64_t low_mux_value;
				uint64_t low_mux_mask;
				uint64_t high_mux_value;
				uint64_t high_mux_mask;
				uint32_t d0_net = plan->standard_mux_data_net_ids[mux->data_start + bit];
				uint32_t d1_net = plan->standard_mux_data_net_ids[mux->data_start + mux->width_bits + bit];
				uint32_t d2_net = plan->standard_mux_data_net_ids[mux->data_start + (mux->width_bits * 2U) + bit];
				uint32_t d3_net = plan->standard_mux_data_net_ids[mux->data_start + (mux->width_bits * 3U) + bit];

				LXS_EVAL_AND(
					net_value[d0_net],
					net_mask[d0_net],
					ns0_value,
					ns0_mask,
					lo_value,
					lo_mask);
				LXS_EVAL_AND(
					net_value[d1_net],
					net_mask[d1_net],
					s0_value,
					s0_mask,
					hi_value,
					hi_mask);
				LXS_EVAL_OR(lo_value, lo_mask, hi_value, hi_mask, low_mux_value, low_mux_mask);

				LXS_EVAL_AND(
					net_value[d2_net],
					net_mask[d2_net],
					ns0_value,
					ns0_mask,
					lo_value,
					lo_mask);
				LXS_EVAL_AND(
					net_value[d3_net],
					net_mask[d3_net],
					s0_value,
					s0_mask,
					hi_value,
					hi_mask);
				LXS_EVAL_OR(lo_value, lo_mask, hi_value, hi_mask, high_mux_value, high_mux_mask);

				LXS_EVAL_AND(
					low_mux_value,
					low_mux_mask,
					ns1_value,
					ns1_mask,
					lo_value,
					lo_mask);
				LXS_EVAL_AND(
					high_mux_value,
					high_mux_mask,
					s1_value,
					s1_mask,
					hi_value,
					hi_mask);
				LXS_EVAL_OR(lo_value, lo_mask, hi_value, hi_mask, out_value, out_mask);
				}

			net_value[out_net] = out_value;
			net_mask[out_net] = out_mask;
			}

		ctx->probes.chunk_exec++;
		ctx->probes.gate_eval += mux->gate_equiv_count;
		}
	}

static void lxs_eval_functional_op(
	uint8_t type,
	const uint64_t *src_value,
	const uint64_t *src_mask,
	uint32_t arity,
	uint64_t *out_value,
	uint64_t *out_mask)
	{
	uint64_t value = src_value[0];
	uint64_t mask = src_mask[0];
	switch (type)
		{
		case LXS_FUNCTIONAL_REGION_OP_BUF:
			LXS_EVAL_BUF(src_value[0], src_mask[0], *out_value, *out_mask);
			break;
		case LXS_FUNCTIONAL_REGION_OP_NOT:
			LXS_EVAL_NOT(src_value[0], src_mask[0], *out_value, *out_mask);
			break;
		case LXS_FUNCTIONAL_REGION_OP_AND:
			for (uint32_t i = 1; i < arity; ++i)
				{
				LXS_EVAL_AND(value, mask, src_value[i], src_mask[i], value, mask);
				}
			*out_value = value;
			*out_mask = mask;
			break;
		case LXS_FUNCTIONAL_REGION_OP_OR:
			for (uint32_t i = 1; i < arity; ++i)
				{
				LXS_EVAL_OR(value, mask, src_value[i], src_mask[i], value, mask);
				}
			*out_value = value;
			*out_mask = mask;
			break;
		case LXS_FUNCTIONAL_REGION_OP_XOR:
			for (uint32_t i = 1; i < arity; ++i)
				{
				LXS_EVAL_XOR(value, mask, src_value[i], src_mask[i], value, mask);
				}
			*out_value = value;
			*out_mask = mask;
			break;
		case LXS_FUNCTIONAL_REGION_OP_NAND:
			for (uint32_t i = 1; i < arity; ++i)
				{
				LXS_EVAL_AND(value, mask, src_value[i], src_mask[i], value, mask);
				}
			*out_value = value;
			*out_mask = mask;
			LXS_EVAL_NOT(*out_value, *out_mask, *out_value, *out_mask);
			break;
		case LXS_FUNCTIONAL_REGION_OP_NOR:
			for (uint32_t i = 1; i < arity; ++i)
				{
				LXS_EVAL_OR(value, mask, src_value[i], src_mask[i], value, mask);
				}
			*out_value = value;
			*out_mask = mask;
			LXS_EVAL_NOT(*out_value, *out_mask, *out_value, *out_mask);
			break;
		case LXS_FUNCTIONAL_REGION_OP_MAJ3:
			{
			uint64_t ab_value;
			uint64_t ab_mask;
			uint64_t ac_value;
			uint64_t ac_mask;
			uint64_t bc_value;
			uint64_t bc_mask;

			LXS_EVAL_AND(src_value[0], src_mask[0], src_value[1], src_mask[1], ab_value, ab_mask);
			LXS_EVAL_AND(src_value[0], src_mask[0], src_value[2], src_mask[2], ac_value, ac_mask);
			LXS_EVAL_AND(src_value[1], src_mask[1], src_value[2], src_mask[2], bc_value, bc_mask);
			LXS_EVAL_OR(ab_value, ab_mask, ac_value, ac_mask, value, mask);
			LXS_EVAL_OR(value, mask, bc_value, bc_mask, *out_value, *out_mask);
			break;
			}
		case LXS_FUNCTIONAL_REGION_OP_HA2:
			LXS_EVAL_XOR(src_value[0], src_mask[0], src_value[1], src_mask[1], *out_value, *out_mask);
			break;
		case LXS_FUNCTIONAL_REGION_OP_FA3:
			for (uint32_t i = 1; i < arity; ++i)
				{
				LXS_EVAL_XOR(value, mask, src_value[i], src_mask[i], value, mask);
				}
			*out_value = value;
			*out_mask = mask;
			break;
		default:
			for (uint32_t i = 1; i < arity; ++i)
				{
				LXS_EVAL_XOR(value, mask, src_value[i], src_mask[i], value, mask);
				}
			*out_value = value;
			*out_mask = mask;
			LXS_EVAL_NOT(*out_value, *out_mask, *out_value, *out_mask);
			break;
		}
	}

static void lxs_eval_functional_expr(
	const lxs_functional_region_plan *region,
	uint32_t op_index,
	const uint64_t *net_value,
	const uint64_t *net_mask,
	uint64_t *memo_value,
	uint64_t *memo_mask,
	uint8_t *memo_valid,
	uint64_t *out_value,
	uint64_t *out_mask)
	{
	const lxs_functional_region_op *op = &region->ops[op_index];
	uint64_t src_value[4] = { 0 };
	uint64_t src_mask[4] = { 0 };

	for (uint32_t src_index = 0; src_index < op->arity; ++src_index)
		{
		if (op->src[src_index] < region->input_count)
			{
			uint32_t net_id = region->inputs[op->src[src_index]];
			src_value[src_index] = net_value[net_id];
			src_mask[src_index] = net_mask[net_id];
			}
		else
			{
			uint32_t temp_index = op->src[src_index] - region->input_count;
			if (!memo_valid[temp_index])
				{
				for (uint32_t i = 0; i < region->op_count; ++i)
					{
					for (uint32_t dst_index = 0; dst_index < region->ops[i].dst_count; ++dst_index)
						{
						if (region->ops[i].dst_kind[dst_index] == LXS_FUNCTIONAL_REGION_DST_TEMP &&
							region->ops[i].dst[dst_index] == temp_index)
							{
							lxs_eval_functional_expr(
								region,
								i,
								net_value,
								net_mask,
								memo_value,
								memo_mask,
								memo_valid,
								&memo_value[temp_index],
								&memo_mask[temp_index]);
							memo_valid[temp_index] = 1U;
							break;
							}
						}
					if (memo_valid[temp_index])
						{
						break;
						}
					}
				}
			src_value[src_index] = memo_value[temp_index];
			src_mask[src_index] = memo_mask[temp_index];
			}
		}

	lxs_eval_functional_op(
		op->type,
		src_value,
		src_mask,
		op->arity,
		out_value,
		out_mask);
	}

static void lxs_execute_functional_regions(
	lxs_engine_ctx *ctx,
	const lxs_functional_region_plan *regions,
	uint32_t count)
	{
	uint64_t *restrict net_value = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_value);
	uint64_t *restrict net_mask = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_mask);

	for (uint32_t i = 0; i < count; ++i)
		{
		const lxs_functional_region_plan *region = &regions[i];
		uint64_t out_value[8] = { 0 };
		uint64_t out_mask[8] = { 0 };
		if (region->exec_kind == LXS_FUNCTIONAL_REGION_EXEC_EXPR)
			{
			uint64_t memo_value[LXS_FUNCTIONAL_REGION_MAX_TEMPS] = { 0 };
			uint64_t memo_mask[LXS_FUNCTIONAL_REGION_MAX_TEMPS] = { 0 };
			uint8_t memo_valid[LXS_FUNCTIONAL_REGION_MAX_TEMPS] = { 0 };
			uint32_t root_index = region->op_count > 0U ? (region->op_count - 1U) : 0U;
			uint64_t expr_value = 0ULL;
			uint64_t expr_mask = 0ULL;
			lxs_eval_functional_expr(
				region,
				root_index,
				net_value,
				net_mask,
				memo_value,
				memo_mask,
				memo_valid,
				&expr_value,
				&expr_mask);
			out_value[0] = expr_value;
			out_mask[0] = expr_mask;
			}
		else
			{
			uint64_t temp_value[LXS_FUNCTIONAL_REGION_MAX_TEMPS] = { 0 };
			uint64_t temp_mask[LXS_FUNCTIONAL_REGION_MAX_TEMPS] = { 0 };

			for (uint32_t op_index = 0; op_index < region->op_count; ++op_index)
				{
				const lxs_functional_region_op *op = &region->ops[op_index];
				uint64_t src_value[4] = { 0 };
				uint64_t src_mask[4] = { 0 };
				uint64_t next_value[2] = { 0ULL, 0ULL };
				uint64_t next_mask[2] = { 0ULL, 0ULL };

				for (uint32_t src_index = 0; src_index < op->arity; ++src_index)
					{
					if (op->src[src_index] < region->input_count)
						{
						uint32_t net_id = region->inputs[op->src[src_index]];
						src_value[src_index] = net_value[net_id];
						src_mask[src_index] = net_mask[net_id];
						}
					else
						{
						uint32_t temp_index = op->src[src_index] - region->input_count;
						src_value[src_index] = temp_value[temp_index];
						src_mask[src_index] = temp_mask[temp_index];
						}
					}

				if (op->type == LXS_FUNCTIONAL_REGION_OP_HA2)
					{
					LXS_EVAL_XOR(src_value[0], src_mask[0], src_value[1], src_mask[1], next_value[0], next_mask[0]);
					LXS_EVAL_AND(src_value[0], src_mask[0], src_value[1], src_mask[1], next_value[1], next_mask[1]);
					}
				else if (op->type == LXS_FUNCTIONAL_REGION_OP_FA3)
					{
					uint64_t ab_value;
					uint64_t ab_mask;
					uint64_t ac_value;
					uint64_t ac_mask;
					uint64_t bc_value;
					uint64_t bc_mask;
					uint64_t maj_value;
					uint64_t maj_mask;
					lxs_eval_functional_op(
						LXS_FUNCTIONAL_REGION_OP_FA3,
						src_value,
						src_mask,
						op->arity,
						&next_value[0],
						&next_mask[0]);
					LXS_EVAL_AND(src_value[0], src_mask[0], src_value[1], src_mask[1], ab_value, ab_mask);
					LXS_EVAL_AND(src_value[0], src_mask[0], src_value[2], src_mask[2], ac_value, ac_mask);
					LXS_EVAL_AND(src_value[1], src_mask[1], src_value[2], src_mask[2], bc_value, bc_mask);
					LXS_EVAL_OR(ab_value, ab_mask, ac_value, ac_mask, maj_value, maj_mask);
					LXS_EVAL_OR(maj_value, maj_mask, bc_value, bc_mask, next_value[1], next_mask[1]);
					}
				else
					{
					lxs_eval_functional_op(
						op->type,
						src_value,
						src_mask,
						op->arity,
						&next_value[0],
						&next_mask[0]);
					}

				for (uint32_t dst_index = 0; dst_index < op->dst_count; ++dst_index)
					{
					if (op->dst_kind[dst_index] == LXS_FUNCTIONAL_REGION_DST_OUTPUT)
						{
						out_value[op->dst[dst_index]] = next_value[dst_index];
						out_mask[op->dst[dst_index]] = next_mask[dst_index];
						}
					else
						{
						temp_value[op->dst[dst_index]] = next_value[dst_index];
						temp_mask[op->dst[dst_index]] = next_mask[dst_index];
						}
					}
				}
			}

		for (uint32_t output_index = 0; output_index < region->output_count; ++output_index)
			{
			net_value[region->outputs[output_index]] = out_value[output_index];
			net_mask[region->outputs[output_index]] = out_mask[output_index];
			}
		ctx->probes.chunk_exec++;
		ctx->probes.gate_eval += region->gate_equiv_count;
		}
	}

static void lxs_execute_chunk(
	lxs_engine_ctx *ctx,
	const lxs_chunk_plan *chunk,
	const lxs_gate_ir *gates)
	{
	ctx->probes.chunk_exec++;
	ctx->probes.gate_eval += chunk->gate_equiv_count;

	if (chunk->type == LXS_GATE_NOT || chunk->type == LXS_GATE_BUF)
		{
		lxs_execute_unary_chunk(ctx, gates, chunk->count, chunk->type);
		}
	else
		{
		lxs_execute_logic_chunk(ctx, gates, chunk->count, chunk->type);
		}

	}

int lxs_init_engine(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	memset(ctx, 0, sizeof(*ctx));

	ctx->net_value = lxs_calloc_aligned(plan->net_count, sizeof(uint64_t));
	ctx->net_mask = lxs_calloc_aligned(plan->net_count, sizeof(uint64_t));
	ctx->next_state_value = lxs_calloc_aligned(plan->state.count, sizeof(uint64_t));
	ctx->next_state_mask = lxs_calloc_aligned(plan->state.count, sizeof(uint64_t));
	ctx->output_value = lxs_calloc_aligned(plan->outputs.count, sizeof(uint64_t));
	ctx->output_mask = lxs_calloc_aligned(plan->outputs.count, sizeof(uint64_t));
	ctx->register_value = lxs_calloc_aligned(plan->register_bit_count, sizeof(uint64_t));
	ctx->register_mask = lxs_calloc_aligned(plan->register_bit_count, sizeof(uint64_t));
	ctx->next_register_value = lxs_calloc_aligned(plan->register_bit_count, sizeof(uint64_t));
	ctx->next_register_mask = lxs_calloc_aligned(plan->register_bit_count, sizeof(uint64_t));
	ctx->ram_value = lxs_calloc_aligned(plan->ram_storage_bit_count, sizeof(uint64_t));
	ctx->ram_mask = lxs_calloc_aligned(plan->ram_storage_bit_count, sizeof(uint64_t));
	ctx->ram_stage_value = lxs_calloc_aligned(plan->ram_stage_bit_count, sizeof(uint64_t));
	ctx->ram_stage_mask = lxs_calloc_aligned(plan->ram_stage_bit_count, sizeof(uint64_t));
	ctx->ram_pending_addr = lxs_calloc_aligned(plan->ram_count, sizeof(uint32_t));
	ctx->ram_pending_write = lxs_calloc_aligned(plan->ram_count, sizeof(uint8_t));

#if LXS_TEST_PROBES
	ctx->input_shadow_value = lxs_calloc_aligned(plan->inputs.count, sizeof(uint64_t));
	ctx->input_shadow_mask = lxs_calloc_aligned(plan->inputs.count, sizeof(uint64_t));
#endif

	if ((plan->net_count && (!ctx->net_value || !ctx->net_mask)) ||
		(plan->state.count && (!ctx->next_state_value || !ctx->next_state_mask)) ||
		(plan->outputs.count && (!ctx->output_value || !ctx->output_mask)) ||
		(plan->register_bit_count &&
			(!ctx->register_value || !ctx->register_mask ||
			!ctx->next_register_value || !ctx->next_register_mask)) ||
		(plan->ram_storage_bit_count && (!ctx->ram_value || !ctx->ram_mask)) ||
		(plan->ram_stage_bit_count && (!ctx->ram_stage_value || !ctx->ram_stage_mask)) ||
		(plan->ram_count && (!ctx->ram_pending_addr || !ctx->ram_pending_write))
#if LXS_TEST_PROBES
		|| (plan->inputs.count && (!ctx->input_shadow_value || !ctx->input_shadow_mask))
#endif
		)
		{
		lxs_free_engine(ctx);
		return 0;
		}

	lxs_reset_engine(ctx, plan);
	return 1;
	}

void lxs_reset_engine(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	if (plan->net_count > 0U)
		{
		memset(ctx->net_value, 0, (size_t)plan->net_count * sizeof(uint64_t));
		memset(ctx->net_mask, 0, (size_t)plan->net_count * sizeof(uint64_t));
		}

	if (plan->state.count > 0U)
		{
		memset(ctx->next_state_value, 0, (size_t)plan->state.count * sizeof(uint64_t));
		memset(ctx->next_state_mask, 0, (size_t)plan->state.count * sizeof(uint64_t));
		}

	if (plan->outputs.count > 0U)
		{
		memset(ctx->output_value, 0, (size_t)plan->outputs.count * sizeof(uint64_t));
		memset(ctx->output_mask, 0, (size_t)plan->outputs.count * sizeof(uint64_t));
		}

	if (plan->register_bit_count > 0U)
		{
		memcpy(
			ctx->register_value,
			plan->register_init_value,
			(size_t)plan->register_bit_count * sizeof(uint64_t));
		memcpy(
			ctx->register_mask,
			plan->register_init_mask,
			(size_t)plan->register_bit_count * sizeof(uint64_t));
		memset(
			ctx->next_register_value,
			0,
			(size_t)plan->register_bit_count * sizeof(uint64_t));
		memset(
			ctx->next_register_mask,
			0,
			(size_t)plan->register_bit_count * sizeof(uint64_t));
		}

	if (plan->ram_storage_bit_count > 0U)
		{
		memcpy(
			ctx->ram_value,
			plan->ram_init_value,
			(size_t)plan->ram_storage_bit_count * sizeof(uint64_t));
		memcpy(
			ctx->ram_mask,
			plan->ram_init_mask,
			(size_t)plan->ram_storage_bit_count * sizeof(uint64_t));
		}

	if (plan->ram_stage_bit_count > 0U)
		{
		memset(
			ctx->ram_stage_value,
			0,
			(size_t)plan->ram_stage_bit_count * sizeof(uint64_t));
		memset(
			ctx->ram_stage_mask,
			0,
			(size_t)plan->ram_stage_bit_count * sizeof(uint64_t));
		}

	if (plan->ram_count > 0U)
		{
		memset(ctx->ram_pending_addr, 0, (size_t)plan->ram_count * sizeof(uint32_t));
		memset(ctx->ram_pending_write, 0, (size_t)plan->ram_count * sizeof(uint8_t));
		}

#if LXS_TEST_PROBES
	if (plan->inputs.count > 0U)
		{
		memset(ctx->input_shadow_value, 0, (size_t)plan->inputs.count * sizeof(uint64_t));
		memset(ctx->input_shadow_mask, 0, (size_t)plan->inputs.count * sizeof(uint64_t));
		}
#endif

	memset(&ctx->probes, 0, sizeof(ctx->probes));
	lxs_materialize_register_outputs(ctx, plan);
	}

void lxs_free_engine(lxs_engine_ctx *ctx)
	{
	lxs_free_aligned(ctx->net_value);
	lxs_free_aligned(ctx->net_mask);
	lxs_free_aligned(ctx->next_state_value);
	lxs_free_aligned(ctx->next_state_mask);
	lxs_free_aligned(ctx->output_value);
	lxs_free_aligned(ctx->output_mask);
	lxs_free_aligned(ctx->register_value);
	lxs_free_aligned(ctx->register_mask);
	lxs_free_aligned(ctx->next_register_value);
	lxs_free_aligned(ctx->next_register_mask);
	lxs_free_aligned(ctx->ram_value);
	lxs_free_aligned(ctx->ram_mask);
	lxs_free_aligned(ctx->ram_stage_value);
	lxs_free_aligned(ctx->ram_stage_mask);
	lxs_free_aligned(ctx->ram_pending_addr);
	lxs_free_aligned(ctx->ram_pending_write);

#if LXS_TEST_PROBES
	lxs_free_aligned(ctx->input_shadow_value);
	lxs_free_aligned(ctx->input_shadow_mask);
#endif

	memset(ctx, 0, sizeof(*ctx));
	}

void lxs_apply_inputs(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const uint64_t *values,
	const uint64_t *masks)
	{
#if !LXS_TEST_PROBES
	if (plan->inputs.is_contiguous && plan->inputs.count > 0U)
		{
		if (values)
			{
			memcpy(
				ctx->net_value + plan->inputs.contiguous_base,
				values,
				(size_t)plan->inputs.count * sizeof(uint64_t));
			}
		else
			{
			memset(
				ctx->net_value + plan->inputs.contiguous_base,
				0,
				(size_t)plan->inputs.count * sizeof(uint64_t));
			}

		if (masks)
			{
			memcpy(
				ctx->net_mask + plan->inputs.contiguous_base,
				masks,
				(size_t)plan->inputs.count * sizeof(uint64_t));
			}
		else
			{
			memset(
				ctx->net_mask + plan->inputs.contiguous_base,
				0,
				(size_t)plan->inputs.count * sizeof(uint64_t));
			}

		ctx->probes.input_apply += plan->inputs.count;
		return;
		}
#endif

	for (uint32_t i = 0; i < plan->inputs.count; ++i)
		{
		uint32_t net_id = plan->inputs.net_ids[i];
		uint64_t next_value = values ? values[i] : 0ULL;
		uint64_t next_mask = masks ? masks[i] : 0ULL;

#if LXS_TEST_PROBES
		if (ctx->input_shadow_value[i] != next_value || ctx->input_shadow_mask[i] != next_mask)
			{
			ctx->probes.input_toggle++;
			}
		ctx->input_shadow_value[i] = next_value;
		ctx->input_shadow_mask[i] = next_mask;
		if (next_mask != 0ULL)
			{
			ctx->probes.unknown_state_materialize_count++;
			}
#endif

		ctx->net_value[net_id] = next_value;
		ctx->net_mask[net_id] = next_mask;
		}

	ctx->probes.input_apply += plan->inputs.count;
	}

void lxs_begin_tick(lxs_engine_ctx *ctx)
	{
	ctx->probes.tick_count++;
	}

static void lxs_execute_level_chunks(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const lxs_level_plan *level_plan)
	{
	for (uint32_t i = 0; i < level_plan->chunk_count; ++i)
		{
		const lxs_chunk_plan *chunk = &plan->chunks[level_plan->chunk_start + i];
		lxs_execute_chunk(ctx, chunk, plan->comb_gates + chunk->start);
		}
	}

static uint8_t lxs_chunk_is_tiny(const lxs_chunk_plan *chunk)
	{
	return (uint8_t)(chunk->count <= 2U);
	}

static void lxs_execute_level_chunks_by_size(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const lxs_level_plan *level_plan,
	uint8_t execute_tiny)
	{
	for (uint32_t i = 0; i < level_plan->chunk_count; ++i)
		{
		const lxs_chunk_plan *chunk = &plan->chunks[level_plan->chunk_start + i];
		if (lxs_chunk_is_tiny(chunk) != execute_tiny)
			{
			continue;
			}
		lxs_execute_chunk(ctx, chunk, plan->comb_gates + chunk->start);
		}
	}

static void lxs_execute_level_macros_only(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const lxs_level_plan *level_plan)
	{
	if (level_plan->macro_count > 0U)
		{
		lxs_execute_macros(
			ctx,
			plan->macros + level_plan->macro_start,
			level_plan->macro_count);
		}
	}

static void lxs_execute_level_multi_macros_only(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const lxs_level_plan *level_plan)
	{
	if (level_plan->multi_macro_count > 0U)
		{
		lxs_execute_multi_macros(
			ctx,
			plan->multi_macros + level_plan->multi_macro_start,
			level_plan->multi_macro_count);
		}
	}

static void lxs_execute_level_standard_muxes_only(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const lxs_level_plan *level_plan)
	{
	if (level_plan->standard_mux_count > 0U)
		{
		lxs_execute_standard_muxes(
			ctx,
			plan,
			plan->standard_muxes + level_plan->standard_mux_start,
			level_plan->standard_mux_count);
		}
	}

static void lxs_execute_level_functional_regions_only(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const lxs_level_plan *level_plan)
	{
	if (level_plan->functional_region_count > 0U)
		{
		lxs_execute_functional_regions(
			ctx,
			plan->functional_regions + level_plan->functional_region_start,
			level_plan->functional_region_count);
		}
	}

static uint8_t lxs_level_is_mixed_boundary(const lxs_level_plan *level_plan)
	{
	return (uint8_t)(
		level_plan->chunk_count > 0U &&
		(level_plan->macro_count > 0U ||
		 level_plan->multi_macro_count > 0U ||
		 level_plan->standard_mux_count > 0U ||
		 level_plan->functional_region_count > 0U));
	}

void lxs_execute_levels(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	for (uint32_t level = 0; level < plan->level_count; ++level)
		{
		const lxs_level_plan *level_plan = &plan->levels[level];
		if (lxs_level_is_mixed_boundary(level_plan))
			{
#if LXS_MIXED_LEVEL_EXEC_MODE == 1
			lxs_execute_level_macros_only(ctx, plan, level_plan);
			lxs_execute_level_multi_macros_only(ctx, plan, level_plan);
			lxs_execute_level_standard_muxes_only(ctx, plan, level_plan);
			lxs_execute_level_functional_regions_only(ctx, plan, level_plan);
			lxs_execute_level_chunks(ctx, plan, level_plan);
#elif LXS_MIXED_LEVEL_EXEC_MODE == 2
			lxs_execute_level_chunks_by_size(ctx, plan, level_plan, 0U);
			lxs_execute_level_macros_only(ctx, plan, level_plan);
			lxs_execute_level_multi_macros_only(ctx, plan, level_plan);
			lxs_execute_level_standard_muxes_only(ctx, plan, level_plan);
			lxs_execute_level_functional_regions_only(ctx, plan, level_plan);
			lxs_execute_level_chunks_by_size(ctx, plan, level_plan, 1U);
#elif LXS_MIXED_LEVEL_EXEC_MODE == 3
			lxs_execute_level_multi_macros_only(ctx, plan, level_plan);
			lxs_execute_level_macros_only(ctx, plan, level_plan);
			lxs_execute_level_standard_muxes_only(ctx, plan, level_plan);
			lxs_execute_level_functional_regions_only(ctx, plan, level_plan);
			lxs_execute_level_chunks_by_size(ctx, plan, level_plan, 0U);
			lxs_execute_level_chunks_by_size(ctx, plan, level_plan, 1U);
#else
			lxs_execute_level_chunks(ctx, plan, level_plan);
			lxs_execute_level_macros_only(ctx, plan, level_plan);
			lxs_execute_level_multi_macros_only(ctx, plan, level_plan);
			lxs_execute_level_standard_muxes_only(ctx, plan, level_plan);
			lxs_execute_level_functional_regions_only(ctx, plan, level_plan);
#endif
			continue;
			}

		lxs_execute_level_chunks(ctx, plan, level_plan);
		lxs_execute_level_macros_only(ctx, plan, level_plan);
		lxs_execute_level_multi_macros_only(ctx, plan, level_plan);
		lxs_execute_level_standard_muxes_only(ctx, plan, level_plan);
		lxs_execute_level_functional_regions_only(ctx, plan, level_plan);
		}
	}

void lxs_capture_outputs(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
#if !LXS_TEST_PROBES
	if (plan->outputs.is_contiguous && plan->outputs.count > 0U)
		{
		memcpy(
			ctx->output_value,
			ctx->net_value + plan->outputs.contiguous_base,
			(size_t)plan->outputs.count * sizeof(uint64_t));
		memcpy(
			ctx->output_mask,
			ctx->net_mask + plan->outputs.contiguous_base,
			(size_t)plan->outputs.count * sizeof(uint64_t));
		return;
		}
#endif

	for (uint32_t i = 0; i < plan->outputs.count; ++i)
		{
		uint32_t net_id = plan->outputs.net_ids[i];
		ctx->output_value[i] = ctx->net_value[net_id];
		ctx->output_mask[i] = ctx->net_mask[net_id];
		}
	}

void lxs_capture_next_state(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
#if !LXS_TEST_PROBES
	if (plan->state.d_is_contiguous && plan->state.count > 0U)
		{
		memcpy(
			ctx->next_state_value,
			ctx->net_value + plan->state.d_contiguous_base,
			(size_t)plan->state.count * sizeof(uint64_t));
		memcpy(
			ctx->next_state_mask,
			ctx->net_mask + plan->state.d_contiguous_base,
			(size_t)plan->state.count * sizeof(uint64_t));
		ctx->probes.dff_exec += plan->state.count;
		return;
		}
#endif

	for (uint32_t i = 0; i < plan->state.count; ++i)
		{
		uint32_t d_input = plan->state.d_inputs[i];
		LXS_EVAL_DFF(
			ctx->net_value[d_input],
			ctx->net_mask[d_input],
			ctx->next_state_value[i],
			ctx->next_state_mask[i]);
		}

	ctx->probes.dff_exec += plan->state.count;
	}

void lxs_commit_state(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
#if !LXS_TEST_PROBES
	if (plan->state.q_is_contiguous && plan->state.count > 0U)
		{
		memcpy(
			ctx->net_value + plan->state.q_contiguous_base,
			ctx->next_state_value,
			(size_t)plan->state.count * sizeof(uint64_t));
		memcpy(
			ctx->net_mask + plan->state.q_contiguous_base,
			ctx->next_state_mask,
			(size_t)plan->state.count * sizeof(uint64_t));
		ctx->probes.state_commit_count += plan->state.count;
		return;
		}
#endif

	for (uint32_t i = 0; i < plan->state.count; ++i)
		{
		uint32_t q_output = plan->state.q_outputs[i];

#if LXS_TEST_PROBES
		if (ctx->net_value[q_output] != ctx->next_state_value[i] ||
			ctx->net_mask[q_output] != ctx->next_state_mask[i])
			{
			ctx->probes.state_change_commit++;
			}
#endif

		ctx->net_value[q_output] = ctx->next_state_value[i];
		ctx->net_mask[q_output] = ctx->next_state_mask[i];
		}

	ctx->probes.state_commit_count += plan->state.count;
	}

void lxs_execute_plan(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	lxs_begin_tick(ctx);
	lxs_execute_roms(ctx, plan);
	lxs_execute_rams(ctx, plan);
	lxs_execute_levels(ctx, plan);
	lxs_capture_outputs(ctx, plan);
	lxs_capture_next_state(ctx, plan);
	lxs_capture_next_registers(ctx, plan);
	lxs_capture_rams(ctx, plan);
	lxs_commit_state(ctx, plan);
	lxs_commit_registers(ctx, plan);
	lxs_commit_rams(ctx, plan);
	}

void lxs_read_outputs(
	const lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	uint64_t *values,
	uint64_t *masks)
	{
	if (values && plan->outputs.count > 0U)
		{
		memcpy(values, ctx->output_value, (size_t)plan->outputs.count * sizeof(uint64_t));
		}

	if (masks && plan->outputs.count > 0U)
		{
		memcpy(masks, ctx->output_mask, (size_t)plan->outputs.count * sizeof(uint64_t));
		}
	}

lxs_probes lxs_get_probes(const lxs_engine_ctx *ctx)
	{
	return ctx->probes;
	}
