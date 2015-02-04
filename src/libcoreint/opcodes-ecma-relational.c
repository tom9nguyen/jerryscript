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

#include "opcodes.h"
#include "opcodes-ecma-support.h"

/**
 * 'Less-than' opcode handler.
 *
 * See also: ECMA-262 v5, 11.8.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_less_than (ecma_completion_value_t &ret_value, /**< out: completion value */
                  opcode_t opdata, /**< operation data */
                  int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.less_than.dst;
  const idx_t left_var_idx = opdata.data.less_than.var_left;
  const idx_t right_var_idx = opdata.data.less_than.var_right;

  ECMA_TRY_CATCH (ret_value, get_variable_value, left_value, int_data, left_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, right_value, int_data, right_var_idx, false);
  ECMA_TRY_CATCH (ret_value, ecma_op_abstract_relational_compare, compare_result, left_value, right_value, true);

  ecma_simple_value_t res;

  if (ecma_is_value_undefined (compare_result))
  {
    res = ECMA_SIMPLE_VALUE_FALSE;
  }
  else
  {
    JERRY_ASSERT (ecma_is_value_boolean (compare_result));

    res = (ecma_is_value_true (compare_result) ? ECMA_SIMPLE_VALUE_TRUE : ECMA_SIMPLE_VALUE_FALSE);
  }

  set_variable_value (ret_value, int_data, int_data->pos, dst_var_idx, ecma_value_t (res));

  ECMA_FINALIZE (compare_result);
  ECMA_FINALIZE (right_value);
  ECMA_FINALIZE (left_value);

  int_data->pos++;
} /* opfunc_less_than */

/**
 * 'Greater-than' opcode handler.
 *
 * See also: ECMA-262 v5, 11.8.2
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_greater_than (ecma_completion_value_t &ret_value, /**< out: completion value */
                     opcode_t opdata, /**< operation data */
                     int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.greater_than.dst;
  const idx_t left_var_idx = opdata.data.greater_than.var_left;
  const idx_t right_var_idx = opdata.data.greater_than.var_right;

  ECMA_TRY_CATCH (ret_value, get_variable_value, left_value, int_data, left_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, right_value, int_data, right_var_idx, false);
  ECMA_TRY_CATCH (ret_value, ecma_op_abstract_relational_compare, compare_result, right_value, left_value, false);

  ecma_simple_value_t res;

  if (ecma_is_value_undefined (compare_result))
  {
    res = ECMA_SIMPLE_VALUE_FALSE;
  }
  else
  {
    JERRY_ASSERT (ecma_is_value_boolean (compare_result));

    res = (ecma_is_value_true (compare_result) ? ECMA_SIMPLE_VALUE_TRUE : ECMA_SIMPLE_VALUE_FALSE);
  }

  set_variable_value (ret_value, int_data, int_data->pos, dst_var_idx, ecma_value_t (res));

  ECMA_FINALIZE (compare_result);
  ECMA_FINALIZE (right_value);
  ECMA_FINALIZE (left_value);

  int_data->pos++;
} /* opfunc_greater_than */

/**
 * 'Less-than-or-equal' opcode handler.
 *
 * See also: ECMA-262 v5, 11.8.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_less_or_equal_than (ecma_completion_value_t &ret_value, /**< out: completion value */
                           opcode_t opdata, /**< operation data */
                           int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.less_or_equal_than.dst;
  const idx_t left_var_idx = opdata.data.less_or_equal_than.var_left;
  const idx_t right_var_idx = opdata.data.less_or_equal_than.var_right;

  ECMA_TRY_CATCH (ret_value, get_variable_value, left_value, int_data, left_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, right_value, int_data, right_var_idx, false);
  ECMA_TRY_CATCH (ret_value, ecma_op_abstract_relational_compare, compare_result, right_value, left_value, false);

  ecma_simple_value_t res;

  if (ecma_is_value_undefined (compare_result))
  {
    res = ECMA_SIMPLE_VALUE_FALSE;
  }
  else
  {
    JERRY_ASSERT (ecma_is_value_boolean (compare_result));

    if (ecma_is_value_true (compare_result))
    {
      res = ECMA_SIMPLE_VALUE_FALSE;
    }
    else
    {
      res = ECMA_SIMPLE_VALUE_TRUE;
    }
  }

  set_variable_value (ret_value, int_data, int_data->pos, dst_var_idx, ecma_value_t (res));

  ECMA_FINALIZE (compare_result);
  ECMA_FINALIZE (right_value);
  ECMA_FINALIZE (left_value);

  int_data->pos++;
} /* opfunc_less_or_equal_than */

/**
 * 'Greater-than-or-equal' opcode handler.
 *
 * See also: ECMA-262 v5, 11.8.4
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
opfunc_greater_or_equal_than (ecma_completion_value_t &ret_value, /**< out: completion value */
                              opcode_t opdata, /**< operation data */
                              int_data_t *int_data) /**< interpreter context */
{
  const idx_t dst_var_idx = opdata.data.greater_or_equal_than.dst;
  const idx_t left_var_idx = opdata.data.greater_or_equal_than.var_left;
  const idx_t right_var_idx = opdata.data.greater_or_equal_than.var_right;

  ECMA_TRY_CATCH (ret_value, get_variable_value, left_value, int_data, left_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, right_value, int_data, right_var_idx, false);
  ECMA_TRY_CATCH (ret_value, ecma_op_abstract_relational_compare, compare_result, left_value, right_value, true);

  ecma_simple_value_t res;

  if (ecma_is_value_undefined (compare_result))
  {
    res = ECMA_SIMPLE_VALUE_FALSE;
  }
  else
  {
    JERRY_ASSERT (ecma_is_value_boolean (compare_result));

    if (ecma_is_value_true (compare_result))
    {
      res = ECMA_SIMPLE_VALUE_FALSE;
    }
    else
    {
      res = ECMA_SIMPLE_VALUE_TRUE;
    }
  }

  set_variable_value (ret_value, int_data, int_data->pos, dst_var_idx, ecma_value_t (res));

  ECMA_FINALIZE (compare_result);
  ECMA_FINALIZE (right_value);
  ECMA_FINALIZE (left_value);

  int_data->pos++;
} /* opfunc_greater_or_equal_than */

/**
 * 'instanceof' opcode handler.
 *
 * See also: ECMA-262 v5, 11.8.6
 *
 * @return completion value
 *         returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_instanceof (ecma_completion_value_t &ret_value, /**< out: completion value */
                   opcode_t opdata __unused, /**< operation data */
                   int_data_t *int_data __unused) /**< interpreter context */
{
  const idx_t dst_idx = opdata.data.instanceof.dst;
  const idx_t left_var_idx = opdata.data.instanceof.var_left;
  const idx_t right_var_idx = opdata.data.instanceof.var_right;

  ECMA_TRY_CATCH (ret_value, get_variable_value, left_value, int_data, left_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, right_value, int_data, right_var_idx, false);

  if (!ecma_is_value_object (right_value))
  {
    ecma_object_ptr_t exception_obj_p;
    ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
    ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
  }
  else
  {
    ecma_object_ptr_t right_value_obj_p;
    ecma_get_object_from_value (right_value_obj_p, right_value);

    ECMA_TRY_CATCH (ret_value, ecma_op_object_has_instance, is_instance_of, right_value_obj_p, left_value);

    set_variable_value (ret_value, int_data, int_data->pos, dst_idx, is_instance_of);

    ECMA_FINALIZE (is_instance_of);
  }

  ECMA_FINALIZE (right_value);
  ECMA_FINALIZE (left_value);

  int_data->pos++;
} /* opfunc_instanceof */

/**
 * 'in' opcode handler.
 *
 * See also: ECMA-262 v5, 11.8.7
 *
 * @return completion value
 *         returned value must be freed with ecma_free_completion_value.
 */
void
opfunc_in (ecma_completion_value_t &ret_value, /**< out: completion value */
           opcode_t opdata __unused, /**< operation data */
           int_data_t *int_data __unused) /**< interpreter context */
{
  const idx_t dst_idx = opdata.data.in.dst;
  const idx_t left_var_idx = opdata.data.in.var_left;
  const idx_t right_var_idx = opdata.data.in.var_right;

  ECMA_TRY_CATCH (ret_value, get_variable_value, left_value, int_data, left_var_idx, false);
  ECMA_TRY_CATCH (ret_value, get_variable_value, right_value, int_data, right_var_idx, false);

  if (!ecma_is_value_object (right_value))
  {
    ecma_object_ptr_t exception_obj_p;
    ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
    ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
  }
  else
  {
    ECMA_TRY_CATCH (ret_value, ecma_op_to_string, str_left_value, left_value);

    ecma_simple_value_t is_in = ECMA_SIMPLE_VALUE_UNDEFINED;
    ecma_string_t *left_value_prop_name_p = ecma_get_string_from_value (str_left_value);
    ecma_object_ptr_t right_value_obj_p;
    ecma_get_object_from_value (right_value_obj_p, right_value);

    if (ecma_op_object_get_property (right_value_obj_p, left_value_prop_name_p) != NULL)
    {
      is_in = ECMA_SIMPLE_VALUE_TRUE;
    }
    else
    {
      is_in = ECMA_SIMPLE_VALUE_FALSE;
    }

    set_variable_value (ret_value, int_data, int_data->pos,
                        dst_idx, ecma_value_t (is_in));

    ECMA_FINALIZE (str_left_value);
  }

  ECMA_FINALIZE (right_value);
  ECMA_FINALIZE (left_value);

  int_data->pos++;
} /* opfunc_in */
