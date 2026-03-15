#include "lxs_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <windows.h>

typedef enum lxs_record_mode
	{
	LXS_RECORD_AGGREGATE = 0,
	LXS_RECORD_TRACE,
	LXS_RECORD_NONE
	} lxs_record_mode;

typedef enum lxs_bench_mode
	{
	LXS_MODE_CONTINUOUS = 0,
	LXS_MODE_COLD
	} lxs_bench_mode;

typedef enum lxs_overwrite_rule
	{
	LXS_OVERWRITE_APPEND = 0,
	LXS_OVERWRITE_TRUNCATE
	} lxs_overwrite_rule;

typedef enum lxs_sample_select
	{
	LXS_SAMPLE_MEDIAN = 0,
	LXS_SAMPLE_MIN,
	LXS_SAMPLE_MAX
	} lxs_sample_select;

typedef struct lxs_bench_options
	{
	const char *benchmark_path;
	lxs_bench_mode mode;
	lxs_record_mode record_mode;
	lxs_overwrite_rule overwrite_rule;
	lxs_sample_select sample_select;
	const char *record_file_path;
	const char *record_file_name;
	const char *record_meta_head;
	uint32_t sample_count;
	uint32_t iterations;
	uint32_t cycles;
	uint32_t record_interval;
	int phase_profile;
	int struct_profile;
	} lxs_bench_options;

typedef struct lxs_path_list
	{
	char **items;
	size_t count;
	size_t cap;
	} lxs_path_list;

typedef struct lxs_bench_result
	{
	char circuit_name[MAX_PATH];
	uint32_t input_count;
	uint32_t output_count;
	uint32_t level_count;
	uint32_t span_count;
	uint32_t max_span_count;
	uint64_t input_apply;
	uint64_t chunk_exec;
	uint64_t gate_eval;
	uint64_t dff_exec;
	uint64_t tick_count;
	uint64_t state_commit_count;
	double input_time_s;
	double comb_time_s;
	double output_capture_time_s;
	double dff_capture_time_s;
	double state_commit_time_s;
	double output_read_time_s;
	double total_time_s;
	double geps;
	} lxs_bench_result;

typedef struct lxs_plan_profile
	{
	char circuit_name[MAX_PATH];
	uint32_t comb_gate_count;
	uint32_t dff_count;
	uint32_t chunk_count;
	uint32_t primitive_step_count;
	uint32_t macro_step_count;
	uint32_t total_step_count;
	uint32_t max_chunk_size;
	uint32_t median_chunk_size;
	uint32_t tiny_chunk_count;
	uint32_t small_chunk_count;
	uint32_t large_chunk_count;
	uint32_t unary_gate_count;
	uint32_t binary_gate_count;
	uint32_t max_level_gate_count;
	uint32_t max_level_chunk_count;
	uint32_t single_chunk_level_count;
	uint32_t primitive_only_level_count;
	uint32_t arithmetic_only_level_count;
	uint32_t mixed_level_count;
	uint32_t arithmetic_level_count;
	uint32_t phase_transition_count;
	uint32_t primitive_to_arithmetic_transition_count;
	uint32_t arithmetic_to_primitive_transition_count;
	uint32_t arithmetic_adjacent_primitive_level_count;
	uint32_t arithmetic_adjacent_primitive_chunk_count;
	uint32_t arithmetic_adjacent_primitive_tiny_chunk_count;
	uint32_t gate_type_count[LXS_GATE_TYPE_COUNT];
	uint32_t recognition_mask;
	uint32_t recognition_mode;
	uint32_t recognition_match_count[LXS_RECOGNITION_FAMILY_COUNT];
	uint32_t recognition_node_reduction[LXS_RECOGNITION_FAMILY_COUNT];
	uint32_t recognition_gate_equiv[LXS_RECOGNITION_FAMILY_COUNT];
	uint64_t recognition_candidate_roots[LXS_RECOGNITION_FAMILY_COUNT];
	uint64_t recognition_nodes_visited[LXS_RECOGNITION_FAMILY_COUNT];
	uint32_t recognition_max_depth[LXS_RECOGNITION_FAMILY_COUNT];
	uint64_t recognition_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT];
	uint64_t recognition_time_us[LXS_RECOGNITION_FAMILY_COUNT];
	uint64_t primitive_gate_equiv;
	uint64_t macro_gate_equiv;
	uint64_t total_gate_equiv;
	uint64_t arithmetic_adjacent_primitive_gate_equiv;
	uint32_t arithmetic_multi_macro_count;
	uint32_t arithmetic_max_input_span;
	uint32_t arithmetic_max_touch_span;
	uint32_t arithmetic_temp_net_count;
	uint32_t arithmetic_temp_max_lifetime;
	double mean_chunk_size;
	double macro_step_share;
	double absorbed_work_share;
	double arithmetic_mean_input_span;
	double arithmetic_mean_touch_span;
	double arithmetic_mean_temp_lifetime;
	double recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_COUNT];
	} lxs_plan_profile;

static uint8_t lxs_is_arithmetic_multi_macro_type(uint32_t type)
	{
	switch (type)
		{
		case LXS_MULTI_MACRO_HALF_ADDER:
		case LXS_MULTI_MACRO_FULL_ADDER:
		case LXS_MULTI_MACRO_RIPPLE_SLICE2:
		case LXS_MULTI_MACRO_RIPPLE_ADD4:
		case LXS_MULTI_MACRO_CARRY_SAVE_ROW4:
		case LXS_MULTI_MACRO_REDUCE_PROPAGATE4:
		case LXS_MULTI_MACRO_FULL_ADDER_CINV:
		case LXS_MULTI_MACRO_RIPPLE_SLICE2_CINV:
			return 1U;
		default:
			return 0U;
		}
	}

typedef struct lxs_trace_row
	{
	char circuit_name[MAX_PATH];
	uint32_t sample;
	uint32_t iteration;
	uint32_t cycle;
	uint64_t input_apply;
	uint64_t chunk_exec;
	uint64_t gate_eval;
	uint64_t dff_exec;
	uint64_t tick_count;
	uint64_t state_commit_count;
	double input_time_s;
	double comb_time_s;
	double output_capture_time_s;
	double dff_capture_time_s;
	double state_commit_time_s;
	double output_read_time_s;
	double cycle_time_s;
	} lxs_trace_row;

static double lxs_now_seconds(void)
	{
	LARGE_INTEGER freq;
	LARGE_INTEGER counter;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	return (double)counter.QuadPart / (double)freq.QuadPart;
	}

typedef struct lxs_result_list
	{
	lxs_bench_result *items;
	size_t count;
	size_t cap;
	} lxs_result_list;

static char* lxs_strdup_local(const char *s)
	{
	size_t len = strlen(s) + 1U;
	char *copy = malloc(len);
	if (!copy)
		{
		return NULL;
		}
	memcpy(copy, s, len);
	return copy;
	}

static int lxs_path_list_push(lxs_path_list *paths, const char *path)
	{
	if (paths->count == paths->cap)
		{
		size_t new_cap = paths->cap == 0 ? 16U : paths->cap * 2U;
		char **new_items = realloc(paths->items, new_cap * sizeof(char*));
		if (!new_items)
			{
			return 0;
			}
		paths->items = new_items;
		paths->cap = new_cap;
		}

	paths->items[paths->count] = lxs_strdup_local(path);
	if (!paths->items[paths->count])
		{
		return 0;
		}
	paths->count++;
	return 1;
	}

static void lxs_path_list_free(lxs_path_list *paths)
	{
	for (size_t i = 0; i < paths->count; ++i)
		{
		free(paths->items[i]);
		}
	free(paths->items);
	memset(paths, 0, sizeof(*paths));
	}

static int lxs_result_list_push(lxs_result_list *results, const lxs_bench_result *result)
	{
	if (results->count == results->cap)
		{
		size_t new_cap = results->cap == 0 ? 16U : results->cap * 2U;
		lxs_bench_result *new_items = realloc(results->items, new_cap * sizeof(lxs_bench_result));
		if (!new_items)
			{
			return 0;
			}
		results->items = new_items;
		results->cap = new_cap;
		}

	results->items[results->count++] = *result;
	return 1;
	}

static int lxs_has_suffix(const char *path, const char *suffix)
	{
	size_t path_len = strlen(path);
	size_t suffix_len = strlen(suffix);
	if (path_len < suffix_len)
		{
		return 0;
		}
	return _stricmp(path + path_len - suffix_len, suffix) == 0;
	}

static int lxs_is_supported_benchmark_file(const char *path)
	{
	return lxs_has_suffix(path, ".bench");
	}

static int lxs_collect_benchmarks(const char *path, lxs_path_list *paths)
	{
	DWORD attrs = GetFileAttributesA(path);
	if (attrs == INVALID_FILE_ATTRIBUTES)
		{
		return 0;
		}

	if ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
		return lxs_is_supported_benchmark_file(path) ? lxs_path_list_push(paths, path) : 1;
		}

	{
		char search_path[MAX_PATH * 4];
		struct _finddata_t fd;
		intptr_t handle;

	snprintf(search_path, sizeof(search_path), "%s\\*", path);
	handle = _findfirst(search_path, &fd);
	if (handle == -1)
		{
		return 0;
		}

	do
		{
			char full_path[MAX_PATH * 4];
		if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
			{
			continue;
			}

		snprintf(full_path, sizeof(full_path), "%s\\%s", path, fd.name);
		if (fd.attrib & _A_SUBDIR)
			{
			if (!lxs_collect_benchmarks(full_path, paths))
				{
				_findclose(handle);
				return 0;
				}
			}
		else if (lxs_is_supported_benchmark_file(full_path))
			{
			if (!lxs_path_list_push(paths, full_path))
				{
				_findclose(handle);
				return 0;
				}
			}
		}
	while (_findnext(handle, &fd) == 0);

	_findclose(handle);
	return 1;
	}
	}

static int __cdecl lxs_compare_path_strings(const void *lhs, const void *rhs)
	{
	const char *const *a = lhs;
	const char *const *b = rhs;
	return _stricmp(*a, *b);
	}

static void lxs_make_circuit_name(const char *root, const char *path, char *out_name, size_t out_name_size)
	{
	const char *name = path;
	size_t root_len = root ? strlen(root) : 0U;

	if (root_len > 0 && _strnicmp(path, root, root_len) == 0)
		{
		name = path + root_len;
		if (*name == '\\' || *name == '/')
			{
			name++;
			}
		if (*name == '\0')
			{
			const char *slash = strrchr(path, '\\');
			const char *alt_slash = strrchr(path, '/');
			if (alt_slash && (!slash || alt_slash > slash))
				{
				slash = alt_slash;
				}
			name = slash ? slash + 1 : path;
			}
		}

	snprintf(out_name, out_name_size, "%s", name);
	{
	char *dot = strrchr(out_name, '.');
	if (dot)
		{
		*dot = '\0';
		}
	}
	}

static uint64_t lxs_hash_seed(const char *text)
	{
	uint64_t hash = 1469598103934665603ULL;
	while (*text)
		{
		hash ^= (unsigned char)(*text++);
		hash *= 1099511628211ULL;
		}
	return hash;
	}

static uint64_t lxs_next_state(uint64_t state)
	{
	state ^= state << 13;
	state ^= state >> 7;
	state ^= state << 17;
	return state;
	}

static void lxs_fill_inputs(
	const char *path,
	uint32_t iteration,
	uint32_t cycle,
	uint32_t input_count,
	uint64_t *values,
	uint64_t *masks)
	{
	uint64_t state = lxs_hash_seed(path) ^ ((uint64_t)iteration << 32) ^ (uint64_t)cycle;
	for (uint32_t i = 0; i < input_count; ++i)
		{
		state = lxs_next_state(state + i + 1U);
		values[i] = (state & 1ULL) ? ~0ULL : 0ULL;
		masks[i] = 0ULL;
		}
	}

static const char* lxs_mode_name(lxs_bench_mode mode)
	{
	return mode == LXS_MODE_COLD ? "cold" : "continuous";
	}

static const char* lxs_sample_select_name(lxs_sample_select sample_select)
	{
	switch (sample_select)
		{
		case LXS_SAMPLE_MIN:
			return "min";
		case LXS_SAMPLE_MAX:
			return "max";
		default:
			return "median";
		}
	}

static const char* lxs_get_revision(void)
	{
	const char *revision = getenv("LXS_REVISION");
	return (revision && revision[0] != '\0') ? revision : "N/A";
	}

static FILE* lxs_open_record_file(const lxs_bench_options *options)
	{
	char full_path[MAX_PATH];
	const char *mode = options->overwrite_rule == LXS_OVERWRITE_TRUNCATE ? "w" : "a";

	if (!options->record_file_path || !options->record_file_name)
		{
		return NULL;
		}

	snprintf(full_path, sizeof(full_path), "%s\\%s", options->record_file_path, options->record_file_name);
	return fopen(full_path, mode);
	}

static void lxs_write_meta(FILE *stream, const lxs_bench_options *options)
	{
	time_t now = time(NULL);
	struct tm local_tm;
	char date_buf[32];
	char time_buf[32];

	localtime_s(&local_tm, &now);
	strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &local_tm);
	strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &local_tm);

	if (options->record_meta_head && options->record_meta_head[0] != '\0')
		{
		fprintf(stream, "# meta,%s\n", options->record_meta_head);
		}
	fprintf(stream, "# revision,%s\n", lxs_get_revision());
	fprintf(stream, "# date,%s\n", date_buf);
	fprintf(stream, "# time,%s\n", time_buf);
	fprintf(stream, "# mode,%s\n", lxs_mode_name(options->mode));
	fprintf(stream, "# samples,%u\n", options->sample_count);
	fprintf(stream, "# sample_select,%s\n", lxs_sample_select_name(options->sample_select));
	fprintf(stream, "# iterations,%u\n", options->iterations);
	fprintf(stream, "# cycles,%u\n", options->cycles);
	}

static void lxs_write_csv_header(FILE *stream)
	{
	fprintf(
		stream,
		"Circuit Name,Input Count,Output Count,Level Count,Span Count,Max Span Count,"
		"Input Apply,Chunk Exec,Gate Eval,DFF Exec,Tick Count,State Commit Count,"
		"Input Time (seconds),Comb Time (seconds),Output Capture Time (seconds),"
		"DFF Capture Time (seconds),State Commit Time (seconds),Output Read Time (seconds),"
		"Total Time (seconds),GEPS\n");
	}

static int __cdecl lxs_compare_u32_values(const void *lhs, const void *rhs)
	{
	const uint32_t a = *(const uint32_t*)lhs;
	const uint32_t b = *(const uint32_t*)rhs;

	if (a < b)
		{
		return -1;
		}

	if (a > b)
		{
		return 1;
		}

	return 0;
	}

static void lxs_build_plan_profile(
	const char *root,
	const char *path,
	const lxs_plan *plan,
	lxs_plan_profile *profile)
	{
	uint32_t *chunk_sizes = NULL;
	uint8_t *is_io_net = NULL;
	uint8_t *level_kind = NULL;
	uint8_t *level_has_arithmetic = NULL;
	uint32_t *level_chunk_counts = NULL;
	uint32_t *level_tiny_chunk_counts = NULL;
	uint64_t *level_primitive_gate_equiv = NULL;
	int32_t *first_arith_touch = NULL;
	int32_t *last_arith_touch = NULL;
	uint64_t arithmetic_input_span_sum = 0ULL;
	uint64_t arithmetic_touch_span_sum = 0ULL;
	uint64_t arithmetic_temp_lifetime_sum = 0ULL;

	memset(profile, 0, sizeof(*profile));
	lxs_make_circuit_name(root, path, profile->circuit_name, sizeof(profile->circuit_name));
	profile->recognition_mask = plan->recognition_mask;
	profile->recognition_mode = plan->recognition_mode;
	memcpy(
		profile->recognition_match_count,
		plan->recognition_match_count,
		sizeof(profile->recognition_match_count));
	memcpy(
		profile->recognition_node_reduction,
		plan->recognition_node_reduction,
		sizeof(profile->recognition_node_reduction));
	memcpy(
		profile->recognition_candidate_roots,
		plan->recognition_candidate_roots,
		sizeof(profile->recognition_candidate_roots));
	memcpy(
		profile->recognition_nodes_visited,
		plan->recognition_nodes_visited,
		sizeof(profile->recognition_nodes_visited));
	memcpy(
		profile->recognition_max_depth,
		plan->recognition_max_depth,
		sizeof(profile->recognition_max_depth));
	memcpy(
		profile->recognition_abort_count,
		plan->recognition_abort_count,
		sizeof(profile->recognition_abort_count));
	memcpy(
		profile->recognition_time_us,
		plan->recognition_time_us,
		sizeof(profile->recognition_time_us));
	profile->comb_gate_count = plan->comb_gate_count;
	profile->dff_count = plan->state.count;
	profile->chunk_count = plan->span_count;
	profile->primitive_step_count = plan->span_count;
	profile->macro_step_count = plan->macro_count + plan->multi_macro_count + plan->standard_mux_count;
	profile->total_step_count = profile->primitive_step_count + profile->macro_step_count;

	if (plan->span_count > 0U)
		{
		chunk_sizes = calloc(plan->span_count, sizeof(uint32_t));
		}
	if (plan->level_count > 0U)
		{
		level_kind = calloc(plan->level_count, sizeof(uint8_t));
		level_has_arithmetic = calloc(plan->level_count, sizeof(uint8_t));
		level_chunk_counts = calloc(plan->level_count, sizeof(uint32_t));
		level_tiny_chunk_counts = calloc(plan->level_count, sizeof(uint32_t));
		level_primitive_gate_equiv = calloc(plan->level_count, sizeof(uint64_t));
		}
	if (plan->net_count > 0U)
		{
		is_io_net = calloc(plan->net_count, sizeof(uint8_t));
		first_arith_touch = malloc((size_t)plan->net_count * sizeof(int32_t));
		last_arith_touch = malloc((size_t)plan->net_count * sizeof(int32_t));
		if (first_arith_touch && last_arith_touch)
			{
			for (uint32_t i = 0; i < plan->net_count; ++i)
				{
				first_arith_touch[i] = -1;
				last_arith_touch[i] = -1;
				}
			}
		if (is_io_net)
			{
			for (uint32_t i = 0; i < plan->inputs.count; ++i)
				{
				is_io_net[plan->inputs.net_ids[i]] = 1U;
				}
			for (uint32_t i = 0; i < plan->outputs.count; ++i)
				{
				is_io_net[plan->outputs.net_ids[i]] = 1U;
				}
			}
		}

	for (uint32_t i = 0; i < plan->span_count; ++i)
		{
		const lxs_chunk_plan *chunk = &plan->chunks[i];
		uint32_t size = chunk->count;

		if (chunk_sizes)
			{
			chunk_sizes[i] = size;
			}

		profile->primitive_gate_equiv += chunk->gate_equiv_count;

		profile->gate_type_count[chunk->type] += size;
		if (size > profile->max_chunk_size)
			{
			profile->max_chunk_size = size;
			}

		if (size <= 2U)
			{
			profile->tiny_chunk_count++;
			}

		if (size <= 4U)
			{
			profile->small_chunk_count++;
			}

		if (size >= 32U)
			{
			profile->large_chunk_count++;
			}
		}

	profile->unary_gate_count =
		profile->gate_type_count[LXS_GATE_NOT] +
		profile->gate_type_count[LXS_GATE_BUF];
	profile->binary_gate_count = plan->comb_gate_count - profile->unary_gate_count;
	profile->mean_chunk_size =
		plan->span_count > 0U ? (double)plan->comb_gate_count / (double)plan->span_count : 0.0;
	for (uint32_t i = 0; i < plan->macro_count; ++i)
		{
		profile->macro_gate_equiv += plan->macros[i].gate_equiv_count;
		}
	for (uint32_t i = 0; i < plan->multi_macro_count; ++i)
		{
		profile->macro_gate_equiv += plan->multi_macros[i].gate_equiv_count;
		if (lxs_is_arithmetic_multi_macro_type(plan->multi_macros[i].type))
			{
			const lxs_multi_macro_plan *macro = &plan->multi_macros[i];
			uint32_t min_input = UINT32_MAX;
			uint32_t max_input = 0U;
			uint32_t min_touch = UINT32_MAX;
			uint32_t max_touch = 0U;
			uint32_t arithmetic_step = profile->arithmetic_multi_macro_count;

			for (uint32_t j = 0; j < macro->input_count; ++j)
				{
				uint32_t net_id = macro->inputs[j];
				if (net_id < min_input)
					{
					min_input = net_id;
					}
				if (net_id > max_input)
					{
					max_input = net_id;
					}
				if (net_id < min_touch)
					{
					min_touch = net_id;
					}
				if (net_id > max_touch)
					{
					max_touch = net_id;
					}
				if (first_arith_touch && last_arith_touch && net_id < plan->net_count)
					{
					if (first_arith_touch[net_id] < 0)
						{
						first_arith_touch[net_id] = (int32_t)arithmetic_step;
						}
					last_arith_touch[net_id] = (int32_t)arithmetic_step;
					}
				}
			for (uint32_t j = 0; j < macro->output_count; ++j)
				{
				uint32_t net_id = macro->outputs[j];
				if (net_id < min_touch)
					{
					min_touch = net_id;
					}
				if (net_id > max_touch)
					{
					max_touch = net_id;
					}
				if (first_arith_touch && last_arith_touch && net_id < plan->net_count)
					{
					if (first_arith_touch[net_id] < 0)
						{
						first_arith_touch[net_id] = (int32_t)arithmetic_step;
						}
					last_arith_touch[net_id] = (int32_t)arithmetic_step;
					}
				}

			if (macro->input_count > 0U)
				{
				uint32_t input_span = max_input - min_input;
				arithmetic_input_span_sum += input_span;
				if (input_span > profile->arithmetic_max_input_span)
					{
					profile->arithmetic_max_input_span = input_span;
					}
				}
			if (macro->input_count > 0U || macro->output_count > 0U)
				{
				uint32_t touch_span = max_touch - min_touch;
				arithmetic_touch_span_sum += touch_span;
				if (touch_span > profile->arithmetic_max_touch_span)
					{
					profile->arithmetic_max_touch_span = touch_span;
					}
				}
			profile->arithmetic_multi_macro_count++;
			}
		}
	for (uint32_t i = 0; i < plan->standard_mux_count; ++i)
		{
		profile->macro_gate_equiv += plan->standard_muxes[i].gate_equiv_count;
		}
	profile->total_gate_equiv = profile->primitive_gate_equiv + profile->macro_gate_equiv;
	if (profile->total_step_count > 0U)
		{
		profile->macro_step_share =
			(double)profile->macro_step_count / (double)profile->total_step_count;
		}
	if (profile->total_gate_equiv > 0U)
		{
		profile->absorbed_work_share =
			(double)profile->macro_gate_equiv / (double)profile->total_gate_equiv;
		}
	for (uint32_t family = 0; family < LXS_RECOGNITION_FAMILY_COUNT; ++family)
		{
		profile->recognition_gate_equiv[family] =
			profile->recognition_match_count[family] + profile->recognition_node_reduction[family];
		if (profile->total_gate_equiv > 0U)
			{
			profile->recognition_absorbed_work_share[family] =
				(double)profile->recognition_gate_equiv[family] / (double)profile->total_gate_equiv;
			}
		}

	for (uint32_t level = 0; level < plan->level_count; ++level)
		{
		const lxs_level_plan *level_plan = &plan->levels[level];
		uint32_t level_gate_count = 0U;
		uint32_t level_tiny_chunk_count = 0U;
		uint32_t level_arithmetic_multi_count = 0U;
		uint64_t level_primitive_gate_equiv_total = 0ULL;
		uint8_t has_primitive = level_plan->chunk_count > 0U ? 1U : 0U;
		uint8_t kind = 0U;

		if (level_plan->chunk_count == 1U)
			{
			profile->single_chunk_level_count++;
			}

		if (level_plan->chunk_count > profile->max_level_chunk_count)
			{
			profile->max_level_chunk_count = level_plan->chunk_count;
			}

		for (uint32_t i = 0; i < level_plan->chunk_count; ++i)
			{
			const lxs_chunk_plan *chunk = &plan->chunks[level_plan->chunk_start + i];
			level_gate_count += chunk->count;
			level_primitive_gate_equiv_total += chunk->gate_equiv_count;
			if (chunk->count <= 2U)
				{
				level_tiny_chunk_count++;
				}
			}

		for (uint32_t i = 0; i < level_plan->multi_macro_count; ++i)
			{
			const lxs_multi_macro_plan *macro = &plan->multi_macros[level_plan->multi_macro_start + i];
			if (lxs_is_arithmetic_multi_macro_type(macro->type))
				{
				level_arithmetic_multi_count++;
				}
			}

		if (level_gate_count > profile->max_level_gate_count)
			{
			profile->max_level_gate_count = level_gate_count;
			}

		if (level_arithmetic_multi_count > 0U)
			{
			profile->arithmetic_level_count++;
			}

		if (has_primitive && level_arithmetic_multi_count > 0U)
			{
			profile->mixed_level_count++;
			kind = 3U;
			}
		else if (has_primitive && level_plan->macro_count == 0U &&
			level_plan->multi_macro_count == 0U && level_plan->standard_mux_count == 0U &&
			level_plan->functional_region_count == 0U)
			{
			profile->primitive_only_level_count++;
			kind = 1U;
			}
		else if (!has_primitive && level_arithmetic_multi_count > 0U &&
			level_plan->macro_count == 0U &&
			level_plan->standard_mux_count == 0U &&
			level_plan->functional_region_count == 0U &&
			level_arithmetic_multi_count == level_plan->multi_macro_count)
			{
			profile->arithmetic_only_level_count++;
			kind = 2U;
			}
		else if (has_primitive || level_plan->macro_count > 0U ||
			level_plan->multi_macro_count > 0U || level_plan->standard_mux_count > 0U ||
			level_plan->functional_region_count > 0U)
			{
			kind = 4U;
			}

		if (level_kind)
			{
			level_kind[level] = kind;
			}
		if (level_has_arithmetic)
			{
			level_has_arithmetic[level] = level_arithmetic_multi_count > 0U ? 1U : 0U;
			}
		if (level_chunk_counts)
			{
			level_chunk_counts[level] = level_plan->chunk_count;
			}
		if (level_tiny_chunk_counts)
			{
			level_tiny_chunk_counts[level] = level_tiny_chunk_count;
			}
		if (level_primitive_gate_equiv)
			{
			level_primitive_gate_equiv[level] = level_primitive_gate_equiv_total;
			}
		}

	for (uint32_t level = 0; level < plan->level_count; ++level)
		{
		uint8_t kind = level_kind ? level_kind[level] : 0U;
		uint8_t prev_kind = (level > 0U && level_kind) ? level_kind[level - 1U] : 0U;
		uint8_t prev_has_arith = (level > 0U && level_has_arithmetic) ? level_has_arithmetic[level - 1U] : 0U;
		uint8_t next_has_arith = (level + 1U < plan->level_count && level_has_arithmetic) ? level_has_arithmetic[level + 1U] : 0U;

		if (level > 0U && kind != 0U && prev_kind != 0U && kind != prev_kind)
			{
			profile->phase_transition_count++;
			if (prev_kind == 1U && kind == 2U)
				{
				profile->primitive_to_arithmetic_transition_count++;
				}
			else if (prev_kind == 2U && kind == 1U)
				{
				profile->arithmetic_to_primitive_transition_count++;
				}
			}

		if (kind == 1U && (prev_has_arith || next_has_arith))
			{
			profile->arithmetic_adjacent_primitive_level_count++;
			if (level_chunk_counts)
				{
				profile->arithmetic_adjacent_primitive_chunk_count += level_chunk_counts[level];
				}
			if (level_tiny_chunk_counts)
				{
				profile->arithmetic_adjacent_primitive_tiny_chunk_count += level_tiny_chunk_counts[level];
				}
			if (level_primitive_gate_equiv)
				{
				profile->arithmetic_adjacent_primitive_gate_equiv += level_primitive_gate_equiv[level];
				}
			}
		}

	if (profile->arithmetic_multi_macro_count > 0U)
		{
		profile->arithmetic_mean_input_span =
			(double)arithmetic_input_span_sum / (double)profile->arithmetic_multi_macro_count;
		profile->arithmetic_mean_touch_span =
			(double)arithmetic_touch_span_sum / (double)profile->arithmetic_multi_macro_count;
		}
	if (first_arith_touch && last_arith_touch && is_io_net)
		{
		for (uint32_t i = 0; i < plan->net_count; ++i)
			{
			if (is_io_net[i] || first_arith_touch[i] < 0 || last_arith_touch[i] < 0)
				{
				continue;
				}
			{
			uint32_t lifetime = (uint32_t)(last_arith_touch[i] - first_arith_touch[i] + 1);
			arithmetic_temp_lifetime_sum += lifetime;
			profile->arithmetic_temp_net_count++;
			if (lifetime > profile->arithmetic_temp_max_lifetime)
				{
				profile->arithmetic_temp_max_lifetime = lifetime;
				}
			}
			}
		if (profile->arithmetic_temp_net_count > 0U)
			{
			profile->arithmetic_mean_temp_lifetime =
				(double)arithmetic_temp_lifetime_sum / (double)profile->arithmetic_temp_net_count;
			}
		}

	if (chunk_sizes)
		{
		qsort(chunk_sizes, plan->span_count, sizeof(uint32_t), lxs_compare_u32_values);
		profile->median_chunk_size = chunk_sizes[plan->span_count / 2U];
		free(chunk_sizes);
		}
	free(level_kind);
	free(level_has_arithmetic);
	free(level_chunk_counts);
	free(level_tiny_chunk_counts);
	free(level_primitive_gate_equiv);
	free(is_io_net);
	free(first_arith_touch);
	free(last_arith_touch);
	}

static void lxs_write_plan_profile(FILE *stream, const lxs_plan_profile *profile)
	{
	fprintf(
		stream,
		"# struct,%s,comb_gates=%u,dff=%u,chunks=%u,primitive_steps=%u,macro_steps=%u,total_steps=%u,"
		"macro_step_share=%.6f,primitive_gate_equiv=%llu,macro_gate_equiv=%llu,total_gate_equiv=%llu,"
		"absorbed_work_share=%.6f,max_chunk=%u,median_chunk=%u,mean_chunk=%.3f,"
		"tiny_chunks=%u,small_chunks=%u,large_chunks=%u,unary_gates=%u,binary_gates=%u,"
		"max_level_gates=%u,max_level_chunks=%u,single_chunk_levels=%u,"
		"primitive_only_levels=%u,arithmetic_only_levels=%u,mixed_levels=%u,arith_levels=%u,"
		"phase_transitions=%u,primitive_to_arith=%u,arith_to_primitive=%u,"
		"arith_adj_prim_levels=%u,arith_adj_prim_chunks=%u,arith_adj_prim_tiny_chunks=%u,"
		"arith_adj_prim_gate_equiv=%llu,"
		"arith_multi=%u,arith_mean_input_span=%.3f,arith_max_input_span=%u,"
		"arith_mean_touch_span=%.3f,arith_max_touch_span=%u,"
		"arith_temp_nets=%u,arith_mean_temp_lifetime=%.3f,arith_max_temp_lifetime=%u,"
		"and=%u,or=%u,xor=%u,tri=%u,not=%u,nand=%u,nor=%u,xnor=%u,buf=%u\n",
		profile->circuit_name,
		profile->comb_gate_count,
		profile->dff_count,
		profile->chunk_count,
		profile->primitive_step_count,
		profile->macro_step_count,
		profile->total_step_count,
		profile->macro_step_share,
		(unsigned long long)profile->primitive_gate_equiv,
		(unsigned long long)profile->macro_gate_equiv,
		(unsigned long long)profile->total_gate_equiv,
		profile->absorbed_work_share,
		profile->max_chunk_size,
		profile->median_chunk_size,
		profile->mean_chunk_size,
		profile->tiny_chunk_count,
		profile->small_chunk_count,
		profile->large_chunk_count,
		profile->unary_gate_count,
		profile->binary_gate_count,
		profile->max_level_gate_count,
		profile->max_level_chunk_count,
		profile->single_chunk_level_count,
		profile->primitive_only_level_count,
		profile->arithmetic_only_level_count,
		profile->mixed_level_count,
		profile->arithmetic_level_count,
		profile->phase_transition_count,
		profile->primitive_to_arithmetic_transition_count,
		profile->arithmetic_to_primitive_transition_count,
		profile->arithmetic_adjacent_primitive_level_count,
		profile->arithmetic_adjacent_primitive_chunk_count,
		profile->arithmetic_adjacent_primitive_tiny_chunk_count,
		(unsigned long long)profile->arithmetic_adjacent_primitive_gate_equiv,
		profile->arithmetic_multi_macro_count,
		profile->arithmetic_mean_input_span,
		profile->arithmetic_max_input_span,
		profile->arithmetic_mean_touch_span,
		profile->arithmetic_max_touch_span,
		profile->arithmetic_temp_net_count,
		profile->arithmetic_mean_temp_lifetime,
		profile->arithmetic_temp_max_lifetime,
		profile->gate_type_count[LXS_GATE_AND],
		profile->gate_type_count[LXS_GATE_OR],
		profile->gate_type_count[LXS_GATE_XOR],
		profile->gate_type_count[LXS_GATE_TRI],
		profile->gate_type_count[LXS_GATE_NOT],
		profile->gate_type_count[LXS_GATE_NAND],
		profile->gate_type_count[LXS_GATE_NOR],
		profile->gate_type_count[LXS_GATE_XNOR],
		profile->gate_type_count[LXS_GATE_BUF]);

	fprintf(
		stream,
		"# recognition,%s,mask=%u,mode=%u,parity_matches=%u,parity_node_reduction=%u,"
		"parity_gate_equiv=%u,parity_absorbed_work_share=%.6f,"
		"shared_xor_matches=%u,shared_xor_node_reduction=%u,"
		"shared_xor_gate_equiv=%u,shared_xor_absorbed_work_share=%.6f,"
		"shared_and_matches=%u,shared_and_node_reduction=%u,"
		"shared_and_gate_equiv=%u,shared_and_absorbed_work_share=%.6f,"
		"compare_matches=%u,compare_node_reduction=%u,"
		"compare_gate_equiv=%u,compare_absorbed_work_share=%.6f,"
		"register_en_matches=%u,register_en_node_reduction=%u,"
		"register_en_gate_equiv=%u,register_en_absorbed_work_share=%.6f,"
		"arithmetic_matches=%u,arithmetic_node_reduction=%u,"
		"arithmetic_gate_equiv=%u,arithmetic_absorbed_work_share=%.6f,"
		"control_matches=%u,control_node_reduction=%u,"
		"control_gate_equiv=%u,control_absorbed_work_share=%.6f,"
		"functional_matches=%u,functional_node_reduction=%u,"
		"functional_gate_equiv=%u,functional_absorbed_work_share=%.6f\n",
		profile->circuit_name,
		profile->recognition_mask,
		profile->recognition_mode,
		profile->recognition_match_count[LXS_RECOGNITION_FAMILY_PARITY],
		profile->recognition_node_reduction[LXS_RECOGNITION_FAMILY_PARITY],
		profile->recognition_gate_equiv[LXS_RECOGNITION_FAMILY_PARITY],
		profile->recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_PARITY],
		profile->recognition_match_count[LXS_RECOGNITION_FAMILY_SHARED_XOR],
		profile->recognition_node_reduction[LXS_RECOGNITION_FAMILY_SHARED_XOR],
		profile->recognition_gate_equiv[LXS_RECOGNITION_FAMILY_SHARED_XOR],
		profile->recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_SHARED_XOR],
		profile->recognition_match_count[LXS_RECOGNITION_FAMILY_SHARED_AND],
		profile->recognition_node_reduction[LXS_RECOGNITION_FAMILY_SHARED_AND],
		profile->recognition_gate_equiv[LXS_RECOGNITION_FAMILY_SHARED_AND],
		profile->recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_SHARED_AND],
		profile->recognition_match_count[LXS_RECOGNITION_FAMILY_COMPARE],
		profile->recognition_node_reduction[LXS_RECOGNITION_FAMILY_COMPARE],
		profile->recognition_gate_equiv[LXS_RECOGNITION_FAMILY_COMPARE],
		profile->recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_COMPARE],
		profile->recognition_match_count[LXS_RECOGNITION_FAMILY_REGISTER_EN],
		profile->recognition_node_reduction[LXS_RECOGNITION_FAMILY_REGISTER_EN],
		profile->recognition_gate_equiv[LXS_RECOGNITION_FAMILY_REGISTER_EN],
		profile->recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_REGISTER_EN],
		profile->recognition_match_count[LXS_RECOGNITION_FAMILY_ARITHMETIC],
		profile->recognition_node_reduction[LXS_RECOGNITION_FAMILY_ARITHMETIC],
		profile->recognition_gate_equiv[LXS_RECOGNITION_FAMILY_ARITHMETIC],
		profile->recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_ARITHMETIC],
		profile->recognition_match_count[LXS_RECOGNITION_FAMILY_CONTROL],
		profile->recognition_node_reduction[LXS_RECOGNITION_FAMILY_CONTROL],
		profile->recognition_gate_equiv[LXS_RECOGNITION_FAMILY_CONTROL],
		profile->recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_CONTROL],
		profile->recognition_match_count[LXS_RECOGNITION_FAMILY_FUNCTIONAL],
		profile->recognition_node_reduction[LXS_RECOGNITION_FAMILY_FUNCTIONAL],
		profile->recognition_gate_equiv[LXS_RECOGNITION_FAMILY_FUNCTIONAL],
		profile->recognition_absorbed_work_share[LXS_RECOGNITION_FAMILY_FUNCTIONAL]);

	for (uint32_t family = 0; family < LXS_RECOGNITION_FAMILY_COUNT; ++family)
		{
		const char *family_name = "";
		switch (family)
			{
			case LXS_RECOGNITION_FAMILY_PARITY:
				family_name = "parity";
				break;
			case LXS_RECOGNITION_FAMILY_SHARED_XOR:
				family_name = "shared_xor";
				break;
			case LXS_RECOGNITION_FAMILY_SHARED_AND:
				family_name = "shared_and";
				break;
			case LXS_RECOGNITION_FAMILY_COMPARE:
				family_name = "compare";
				break;
			case LXS_RECOGNITION_FAMILY_REGISTER_EN:
				family_name = "register_en";
				break;
			case LXS_RECOGNITION_FAMILY_ARITHMETIC:
				family_name = "arithmetic";
				break;
			case LXS_RECOGNITION_FAMILY_CONTROL:
				family_name = "control";
				break;
			case LXS_RECOGNITION_FAMILY_FUNCTIONAL:
				family_name = "functional";
				break;
			default:
				family_name = "unknown";
				break;
			}

		fprintf(
			stream,
			"\n# recognition_telemetry,%s,family=%s,candidate_roots=%llu,nodes_visited=%llu,max_depth=%u,"
			"abort_shape=%llu,abort_fanout=%llu,abort_branch=%llu,abort_depth=%llu,"
			"abort_node_budget=%llu,abort_overlap=%llu,time_us=%llu",
			profile->circuit_name,
			family_name,
			(unsigned long long)profile->recognition_candidate_roots[family],
			(unsigned long long)profile->recognition_nodes_visited[family],
			profile->recognition_max_depth[family],
			(unsigned long long)profile->recognition_abort_count[family][LXS_RECOGNITION_ABORT_SHAPE],
			(unsigned long long)profile->recognition_abort_count[family][LXS_RECOGNITION_ABORT_FANOUT],
			(unsigned long long)profile->recognition_abort_count[family][LXS_RECOGNITION_ABORT_BRANCH],
			(unsigned long long)profile->recognition_abort_count[family][LXS_RECOGNITION_ABORT_DEPTH],
			(unsigned long long)profile->recognition_abort_count[family][LXS_RECOGNITION_ABORT_NODE_BUDGET],
			(unsigned long long)profile->recognition_abort_count[family][LXS_RECOGNITION_ABORT_OVERLAP],
			(unsigned long long)profile->recognition_time_us[family]);
		}
	fprintf(stream, "\n");
	}

static void lxs_write_trace_header(FILE *stream)
	{
	fprintf(
		stream,
		"Trace Circuit,Sample,Iteration,Cycle,Input Apply,Chunk Exec,Gate Eval,DFF Exec,Tick Count,State Commit Count,"
		"Input Time (seconds),Comb Time (seconds),Output Capture Time (seconds),"
		"DFF Capture Time (seconds),State Commit Time (seconds),Output Read Time (seconds),Cycle Time (seconds)\n");
	}

static void lxs_write_result(FILE *stream, const lxs_bench_result *result)
	{
	fprintf(
		stream,
		"%s,%u,%u,%u,%u,%u,%llu,%llu,%llu,%llu,%llu,%llu,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
		result->circuit_name,
		result->input_count,
		result->output_count,
		result->level_count,
		result->span_count,
		result->max_span_count,
		(unsigned long long)result->input_apply,
		(unsigned long long)result->chunk_exec,
		(unsigned long long)result->gate_eval,
		(unsigned long long)result->dff_exec,
		(unsigned long long)result->tick_count,
		(unsigned long long)result->state_commit_count,
		result->input_time_s,
		result->comb_time_s,
		result->output_capture_time_s,
		result->dff_capture_time_s,
		result->state_commit_time_s,
		result->output_read_time_s,
		result->total_time_s,
		result->geps);
	}

static void lxs_write_trace_row(FILE *stream, const lxs_trace_row *row)
	{
	fprintf(
		stream,
		"%s,%u,%u,%u,%llu,%llu,%llu,%llu,%llu,%llu,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f\n",
		row->circuit_name,
		row->sample,
		row->iteration,
		row->cycle,
		(unsigned long long)row->input_apply,
		(unsigned long long)row->chunk_exec,
		(unsigned long long)row->gate_eval,
		(unsigned long long)row->dff_exec,
		(unsigned long long)row->tick_count,
		(unsigned long long)row->state_commit_count,
		row->input_time_s,
		row->comb_time_s,
		row->output_capture_time_s,
		row->dff_capture_time_s,
		row->state_commit_time_s,
		row->output_read_time_s,
		row->cycle_time_s);
	}

static void lxs_write_summary(FILE *stream, const lxs_result_list *results)
	{
	uint64_t total_input_apply = 0;
	uint64_t total_chunk_exec = 0;
	uint64_t total_gate_eval = 0;
	uint64_t total_dff_exec = 0;
	uint64_t total_tick_count = 0;
	uint64_t total_state_commit_count = 0;
	double total_input_time = 0.0;
	double total_comb_time = 0.0;
	double total_output_capture_time = 0.0;
	double total_dff_capture_time = 0.0;
	double total_state_commit_time = 0.0;
	double total_output_read_time = 0.0;
	double total_time = 0.0;
	double mean_geps = 0.0;

	for (size_t i = 0; i < results->count; ++i)
		{
		total_input_apply += results->items[i].input_apply;
		total_chunk_exec += results->items[i].chunk_exec;
		total_gate_eval += results->items[i].gate_eval;
		total_dff_exec += results->items[i].dff_exec;
		total_tick_count += results->items[i].tick_count;
		total_state_commit_count += results->items[i].state_commit_count;
		total_input_time += results->items[i].input_time_s;
		total_comb_time += results->items[i].comb_time_s;
		total_output_capture_time += results->items[i].output_capture_time_s;
		total_dff_capture_time += results->items[i].dff_capture_time_s;
		total_state_commit_time += results->items[i].state_commit_time_s;
		total_output_read_time += results->items[i].output_read_time_s;
		total_time += results->items[i].total_time_s;
		mean_geps += results->items[i].geps;
		}

	if (results->count > 0)
		{
		mean_geps /= (double)results->count;
		}

	fprintf(
		stream,
		"TOTAL,N/A,N/A,N/A,N/A,N/A,%llu,%llu,%llu,%llu,%llu,%llu,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
		(unsigned long long)total_input_apply,
		(unsigned long long)total_chunk_exec,
		(unsigned long long)total_gate_eval,
		(unsigned long long)total_dff_exec,
		(unsigned long long)total_tick_count,
		(unsigned long long)total_state_commit_count,
		total_input_time,
		total_comb_time,
		total_output_capture_time,
		total_dff_capture_time,
		total_state_commit_time,
		total_output_read_time,
		total_time,
		total_time > 0.0 ? (double)total_gate_eval / total_time : 0.0);

	fprintf(stream, "AVERAGE,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,N/A,%.6f\n", mean_geps);
	}

static int __cdecl lxs_compare_bench_result_time(const void *lhs, const void *rhs)
	{
	const lxs_bench_result *a = (const lxs_bench_result*)lhs;
	const lxs_bench_result *b = (const lxs_bench_result*)rhs;
	if (a->total_time_s < b->total_time_s)
		{
		return -1;
		}
	if (a->total_time_s > b->total_time_s)
		{
		return 1;
		}
	return 0;
	}

static int lxs_select_sample_result(
	const lxs_bench_result *samples,
	uint32_t sample_count,
	lxs_sample_select sample_select,
	lxs_bench_result *selected)
	{
	lxs_bench_result *ordered;
	uint32_t selected_index = 0;

	ordered = calloc(sample_count, sizeof(lxs_bench_result));
	if (!ordered)
		{
		return 0;
		}

	memcpy(ordered, samples, (size_t)sample_count * sizeof(lxs_bench_result));
	qsort(ordered, sample_count, sizeof(lxs_bench_result), lxs_compare_bench_result_time);

	if (sample_select == LXS_SAMPLE_MIN)
		{
		selected_index = 0U;
		}
	else if (sample_select == LXS_SAMPLE_MAX)
		{
		selected_index = sample_count - 1U;
		}
	else
		{
		selected_index = sample_count / 2U;
		}

	*selected = ordered[selected_index];
	free(ordered);
	return 1;
	}

static int lxs_run_benchmark_sample(
	const char *root,
	const char *path,
	const lxs_bench_options *options,
	FILE *trace_stdout,
	FILE *trace_file,
	uint32_t sample_index,
	lxs_bench_result *result)
	{
	lxs_netlist *nl = NULL;
	lxs_plan *plan = NULL;
	lxs_engine_ctx ctx;
	uint64_t *input_values = NULL;
	uint64_t *input_masks = NULL;
	uint64_t *output_values = NULL;
	uint64_t *output_masks = NULL;
	volatile uint64_t output_sink = 0ULL;
	double measured_total_s = 0.0;
	lxs_probes probes;
	int ok = 0;

	memset(&ctx, 0, sizeof(ctx));
	memset(result, 0, sizeof(*result));
	lxs_make_circuit_name(root, path, result->circuit_name, sizeof(result->circuit_name));

	nl = lxs_load_iscas(path);
	if (!nl)
		{
		goto cleanup;
		}

	plan = lxs_compile_to_plan(nl);
	if (!plan)
		{
		goto cleanup;
		}

	if (!lxs_init_engine(&ctx, plan))
		{
		goto cleanup;
		}

	input_values = calloc(plan->inputs.count, sizeof(uint64_t));
	input_masks = calloc(plan->inputs.count, sizeof(uint64_t));
	output_values = calloc(plan->outputs.count, sizeof(uint64_t));
	output_masks = calloc(plan->outputs.count, sizeof(uint64_t));
	if ((plan->inputs.count > 0 && (!input_values || !input_masks)) ||
		(plan->outputs.count > 0 && (!output_values || !output_masks)))
		{
		goto cleanup;
		}

	for (uint32_t iteration = 0; iteration < options->iterations; ++iteration)
		{
		if (options->mode == LXS_MODE_COLD)
			{
			lxs_reset_engine(&ctx, plan);
			}

		for (uint32_t cycle = 0; cycle < options->cycles; ++cycle)
			{
			lxs_probes before;
			lxs_probes after;
			double phase_start_s;
			double cycle_start_s;
			double cycle_end_s;
			double input_phase_s = 0.0;
			double comb_phase_s = 0.0;
			double output_capture_phase_s = 0.0;
			double dff_capture_phase_s = 0.0;
			double state_commit_phase_s = 0.0;
			double output_read_phase_s = 0.0;

			before = lxs_get_probes(&ctx);
			lxs_fill_inputs(path, iteration, cycle, plan->inputs.count, input_values, input_masks);
			cycle_start_s = lxs_now_seconds();
			if (options->phase_profile)
				{
				phase_start_s = lxs_now_seconds();
				lxs_apply_inputs(&ctx, plan, input_values, input_masks);
				input_phase_s = lxs_now_seconds() - phase_start_s;
				result->input_time_s += input_phase_s;

				lxs_begin_tick(&ctx);

				phase_start_s = lxs_now_seconds();
				lxs_execute_levels(&ctx, plan);
				comb_phase_s = lxs_now_seconds() - phase_start_s;
				result->comb_time_s += comb_phase_s;

				phase_start_s = lxs_now_seconds();
				lxs_capture_outputs(&ctx, plan);
				output_capture_phase_s = lxs_now_seconds() - phase_start_s;
				result->output_capture_time_s += output_capture_phase_s;

				phase_start_s = lxs_now_seconds();
				lxs_capture_next_state(&ctx, plan);
				dff_capture_phase_s = lxs_now_seconds() - phase_start_s;
				result->dff_capture_time_s += dff_capture_phase_s;

				phase_start_s = lxs_now_seconds();
				lxs_commit_state(&ctx, plan);
				state_commit_phase_s = lxs_now_seconds() - phase_start_s;
				result->state_commit_time_s += state_commit_phase_s;

				phase_start_s = lxs_now_seconds();
				lxs_read_outputs(&ctx, plan, output_values, output_masks);
				output_read_phase_s = lxs_now_seconds() - phase_start_s;
				result->output_read_time_s += output_read_phase_s;
				cycle_end_s = lxs_now_seconds();
				}
			else
				{
				lxs_apply_inputs(&ctx, plan, input_values, input_masks);
				lxs_execute_plan(&ctx, plan);
				lxs_read_outputs(&ctx, plan, output_values, output_masks);
				cycle_end_s = lxs_now_seconds();
				}
			measured_total_s += cycle_end_s - cycle_start_s;
			if (plan->outputs.count > 0)
				{
				output_sink ^= output_values[0] ^ output_masks[0];
				}
			after = lxs_get_probes(&ctx);

			if (options->record_mode == LXS_RECORD_TRACE &&
				options->record_interval > 0 &&
				(((cycle + 1U) % options->record_interval) == 0U))
				{
				lxs_trace_row row;
				memset(&row, 0, sizeof(row));
				lxs_make_circuit_name(root, path, row.circuit_name, sizeof(row.circuit_name));
				row.sample = sample_index + 1U;
				row.iteration = iteration;
				row.cycle = cycle + 1U;
				row.input_apply = after.input_apply - before.input_apply;
				row.chunk_exec = after.chunk_exec - before.chunk_exec;
				row.gate_eval = after.gate_eval - before.gate_eval;
				row.dff_exec = after.dff_exec - before.dff_exec;
				row.tick_count = after.tick_count - before.tick_count;
				row.state_commit_count = after.state_commit_count - before.state_commit_count;
				row.input_time_s = input_phase_s;
				row.comb_time_s = comb_phase_s;
				row.output_capture_time_s = output_capture_phase_s;
				row.dff_capture_time_s = dff_capture_phase_s;
				row.state_commit_time_s = state_commit_phase_s;
				row.output_read_time_s = output_read_phase_s;
				row.cycle_time_s = cycle_end_s - cycle_start_s;
				if (trace_stdout)
					{
					lxs_write_trace_row(trace_stdout, &row);
					}
				if (trace_file)
					{
					lxs_write_trace_row(trace_file, &row);
					}
				}
			}
		}

	probes = lxs_get_probes(&ctx);
	result->input_count = plan->inputs.count;
	result->output_count = plan->outputs.count;
	result->level_count = plan->level_count;
	result->span_count = plan->span_count;
	result->max_span_count = plan->max_span_count;
	result->input_apply = probes.input_apply;
	result->chunk_exec = probes.chunk_exec;
	result->gate_eval = probes.gate_eval;
	result->dff_exec = probes.dff_exec;
	result->tick_count = probes.tick_count;
	result->state_commit_count = probes.state_commit_count;
	result->total_time_s = measured_total_s;
	result->geps = result->total_time_s > 0.0 ? (double)result->gate_eval / result->total_time_s : 0.0;
	if (output_sink == 0xFFFFFFFFFFFFFFFFULL)
		{
		result->geps += 0.0;
		}
	ok = 1;

cleanup:
	free(input_values);
	free(input_masks);
	free(output_values);
	free(output_masks);
	lxs_free_engine(&ctx);
	lxs_free_plan(plan);
	lxs_free_netlist(nl);
	return ok;
	}

static int lxs_run_benchmark(
	const char *root,
	const char *path,
	const lxs_bench_options *options,
	FILE *trace_stdout,
	FILE *trace_file,
	lxs_bench_result *result)
	{
	lxs_bench_result *samples;
	int ok = 0;

	samples = calloc(options->sample_count, sizeof(lxs_bench_result));
	if (!samples)
		{
		return 0;
		}

	for (uint32_t sample = 0; sample < options->sample_count; ++sample)
		{
		if (!lxs_run_benchmark_sample(
			root,
			path,
			options,
			trace_stdout,
			trace_file,
			sample,
			&samples[sample]))
			{
			goto cleanup;
			}
		}

	if (!lxs_select_sample_result(samples, options->sample_count, options->sample_select, result))
		{
		goto cleanup;
		}

	ok = 1;

cleanup:
	free(samples);
	return ok;
	}

static void lxs_print_usage(const char *exe_name)
	{
	fprintf(stderr, "Usage: %s <bench_path> [--mode cold|continuous] [--samples N] [--sample-select median|min|max]\n", exe_name);
	fprintf(stderr, "       [--iterations N] [--cycles N]\n");
	fprintf(stderr, "       [--record-mode aggregate|trace|none] [--record-interval N]\n");
	fprintf(stderr, "       [--record-file-path PATH] [--record-file-name NAME]\n");
	fprintf(stderr, "       [--record-overwrite-rule append|overwrite] [--record-meta-head TEXT] [--phase-profile] [--struct-profile]\n");
	}

static int lxs_parse_u32(const char *text, uint32_t *out_value)
	{
	char *end = NULL;
	unsigned long value = strtoul(text, &end, 10);
	if (!end || *end != '\0')
		{
		return 0;
		}
	*out_value = (uint32_t)value;
	return 1;
	}

static int lxs_parse_options(int argc, char **argv, lxs_bench_options *options)
	{
	memset(options, 0, sizeof(*options));
	options->mode = LXS_MODE_CONTINUOUS;
	options->record_mode = LXS_RECORD_AGGREGATE;
	options->overwrite_rule = LXS_OVERWRITE_APPEND;
	options->sample_select = LXS_SAMPLE_MEDIAN;
	options->sample_count = 3U;
	options->iterations = 1U;
	options->cycles = 1000U;
	options->record_interval = 1U;

	if (argc < 2)
		{
		return 0;
		}

	options->benchmark_path = argv[1];
	for (int i = 2; i < argc; ++i)
		{
		if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc)
			{
			i++;
			options->mode = _stricmp(argv[i], "cold") == 0 ? LXS_MODE_COLD : LXS_MODE_CONTINUOUS;
			}
		else if (strcmp(argv[i], "--samples") == 0 && i + 1 < argc)
			{
			if (!lxs_parse_u32(argv[++i], &options->sample_count) || options->sample_count == 0U)
				{
				return 0;
				}
			}
		else if (strcmp(argv[i], "--sample-select") == 0 && i + 1 < argc)
			{
			i++;
			if (_stricmp(argv[i], "min") == 0)
				{
				options->sample_select = LXS_SAMPLE_MIN;
				}
			else if (_stricmp(argv[i], "max") == 0)
				{
				options->sample_select = LXS_SAMPLE_MAX;
				}
			else
				{
				options->sample_select = LXS_SAMPLE_MEDIAN;
				}
			}
		else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc)
			{
			if (!lxs_parse_u32(argv[++i], &options->iterations))
				{
				return 0;
				}
			}
		else if (strcmp(argv[i], "--cycles") == 0 && i + 1 < argc)
			{
			if (!lxs_parse_u32(argv[++i], &options->cycles))
				{
				return 0;
				}
			}
		else if (strcmp(argv[i], "--record-mode") == 0 && i + 1 < argc)
			{
			i++;
			if (_stricmp(argv[i], "trace") == 0)
				{
				options->record_mode = LXS_RECORD_TRACE;
				}
			else if (_stricmp(argv[i], "none") == 0)
				{
				options->record_mode = LXS_RECORD_NONE;
				}
			else
				{
				options->record_mode = LXS_RECORD_AGGREGATE;
				}
			}
		else if (strcmp(argv[i], "--record-interval") == 0 && i + 1 < argc)
			{
			if (!lxs_parse_u32(argv[++i], &options->record_interval))
				{
				return 0;
				}
			}
		else if (strcmp(argv[i], "--record-file-path") == 0 && i + 1 < argc)
			{
			options->record_file_path = argv[++i];
			}
		else if (strcmp(argv[i], "--record-file-name") == 0 && i + 1 < argc)
			{
			options->record_file_name = argv[++i];
			}
		else if (strcmp(argv[i], "--record-overwrite-rule") == 0 && i + 1 < argc)
			{
			i++;
			options->overwrite_rule = _stricmp(argv[i], "overwrite") == 0 ? LXS_OVERWRITE_TRUNCATE : LXS_OVERWRITE_APPEND;
			}
		else if (strcmp(argv[i], "--record-meta-head") == 0 && i + 1 < argc)
			{
			options->record_meta_head = argv[++i];
			}
		else if (strcmp(argv[i], "--phase-profile") == 0)
			{
			options->phase_profile = 1;
			}
		else if (strcmp(argv[i], "--struct-profile") == 0)
			{
			options->struct_profile = 1;
			}
		else
			{
			return 0;
			}
		}

	return options->iterations > 0 && options->cycles > 0;
	}

int main(int argc, char **argv)
	{
	lxs_bench_options options;
	lxs_path_list paths = {0};
	lxs_result_list results = {0};
	FILE *record_stream = NULL;
	int exit_code = 1;

	if (!lxs_parse_options(argc, argv, &options))
		{
		lxs_print_usage(argv[0]);
		return 1;
		}

	if (!lxs_collect_benchmarks(options.benchmark_path, &paths) || paths.count == 0)
		{
		fprintf(stderr, "No benchmark files found under %s\n", options.benchmark_path);
		goto cleanup;
		}

	qsort(paths.items, paths.count, sizeof(char*), lxs_compare_path_strings);

	if (options.record_mode != LXS_RECORD_NONE)
		{
		record_stream = lxs_open_record_file(&options);
		}

		lxs_write_meta(stdout, &options);
	if (options.record_mode == LXS_RECORD_TRACE)
		{
		lxs_write_trace_header(stdout);
		}
	lxs_write_csv_header(stdout);

	if (record_stream)
		{
		lxs_write_meta(record_stream, &options);
		if (options.record_mode == LXS_RECORD_TRACE)
			{
			lxs_write_trace_header(record_stream);
			}
		lxs_write_csv_header(record_stream);
		}

	for (size_t i = 0; i < paths.count; ++i)
		{
		lxs_bench_result result;
		lxs_plan_profile profile;

		memset(&profile, 0, sizeof(profile));

		if (options.struct_profile)
			{
			lxs_netlist *nl = lxs_load_iscas(paths.items[i]);
			lxs_plan *plan = nl ? lxs_compile_to_plan(nl) : NULL;
			if (!plan)
				{
				lxs_free_plan(plan);
				lxs_free_netlist(nl);
				fprintf(stderr, "Failed profile compile: %s\n", paths.items[i]);
				goto cleanup;
				}

			lxs_build_plan_profile(options.benchmark_path, paths.items[i], plan, &profile);
			lxs_free_plan(plan);
			lxs_free_netlist(nl);
			}

		if (!lxs_run_benchmark(
			options.benchmark_path,
			paths.items[i],
			&options,
			options.record_mode == LXS_RECORD_TRACE ? stdout : NULL,
			options.record_mode == LXS_RECORD_TRACE ? record_stream : NULL,
			&result))
			{
			fprintf(stderr, "Failed benchmark: %s\n", paths.items[i]);
			goto cleanup;
			}

		if (!lxs_result_list_push(&results, &result))
			{
			goto cleanup;
			}

		if (options.struct_profile)
			{
			lxs_write_plan_profile(stdout, &profile);
			if (record_stream)
				{
				lxs_write_plan_profile(record_stream, &profile);
				}
			}

		lxs_write_result(stdout, &result);
		if (record_stream)
			{
			lxs_write_result(record_stream, &result);
			}
		}

	lxs_write_summary(stdout, &results);
	if (record_stream)
		{
		lxs_write_summary(record_stream, &results);
		}

	exit_code = 0;

cleanup:
	if (record_stream)
		{
		fclose(record_stream);
		}
	free(results.items);
	lxs_path_list_free(&paths);
	return exit_code;
	}
