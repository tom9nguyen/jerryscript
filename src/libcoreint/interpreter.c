/* Copyright 2014-2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ecma-alloc.h"
#include "ecma-builtins.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-lex-env.h"
#include "ecma-operations.h"
#include "ecma-stack.h"
#include "globals.h"
#include "interpreter.h"
#include "jerry-libc.h"
#include "mem-allocator.h"

#define __INIT_OP_FUNC(name, arg1, arg2, arg3) [ __op__idx_##name ] = opfunc_##name,
static const opfunc __opfuncs[LAST_OP] =
{
  OP_LIST (INIT_OP_FUNC)
};
#undef __INIT_OP_FUNC

JERRY_STATIC_ASSERT (sizeof (opcode_t) <= 4);

const opcode_t *__program = NULL;

#ifdef MEM_STATS
#define __OP_FUNC_NAME(name, arg1, arg2, arg3) #name,
static const char *__op_names[LAST_OP] =
{
  OP_LIST (OP_FUNC_NAME)
};
#undef __OP_FUNC_NAME

#define INTERP_MEM_PRINT_INDENTATION_STEP (5)
#define INTERP_MEM_PRINT_INDENTATION_MAX  (125)
static uint32_t interp_mem_stats_print_indentation = 0;
static bool interp_mem_stats_enabled = false;

static void
interp_mem_stats_print_legend (void)
{
  if (likely (!interp_mem_stats_enabled))
  {
    return;
  }

  __printf ("----- Legend of memory usage trace during interpretation -----\n\n"
            "\tEntering block = beginning execution of initial (global) scope or function.\n\n"
            "\tInformation on each value is formatted as following: (p -> n ( [+-]c, local l, peak g), where:\n"
            "\t p     - value just before starting of item's execution;\n"
            "\t n     - value just after end of item's execution;\n"
            "\t [+-c] - difference between n and p;\n"
            "\t l     - temporary usage of memory during item's execution;\n"
            "\t g     - global peak of the value during program's execution.\n\n"
            "\tChunks are items allocated in a pool."
            " If there is no pool with a free chunk upon chunk allocation request,\n"
            "\tthen new pool is allocated on the heap (that causes increase of number of allocated heap bytes).\n\n");
}

static void
interp_mem_get_stats (mem_heap_stats_t *out_heap_stats_p,
                      mem_pools_stats_t *out_pool_stats_p,
                      bool reset_peak_before,
                      bool reset_peak_after)
{
  if (likely (!interp_mem_stats_enabled))
  {
    return;
  }

  /* Requesting to free as much memory as we currently can */
  ecma_try_to_give_back_some_memory (MEM_TRY_GIVE_MEMORY_BACK_SEVERITY_CRITICAL);

  if (reset_peak_before)
  {
    mem_heap_stats_reset_peak ();
    mem_pools_stats_reset_peak ();
  }

  mem_heap_get_stats (out_heap_stats_p);
  mem_pools_get_stats (out_pool_stats_p);

  if (reset_peak_after)
  {
    mem_heap_stats_reset_peak ();
    mem_pools_stats_reset_peak ();
  }
}

static void
interp_mem_stats_context_enter (int_data_t *int_data_p,
                                opcode_counter_t block_position)
{
  if (likely (!interp_mem_stats_enabled))
  {
    return;
  }

  const uint32_t indentation = JERRY_MIN (interp_mem_stats_print_indentation,
                                          INTERP_MEM_PRINT_INDENTATION_MAX);

  char indent_prefix[INTERP_MEM_PRINT_INDENTATION_MAX + 2];
  __memset (indent_prefix, ' ', sizeof (indent_prefix));
  indent_prefix [indentation] = '|';
  indent_prefix [indentation + 1] = '\0';

  int_data_p->context_peak_allocated_heap_bytes = 0;
  int_data_p->context_peak_waste_heap_bytes = 0;
  int_data_p->context_peak_pools_count = 0;
  int_data_p->context_peak_allocated_pool_chunks = 0;

  interp_mem_get_stats (&int_data_p->heap_stats_context_enter,
                        &int_data_p->pools_stats_context_enter,
                        false, false);

  __printf ("\n%s--- Beginning interpretation of a block at position %u ---\n"
            "%s Allocated heap bytes:  %5u\n"
            "%s Waste heap bytes:      %5u\n"
            "%s Pools:                 %5u\n"
            "%s Allocated pool chunks: %5u\n\n",
            indent_prefix, (uint32_t) block_position,
            indent_prefix, int_data_p->heap_stats_context_enter.allocated_bytes,
            indent_prefix, int_data_p->heap_stats_context_enter.waste_bytes,
            indent_prefix, int_data_p->pools_stats_context_enter.pools_count,
            indent_prefix, int_data_p->pools_stats_context_enter.allocated_chunks);
}

static void
interp_mem_stats_context_exit (int_data_t *int_data_p,
                               opcode_counter_t block_position)
{
  if (likely (!interp_mem_stats_enabled))
  {
    return;
  }

  const uint32_t indentation = JERRY_MIN (interp_mem_stats_print_indentation,
                                          INTERP_MEM_PRINT_INDENTATION_MAX);

  char indent_prefix[INTERP_MEM_PRINT_INDENTATION_MAX + 2];
  __memset (indent_prefix, ' ', sizeof (indent_prefix));
  indent_prefix [indentation] = '|';
  indent_prefix [indentation + 1] = '\0';

  mem_heap_stats_t heap_stats_context_exit;
  mem_pools_stats_t pools_stats_context_exit;

  interp_mem_get_stats (&heap_stats_context_exit,
                        &pools_stats_context_exit,
                        false, true);

  int_data_p->context_peak_allocated_heap_bytes -= JERRY_MAX (int_data_p->heap_stats_context_enter.allocated_bytes,
                                                              heap_stats_context_exit.allocated_bytes);
  int_data_p->context_peak_waste_heap_bytes -= JERRY_MAX (int_data_p->heap_stats_context_enter.waste_bytes,
                                                          heap_stats_context_exit.waste_bytes);
  int_data_p->context_peak_pools_count -= JERRY_MAX (int_data_p->pools_stats_context_enter.pools_count,
                                                     pools_stats_context_exit.pools_count);
  int_data_p->context_peak_allocated_pool_chunks -= JERRY_MAX (int_data_p->pools_stats_context_enter.allocated_chunks,
                                                               pools_stats_context_exit.allocated_chunks);

  __printf ("%sAllocated heap bytes in the context:  %5u -> %5u (%+5d, local %5u, peak %5u)\n",
            indent_prefix,
            int_data_p->heap_stats_context_enter.allocated_bytes,
            heap_stats_context_exit.allocated_bytes,
            heap_stats_context_exit.allocated_bytes - int_data_p->heap_stats_context_enter.allocated_bytes,
            int_data_p->context_peak_allocated_heap_bytes,
            heap_stats_context_exit.global_peak_allocated_bytes);

  __printf ("%sWaste heap bytes in the context:      %5u -> %5u (%+5d, local %5u, peak %5u)\n",
            indent_prefix,
            int_data_p->heap_stats_context_enter.waste_bytes,
            heap_stats_context_exit.waste_bytes,
            heap_stats_context_exit.waste_bytes - int_data_p->heap_stats_context_enter.waste_bytes,
            int_data_p->context_peak_waste_heap_bytes,
            heap_stats_context_exit.global_peak_waste_bytes);

  __printf ("%sPools count in the context:           %5u -> %5u (%+5d, local %5u, peak %5u)\n",
            indent_prefix,
            int_data_p->pools_stats_context_enter.pools_count,
            pools_stats_context_exit.pools_count,
            pools_stats_context_exit.pools_count - int_data_p->pools_stats_context_enter.pools_count,
            int_data_p->context_peak_pools_count,
            pools_stats_context_exit.global_peak_pools_count);

  __printf ("%sAllocated pool chunks in the context: %5u -> %5u (%+5d, local %5u, peak %5u)\n",
            indent_prefix,
            int_data_p->pools_stats_context_enter.allocated_chunks,
            pools_stats_context_exit.allocated_chunks,
            pools_stats_context_exit.allocated_chunks - int_data_p->pools_stats_context_enter.allocated_chunks,
            int_data_p->context_peak_allocated_pool_chunks,
            pools_stats_context_exit.global_peak_allocated_chunks);

  __printf ("\n%s--- End of interpretation of a block at position %u ---\n\n",
            indent_prefix, (uint32_t) block_position);
}

static void
interp_mem_stats_opcode_enter (opcode_counter_t opcode_position,
                               mem_heap_stats_t *out_heap_stats_p,
                               mem_pools_stats_t *out_pools_stats_p)
{
  if (likely (!interp_mem_stats_enabled))
  {
    return;
  }

  const uint32_t indentation = JERRY_MIN (interp_mem_stats_print_indentation,
                                          INTERP_MEM_PRINT_INDENTATION_MAX);

  char indent_prefix[INTERP_MEM_PRINT_INDENTATION_MAX + 2];
  __memset (indent_prefix, ' ', sizeof (indent_prefix));
  indent_prefix [indentation] = '|';
  indent_prefix [indentation + 1] = '\0';

  interp_mem_get_stats (out_heap_stats_p,
                        out_pools_stats_p,
                        true, false);

  opcode_t opcode = read_opcode (opcode_position);

  __printf ("%s-- Opcode: %s (position %u) --\n",
            indent_prefix, __op_names [opcode.op_idx], (uint32_t) opcode_position);

  interp_mem_stats_print_indentation += INTERP_MEM_PRINT_INDENTATION_STEP;
}

static void
interp_mem_stats_opcode_exit (int_data_t *int_data_p,
                              opcode_counter_t opcode_position,
                              mem_heap_stats_t *heap_stats_before_p,
                              mem_pools_stats_t *pools_stats_before_p)
{
  if (likely (!interp_mem_stats_enabled))
  {
    return;
  }

  interp_mem_stats_print_indentation -= INTERP_MEM_PRINT_INDENTATION_STEP;

  const uint32_t indentation = JERRY_MIN (interp_mem_stats_print_indentation,
                                          INTERP_MEM_PRINT_INDENTATION_MAX);

  char indent_prefix[INTERP_MEM_PRINT_INDENTATION_MAX + 2];
  __memset (indent_prefix, ' ', sizeof (indent_prefix));
  indent_prefix [indentation] = '|';
  indent_prefix [indentation + 1] = '\0';

  mem_heap_stats_t heap_stats_after;
  mem_pools_stats_t pools_stats_after;

  interp_mem_get_stats (&heap_stats_after,
                        &pools_stats_after,
                        false, true);

  int_data_p->context_peak_allocated_heap_bytes = JERRY_MAX (int_data_p->context_peak_allocated_heap_bytes,
                                                             heap_stats_after.allocated_bytes);
  int_data_p->context_peak_waste_heap_bytes = JERRY_MAX (int_data_p->context_peak_waste_heap_bytes,
                                                         heap_stats_after.waste_bytes);
  int_data_p->context_peak_pools_count = JERRY_MAX (int_data_p->context_peak_pools_count,
                                                    pools_stats_after.pools_count);
  int_data_p->context_peak_allocated_pool_chunks = JERRY_MAX (int_data_p->context_peak_allocated_pool_chunks,
                                                              pools_stats_after.allocated_chunks);

  opcode_t opcode = read_opcode (opcode_position);

  __printf ("%s Allocated heap bytes:  %5u -> %5u (%+5d, local %5u, peak %5u)\n",
            indent_prefix,
            heap_stats_before_p->allocated_bytes,
            heap_stats_after.allocated_bytes,
            heap_stats_after.allocated_bytes - heap_stats_before_p->allocated_bytes,
            heap_stats_after.peak_allocated_bytes - JERRY_MAX (heap_stats_before_p->allocated_bytes,
                                                               heap_stats_after.allocated_bytes),
            heap_stats_after.global_peak_allocated_bytes);

  if (heap_stats_before_p->waste_bytes != heap_stats_after.waste_bytes)
  {
    __printf ("%s Waste heap bytes:      %5u -> %5u (%+5d, local %5u, peak %5u)\n",
              indent_prefix,
              heap_stats_before_p->waste_bytes,
              heap_stats_after.waste_bytes,
              heap_stats_after.waste_bytes - heap_stats_before_p->waste_bytes,
              heap_stats_after.peak_waste_bytes - JERRY_MAX (heap_stats_before_p->waste_bytes,
                                                             heap_stats_after.waste_bytes),
              heap_stats_after.global_peak_waste_bytes);
  }

  if (pools_stats_before_p->pools_count != pools_stats_after.pools_count)
  {
    __printf ("%s Pools:                 %5u -> %5u (%+5d, local %5u, peak %5u)\n",
              indent_prefix,
              pools_stats_before_p->pools_count,
              pools_stats_after.pools_count,
              pools_stats_after.pools_count - pools_stats_before_p->pools_count,
              pools_stats_after.peak_pools_count - JERRY_MAX (pools_stats_before_p->pools_count,
                                                              pools_stats_after.pools_count),
              pools_stats_after.global_peak_pools_count);
  }

  if (pools_stats_before_p->allocated_chunks != pools_stats_after.allocated_chunks)
  {
    __printf ("%s Allocated pool chunks: %5u -> %5u (%+5d, local %5u, peak %5u)\n",
              indent_prefix,
              pools_stats_before_p->allocated_chunks,
              pools_stats_after.allocated_chunks,
              pools_stats_after.allocated_chunks - pools_stats_before_p->allocated_chunks,
              pools_stats_after.peak_allocated_chunks - JERRY_MAX (pools_stats_before_p->allocated_chunks,
                                                                   pools_stats_after.allocated_chunks),
              pools_stats_after.global_peak_allocated_chunks);
  }

  __printf ("%s-- End of execution of opcode %s (position %u) --\n\n",
            indent_prefix, __op_names [opcode.op_idx], opcode_position);
}
#endif /* MEM_STATS */

/**
 * Initialize interpreter.
 */
void
init_int (const opcode_t *program_p, /**< pointer to byte-code program */
          bool dump_mem_stats) /** dump per-opcode memory usage change statistics */
{
#ifdef MEM_STATS
  interp_mem_stats_enabled = dump_mem_stats;
#else /* MEM_STATS */
  JERRY_ASSERT (!dump_mem_stats);
#endif /* !MEM_STATS */

  JERRY_ASSERT (__program == NULL);

  __program = program_p;
} /* init_int */

bool
run_int (void)
{
  JERRY_ASSERT (__program != NULL);

#ifdef MEM_STATS
  interp_mem_stats_print_legend ();
#endif /* MEM_STATS */

  bool is_strict = false;
  opcode_counter_t start_pos = 0;

  opcode_t first_opcode = read_opcode (start_pos);
  if (first_opcode.op_idx == __op__idx_meta
      && first_opcode.data.meta.type == OPCODE_META_TYPE_STRICT_CODE)
  {
    is_strict = true;
    start_pos++;
  }

  ecma_init ();

  ecma_object_ptr_t glob_obj_p;
  ecma_builtin_get (glob_obj_p, ECMA_BUILTIN_ID_GLOBAL);

  ecma_object_ptr_t lex_env_p;
  ecma_op_create_global_environment (lex_env_p, glob_obj_p);
  ecma_value_t this_binding_value (glob_obj_p);

  ecma_completion_value_t run_completion;
  run_int_from_pos (run_completion,
                    start_pos,
                    this_binding_value,
                    lex_env_p,
                    is_strict,
                    false);

  if (ecma_is_completion_value_exit (run_completion))
  {
    ecma_deref_object (glob_obj_p);
    ecma_deref_object (lex_env_p);
    ecma_finalize ();

    ecma_value_t value_ret;
    ecma_get_completion_value_value (value_ret, run_completion);

    return ecma_is_value_true (value_ret);
  }
  else if (ecma_is_completion_value_throw (run_completion))
  {
    jerry_exit (ERR_UNHANDLED_EXCEPTION);
  }

  JERRY_UNREACHABLE ();
}

void
run_int_loop (ecma_completion_value_t &completion, /**< out: completion value */
              int_data_t *int_data /**< interpretation context */)
{
#ifdef MEM_STATS
  mem_heap_stats_t heap_stats_before;
  mem_pools_stats_t pools_stats_before;

  __memset (&heap_stats_before, 0, sizeof (heap_stats_before));
  __memset (&pools_stats_before, 0, sizeof (pools_stats_before));
#endif /* MEM_STATS */

  while (true)
  {
    do
    {
      const opcode_t *curr = &__program[int_data->pos];

#ifdef MEM_STATS
      const opcode_counter_t opcode_pos = int_data->pos;

      interp_mem_stats_opcode_enter (opcode_pos,
                                     &heap_stats_before,
                                     &pools_stats_before);
#endif /* MEM_STATS */

      __opfuncs[curr->op_idx] (completion, *curr, int_data);

#ifdef MEM_STATS
      interp_mem_stats_opcode_exit (int_data,
                                    opcode_pos,
                                    &heap_stats_before,
                                    &pools_stats_before);
#endif /* MEM_STATS */

      JERRY_ASSERT (!ecma_is_completion_value_normal (completion)
                    || ecma_is_completion_value_empty (completion));
    }
    while (ecma_is_completion_value_normal (completion));

    if (ecma_is_completion_value_meta (completion))
    {
      ecma_make_empty_completion_value (completion);
    }

    return;
  }
}

void
run_int_from_pos (ecma_completion_value_t &completion, /**< out: completion value */
                  opcode_counter_t start_pos, /**< position to start interpretation at */
                  const ecma_value_t& this_binding_value, /**< value of 'this' binding */
                  const ecma_object_ptr_t& lex_env_p, /**< starting lexical environment */
                  bool is_strict, /**< is execution mode strict? */
                  bool is_eval_code) /**< is current code executed with eval? */
{
  const opcode_t *curr = &__program[start_pos];
  JERRY_ASSERT (curr->op_idx == __op__idx_reg_var_decl);

  const idx_t min_reg_num = curr->data.reg_var_decl.min;
  const idx_t max_reg_num = curr->data.reg_var_decl.max;
  JERRY_ASSERT (max_reg_num >= min_reg_num);

  const int32_t regs_num = max_reg_num - min_reg_num + 1;

  MEM_DEFINE_LOCAL_ARRAY (regs, regs_num, ecma_value_packed_t);

  int_data_t int_data;
  int_data.pos = (opcode_counter_t) (start_pos + 1);
  int_data.this_binding_p = &this_binding_value;
  int_data.lex_env_p = &lex_env_p;
  int_data.is_strict = is_strict;
  int_data.is_eval_code = is_eval_code;
  int_data.min_reg_num = min_reg_num;
  int_data.max_reg_num = max_reg_num;
  int_data.tmp_num_p = ecma_alloc_number ();
  ecma_stack_add_frame (&int_data.stack_frame, regs, regs_num);

#ifdef MEM_STATS
  interp_mem_stats_context_enter (&int_data, start_pos);
#endif /* MEM_STATS */

  run_int_loop (completion, &int_data);

  JERRY_ASSERT (ecma_is_completion_value_normal (completion)
                || ecma_is_completion_value_throw (completion)
                || ecma_is_completion_value_return (completion)
                || ecma_is_completion_value_exit (completion));

  ecma_stack_free_frame (&int_data.stack_frame);

  ecma_dealloc_number (int_data.tmp_num_p);

#ifdef MEM_STATS
  interp_mem_stats_context_exit (&int_data, start_pos);
#endif /* MEM_STATS */

  MEM_FINALIZE_LOCAL_ARRAY (regs);
}

/**
 * Get specified opcode from the program.
 */
opcode_t
read_opcode (opcode_counter_t counter) /**< opcode counter */
{
  return __program[ counter ];
} /* read_opcode */
