#ifndef LXS_API_H
#define LXS_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && defined(LXS_API_BUILD_DLL)
#define LXS_API_EXPORT __declspec(dllexport)
#elif defined(_WIN32) && defined(LXS_API_USE_DLL)
#define LXS_API_EXPORT __declspec(dllimport)
#else
#define LXS_API_EXPORT
#endif

typedef struct lxs_api_netlist lxs_api_netlist;
typedef struct lxs_api_plan lxs_api_plan;
typedef struct lxs_api_engine lxs_api_engine;

typedef enum lxs_api_result
	{
	LXS_API_OK = 0,
	LXS_API_ERR_INVALID_ARG,
	LXS_API_ERR_INVALID_HANDLE,
	LXS_API_ERR_LOAD_FAILED,
	LXS_API_ERR_COMPILE_FAILED,
	LXS_API_ERR_INIT_FAILED,
	LXS_API_ERR_BOUNDS,
	LXS_API_ERR_INTERNAL
	} lxs_api_result;

typedef struct lxs_api_plan_counts
	{
	uint32_t input_count;
	uint32_t output_count;
	uint32_t net_count;
	uint32_t level_count;
	uint32_t chunk_count;
	uint32_t macro_count;
	uint32_t multi_macro_count;
	uint32_t standard_mux_count;
	uint32_t standard_add_count;
	uint32_t standard_cmp_count;
	uint32_t standard_alu_count;
	uint32_t functional_region_count;
	uint32_t register_count;
	uint32_t rom_count;
	uint32_t ram_count;
	uint32_t regfile_count;
	} lxs_api_plan_counts;

typedef struct lxs_api_probes
	{
	uint64_t input_apply;
	uint64_t chunk_exec;
	uint64_t gate_eval;
	uint64_t dff_exec;
	uint64_t tick_count;
	uint64_t state_commit_count;
	} lxs_api_probes;

LXS_API_EXPORT const char* lxs_api_result_string(lxs_api_result result);
LXS_API_EXPORT const char* lxs_api_get_last_error(void);

LXS_API_EXPORT lxs_api_result lxs_api_netlist_load_bench(
	const char *path,
	lxs_api_netlist **out_netlist);
LXS_API_EXPORT void lxs_api_netlist_free(lxs_api_netlist *netlist);

LXS_API_EXPORT lxs_api_result lxs_api_plan_compile(
	const lxs_api_netlist *netlist,
	lxs_api_plan **out_plan);
LXS_API_EXPORT void lxs_api_plan_free(lxs_api_plan *plan);
LXS_API_EXPORT lxs_api_result lxs_api_plan_get_counts(
	const lxs_api_plan *plan,
	lxs_api_plan_counts *out_counts);
LXS_API_EXPORT lxs_api_result lxs_api_plan_get_input_net_id(
	const lxs_api_plan *plan,
	uint32_t input_index,
	uint32_t *out_net_id);
LXS_API_EXPORT lxs_api_result lxs_api_plan_get_output_net_id(
	const lxs_api_plan *plan,
	uint32_t output_index,
	uint32_t *out_net_id);
LXS_API_EXPORT lxs_api_result lxs_api_plan_get_input_name(
	const lxs_api_plan *plan,
	uint32_t input_index,
	const char **out_name);
LXS_API_EXPORT lxs_api_result lxs_api_plan_get_output_name(
	const lxs_api_plan *plan,
	uint32_t output_index,
	const char **out_name);
LXS_API_EXPORT lxs_api_result lxs_api_plan_get_net_name(
	const lxs_api_plan *plan,
	uint32_t net_id,
	const char **out_name);
LXS_API_EXPORT lxs_api_result lxs_api_plan_find_net(
	const lxs_api_plan *plan,
	const char *name,
	uint32_t *out_net_id);

LXS_API_EXPORT lxs_api_result lxs_api_engine_create(
	const lxs_api_plan *plan,
	lxs_api_engine **out_engine);
LXS_API_EXPORT void lxs_api_engine_free(lxs_api_engine *engine);
LXS_API_EXPORT lxs_api_result lxs_api_engine_reset(lxs_api_engine *engine);
LXS_API_EXPORT lxs_api_result lxs_api_engine_apply_inputs(
	lxs_api_engine *engine,
	const uint64_t *values,
	const uint64_t *masks,
	uint32_t count);
LXS_API_EXPORT lxs_api_result lxs_api_engine_tick(lxs_api_engine *engine);
LXS_API_EXPORT lxs_api_result lxs_api_engine_tick_many(
	lxs_api_engine *engine,
	uint32_t tick_count);
LXS_API_EXPORT lxs_api_result lxs_api_engine_read_outputs(
	const lxs_api_engine *engine,
	uint64_t *values,
	uint64_t *masks,
	uint32_t count);
LXS_API_EXPORT lxs_api_result lxs_api_engine_read_probes(
	const lxs_api_engine *engine,
	lxs_api_probes *out_probes);
LXS_API_EXPORT lxs_api_result lxs_api_engine_get_input_count(
	const lxs_api_engine *engine,
	uint32_t *out_count);
LXS_API_EXPORT lxs_api_result lxs_api_engine_get_output_count(
	const lxs_api_engine *engine,
	uint32_t *out_count);

#ifdef __cplusplus
}
#endif

#endif
