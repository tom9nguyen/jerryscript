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

#include "deserializer.h"
#include "globals.h"
#include "interpreter.h"
#include "opcodes.h"
#include "opcodes-ecma-support.h"

/**
 * Note:
 *      The note describes exception handling in opcode handlers that perform operations,
 *      that can throw exceptions, and do not themself handle the exceptions.
 *
 *      Generally, each opcode handler consists of sequence of operations.
 *      Some of these operations (exceptionable operations) can throw exceptions and other - cannot.
 *
 *      1. At the beginning of the handler there should be declared opcode handler's 'return value' variable.
 *
 *      2. All exceptionable operations except the last should be enclosed in ECMA_TRY_CATCH macro.
 *         All subsequent operations in the opcode handler should be placed into block between
 *         the ECMA_TRY_CATCH and corresponding ECMA_FINALIZE.
 *
 *      3. The last exceptionable's operation result should be assigned directly to opcode handler's
 *         'return value' variable without using ECMA_TRY_CATCH macro.
 *
 *      4. After last ECMA_FINALIZE statement there should be only one operator.
 *         The operator should return from the opcode handler with it's 'return value'.
 *
 *      5. No other operations with opcode handler's 'return value' variable should be performed.
 */

#define OP_UNIMPLEMENTED_LIST(op) \
    static char __unused unimplemented_list_end

#define DEFINE_UNIMPLEMENTED_OP(op) \
  ecma_completion_value_t opfunc_ ## op (opcode_t opdata, int_data_t *int_data) \
  { \
    JERRY_UNIMPLEMENTED_REF_UNUSED_VARS (opdata, int_data); \
  }

OP_UNIMPLEMENTED_LIST (DEFINE_UNIMPLEMENTED_OP);
#undef DEFINE_UNIMPLEMENTED_OP

/**
 * 'Nop' opcode handler.
 */
void
opfunc_nop (ecma_completion_value_t &ret_value, /**< out: completion value */
            opcode_t opdata __unused, /**< operation data */
            int_data_t *int_data) /**< interpreter context */
{
  int_data->pos++;

  ecma_make_empty_completion_value (ret_value);
} /* opfunc_nop */

/**
 * 'Assignment' opcode handler.
 *
 * Note:
 *      This handler implements case of assignment of a literal's or a variable's
 *      value to a variable. Assignment to an object's property is not implemented
 *      by this opcode.
 *
 * See also: ECMA-262 v5, 11.13.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_assignment (ecma_completion_value_t &ret_value, /**< out: completion value */
                   opcode_t opdata, /**< operation data */
                   int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.assignment.var_left;
  const opcode_arg_type_operand type_value_right = (opcode_arg_type_operand) opdata.data.assignment.type_value_right;
  const idx_t src_val_descr = opdata.data.assignment.value_right;

  if (type_value_right == OPCODE_ARG_TYPE_SIMPLE)
  {
    set_variable_value (ret_value, int_data,
                        int_data->pos,
                        dst_var_idx,
                        ecma_value_t ((ecma_simple_value_t) src_val_descr));
  }
  else if (type_value_right == OPCODE_ARG_TYPE_STRING)
  {
    const literal_index_t lit_id = deserialize_lit_id_by_uid (src_val_descr, int_data->pos);
    ecma_string_t *string_p = ecma_new_ecma_string_from_lit_index (lit_id);

    set_variable_value (ret_value, int_data,
                        int_data->pos,
                        dst_var_idx,
                        ecma_value_t (string_p));

    ecma_deref_ecma_string (string_p);
  }
  else if (type_value_right == OPCODE_ARG_TYPE_VARIABLE)
  {
    ECMA_TRY_CATCH (ret_value, get_variable_value, var_value, int_data, src_val_descr, false);

    set_variable_value (ret_value, int_data,
                        int_data->pos,
                        dst_var_idx,
                        var_value);

    ECMA_FINALIZE (var_value);
  }
  else if (type_value_right == OPCODE_ARG_TYPE_NUMBER)
  {
    ecma_number_t *num_p = int_data->tmp_num_p;

    const literal_index_t lit_id = deserialize_lit_id_by_uid (src_val_descr, int_data->pos);
    const literal lit = deserialize_literal_by_id (lit_id);
    JERRY_ASSERT (lit.type == LIT_NUMBER);

    *num_p = lit.data.num;

    set_variable_value (ret_value, int_data,
                        int_data->pos,
                        dst_var_idx,
                        ecma_value_t (num_p));
  }
  else if (type_value_right == OPCODE_ARG_TYPE_NUMBER_NEGATE)
  {
    ecma_number_t *num_p = int_data->tmp_num_p;

    const literal_index_t lit_id = deserialize_lit_id_by_uid (src_val_descr, int_data->pos);
    const literal lit = deserialize_literal_by_id (lit_id);
    JERRY_ASSERT (lit.type == LIT_NUMBER);

    *num_p = ecma_number_negate (lit.data.num);

    set_variable_value (ret_value, int_data,
                        int_data->pos,
                        dst_var_idx,
                        ecma_value_t (num_p));
  }
  else if (type_value_right == OPCODE_ARG_TYPE_SMALLINT)
  {
    ecma_number_t *num_p = int_data->tmp_num_p;

    *num_p = src_val_descr;

    set_variable_value (ret_value, int_data,
                        int_data->pos,
                        dst_var_idx,
                        ecma_value_t (num_p));
  }
  else
  {
    JERRY_ASSERT (type_value_right == OPCODE_ARG_TYPE_SMALLINT_NEGATE);
    ecma_number_t *num_p = int_data->tmp_num_p;

    *num_p = ecma_number_negate (src_val_descr);

    set_variable_value (ret_value, int_data,
                        int_data->pos,
                        dst_var_idx,
                        ecma_value_t (num_p));
  }

  int_data->pos++;
} /* opfunc_assignment */

/**
 * 'Pre increment' opcode handler.
 *
 * See also: ECMA-262 v5, 11.4.4
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_pre_incr (ecma_completion_value_t &ret_value, /**< out: completion value */
                 opcode_t opdata, /**< operation data */
                 int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.pre_incr.dst;
  const idx_t incr_var_idx = opdata.data.pre_incr.var_right;

  ecma_make_empty_completion_value (ret_value);

  // 1., 2., 3.
  ECMA_TRY_CATCH (ret_value, get_variable_value, old_value, int_data, incr_var_idx, true);
  ECMA_OP_TO_NUMBER_TRY_CATCH (old_num, old_value, ret_value);

  // 4.
  ecma_number_t* new_num_p = int_data->tmp_num_p;

  *new_num_p = ecma_number_add (old_num, ECMA_NUMBER_ONE);

  ecma_value_t new_num_value (new_num_p);

  // 5.
  set_variable_value (ret_value, int_data, int_data->pos, incr_var_idx, new_num_value);

  // assignment of operator result to register variable
  ecma_completion_value_t reg_assignment_res;
  set_variable_value (reg_assignment_res, int_data, int_data->pos, dst_var_idx, new_num_value);
  JERRY_ASSERT (ecma_is_completion_value_empty (reg_assignment_res));

  ECMA_OP_TO_NUMBER_FINALIZE (old_num);
  ECMA_FINALIZE (old_value);

  int_data->pos++;
} /* opfunc_pre_incr */

/**
 * 'Pre decrement' opcode handler.
 *
 * See also: ECMA-262 v5, 11.4.4
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_pre_decr (ecma_completion_value_t &ret_value, /**< out: completion value */
                 opcode_t opdata, /**< operation data */
                 int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.pre_decr.dst;
  const idx_t decr_var_idx = opdata.data.pre_decr.var_right;

  ecma_make_empty_completion_value (ret_value);

  // 1., 2., 3.
  ECMA_TRY_CATCH (ret_value, get_variable_value, old_value, int_data, decr_var_idx, true);
  ECMA_OP_TO_NUMBER_TRY_CATCH (old_num, old_value, ret_value);

  // 4.
  ecma_number_t* new_num_p = int_data->tmp_num_p;

  *new_num_p = ecma_number_substract (old_num, ECMA_NUMBER_ONE);

  ecma_value_t new_num_value (new_num_p);

  // 5.
  set_variable_value (ret_value, int_data, int_data->pos, decr_var_idx, new_num_value);

  // assignment of operator result to register variable
  ecma_completion_value_t reg_assignment_res;
  set_variable_value (reg_assignment_res, int_data, int_data->pos, dst_var_idx, new_num_value);
  JERRY_ASSERT (ecma_is_completion_value_empty (reg_assignment_res));

  ECMA_OP_TO_NUMBER_FINALIZE (old_num);
  ECMA_FINALIZE (old_value);

  int_data->pos++;
} /* opfunc_pre_decr */

/**
 * 'Post increment' opcode handler.
 *
 * See also: ECMA-262 v5, 11.3.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_post_incr (ecma_completion_value_t &ret_value, /**< out: completion value */
                  opcode_t opdata, /**< operation data */
                  int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.post_incr.dst;
  const idx_t incr_var_idx = opdata.data.post_incr.var_right;

  ecma_make_empty_completion_value (ret_value);

  // 1., 2., 3.
  ECMA_TRY_CATCH (ret_value, get_variable_value, old_value, int_data, incr_var_idx, true);
  ECMA_OP_TO_NUMBER_TRY_CATCH (old_num, old_value, ret_value);

  // 4.
  ecma_number_t* new_num_p = int_data->tmp_num_p;

  *new_num_p = ecma_number_add (old_num, ECMA_NUMBER_ONE);

  // 5.
  set_variable_value (ret_value, int_data, int_data->pos, incr_var_idx, ecma_value_t (new_num_p));

  ecma_number_t *tmp_p = int_data->tmp_num_p;
  *tmp_p = old_num;

  // assignment of operator result to register variable
  ecma_completion_value_t reg_assignment_res;
  set_variable_value (reg_assignment_res, int_data, int_data->pos, dst_var_idx, ecma_value_t (tmp_p));
  JERRY_ASSERT (ecma_is_completion_value_empty (reg_assignment_res));

  ECMA_OP_TO_NUMBER_FINALIZE (old_num);
  ECMA_FINALIZE (old_value);

  int_data->pos++;
} /* opfunc_post_incr */

/**
 * 'Post decrement' opcode handler.
 *
 * See also: ECMA-262 v5, 11.3.2
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_post_decr (ecma_completion_value_t &ret_value, /**< out: completion value */
                  opcode_t opdata, /**< operation data */
                  int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.post_decr.dst;
  const idx_t decr_var_idx = opdata.data.post_decr.var_right;

  ecma_make_empty_completion_value (ret_value);

  // 1., 2., 3.
  ECMA_TRY_CATCH (ret_value, get_variable_value, old_value, int_data, decr_var_idx, true);
  ECMA_OP_TO_NUMBER_TRY_CATCH (old_num, old_value, ret_value);

  // 4.
  ecma_number_t* new_num_p = int_data->tmp_num_p;

  *new_num_p = ecma_number_substract (old_num, ECMA_NUMBER_ONE);

  // 5.
  set_variable_value (ret_value, int_data, int_data->pos, decr_var_idx, ecma_value_t (new_num_p));

  ecma_number_t *tmp_p = int_data->tmp_num_p;
  *tmp_p = old_num;

  // assignment of operator result to register variable
  ecma_completion_value_t reg_assignment_res;
  set_variable_value (reg_assignment_res, int_data, int_data->pos, dst_var_idx, ecma_value_t (tmp_p));
  JERRY_ASSERT (ecma_is_completion_value_empty (reg_assignment_res));

  ECMA_OP_TO_NUMBER_FINALIZE (old_num);
  ECMA_FINALIZE (old_value);

  int_data->pos++;
} /* opfunc_post_decr */

/**
 * 'Register variable declaration' opcode handler.
 *
 * The opcode is meta-opcode that is not supposed to be executed.
 */
void
opfunc_reg_var_decl (ecma_completion_value_t &ret_value __unused, /**< out: completion value */
                     opcode_t opdata __unused, /**< operation data */
                     int_data_t *int_data __unused) /**< interpreter context */
{
  JERRY_UNREACHABLE ();
} /* opfunc_reg_var_decl */

/**
 * 'Variable declaration' opcode handler.
 *
 * See also: ECMA-262 v5, 10.5 - Declaration binding instantiation (block 8).
 *
 * @return completion value
 *         Returned value is simple and so need not be freed.
 *         However, ecma_free_completion_value may be called for it, but it is a no-op.
 */
void
opfunc_var_decl (ecma_completion_value_t &ret_value, /**< out: completion value */
                 opcode_t opdata, /**< operation data */
                 int_data_t *int_data) /**< interpreter context */
{
  const literal_index_t lit_id = deserialize_lit_id_by_uid (opdata.data.var_decl.variable_name,
                                                            int_data->pos);
  JERRY_ASSERT (lit_id != INVALID_LITERAL);

  ecma_string_t *var_name_string_p = ecma_new_ecma_string_from_lit_index (lit_id);

  if (!ecma_op_has_binding (*int_data->lex_env_p, var_name_string_p))
  {
    const bool is_configurable_bindings = int_data->is_eval_code;

    ecma_completion_value_t completion;
    ecma_op_create_mutable_binding (completion,
                                    *int_data->lex_env_p,
                                    var_name_string_p,
                                    is_configurable_bindings);

    JERRY_ASSERT (ecma_is_completion_value_empty (completion));

    /* Skipping SetMutableBinding as we have already checked that there were not
     * any binding with specified name in current lexical environment
     * and CreateMutableBinding sets the created binding's value to undefined */
    ecma_op_get_binding_value (completion,
                               *int_data->lex_env_p,
                               var_name_string_p,
                               true);
    JERRY_ASSERT (ecma_is_completion_value_normal_simple_value (completion, ECMA_SIMPLE_VALUE_UNDEFINED));
  }

  ecma_deref_ecma_string (var_name_string_p);

  int_data->pos++;

  ecma_make_empty_completion_value (ret_value);
} /* opfunc_var_decl */

/**
 * Function declaration helper
 *
 * @return completion value
 *         returned value must be freed with ecma_free_completion_value.
 */
static void
function_declaration (ecma_completion_value_t &ret_value, /**< out: completion value */
                      int_data_t *int_data, /**< interpreter context */
                      literal_index_t function_name_lit_id, /**< index of literal
                                                                 with function name */
                      ecma_string_t* args_names[], /**< names of arguments */
                      ecma_length_t args_number) /**< number of arguments */
{
  bool is_strict = int_data->is_strict;
  const bool is_configurable_bindings = int_data->is_eval_code;

  const opcode_counter_t function_code_end_oc = (opcode_counter_t) (
    read_meta_opcode_counter (OPCODE_META_TYPE_FUNCTION_END, int_data) + int_data->pos);
  int_data->pos++;

  opcode_t next_opcode = read_opcode (int_data->pos);
  if (next_opcode.op_idx == __op__idx_meta
      && next_opcode.data.meta.type == OPCODE_META_TYPE_STRICT_CODE)
  {
    is_strict = true;

    int_data->pos++;
  }

  ecma_string_t *function_name_string_p = ecma_new_ecma_string_from_lit_index (function_name_lit_id);

  ecma_op_function_declaration (ret_value,
                                *int_data->lex_env_p,
                                function_name_string_p,
                                int_data->pos,
                                args_names,
                                args_number,
                                is_strict,
                                is_configurable_bindings);
  ecma_deref_ecma_string (function_name_string_p);

  int_data->pos = function_code_end_oc;
} /* function_declaration */

/**
 * 'Function declaration' opcode handler.
 *
 * @return completion value
 *         returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_func_decl_n (ecma_completion_value_t &ret_value, /**< out: completion value */
                    opcode_t opdata, /**< operation data */
                    int_data_t *int_data) /**< interpreter context */
{
  const idx_t function_name_idx = opdata.data.func_decl_n.name_lit_idx;
  const ecma_length_t params_number = opdata.data.func_decl_n.arg_list;

  literal_index_t function_name_lit_id = deserialize_lit_id_by_uid (function_name_idx,
                                                                    int_data->pos);

  int_data->pos++;

  MEM_DEFINE_LOCAL_ARRAY (params_names, params_number, ecma_string_t*);

  fill_params_list (int_data, params_number, params_names);

  function_declaration (ret_value,
                        int_data,
                        function_name_lit_id,
                        params_names,
                        params_number);

  for (uint32_t param_index = 0;
       param_index < params_number;
       param_index++)
  {
    ecma_deref_ecma_string (params_names[param_index]);
  }

  MEM_FINALIZE_LOCAL_ARRAY (params_names);
} /* opfunc_func_decl_n */

/**
 * 'Function expression' opcode handler.
 *
 * @return completion value
 *         returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_func_expr_n (ecma_completion_value_t &ret_value, /**< out: completion value */
                    opcode_t opdata, /**< operation data */
                    int_data_t *int_data) /**< interpreter context */
{
  const opcode_counter_t lit_oc = int_data->pos;

  int_data->pos++;

  const idx_t dst_var_idx = opdata.data.func_expr_n.lhs;
  const idx_t function_name_lit_idx = opdata.data.func_expr_n.name_lit_idx;
  const ecma_length_t params_number = opdata.data.func_expr_n.arg_list;
  const bool is_named_func_expr = (function_name_lit_idx != INVALID_VALUE);

  opcode_counter_t function_code_end_oc;

  MEM_DEFINE_LOCAL_ARRAY (params_names, params_number, ecma_string_t*);

  fill_params_list (int_data, params_number, params_names);

  bool is_strict = int_data->is_strict;

  function_code_end_oc = (opcode_counter_t) (read_meta_opcode_counter (OPCODE_META_TYPE_FUNCTION_END,
                                                                       int_data) + int_data->pos);
  int_data->pos++;

  opcode_t next_opcode = read_opcode (int_data->pos);
  if (next_opcode.op_idx == __op__idx_meta
      && next_opcode.data.meta.type == OPCODE_META_TYPE_STRICT_CODE)
  {
    is_strict = true;

    int_data->pos++;
  }

  ecma_object_ptr_t scope_p;
  ecma_string_t *function_name_string_p = NULL;
  if (is_named_func_expr)
  {
    ecma_create_decl_lex_env (scope_p, *int_data->lex_env_p);

    const literal_index_t lit_id = deserialize_lit_id_by_uid (function_name_lit_idx, lit_oc);
    JERRY_ASSERT (lit_id != INVALID_LITERAL);

    function_name_string_p = ecma_new_ecma_string_from_lit_index (lit_id);
    ecma_op_create_immutable_binding (scope_p,
                                      function_name_string_p);
  }
  else
  {
    scope_p = *int_data->lex_env_p;
    ecma_ref_object (scope_p);
  }

  ecma_object_ptr_t func_obj_p;
  ecma_op_create_function_object (func_obj_p,
                                  params_names,
                                  params_number,
                                  scope_p,
                                  is_strict,
                                  int_data->pos);

  set_variable_value (ret_value, int_data, lit_oc, dst_var_idx, ecma_value_t (func_obj_p));

  if (is_named_func_expr)
  {
    ecma_op_initialize_immutable_binding (scope_p,
                                          function_name_string_p,
                                          ecma_value_t (func_obj_p));
    ecma_deref_ecma_string (function_name_string_p);
  }

  ecma_deref_object (func_obj_p);
  ecma_deref_object (scope_p);

  for (uint32_t param_index = 0;
       param_index < params_number;
       param_index++)
  {
    ecma_deref_ecma_string (params_names[param_index]);
  }

  MEM_FINALIZE_LOCAL_ARRAY (params_names);

  int_data->pos = function_code_end_oc;
} /* opfunc_func_expr_n */

/**
 * 'Function call' opcode handler.
 *
 * See also: ECMA-262 v5, 11.2.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_call_n (ecma_completion_value_t &ret_value, /**< out: completion value */
               opcode_t opdata, /**< operation data */
               int_data_t *int_data) /**< interpreter context */
{
  const idx_t lhs_var_idx = opdata.data.call_n.lhs;
  const idx_t func_name_lit_idx = opdata.data.call_n.name_lit_idx;
  const idx_t args_number_idx = opdata.data.call_n.arg_list;
  const opcode_counter_t lit_oc = int_data->pos;

  ECMA_TRY_CATCH (ret_value, get_variable_value, func_value, int_data, func_name_lit_idx, false);

  int_data->pos++;

  bool this_arg_var_idx_set = false;
  idx_t this_arg_var_idx = INVALID_VALUE;
  idx_t args_number;

  opcode_t next_opcode = read_opcode (int_data->pos);
  if (next_opcode.op_idx == __op__idx_meta
      && next_opcode.data.meta.type == OPCODE_META_TYPE_THIS_ARG)
  {
    this_arg_var_idx = next_opcode.data.meta.data_1;
    JERRY_ASSERT (is_reg_variable (int_data, this_arg_var_idx));

    this_arg_var_idx_set = true;

    JERRY_ASSERT (args_number_idx > 0);
    args_number = (idx_t) (args_number_idx - 1);

    int_data->pos++;
  }
  else
  {
    args_number = args_number_idx;
  }

  MEM_DEFINE_LOCAL_ARRAY (arg_values, args_number, ecma_value_t);

  ecma_length_t args_read;
  ecma_completion_value_t get_arg_completion;
  fill_varg_list (get_arg_completion,
                  int_data,
                  args_number,
                  arg_values,
                  &args_read);

  if (ecma_is_completion_value_empty (get_arg_completion))
  {
    JERRY_ASSERT (args_read == args_number);

    ecma_completion_value_t get_this_completion_value;

    if (this_arg_var_idx_set)
    {
      get_variable_value (get_this_completion_value, int_data, this_arg_var_idx, false);
    }
    else
    {
      ecma_op_implicit_this_value (get_this_completion_value, *int_data->lex_env_p);
    }
    JERRY_ASSERT (ecma_is_completion_value_normal (get_this_completion_value));

    ecma_value_t this_value;
    ecma_get_completion_value_value (this_value, get_this_completion_value);

    if (!ecma_op_is_callable (func_value))
    {
      ecma_object_ptr_t exception_obj_p;
      ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
      ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
    }
    else
    {
      ecma_object_ptr_t func_obj_p;
      ecma_get_object_from_value (func_obj_p, func_value);

      ECMA_TRY_CATCH (ret_value,
                      ecma_op_function_call, call_ret_value, func_obj_p, this_value, arg_values, args_number);

      set_variable_value (ret_value, int_data, lit_oc, lhs_var_idx, call_ret_value);

      ECMA_FINALIZE (call_ret_value);
    }

    ecma_free_completion_value (get_this_completion_value);
  }
  else
  {
    JERRY_ASSERT (!ecma_is_completion_value_normal (get_arg_completion));

    ret_value = get_arg_completion;
  }

  for (ecma_length_t arg_index = 0;
       arg_index < args_read;
       arg_index++)
  {
    ecma_free_value (arg_values[arg_index], true);
  }

  MEM_FINALIZE_LOCAL_ARRAY (arg_values);

  ECMA_FINALIZE (func_value);
} /* opfunc_call_n */

/**
 * 'Constructor call' opcode handler.
 *
 * See also: ECMA-262 v5, 11.2.2
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_construct_n (ecma_completion_value_t &ret_value, /**< out: completion value */
                    opcode_t opdata, /**< operation data */
                    int_data_t *int_data) /**< interpreter context */
{
  const idx_t lhs_var_idx = opdata.data.construct_n.lhs;
  const idx_t constructor_name_lit_idx = opdata.data.construct_n.name_lit_idx;
  const idx_t args_number = opdata.data.construct_n.arg_list;
  const opcode_counter_t lit_oc = int_data->pos;

  ECMA_TRY_CATCH (ret_value, get_variable_value, constructor_value, int_data, constructor_name_lit_idx, false);

  MEM_DEFINE_LOCAL_ARRAY (arg_values, args_number, ecma_value_t);

  int_data->pos++;

  ecma_length_t args_read;
  ecma_completion_value_t get_arg_completion;
  fill_varg_list (get_arg_completion,
                  int_data,
                  args_number,
                  arg_values,
                  &args_read);

  if (ecma_is_completion_value_empty (get_arg_completion))
  {
    JERRY_ASSERT (args_read == args_number);

    if (!ecma_is_constructor (constructor_value))
    {
      ecma_object_ptr_t exception_obj_p;
      ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
      ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
    }
    else
    {
      ecma_object_ptr_t constructor_obj_p;
      ecma_get_object_from_value (constructor_obj_p, constructor_value);

      ECMA_TRY_CATCH (ret_value,
                      ecma_op_function_construct, construction_ret_value, constructor_obj_p, arg_values, args_number);

      set_variable_value (ret_value, int_data, lit_oc, lhs_var_idx, construction_ret_value);

      ECMA_FINALIZE (construction_ret_value);
    }
  }
  else
  {
    JERRY_ASSERT (!ecma_is_completion_value_normal (get_arg_completion));

    ret_value = get_arg_completion;
  }

  for (ecma_length_t arg_index = 0;
       arg_index < args_read;
       arg_index++)
  {
    ecma_free_value (arg_values[arg_index], true);
  }

  MEM_FINALIZE_LOCAL_ARRAY (arg_values);

  ECMA_FINALIZE (constructor_value);
} /* opfunc_construct_n */

/**
 * 'Array initializer' opcode handler.
 *
 * See also: ECMA-262 v5, 11.1.4
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_array_decl (ecma_completion_value_t &ret_value, /**< out: completion value */
                   opcode_t opdata, /**< operation data */
                   int_data_t *int_data) /**< interpreter context */
{
  const idx_t lhs_var_idx = opdata.data.array_decl.lhs;
  const idx_t args_number = opdata.data.array_decl.list;
  const opcode_counter_t lit_oc = int_data->pos;

  int_data->pos++;

  MEM_DEFINE_LOCAL_ARRAY (arg_values, args_number, ecma_value_t);

  ecma_length_t args_read;
  ecma_completion_value_t get_arg_completion;
  fill_varg_list (get_arg_completion,
                  int_data,
                  args_number,
                  arg_values,
                  &args_read);

  if (ecma_is_completion_value_empty (get_arg_completion))
  {
    JERRY_ASSERT (args_read == args_number);

    ECMA_TRY_CATCH (ret_value,
                    ecma_op_create_array_object, array_obj_value, arg_values, args_number, false);

    set_variable_value (ret_value, int_data, lit_oc, lhs_var_idx, array_obj_value);

    ECMA_FINALIZE (array_obj_value);
  }
  else
  {
    JERRY_ASSERT (!ecma_is_completion_value_normal (get_arg_completion));

    ret_value = get_arg_completion;
  }

  for (ecma_length_t arg_index = 0;
       arg_index < args_read;
       arg_index++)
  {
    ecma_free_value (arg_values[arg_index], true);
  }

  MEM_FINALIZE_LOCAL_ARRAY (arg_values);
} /* opfunc_array_decl */

/**
 * 'Object initializer' opcode handler.
 *
 * See also: ECMA-262 v5, 11.1.5
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_obj_decl (ecma_completion_value_t &ret_value, /**< out: completion value */
                 opcode_t opdata, /**< operation data */
                 int_data_t *int_data) /**< interpreter context */
{
  const idx_t lhs_var_idx = opdata.data.obj_decl.lhs;
  const idx_t args_number = opdata.data.obj_decl.list;
  const opcode_counter_t obj_lit_oc = int_data->pos;

  int_data->pos++;

  ecma_completion_value_t completion;
  ecma_object_ptr_t obj_p;
  ecma_op_create_object_object_noarg (obj_p);

  for (uint32_t prop_index = 0;
       prop_index < args_number;
       prop_index++)
  {
    ecma_completion_value_t evaluate_prop_completion;
    run_int_loop (evaluate_prop_completion, int_data);

    if (ecma_is_completion_value_normal (evaluate_prop_completion))
    {
      JERRY_ASSERT (ecma_is_completion_value_empty (evaluate_prop_completion));

      opcode_t next_opcode = read_opcode (int_data->pos);
      JERRY_ASSERT (next_opcode.op_idx == __op__idx_meta);

      const opcode_meta_type type = (opcode_meta_type) next_opcode.data.meta.type;
      JERRY_ASSERT (type == OPCODE_META_TYPE_VARG_PROP_DATA
                    || type == OPCODE_META_TYPE_VARG_PROP_GETTER
                    || type == OPCODE_META_TYPE_VARG_PROP_SETTER);

      const idx_t prop_name_var_idx = next_opcode.data.meta.data_1;
      const idx_t value_for_prop_desc_var_idx = next_opcode.data.meta.data_2;

      ecma_completion_value_t value_for_prop_desc_completion;
      get_variable_value (value_for_prop_desc_completion,
                          int_data,
                          value_for_prop_desc_var_idx,
                          false);

      if (ecma_is_completion_value_normal (value_for_prop_desc_completion))
      {
        JERRY_ASSERT (is_reg_variable (int_data, prop_name_var_idx));

        ECMA_TRY_CATCH (ret_value, get_variable_value, prop_name_value, int_data, prop_name_var_idx, false);
        ECMA_TRY_CATCH (ret_value, ecma_op_to_string, prop_name_str_value, prop_name_value);

        bool is_throw_syntax_error = false;

        ecma_string_t *prop_name_string_p = ecma_get_string_from_value (prop_name_str_value);
        ecma_property_t *previous_p = ecma_op_object_get_own_property (obj_p, prop_name_string_p);

        const bool is_previous_undefined = (previous_p == NULL);
        const bool is_previous_data_desc = (!is_previous_undefined
                                            && previous_p->type == ECMA_PROPERTY_NAMEDDATA);
        const bool is_previous_accessor_desc = (!is_previous_undefined
                                                && previous_p->type == ECMA_PROPERTY_NAMEDACCESSOR);
        JERRY_ASSERT (is_previous_undefined || is_previous_data_desc || is_previous_accessor_desc);

        ecma_property_descriptor_t prop_desc = ecma_make_empty_property_descriptor ();
        {
          prop_desc.is_enumerable_defined = true;
          prop_desc.is_enumerable = true;

          prop_desc.is_configurable_defined = true;
          prop_desc.is_configurable = true;
        }

        ecma_value_t value_for_prop_desc;
        ecma_get_completion_value_value (value_for_prop_desc, value_for_prop_desc_completion);

        if (type == OPCODE_META_TYPE_VARG_PROP_DATA)
        {
          prop_desc.is_value_defined = true;

          prop_desc.value = (ecma_value_packed_t) value_for_prop_desc;

          prop_desc.is_writable_defined = true;
          prop_desc.is_writable = true;

          if (!is_previous_undefined
              && ((is_previous_data_desc
                   && int_data->is_strict)
                  || is_previous_accessor_desc))
          {
            is_throw_syntax_error = true;
          }
        }
        else if (type == OPCODE_META_TYPE_VARG_PROP_GETTER)
        {
          prop_desc.is_get_defined = true;
          ecma_object_ptr_t get_p;
          ecma_get_object_from_value (get_p, value_for_prop_desc);
          prop_desc.get_p = (ecma_object_t*) get_p;

          if (!is_previous_undefined
              && is_previous_data_desc)
          {
            is_throw_syntax_error = true;
          }
        }
        else
        {
          prop_desc.is_set_defined = true;
          ecma_object_ptr_t set_p;
          ecma_get_object_from_value (set_p, value_for_prop_desc);
          prop_desc.set_p = (ecma_object_t*) set_p;

          if (!is_previous_undefined
              && is_previous_data_desc)
          {
            is_throw_syntax_error = true;
          }
        }

        /* The SyntaxError should be treated as an early error  */
        JERRY_ASSERT (!is_throw_syntax_error);

        ecma_completion_value_t define_prop_completion;
        ecma_op_object_define_own_property (define_prop_completion,
                                            obj_p,
                                            prop_name_string_p,
                                            &prop_desc,
                                            false);
        JERRY_ASSERT (ecma_is_completion_value_normal_true (define_prop_completion)
                      || ecma_is_completion_value_normal_false (define_prop_completion));

        ecma_free_completion_value (value_for_prop_desc_completion);

        ECMA_FINALIZE (prop_name_str_value);
        ECMA_FINALIZE (prop_name_value);
      }
      else
      {
        completion = value_for_prop_desc_completion;

        break;
      }

      int_data->pos++;
    }
    else
    {
      JERRY_ASSERT (!ecma_is_completion_value_normal (evaluate_prop_completion));

      completion = evaluate_prop_completion;

      break;
    }
  }

  if (ecma_is_completion_value_empty (completion))
  {
    set_variable_value (ret_value, int_data, obj_lit_oc, lhs_var_idx, ecma_value_t (obj_p));
  }
  else
  {
    ret_value = completion;
  }

  ecma_deref_object (obj_p);
} /* opfunc_obj_decl */
/**
 * 'Return with no expression' opcode handler.
 *
 * See also: ECMA-262 v5, 12.9
 *
 * @return completion value
 *         Returned value is simple and so need not be freed.
 *         However, ecma_free_completion_value may be called for it, but it is a no-op.
 */
void
opfunc_ret (ecma_completion_value_t &ret_value, /**< out: completion value */
            opcode_t opdata __unused, /**< operation data */
            int_data_t *int_data __unused) /**< interpreter context */
{
  ecma_make_return_completion_value (ret_value, ecma_value_t (ECMA_SIMPLE_VALUE_UNDEFINED));
} /* opfunc_ret */

/**
 * 'Return with expression' opcode handler.
 *
 * See also: ECMA-262 v5, 12.9
 *
 * @return completion value
 *         Returned value is simple and so need not be freed.
 *         However, ecma_free_completion_value may be called for it, but it is a no-op.
 */
void
opfunc_retval (ecma_completion_value_t &ret_value, /**< out: completion value */
               opcode_t opdata __unused, /**< operation data */
               int_data_t *int_data __unused) /**< interpreter context */
{
  ECMA_TRY_CATCH (ret_value, get_variable_value, expr_val, int_data, opdata.data.retval.ret_value, false);

  ecma_value_t value_copy;
  ecma_copy_value (value_copy, expr_val, true);

  ecma_make_return_completion_value (ret_value, value_copy);

  ECMA_FINALIZE (expr_val);
} /* opfunc_retval */

/**
 * 'Property getter' opcode handler.
 *
 * See also: ECMA-262 v5, 11.2.1
 *           ECMA-262 v5, 11.13.1
 *
 * @return completion value
 *         returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_prop_getter (ecma_completion_value_t &ret_value, /**< out: completion value */
                    opcode_t opdata __unused, /**< operation data */
                    int_data_t *int_data __unused) /**< interpreter context */
{
  const idx_t lhs_var_idx = opdata.data.prop_getter.lhs;
  const idx_t base_var_idx = opdata.data.prop_getter.obj;
  const idx_t prop_name_var_idx = opdata.data.prop_getter.prop;

  ECMA_TRY_CATCH (ret_value, get_variable_value, base_value, int_data, base_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, prop_name_value, int_data, prop_name_var_idx, false);
  ECMA_TRY_CATCH (ret_value, ecma_op_check_object_coercible, check_coercible_ret, base_value);
  ECMA_TRY_CATCH (ret_value, ecma_op_to_string, prop_name_str_value, prop_name_value);

  ecma_string_t *prop_name_string_p = ecma_get_string_from_value (prop_name_str_value);

  ecma_reference_t ref;
  ecma_make_reference (ref, base_value, prop_name_string_p, int_data->is_strict);

  ECMA_TRY_CATCH (ret_value, ecma_op_get_value_object_base, prop_value, ref);

  set_variable_value (ret_value, int_data, int_data->pos, lhs_var_idx, prop_value);

  ECMA_FINALIZE (prop_value);

  ecma_free_reference (ref);

  ECMA_FINALIZE (prop_name_str_value);
  ECMA_FINALIZE (check_coercible_ret);
  ECMA_FINALIZE (prop_name_value);
  ECMA_FINALIZE (base_value);

  int_data->pos++;
} /* opfunc_prop_getter */

/**
 * 'Property setter' opcode handler.
 *
 * See also: ECMA-262 v5, 11.2.1
 *           ECMA-262 v5, 11.13.1
 *
 * @return completion value
 *         returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_prop_setter (ecma_completion_value_t &ret_value, /**< out: completion value */
                    opcode_t opdata __unused, /**< operation data */
                    int_data_t *int_data __unused) /**< interpreter context */
{
  const idx_t base_var_idx = opdata.data.prop_setter.obj;
  const idx_t prop_name_var_idx = opdata.data.prop_setter.prop;
  const idx_t rhs_var_idx = opdata.data.prop_setter.rhs;

  ECMA_TRY_CATCH (ret_value, get_variable_value, base_value, int_data, base_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, prop_name_value, int_data, prop_name_var_idx, false);
  ECMA_TRY_CATCH (ret_value, ecma_op_check_object_coercible, check_coercible_ret, base_value);
  ECMA_TRY_CATCH (ret_value, ecma_op_to_string, prop_name_str_value, prop_name_value);

  ecma_string_t *prop_name_string_p = ecma_get_string_from_value (prop_name_str_value);

  ecma_reference_t ref;
  ecma_make_reference (ref,
                       base_value,
                       prop_name_string_p,
                       int_data->is_strict);

  ECMA_TRY_CATCH (ret_value, get_variable_value, rhs_value, int_data, rhs_var_idx, false);
  ecma_op_put_value_object_base (ret_value, ref, rhs_value);
  ECMA_FINALIZE (rhs_value);

  ecma_free_reference (ref);

  ECMA_FINALIZE (prop_name_str_value);
  ECMA_FINALIZE (check_coercible_ret);
  ECMA_FINALIZE (prop_name_value);
  ECMA_FINALIZE (base_value);

  int_data->pos++;
} /* opfunc_prop_setter */

/**
 * Exit from script with specified status code:
 *   0 - for successful completion
 *   1 - to indicate failure.
 *
 * Note: this is not ECMA specification-defined, but internal
 *       implementation-defined opcode for end of script
 *       and assertions inside of unit tests.
 *
 * @return completion value
 *         Returned value is simple and so need not be freed.
 *         However, ecma_free_completion_value may be called for it, but it is a no-op.
 */
void
opfunc_exitval (ecma_completion_value_t &ret_value, /**< out: completion value */
                opcode_t opdata, /**< operation data */
                int_data_t *int_data __unused) /**< interpreter context */
{
  JERRY_ASSERT (opdata.data.exitval.status_code == 0
                || opdata.data.exitval.status_code == 1);

  ecma_make_exit_completion_value (ret_value, opdata.data.exitval.status_code == 0);
} /* opfunc_exitval */

/**
 * 'Logical NOT Operator' opcode handler.
 *
 * See also: ECMA-262 v5, 11.4.9
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_logical_not (ecma_completion_value_t &ret_value, /**< out: completion value */
                    opcode_t opdata, /**< operation data */
                    int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.logical_not.dst;
  const idx_t right_var_idx = opdata.data.logical_not.var_right;

  ECMA_TRY_CATCH (ret_value, get_variable_value, right_value, int_data, right_var_idx, false);

  ecma_simple_value_t old_value = ECMA_SIMPLE_VALUE_TRUE;
  ecma_completion_value_t to_bool_value;
  ecma_op_to_boolean (to_bool_value, right_value);

  if (ecma_is_completion_value_normal_true (to_bool_value))
  {
    old_value = ECMA_SIMPLE_VALUE_FALSE;
  }

  set_variable_value (ret_value, int_data, int_data->pos, dst_var_idx, ecma_value_t (old_value));

  ECMA_FINALIZE (right_value);

  int_data->pos++;
} /* opfunc_logical_not */

/**
 * 'This' opcode handler.
 *
 * See also: ECMA-262 v5, 11.1.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_this_binding (ecma_completion_value_t &ret_value, /**< out: completion value */
                     opcode_t opdata, /**< operation data */
                     int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.this_binding.lhs;
  const opcode_counter_t lit_oc = int_data->pos;

  int_data->pos++;

  set_variable_value (ret_value, int_data, lit_oc, dst_var_idx, *int_data->this_binding_p);
} /* opfunc_this_binding */

/**
 * 'With' opcode handler.
 *
 * See also: ECMA-262 v5, 12.10
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_with (ecma_completion_value_t &ret_value, /**< out: completion value */
             opcode_t opdata, /**< operation data */
             int_data_t *int_data) /**< interpreter context */
{
  const idx_t expr_var_idx = opdata.data.with.expr;

  ECMA_TRY_CATCH (ret_value, get_variable_value, expr_value, int_data, expr_var_idx, false);
  ECMA_TRY_CATCH (ret_value, ecma_op_to_object, obj_expr_value, expr_value);

  int_data->pos++;

  ecma_object_ptr_t obj_p;
  ecma_get_object_from_value (obj_p, obj_expr_value);

  const ecma_object_ptr_t* old_env_p = int_data->lex_env_p;
  ecma_object_ptr_t new_env_p;
  ecma_create_object_lex_env (new_env_p,
                              *old_env_p,
                              obj_p,
                              true);
  int_data->lex_env_p = &new_env_p;

  ecma_completion_value_t evaluation_completion;
  run_int_loop (evaluation_completion, int_data);

  if (ecma_is_completion_value_normal (evaluation_completion))
  {
    JERRY_ASSERT (ecma_is_completion_value_empty (evaluation_completion));

    opcode_t meta_opcode = read_opcode (int_data->pos);
    JERRY_ASSERT (meta_opcode.op_idx == __op__idx_meta);
    JERRY_ASSERT (meta_opcode.data.meta.type == OPCODE_META_TYPE_END_WITH);

    int_data->pos++;

    ecma_make_empty_completion_value (ret_value);
  }
  else
  {
    ret_value = evaluation_completion;
  }

  int_data->lex_env_p = old_env_p;

  ecma_deref_object (new_env_p);

  ECMA_FINALIZE (obj_expr_value);
  ECMA_FINALIZE (expr_value);
} /* opfunc_with */

/**
 * 'Throw' opcode handler.
 *
 * See also: ECMA-262 v5, 12.13
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_throw_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                    opcode_t opdata, /**< operation data */
                    int_data_t *int_data) /**< interpreter context */
{
  const idx_t var_idx = opdata.data.throw_value.var;

  ECMA_TRY_CATCH (ret_value, get_variable_value, var_value, int_data, var_idx, false);

  ecma_value_t var_value_copy;
  ecma_copy_value (var_value_copy, var_value, true);

  ecma_make_throw_completion_value (ret_value, var_value_copy);

  ECMA_FINALIZE (var_value);

  int_data->pos++;
} /* opfunc_throw_value */

/**
 * Evaluate argument of typeof.
 *
 * See also: ECMA-262 v5, 11.4.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
static void
evaluate_arg_for_typeof (ecma_completion_value_t &ret_value, /**< out: completion value */
                         int_data_t *int_data, /**< interpreter context */
                         idx_t var_idx) /**< arg variable identifier */
{
  if (is_reg_variable (int_data, var_idx))
  {
    // 2.b
    get_variable_value (ret_value,
                        int_data,
                        var_idx,
                        false);
    JERRY_ASSERT (ecma_is_completion_value_normal (ret_value));
  }
  else
  {
    const literal_index_t lit_id = deserialize_lit_id_by_uid (var_idx, int_data->pos);
    JERRY_ASSERT (lit_id != INVALID_LITERAL);

    ecma_string_t *var_name_string_p = ecma_new_ecma_string_from_lit_index (lit_id);

    ecma_object_ptr_t ref_base_lex_env_p;
    ecma_op_resolve_reference_base (ref_base_lex_env_p, *int_data->lex_env_p, var_name_string_p);
    if (ref_base_lex_env_p.is_null ())
    {
      ecma_make_simple_completion_value (ret_value, ECMA_SIMPLE_VALUE_UNDEFINED);
    }
    else
    {
      ecma_op_get_value_lex_env_base (ret_value,
                                      ref_base_lex_env_p,
                                      var_name_string_p,
                                      int_data->is_strict);
    }

    ecma_deref_ecma_string (var_name_string_p);
  }
} /* evaluate_arg_for_typeof */

/**
 * 'typeof' opcode handler.
 *
 * See also: ECMA-262 v5, 11.4.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_typeof (ecma_completion_value_t &ret_value, /**< out: completion value */
               opcode_t opdata, /**< operation data */
               int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.typeof.lhs;
  const idx_t obj_var_idx = opdata.data.typeof.obj;

  ECMA_TRY_CATCH (ret_value, evaluate_arg_for_typeof, typeof_arg, int_data, obj_var_idx);

  ecma_string_t *type_str_p = NULL;

  if (ecma_is_value_undefined (typeof_arg))
  {
    type_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_UNDEFINED);
  }
  else if (ecma_is_value_null (typeof_arg))
  {
    type_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_OBJECT);
  }
  else if (ecma_is_value_boolean (typeof_arg))
  {
    type_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_BOOLEAN);
  }
  else if (ecma_is_value_number (typeof_arg))
  {
    type_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NUMBER);
  }
  else if (ecma_is_value_string (typeof_arg))
  {
    type_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_STRING);
  }
  else
  {
    JERRY_ASSERT (ecma_is_value_object (typeof_arg));

    if (ecma_op_is_callable (typeof_arg))
    {
      type_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_FUNCTION);
    }
    else
    {
      type_str_p = ecma_get_magic_string (ECMA_MAGIC_STRING_OBJECT);
    }
  }

  set_variable_value (ret_value, int_data, int_data->pos, dst_var_idx, ecma_value_t (type_str_p));

  ecma_deref_ecma_string (type_str_p);

  ECMA_FINALIZE (typeof_arg);

  int_data->pos++;
} /* opfunc_typeof */

/**
 * 'delete' opcode handler.
 *
 * See also: ECMA-262 v5, 11.4.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_delete_var (ecma_completion_value_t &ret_value, /**< out: completion value */
                   opcode_t opdata, /**< operation data */
                   int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.delete_var.lhs;
  const idx_t name_lit_idx = opdata.data.delete_var.name;
  const opcode_counter_t lit_oc = int_data->pos;

  int_data->pos++;

  const literal_index_t lit_id = deserialize_lit_id_by_uid (name_lit_idx, lit_oc);
  JERRY_ASSERT (lit_id != INVALID_LITERAL);

  ecma_string_t *name_string_p = ecma_new_ecma_string_from_lit_index (lit_id);

  ecma_reference_t ref;
  ecma_op_get_identifier_reference (ref,
                                    *int_data->lex_env_p,
                                    name_string_p,
                                    int_data->is_strict);

  if (ref.is_strict)
  {
    /* SyntaxError should be treated as an early error */
    JERRY_UNREACHABLE ();
  }
  else
  {
    if (ecma_is_value_undefined (ref.base))
    {
      set_variable_value (ret_value, int_data, lit_oc, dst_var_idx, ecma_value_t (ECMA_SIMPLE_VALUE_TRUE));
    }
    else
    {
      ecma_object_ptr_t bindings_p;
      ecma_get_object_from_value (bindings_p, ref.base);
      JERRY_ASSERT (ecma_is_lexical_environment (bindings_p));

      ECMA_TRY_CATCH (ret_value, ecma_op_delete_binding, delete_completion, bindings_p,
                      ECMA_GET_NON_NULL_POINTER (ecma_string_t,
                                                 ref.referenced_name_cp));

      set_variable_value (ret_value, int_data, lit_oc, dst_var_idx, delete_completion);

      ECMA_FINALIZE (delete_completion);
    }
  }

  ecma_free_reference (ref);

  ecma_deref_ecma_string (name_string_p);
} /* opfunc_delete_var */


/**
 * 'delete' opcode handler.
 *
 * See also: ECMA-262 v5, 11.4.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_delete_prop (ecma_completion_value_t &ret_value, /**< out: completion value */
                    opcode_t opdata, /**< operation data */
                    int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.delete_prop.lhs;
  const idx_t base_var_idx = opdata.data.delete_prop.base;
  const idx_t name_var_idx = opdata.data.delete_prop.name;

  ECMA_TRY_CATCH (ret_value, get_variable_value, base_value, int_data, base_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, name_value, int_data, name_var_idx, false);
  ECMA_TRY_CATCH (ret_value, ecma_op_check_object_coercible, check_coercible_ret, base_value);
  ECMA_TRY_CATCH (ret_value, ecma_op_to_string, str_name_value, name_value);

  JERRY_ASSERT (ecma_is_value_string (str_name_value));
  ecma_string_t *name_string_p = ecma_get_string_from_value (str_name_value);

  if (ecma_is_value_undefined (base_value))
  {
    if (int_data->is_strict)
    {
      /* SyntaxError should be treated as an early error */
      JERRY_UNREACHABLE ();
    }
    else
    {
      ecma_make_simple_completion_value (ret_value, ECMA_SIMPLE_VALUE_TRUE);
    }
  }
  else
  {
    ECMA_TRY_CATCH (ret_value, ecma_op_to_object, obj_value, base_value);

    JERRY_ASSERT (ecma_is_value_object (obj_value));
    ecma_object_ptr_t obj_p;
    ecma_get_object_from_value (obj_p, obj_value);
    JERRY_ASSERT (!ecma_is_lexical_environment (obj_p));

    ECMA_TRY_CATCH (ret_value, ecma_op_object_delete, delete_op_ret_val, obj_p, name_string_p, int_data->is_strict);

    set_variable_value (ret_value, int_data, int_data->pos, dst_var_idx, delete_op_ret_val);

    ECMA_FINALIZE (delete_op_ret_val);
    ECMA_FINALIZE (obj_value);
  }

  ECMA_FINALIZE (str_name_value);
  ECMA_FINALIZE (check_coercible_ret);
  ECMA_FINALIZE (name_value);
  ECMA_FINALIZE (base_value);

  int_data->pos++;
} /* opfunc_delete_prop */

/**
 * 'meta' opcode handler.
 *
 * @return implementation-defined meta completion value
 */
void
opfunc_meta (ecma_completion_value_t &ret_value, /**< out: completion value */
             opcode_t opdata, /**< operation data */
             int_data_t *int_data __unused) /**< interpreter context */
{
  const opcode_meta_type type = (opcode_meta_type) opdata.data.meta.type;

  switch (type)
  {
    case OPCODE_META_TYPE_VARG:
    case OPCODE_META_TYPE_VARG_PROP_DATA:
    case OPCODE_META_TYPE_VARG_PROP_GETTER:
    case OPCODE_META_TYPE_VARG_PROP_SETTER:
    case OPCODE_META_TYPE_END_WITH:
    case OPCODE_META_TYPE_CATCH:
    case OPCODE_META_TYPE_FINALLY:
    case OPCODE_META_TYPE_END_TRY_CATCH_FINALLY:
    {
      ecma_make_meta_completion_value (ret_value);
      return;
    }

    case OPCODE_META_TYPE_STRICT_CODE:
    {
      FIXME (/* Handle in run_int_from_pos */);
      ecma_make_meta_completion_value (ret_value);
      return;
    }

    case OPCODE_META_TYPE_UNDEFINED:
    case OPCODE_META_TYPE_THIS_ARG:
    case OPCODE_META_TYPE_FUNCTION_END:
    case OPCODE_META_TYPE_CATCH_EXCEPTION_IDENTIFIER:
    {
      JERRY_UNREACHABLE ();
    }
  }

  JERRY_UNREACHABLE ();
} /* opfunc_meta */

/**
 * Calculate opcode counter from 'meta' opcode's data arguments.
 *
 * @return opcode counter
 */
opcode_counter_t
calc_opcode_counter_from_idx_idx (const idx_t oc_idx_1, /**< first idx */
                                  const idx_t oc_idx_2) /**< second idx */
{
  opcode_counter_t counter;

  counter = oc_idx_1;
  counter = (opcode_counter_t) (counter << (sizeof (idx_t) * JERRY_BITSINBYTE));
  counter = (opcode_counter_t) (counter | oc_idx_2);

  return counter;
} /* calc_meta_opcode_counter_from_meta_data */

/**
 * Read opcode counter from current opcode,
 * that should be 'meta' opcode of type 'opcode counter'.
 */
opcode_counter_t
read_meta_opcode_counter (opcode_meta_type expected_type, /**< expected type of meta opcode */
                          int_data_t *int_data) /**< interpreter context */
{
  opcode_t meta_opcode = read_opcode (int_data->pos);
  JERRY_ASSERT (meta_opcode.data.meta.type == expected_type);

  const idx_t data_1 = meta_opcode.data.meta.data_1;
  const idx_t data_2 = meta_opcode.data.meta.data_2;

  return calc_opcode_counter_from_idx_idx (data_1, data_2);
} /* read_meta_opcode_counter */

#define GETOP_DEF_1(a, name, field1) \
        opcode_t getop_##name (idx_t arg1) \
        { \
          opcode_t opdata; \
          opdata.op_idx = __op__idx_##name; \
          opdata.data.name.field1 = arg1; \
          return opdata; \
        }

#define GETOP_DEF_2(a, name, field1, field2) \
        opcode_t getop_##name (idx_t arg1, idx_t arg2) \
        { \
          opcode_t opdata; \
          opdata.op_idx = __op__idx_##name; \
          opdata.data.name.field1 = arg1; \
          opdata.data.name.field2 = arg2; \
          return opdata; \
        }

#define GETOP_DEF_3(a, name, field1, field2, field3) \
        opcode_t getop_##name (idx_t arg1, idx_t arg2, idx_t arg3) \
        { \
          opcode_t opdata; \
          opdata.op_idx = __op__idx_##name; \
          opdata.data.name.field1 = arg1; \
          opdata.data.name.field2 = arg2; \
          opdata.data.name.field3 = arg3; \
          return opdata; \
        }

OP_ARGS_LIST (GETOP_DEF)
