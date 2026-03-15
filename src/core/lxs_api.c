#include "lxs_api.h"
#include "lxs_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct lxs_api_netlist
	{
	lxs_netlist *nl;
	};

struct lxs_api_plan
	{
	lxs_plan *plan;
	char **net_names;
	uint32_t net_name_count;
	};

struct lxs_api_engine
	{
	const lxs_plan *plan;
	lxs_engine_ctx ctx;
	};

static _Thread_local char lxs_api_last_error[512];
static _Thread_local lxs_api_result lxs_api_last_error_code = LXS_API_OK;

lxs_api_result lxs_api_plan_get_net_name(
	const lxs_api_plan *plan,
	uint32_t net_id,
	const char **out_name);

static lxs_api_result lxs_api_set_error(
	lxs_api_result result,
	const char *message)
	{
	if (message)
		{
		snprintf(lxs_api_last_error, sizeof(lxs_api_last_error), "%s", message);
		}
	else
		{
		lxs_api_last_error[0] = '\0';
		}
	lxs_api_last_error_code = result;
	return result;
	}

static lxs_api_result lxs_api_set_printf_error(
	lxs_api_result result,
	const char *fmt,
	const char *arg)
	{
	if (fmt)
		{
		snprintf(lxs_api_last_error, sizeof(lxs_api_last_error), fmt, arg ? arg : "");
		}
	else
		{
		lxs_api_last_error[0] = '\0';
		}
	lxs_api_last_error_code = result;
	return result;
	}

static char* lxs_api_strdup(const char *src)
	{
	size_t len;
	char *copy;

	if (!src)
		{
		return NULL;
		}

	len = strlen(src) + 1U;
	copy = (char*)malloc(len);
	if (!copy)
		{
		return NULL;
		}

	memcpy(copy, src, len);
	return copy;
	}

static void lxs_api_free_name_table(char **names, uint32_t count)
	{
	if (!names)
		{
		return;
		}

	for (uint32_t i = 0; i < count; ++i)
		{
		free(names[i]);
		}
	free(names);
	}

static int lxs_api_copy_plan_names(
	lxs_api_plan *api_plan,
	const lxs_netlist *netlist)
	{
	if (!api_plan || !api_plan->plan || !netlist)
		{
		return 0;
		}

	api_plan->net_name_count = api_plan->plan->net_count;
	if (api_plan->net_name_count == 0U)
		{
		return 1;
		}

	api_plan->net_names = (char**)calloc(api_plan->net_name_count, sizeof(char*));
	if (!api_plan->net_names)
		{
		return 0;
		}

	for (uint32_t i = 0; i < api_plan->net_name_count; ++i)
		{
		if (i < netlist->net_count && netlist->net_names && netlist->net_names[i])
			{
			api_plan->net_names[i] = lxs_api_strdup(netlist->net_names[i]);
			if (!api_plan->net_names[i])
				{
				lxs_api_free_name_table(api_plan->net_names, api_plan->net_name_count);
				api_plan->net_names = NULL;
				api_plan->net_name_count = 0U;
				return 0;
				}
			}
		}

	return 1;
	}

static lxs_api_result lxs_api_copy_state_bits(
	const uint64_t *src_values,
	const uint64_t *src_masks,
	uint32_t offset,
	uint32_t count,
	uint64_t *values,
	uint64_t *masks)
	{
	if (!src_values || !src_masks || !values || !masks)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "state_copy: invalid argument");
		}

	for (uint32_t i = 0; i < count; ++i)
		{
		values[i] = src_values[offset + i];
		masks[i] = src_masks[offset + i];
		}

	return lxs_api_set_error(LXS_API_OK, NULL);
	}

const char* lxs_api_result_string(lxs_api_result result)
	{
	switch (result)
		{
		case LXS_API_OK:
			return "ok";
		case LXS_API_ERR_INVALID_ARG:
			return "invalid_arg";
		case LXS_API_ERR_INVALID_HANDLE:
			return "invalid_handle";
		case LXS_API_ERR_LOAD_FAILED:
			return "load_failed";
		case LXS_API_ERR_COMPILE_FAILED:
			return "compile_failed";
		case LXS_API_ERR_INIT_FAILED:
			return "init_failed";
		case LXS_API_ERR_BOUNDS:
			return "bounds";
		case LXS_API_ERR_INTERNAL:
			return "internal";
		default:
			return "unknown";
		}
	}

const char* lxs_api_get_last_error(void)
	{
	return lxs_api_last_error;
	}

lxs_api_result lxs_api_diag_get_last_error_code(void)
	{
	return lxs_api_last_error_code;
	}

void lxs_api_diag_clear_last_error(void)
	{
	lxs_api_last_error[0] = '\0';
	lxs_api_last_error_code = LXS_API_OK;
	}

lxs_api_result lxs_api_netlist_load_bench(
	const char *path,
	lxs_api_netlist **out_netlist)
	{
	lxs_api_netlist *handle;

	if (!path || !out_netlist)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "netlist_load_bench: invalid argument");
		}

	*out_netlist = NULL;
	handle = (lxs_api_netlist*)calloc(1U, sizeof(*handle));
	if (!handle)
		{
		return lxs_api_set_error(LXS_API_ERR_INTERNAL, "netlist_load_bench: allocation failure");
		}

	handle->nl = lxs_load_iscas(path);
	if (!handle->nl)
		{
		free(handle);
		return lxs_api_set_printf_error(LXS_API_ERR_LOAD_FAILED, "netlist_load_bench: failed to load '%s'", path);
		}

	*out_netlist = handle;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

void lxs_api_netlist_free(lxs_api_netlist *netlist)
	{
	if (!netlist)
		{
		return;
		}

	lxs_free_netlist(netlist->nl);
	free(netlist);
	}

lxs_api_result lxs_api_plan_compile(
	const lxs_api_netlist *netlist,
	lxs_api_plan **out_plan)
	{
	lxs_api_plan *handle;

	if (!netlist || !netlist->nl || !out_plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_compile: invalid argument");
		}

	*out_plan = NULL;
	handle = (lxs_api_plan*)calloc(1U, sizeof(*handle));
	if (!handle)
		{
		return lxs_api_set_error(LXS_API_ERR_INTERNAL, "plan_compile: allocation failure");
		}

	handle->plan = lxs_compile_to_plan(netlist->nl);
	if (!handle->plan)
		{
		free(handle);
		return lxs_api_set_error(LXS_API_ERR_COMPILE_FAILED, "plan_compile: compile failed");
		}
	if (!lxs_api_copy_plan_names(handle, netlist->nl))
		{
		lxs_free_plan(handle->plan);
		free(handle);
		return lxs_api_set_error(LXS_API_ERR_INTERNAL, "plan_compile: name table allocation failure");
		}

	*out_plan = handle;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

void lxs_api_plan_free(lxs_api_plan *plan)
	{
	if (!plan)
		{
		return;
		}

	lxs_api_free_name_table(plan->net_names, plan->net_name_count);
	lxs_free_plan(plan->plan);
	free(plan);
	}

lxs_api_result lxs_api_plan_get_counts(
	const lxs_api_plan *plan,
	lxs_api_plan_counts *out_counts)
	{
	if (!plan || !plan->plan || !out_counts)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_counts: invalid argument");
		}

	memset(out_counts, 0, sizeof(*out_counts));
	out_counts->input_count = plan->plan->inputs.count;
	out_counts->output_count = plan->plan->outputs.count;
	out_counts->net_count = plan->plan->net_count;
	out_counts->level_count = plan->plan->level_count;
	out_counts->chunk_count = plan->plan->span_count;
	out_counts->macro_count = plan->plan->macro_count;
	out_counts->multi_macro_count = plan->plan->multi_macro_count;
	out_counts->standard_mux_count = plan->plan->standard_mux_count;
	out_counts->standard_add_count = plan->plan->standard_add_count;
	out_counts->standard_cmp_count = plan->plan->standard_cmp_count;
	out_counts->standard_alu_count = plan->plan->standard_alu_count;
	out_counts->functional_region_count = plan->plan->functional_region_count;
	out_counts->register_count = plan->plan->register_count;
	out_counts->rom_count = plan->plan->rom_count;
	out_counts->ram_count = plan->plan->ram_count;
	out_counts->regfile_count = plan->plan->regfile_count;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_plan_get_input_net_id(
	const lxs_api_plan *plan,
	uint32_t input_index,
	uint32_t *out_net_id)
	{
	if (!plan || !plan->plan || !out_net_id)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_input_net_id: invalid argument");
		}
	if (input_index >= plan->plan->inputs.count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "plan_get_input_net_id: input index out of bounds");
		}

	*out_net_id = plan->plan->inputs.net_ids[input_index];
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_plan_get_output_net_id(
	const lxs_api_plan *plan,
	uint32_t output_index,
	uint32_t *out_net_id)
	{
	if (!plan || !plan->plan || !out_net_id)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_output_net_id: invalid argument");
		}
	if (output_index >= plan->plan->outputs.count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "plan_get_output_net_id: output index out of bounds");
		}

	*out_net_id = plan->plan->outputs.net_ids[output_index];
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_plan_get_input_name(
	const lxs_api_plan *plan,
	uint32_t input_index,
	const char **out_name)
	{
	uint32_t net_id;
	lxs_api_result result;

	if (!out_name)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_input_name: invalid argument");
		}

	result = lxs_api_plan_get_input_net_id(plan, input_index, &net_id);
	if (result != LXS_API_OK)
		{
		return result;
		}

	return lxs_api_plan_get_net_name(plan, net_id, out_name);
	}

lxs_api_result lxs_api_plan_get_output_name(
	const lxs_api_plan *plan,
	uint32_t output_index,
	const char **out_name)
	{
	uint32_t net_id;
	lxs_api_result result;

	if (!out_name)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_output_name: invalid argument");
		}

	result = lxs_api_plan_get_output_net_id(plan, output_index, &net_id);
	if (result != LXS_API_OK)
		{
		return result;
		}

	return lxs_api_plan_get_net_name(plan, net_id, out_name);
	}

lxs_api_result lxs_api_plan_get_net_name(
	const lxs_api_plan *plan,
	uint32_t net_id,
	const char **out_name)
	{
	if (!plan || !plan->plan || !out_name)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_net_name: invalid argument");
		}
	if (net_id >= plan->net_name_count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "plan_get_net_name: net id out of bounds");
		}

	*out_name = plan->net_names ? plan->net_names[net_id] : NULL;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_plan_find_net(
	const lxs_api_plan *plan,
	const char *name,
	uint32_t *out_net_id)
	{
	if (!plan || !plan->plan || !name || !out_net_id)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_find_net: invalid argument");
		}

	for (uint32_t i = 0; i < plan->net_name_count; ++i)
		{
		if (plan->net_names && plan->net_names[i] && strcmp(plan->net_names[i], name) == 0)
			{
			*out_net_id = i;
			return lxs_api_set_error(LXS_API_OK, NULL);
			}
		}

	return lxs_api_set_printf_error(LXS_API_ERR_BOUNDS, "plan_find_net: net not found '%s'", name);
	}

lxs_api_result lxs_api_engine_create(
	const lxs_api_plan *plan,
	lxs_api_engine **out_engine)
	{
	lxs_api_engine *handle;

	if (!plan || !plan->plan || !out_engine)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "engine_create: invalid argument");
		}

	*out_engine = NULL;
	handle = (lxs_api_engine*)calloc(1U, sizeof(*handle));
	if (!handle)
		{
		return lxs_api_set_error(LXS_API_ERR_INTERNAL, "engine_create: allocation failure");
		}

	handle->plan = plan->plan;
	if (!lxs_init_engine(&handle->ctx, handle->plan))
		{
		free(handle);
		return lxs_api_set_error(LXS_API_ERR_INIT_FAILED, "engine_create: init failed");
		}

	*out_engine = handle;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

void lxs_api_engine_free(lxs_api_engine *engine)
	{
	if (!engine)
		{
		return;
		}

	lxs_free_engine(&engine->ctx);
	free(engine);
	}

lxs_api_result lxs_api_engine_reset(lxs_api_engine *engine)
	{
	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_reset: invalid engine");
		}

	lxs_reset_engine(&engine->ctx, engine->plan);
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_apply_inputs(
	lxs_api_engine *engine,
	const uint64_t *values,
	const uint64_t *masks,
	uint32_t count)
	{
	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_apply_inputs: invalid engine");
		}
	if (count != engine->plan->inputs.count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_apply_inputs: input count mismatch");
		}

	lxs_apply_inputs(&engine->ctx, engine->plan, values, masks);
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_tick(lxs_api_engine *engine)
	{
	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_tick: invalid engine");
		}

	lxs_execute_plan(&engine->ctx, engine->plan);
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_tick_many(
	lxs_api_engine *engine,
	uint32_t tick_count)
	{
	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_tick_many: invalid engine");
		}

	for (uint32_t i = 0; i < tick_count; ++i)
		{
		lxs_execute_plan(&engine->ctx, engine->plan);
		}
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_clear_probes(lxs_api_engine *engine)
	{
	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_clear_probes: invalid engine");
		}

	memset(&engine->ctx.probes, 0, sizeof(engine->ctx.probes));
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_read_outputs(
	const lxs_api_engine *engine,
	uint64_t *values,
	uint64_t *masks,
	uint32_t count)
	{
	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_read_outputs: invalid engine");
		}
	if (count != engine->plan->outputs.count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_outputs: output count mismatch");
		}

	lxs_read_outputs(&engine->ctx, engine->plan, values, masks);
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_read_net(
	const lxs_api_engine *engine,
	uint32_t net_id,
	uint64_t *out_value,
	uint64_t *out_mask)
	{
	if (!engine || !engine->plan || !out_value || !out_mask)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "engine_read_net: invalid argument");
		}
	if (net_id >= engine->plan->net_count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_net: net id out of bounds");
		}

	*out_value = engine->ctx.net_value[net_id];
	*out_mask = engine->ctx.net_mask[net_id];
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_plan_get_register_info(
	const lxs_api_plan *plan,
	uint32_t register_index,
	lxs_api_register_info *out_info)
	{
	const lxs_register_plan *reg;

	if (!plan || !plan->plan || !out_info)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_register_info: invalid argument");
		}
	if (register_index >= plan->plan->register_count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "plan_get_register_info: register index out of bounds");
		}

	reg = &plan->plan->registers[register_index];
	out_info->width_bits = reg->width_bits;
	out_info->mode = reg->mode;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_read_register(
	const lxs_api_engine *engine,
	uint32_t register_index,
	uint64_t *values,
	uint64_t *masks,
	uint32_t count)
	{
	const lxs_register_plan *reg;

	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_read_register: invalid engine");
		}
	if (!values || !masks)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "engine_read_register: invalid argument");
		}
	if (register_index >= engine->plan->register_count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_register: register index out of bounds");
		}

	reg = &engine->plan->registers[register_index];
	if (count != reg->width_bits)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_register: width mismatch");
		}

	return lxs_api_copy_state_bits(
		engine->ctx.register_value,
		engine->ctx.register_mask,
		reg->storage_offset,
		reg->width_bits,
		values,
		masks);
	}

lxs_api_result lxs_api_plan_get_ram_info(
	const lxs_api_plan *plan,
	uint32_t ram_index,
	lxs_api_ram_info *out_info)
	{
	const lxs_ram_plan *ram;

	if (!plan || !plan->plan || !out_info)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_ram_info: invalid argument");
		}
	if (ram_index >= plan->plan->ram_count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "plan_get_ram_info: ram index out of bounds");
		}

	ram = &plan->plan->rams[ram_index];
	out_info->addr_width = ram->addr_width;
	out_info->data_width = ram->data_width;
	out_info->depth = ram->depth;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_read_ram_word(
	const lxs_api_engine *engine,
	uint32_t ram_index,
	uint32_t address,
	uint64_t *values,
	uint64_t *masks,
	uint32_t count)
	{
	const lxs_ram_plan *ram;
	uint32_t offset;

	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_read_ram_word: invalid engine");
		}
	if (!values || !masks)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "engine_read_ram_word: invalid argument");
		}
	if (ram_index >= engine->plan->ram_count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_ram_word: ram index out of bounds");
		}

	ram = &engine->plan->rams[ram_index];
	if (count != ram->data_width)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_ram_word: width mismatch");
		}
	if (address >= ram->depth)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_ram_word: address out of bounds");
		}

	offset = ram->storage_offset + (address * ram->data_width);
	return lxs_api_copy_state_bits(
		engine->ctx.ram_value,
		engine->ctx.ram_mask,
		offset,
		ram->data_width,
		values,
		masks);
	}

lxs_api_result lxs_api_plan_get_regfile_info(
	const lxs_api_plan *plan,
	uint32_t regfile_index,
	lxs_api_regfile_info *out_info)
	{
	const lxs_regfile_plan *regfile;

	if (!plan || !plan->plan || !out_info)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "plan_get_regfile_info: invalid argument");
		}
	if (regfile_index >= plan->plan->regfile_count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "plan_get_regfile_info: regfile index out of bounds");
		}

	regfile = &plan->plan->regfiles[regfile_index];
	out_info->addr_width = regfile->addr_width;
	out_info->data_width = regfile->data_width;
	out_info->depth = regfile->depth;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_read_regfile_word(
	const lxs_api_engine *engine,
	uint32_t regfile_index,
	uint32_t address,
	uint64_t *values,
	uint64_t *masks,
	uint32_t count)
	{
	const lxs_regfile_plan *regfile;
	uint32_t offset;

	if (!engine || !engine->plan)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_HANDLE, "engine_read_regfile_word: invalid engine");
		}
	if (!values || !masks)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "engine_read_regfile_word: invalid argument");
		}
	if (regfile_index >= engine->plan->regfile_count)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_regfile_word: regfile index out of bounds");
		}

	regfile = &engine->plan->regfiles[regfile_index];
	if (count != regfile->data_width)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_regfile_word: width mismatch");
		}
	if (address >= regfile->depth)
		{
		return lxs_api_set_error(LXS_API_ERR_BOUNDS, "engine_read_regfile_word: address out of bounds");
		}

	offset = regfile->storage_offset + (address * regfile->data_width);
	return lxs_api_copy_state_bits(
		engine->ctx.regfile_value,
		engine->ctx.regfile_mask,
		offset,
		regfile->data_width,
		values,
		masks);
	}

lxs_api_result lxs_api_engine_read_probes(
	const lxs_api_engine *engine,
	lxs_api_probes *out_probes)
	{
	lxs_probes probes;

	if (!engine || !engine->plan || !out_probes)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "engine_read_probes: invalid argument");
		}

	probes = lxs_get_probes(&engine->ctx);
	out_probes->input_apply = probes.input_apply;
	out_probes->chunk_exec = probes.chunk_exec;
	out_probes->gate_eval = probes.gate_eval;
	out_probes->dff_exec = probes.dff_exec;
	out_probes->tick_count = probes.tick_count;
	out_probes->state_commit_count = probes.state_commit_count;
#if LXS_TEST_PROBES
	out_probes->input_toggle = probes.input_toggle;
	out_probes->state_change_commit = probes.state_change_commit;
	out_probes->contention_count = probes.contention_count;
	out_probes->unknown_state_materialize_count = probes.unknown_state_materialize_count;
	out_probes->highz_materialize_count = probes.highz_materialize_count;
	out_probes->multi_driver_resolve_count = probes.multi_driver_resolve_count;
	out_probes->tri_no_drive_count = probes.tri_no_drive_count;
	out_probes->pup_z_source_count = probes.pup_z_source_count;
	out_probes->pdn_z_source_count = probes.pdn_z_source_count;
#else
	out_probes->input_toggle = 0ULL;
	out_probes->state_change_commit = 0ULL;
	out_probes->contention_count = 0ULL;
	out_probes->unknown_state_materialize_count = 0ULL;
	out_probes->highz_materialize_count = 0ULL;
	out_probes->multi_driver_resolve_count = 0ULL;
	out_probes->tri_no_drive_count = 0ULL;
	out_probes->pup_z_source_count = 0ULL;
	out_probes->pdn_z_source_count = 0ULL;
#endif
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_get_input_count(
	const lxs_api_engine *engine,
	uint32_t *out_count)
	{
	if (!engine || !engine->plan || !out_count)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "engine_get_input_count: invalid argument");
		}

	*out_count = engine->plan->inputs.count;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}

lxs_api_result lxs_api_engine_get_output_count(
	const lxs_api_engine *engine,
	uint32_t *out_count)
	{
	if (!engine || !engine->plan || !out_count)
		{
		return lxs_api_set_error(LXS_API_ERR_INVALID_ARG, "engine_get_output_count: invalid argument");
		}

	*out_count = engine->plan->outputs.count;
	return lxs_api_set_error(LXS_API_OK, NULL);
	}
