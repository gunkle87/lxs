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

static void lxs_execute_binary_chunk(
	lxs_engine_ctx *ctx,
	const lxs_gate_ir *gates,
	uint32_t count,
	uint32_t type)
	{
	uint64_t *restrict net_value = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_value);
	uint64_t *restrict net_mask = (uint64_t*)LXS_ASSUME_ALIGNED_64(ctx->net_mask);
	const lxs_gate_ir *gate = gates;
	const lxs_gate_ir *gate_end = gates + count;

	switch (type)
		{
		case LXS_GATE_AND:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;

				LXS_EVAL_AND(
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
		case LXS_GATE_OR:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;

				LXS_EVAL_OR(
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
		case LXS_GATE_XOR:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;

				LXS_EVAL_XOR(
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

				LXS_EVAL_NAND(
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
		case LXS_GATE_NOR:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;

				LXS_EVAL_NOR(
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
		case LXS_GATE_XNOR:
			for (; gate != gate_end; ++gate)
				{
				uint64_t out_value;
				uint64_t out_mask;

				LXS_EVAL_XNOR(
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

static void lxs_execute_chunk(
	lxs_engine_ctx *ctx,
	const lxs_chunk_plan *chunk,
	const lxs_gate_ir *gates)
	{
	ctx->probes.chunk_exec++;
	ctx->probes.gate_eval += chunk->count;

	if (chunk->type == LXS_GATE_NOT || chunk->type == LXS_GATE_BUF)
		{
		lxs_execute_unary_chunk(ctx, gates, chunk->count, chunk->type);
		}
	else
		{
		lxs_execute_binary_chunk(ctx, gates, chunk->count, chunk->type);
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

#if LXS_TEST_PROBES
	ctx->input_shadow_value = lxs_calloc_aligned(plan->inputs.count, sizeof(uint64_t));
	ctx->input_shadow_mask = lxs_calloc_aligned(plan->inputs.count, sizeof(uint64_t));
#endif

	if ((plan->net_count && (!ctx->net_value || !ctx->net_mask)) ||
		(plan->state.count && (!ctx->next_state_value || !ctx->next_state_mask)) ||
		(plan->outputs.count && (!ctx->output_value || !ctx->output_mask))
#if LXS_TEST_PROBES
		|| (plan->inputs.count && (!ctx->input_shadow_value || !ctx->input_shadow_mask))
#endif
		)
		{
		lxs_free_engine(ctx);
		return 0;
		}

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

#if LXS_TEST_PROBES
	if (plan->inputs.count > 0U)
		{
		memset(ctx->input_shadow_value, 0, (size_t)plan->inputs.count * sizeof(uint64_t));
		memset(ctx->input_shadow_mask, 0, (size_t)plan->inputs.count * sizeof(uint64_t));
		}
#endif

	memset(&ctx->probes, 0, sizeof(ctx->probes));
	}

void lxs_free_engine(lxs_engine_ctx *ctx)
	{
	lxs_free_aligned(ctx->net_value);
	lxs_free_aligned(ctx->net_mask);
	lxs_free_aligned(ctx->next_state_value);
	lxs_free_aligned(ctx->next_state_mask);
	lxs_free_aligned(ctx->output_value);
	lxs_free_aligned(ctx->output_mask);

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

void lxs_execute_levels(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	for (uint32_t level = 0; level < plan->level_count; ++level)
		{
		const lxs_level_plan *level_plan = &plan->levels[level];
		for (uint32_t i = 0; i < level_plan->chunk_count; ++i)
			{
			const lxs_chunk_plan *chunk = &plan->chunks[level_plan->chunk_start + i];
			const lxs_gate_ir *gates = plan->comb_gates + chunk->start;
			lxs_execute_chunk(ctx, chunk, gates);
			}
		if (level_plan->macro_count > 0U)
			{
			lxs_execute_macros(
				ctx,
				plan->macros + level_plan->macro_start,
				level_plan->macro_count);
			}
		if (level_plan->multi_macro_count > 0U)
			{
			lxs_execute_multi_macros(
				ctx,
				plan->multi_macros + level_plan->multi_macro_start,
				level_plan->multi_macro_count);
			}
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
	lxs_execute_levels(ctx, plan);
	lxs_capture_outputs(ctx, plan);
	lxs_capture_next_state(ctx, plan);
	lxs_commit_state(ctx, plan);
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
