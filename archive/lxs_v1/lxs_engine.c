#include "lxs_types.h"
#include "lxs_primitives.h"
#include <string.h>
#include <malloc.h>

#define ALIGN_SIZE 64U

static void* lxs_malloc_aligned(size_t size)
	{
	if (size == 0)
		{
		return NULL;
		}

#ifdef _WIN32
	return _aligned_malloc(size, ALIGN_SIZE);
#else
	void *ptr = NULL;
	if (posix_memalign(&ptr, ALIGN_SIZE, size) != 0)
		{
		return NULL;
		}
	return ptr;
#endif
	}

static void* lxs_calloc_aligned(size_t count, size_t size)
	{
	size_t total_size = count * size;
	void *ptr = lxs_malloc_aligned(total_size);
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

static void lxs_eval_stream_binary(
	const uint64_t *restrict v_a,
	const uint64_t *restrict m_a,
	const uint64_t *restrict v_b,
	const uint64_t *restrict m_b,
	uint64_t *restrict v_out,
	uint64_t *restrict m_out,
	uint32_t count,
	uint32_t type)
	{
	v_a = __builtin_assume_aligned(v_a, 64);
	m_a = __builtin_assume_aligned(m_a, 64);
	v_b = __builtin_assume_aligned(v_b, 64);
	m_b = __builtin_assume_aligned(m_b, 64);
	v_out = __builtin_assume_aligned(v_out, 64);
	m_out = __builtin_assume_aligned(m_out, 64);

	switch (type)
		{
		case LXS_GATE_AND:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_AND(v_a[i], m_a[i], v_b[i], m_b[i], v_out[i], m_out[i]);
				}
			break;
		case LXS_GATE_OR:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_OR(v_a[i], m_a[i], v_b[i], m_b[i], v_out[i], m_out[i]);
				}
			break;
		case LXS_GATE_XOR:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_XOR(v_a[i], m_a[i], v_b[i], m_b[i], v_out[i], m_out[i]);
				}
			break;
		case LXS_GATE_TRI:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_TRI(v_a[i], m_a[i], v_b[i], m_b[i], v_out[i], m_out[i]);
				}
			break;
		case LXS_GATE_NAND:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_NAND(v_a[i], m_a[i], v_b[i], m_b[i], v_out[i], m_out[i]);
				}
			break;
		case LXS_GATE_NOR:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_NOR(v_a[i], m_a[i], v_b[i], m_b[i], v_out[i], m_out[i]);
				}
			break;
		case LXS_GATE_XNOR:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_XNOR(v_a[i], m_a[i], v_b[i], m_b[i], v_out[i], m_out[i]);
				}
			break;
		default:
			break;
		}
	}

static void lxs_eval_stream_unary(
	const uint64_t *restrict v_a,
	const uint64_t *restrict m_a,
	uint64_t *restrict v_out,
	uint64_t *restrict m_out,
	uint32_t count,
	uint32_t type)
	{
	v_a = __builtin_assume_aligned(v_a, 64);
	m_a = __builtin_assume_aligned(m_a, 64);
	v_out = __builtin_assume_aligned(v_out, 64);
	m_out = __builtin_assume_aligned(m_out, 64);

	switch (type)
		{
		case LXS_GATE_NOT:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_NOT(v_a[i], m_a[i], v_out[i], m_out[i]);
				}
			break;
		case LXS_GATE_BUF:
			for (uint32_t i = 0; i < count; ++i)
				{
				LXS_EVAL_BUF(v_a[i], m_a[i], v_out[i], m_out[i]);
				}
			break;
		default:
			break;
		}
	}

static void lxs_gather_signal_contig(
	const uint64_t *restrict net_value,
	const uint64_t *restrict net_mask,
	uint32_t base,
	uint64_t *restrict out_value,
	uint64_t *restrict out_mask,
	uint32_t count)
	{
	memcpy(out_value, net_value + base, (size_t)count * sizeof(uint64_t));
	memcpy(out_mask, net_mask + base, (size_t)count * sizeof(uint64_t));
	}

static void lxs_scatter_signal_contig(
	uint64_t *restrict net_value,
	uint64_t *restrict net_mask,
	uint32_t base,
	const uint64_t *restrict src_value,
	const uint64_t *restrict src_mask,
	uint32_t count)
	{
	memcpy(net_value + base, src_value, (size_t)count * sizeof(uint64_t));
	memcpy(net_mask + base, src_mask, (size_t)count * sizeof(uint64_t));
	}

static void lxs_gather_signal(
	const uint64_t *restrict net_value,
	const uint64_t *restrict net_mask,
	const uint32_t *restrict ids,
	uint64_t *restrict out_value,
	uint64_t *restrict out_mask,
	uint32_t count)
	{
	for (uint32_t i = 0; i < count; ++i)
		{
		uint32_t net_id = ids[i];
		out_value[i] = net_value[net_id];
		out_mask[i] = net_mask[net_id];
		}
	}

static void lxs_gather_signal_runs(
	const uint64_t *restrict net_value,
	const uint64_t *restrict net_mask,
	const lxs_copy_run *restrict runs,
	uint32_t run_count,
	uint64_t *restrict out_value,
	uint64_t *restrict out_mask)
	{
	for (uint32_t i = 0; i < run_count; ++i)
		{
		memcpy(
			out_value + runs[i].span_base,
			net_value + runs[i].net_base,
			(size_t)runs[i].count * sizeof(uint64_t));
		memcpy(
			out_mask + runs[i].span_base,
			net_mask + runs[i].net_base,
			(size_t)runs[i].count * sizeof(uint64_t));
		}
	}

static void lxs_scatter_signal(
	uint64_t *restrict net_value,
	uint64_t *restrict net_mask,
	const uint32_t *restrict ids,
	const uint64_t *restrict src_value,
	const uint64_t *restrict src_mask,
	uint32_t count)
	{
	for (uint32_t i = 0; i < count; ++i)
		{
		uint32_t net_id = ids[i];
		net_value[net_id] = src_value[i];
		net_mask[net_id] = src_mask[i];
		}
	}

static void lxs_execute_span(lxs_engine_ctx *ctx, const lxs_span_plan *span)
	{
	uint32_t direct_src_a = (span->layout_flags & LXS_SPAN_SRC_A_CONTIG) != 0U;
	uint32_t direct_src_b = (span->layout_flags & LXS_SPAN_SRC_B_CONTIG) != 0U;
	uint32_t direct_dst = (span->layout_flags & LXS_SPAN_DST_CONTIG) != 0U;
	uint32_t packed = (span->layout_flags & LXS_SPAN_PACKED) != 0U;
	uint32_t unary = (span->type == LXS_GATE_NOT || span->type == LXS_GATE_BUF);

	if (unary)
		{
		if (direct_src_a && direct_dst)
			{
			ctx->probes.direct_span_exec++;
			ctx->probes.direct_gate_eval += span->count;
			lxs_eval_stream_unary(
				ctx->net_value + span->src_a_base,
				ctx->net_mask + span->src_a_base,
				ctx->net_value + span->dst_base,
				ctx->net_mask + span->dst_base,
				span->count,
				span->type);
			return;
			}

		if (packed)
			{
			ctx->probes.packed_span_exec++;
			ctx->probes.packed_gate_eval += span->count;
			lxs_gather_signal_runs(
				ctx->net_value,
				ctx->net_mask,
				span->src_a_runs,
				span->src_a_run_count,
				ctx->span_a_value,
				ctx->span_a_mask);
			lxs_eval_stream_unary(
				ctx->span_a_value,
				ctx->span_a_mask,
				ctx->net_value + span->dst_base,
				ctx->net_mask + span->dst_base,
				span->count,
				span->type);
			return;
			}

		ctx->probes.scratch_span_exec++;
		ctx->probes.scratch_gate_eval += span->count;

		if (direct_src_a)
			{
			lxs_gather_signal_contig(
				ctx->net_value,
				ctx->net_mask,
				span->src_a_base,
				ctx->span_a_value,
				ctx->span_a_mask,
				span->count);
			}
		else
			{
			lxs_gather_signal(
				ctx->net_value,
				ctx->net_mask,
				span->src_a,
				ctx->span_a_value,
				ctx->span_a_mask,
				span->count);
			}

		lxs_eval_stream_unary(
			ctx->span_a_value,
			ctx->span_a_mask,
			ctx->span_out_value,
			ctx->span_out_mask,
			span->count,
			span->type);

		if (direct_dst)
			{
			lxs_scatter_signal_contig(
				ctx->net_value,
				ctx->net_mask,
				span->dst_base,
				ctx->span_out_value,
				ctx->span_out_mask,
				span->count);
			}
		else
			{
			lxs_scatter_signal(
				ctx->net_value,
				ctx->net_mask,
				span->dst,
				ctx->span_out_value,
				ctx->span_out_mask,
				span->count);
			}
		return;
		}

	if (direct_src_a && direct_dst && direct_src_b)
		{
		ctx->probes.direct_span_exec++;
		ctx->probes.direct_gate_eval += span->count;
		lxs_eval_stream_binary(
			ctx->net_value + span->src_a_base,
			ctx->net_mask + span->src_a_base,
			ctx->net_value + span->src_b_base,
			ctx->net_mask + span->src_b_base,
			ctx->net_value + span->dst_base,
			ctx->net_mask + span->dst_base,
			span->count,
			span->type);
		return;
		}

	if (packed)
		{
		const uint64_t *src_a_value;
		const uint64_t *src_a_mask;
		const uint64_t *src_b_value;
		const uint64_t *src_b_mask;

		ctx->probes.packed_span_exec++;
		ctx->probes.packed_gate_eval += span->count;

		if (direct_src_a)
			{
			src_a_value = ctx->net_value + span->src_a_base;
			src_a_mask = ctx->net_mask + span->src_a_base;
			}
		else
			{
			lxs_gather_signal_runs(
				ctx->net_value,
				ctx->net_mask,
				span->src_a_runs,
				span->src_a_run_count,
				ctx->span_a_value,
				ctx->span_a_mask);
			src_a_value = ctx->span_a_value;
			src_a_mask = ctx->span_a_mask;
			}

		if (direct_src_b)
			{
			src_b_value = ctx->net_value + span->src_b_base;
			src_b_mask = ctx->net_mask + span->src_b_base;
			}
		else
			{
			lxs_gather_signal_runs(
				ctx->net_value,
				ctx->net_mask,
				span->src_b_runs,
				span->src_b_run_count,
				ctx->span_b_value,
				ctx->span_b_mask);
			src_b_value = ctx->span_b_value;
			src_b_mask = ctx->span_b_mask;
			}

		lxs_eval_stream_binary(
			src_a_value,
			src_a_mask,
			src_b_value,
			src_b_mask,
			ctx->net_value + span->dst_base,
			ctx->net_mask + span->dst_base,
			span->count,
			span->type);
		return;
		}

	ctx->probes.scratch_span_exec++;
	ctx->probes.scratch_gate_eval += span->count;

	if (direct_src_a)
		{
		lxs_gather_signal_contig(
			ctx->net_value,
			ctx->net_mask,
			span->src_a_base,
			ctx->span_a_value,
			ctx->span_a_mask,
			span->count);
		}
	else
		{
		lxs_gather_signal(
			ctx->net_value,
			ctx->net_mask,
			span->src_a,
			ctx->span_a_value,
			ctx->span_a_mask,
			span->count);
		}

	if (direct_src_b)
		{
		lxs_gather_signal_contig(
			ctx->net_value,
			ctx->net_mask,
			span->src_b_base,
			ctx->span_b_value,
			ctx->span_b_mask,
			span->count);
		}
	else
		{
		lxs_gather_signal(
			ctx->net_value,
			ctx->net_mask,
			span->src_b,
			ctx->span_b_value,
			ctx->span_b_mask,
			span->count);
		}

	lxs_eval_stream_binary(
		ctx->span_a_value,
		ctx->span_a_mask,
		ctx->span_b_value,
		ctx->span_b_mask,
		ctx->span_out_value,
		ctx->span_out_mask,
		span->count,
		span->type);

	if (direct_dst)
		{
		lxs_scatter_signal_contig(
			ctx->net_value,
			ctx->net_mask,
			span->dst_base,
			ctx->span_out_value,
			ctx->span_out_mask,
			span->count);
		}
	else
		{
		lxs_scatter_signal(
			ctx->net_value,
			ctx->net_mask,
			span->dst,
			ctx->span_out_value,
			ctx->span_out_mask,
			span->count);
		}
	}

int lxs_init_engine(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	memset(ctx, 0, sizeof(*ctx));
	ctx->net_value = lxs_calloc_aligned(plan->net_count, sizeof(uint64_t));
	ctx->net_mask = lxs_calloc_aligned(plan->net_count, sizeof(uint64_t));
	ctx->span_a_value = lxs_calloc_aligned(plan->max_span_count, sizeof(uint64_t));
	ctx->span_a_mask = lxs_calloc_aligned(plan->max_span_count, sizeof(uint64_t));
	ctx->span_b_value = lxs_calloc_aligned(plan->max_span_count, sizeof(uint64_t));
	ctx->span_b_mask = lxs_calloc_aligned(plan->max_span_count, sizeof(uint64_t));
	ctx->span_out_value = lxs_calloc_aligned(plan->max_span_count, sizeof(uint64_t));
	ctx->span_out_mask = lxs_calloc_aligned(plan->max_span_count, sizeof(uint64_t));
	ctx->next_state_value = lxs_calloc_aligned(plan->state.count, sizeof(uint64_t));
	ctx->next_state_mask = lxs_calloc_aligned(plan->state.count, sizeof(uint64_t));
	ctx->output_value = lxs_calloc_aligned(plan->outputs.count, sizeof(uint64_t));
	ctx->output_mask = lxs_calloc_aligned(plan->outputs.count, sizeof(uint64_t));

#if HEBS_TEST_PROBES
	ctx->input_shadow_value = lxs_calloc_aligned(plan->inputs.count, sizeof(uint64_t));
	ctx->input_shadow_mask = lxs_calloc_aligned(plan->inputs.count, sizeof(uint64_t));
#endif

	if ((plan->net_count && (!ctx->net_value || !ctx->net_mask)) ||
		(plan->max_span_count && (!ctx->span_a_value || !ctx->span_a_mask ||
			!ctx->span_b_value || !ctx->span_b_mask ||
			!ctx->span_out_value || !ctx->span_out_mask)) ||
		(plan->state.count && (!ctx->next_state_value || !ctx->next_state_mask)) ||
		(plan->outputs.count && (!ctx->output_value || !ctx->output_mask))
#if HEBS_TEST_PROBES
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
	if (plan->net_count > 0)
		{
		memset(ctx->net_value, 0, (size_t)plan->net_count * sizeof(uint64_t));
		memset(ctx->net_mask, 0, (size_t)plan->net_count * sizeof(uint64_t));
		}

	if (plan->max_span_count > 0)
		{
		memset(ctx->span_a_value, 0, (size_t)plan->max_span_count * sizeof(uint64_t));
		memset(ctx->span_a_mask, 0, (size_t)plan->max_span_count * sizeof(uint64_t));
		memset(ctx->span_b_value, 0, (size_t)plan->max_span_count * sizeof(uint64_t));
		memset(ctx->span_b_mask, 0, (size_t)plan->max_span_count * sizeof(uint64_t));
		memset(ctx->span_out_value, 0, (size_t)plan->max_span_count * sizeof(uint64_t));
		memset(ctx->span_out_mask, 0, (size_t)plan->max_span_count * sizeof(uint64_t));
		}

	if (plan->state.count > 0)
		{
		memset(ctx->next_state_value, 0, (size_t)plan->state.count * sizeof(uint64_t));
		memset(ctx->next_state_mask, 0, (size_t)plan->state.count * sizeof(uint64_t));
		}

	if (plan->outputs.count > 0)
		{
		memset(ctx->output_value, 0, (size_t)plan->outputs.count * sizeof(uint64_t));
		memset(ctx->output_mask, 0, (size_t)plan->outputs.count * sizeof(uint64_t));
		}

#if HEBS_TEST_PROBES
	if (plan->inputs.count > 0)
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
	lxs_free_aligned(ctx->span_a_value);
	lxs_free_aligned(ctx->span_a_mask);
	lxs_free_aligned(ctx->span_b_value);
	lxs_free_aligned(ctx->span_b_mask);
	lxs_free_aligned(ctx->span_out_value);
	lxs_free_aligned(ctx->span_out_mask);
	lxs_free_aligned(ctx->next_state_value);
	lxs_free_aligned(ctx->next_state_mask);
	lxs_free_aligned(ctx->output_value);
	lxs_free_aligned(ctx->output_mask);

#if HEBS_TEST_PROBES
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
	for (uint32_t i = 0; i < plan->inputs.count; ++i)
		{
		uint32_t net_id = plan->inputs.net_ids[i];
		uint64_t next_value = values ? values[i] : 0ULL;
		uint64_t next_mask = masks ? masks[i] : 0ULL;

#if HEBS_TEST_PROBES
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
		lxs_level_plan *level_plan = &plan->levels[level];
		for (uint32_t span_index = 0; span_index < level_plan->span_count; ++span_index)
			{
			lxs_span_plan *span = &level_plan->spans[span_index];
			ctx->probes.chunk_exec++;
			ctx->probes.gate_eval += span->count;
			lxs_execute_span(ctx, span);
			}
		}
	}

void lxs_capture_outputs(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->outputs.count; ++i)
		{
		uint32_t net_id = plan->outputs.net_ids[i];
		ctx->output_value[i] = ctx->net_value[net_id];
		ctx->output_mask[i] = ctx->net_mask[net_id];
		}
	}

void lxs_capture_next_state(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->state.count; ++i)
		{
		uint32_t source_net = plan->state.d_inputs[i];
		ctx->next_state_value[i] = ctx->net_value[source_net];
		ctx->next_state_mask[i] = ctx->net_mask[source_net];
		}
	ctx->probes.dff_exec += plan->state.count;
	}

void lxs_commit_state(lxs_engine_ctx *ctx, const lxs_plan *plan)
	{
	for (uint32_t i = 0; i < plan->state.count; ++i)
		{
		uint32_t target_net = plan->state.q_outputs[i];

#if HEBS_TEST_PROBES
		if (ctx->net_value[target_net] != ctx->next_state_value[i] ||
			ctx->net_mask[target_net] != ctx->next_state_mask[i])
			{
			ctx->probes.state_change_commit++;
			}
#endif

		ctx->net_value[target_net] = ctx->next_state_value[i];
		ctx->net_mask[target_net] = ctx->next_state_mask[i];
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
	if (values && plan->outputs.count > 0)
		{
		memcpy(values, ctx->output_value, (size_t)plan->outputs.count * sizeof(uint64_t));
		}

	if (masks && plan->outputs.count > 0)
		{
		memcpy(masks, ctx->output_mask, (size_t)plan->outputs.count * sizeof(uint64_t));
		}
	}

lxs_probes lxs_get_probes(const lxs_engine_ctx *ctx)
	{
	return ctx->probes;
	}
