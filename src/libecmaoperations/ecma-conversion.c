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

/**
 * Implementation of ECMA-defined conversion routines
 */

#include "ecma-alloc.h"
#include "ecma-boolean-object.h"
#include "ecma-conversion.h"
#include "ecma-exceptions.h"
#include "ecma-function-object.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-number-object.h"
#include "ecma-objects.h"
#include "ecma-objects-general.h"
#include "ecma-string-object.h"
#include "ecma-try-catch-macro.h"
#include "jerry-libc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmaconversion ECMA conversion routines
 * @{
 */

/**
 * CheckObjectCoercible operation.
 *
 * See also:
 *          ECMA-262 v5, 9.10
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
ecma_op_check_object_coercible (ecma_completion_value_t &ret_value, /**< out: completion value */
                                const ecma_value_t& value) /**< ecma-value */
{
  ecma_check_value_type_is_spec_defined (value);

  if (ecma_is_value_undefined (value)
      || ecma_is_value_null (value))
  {
    ecma_object_ptr_t exception_obj_p;
    ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
    ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
  }
  else
  {
    ecma_make_empty_completion_value (ret_value);
  }
} /* ecma_op_check_object_coercible */

/**
 * SameValue operation.
 *
 * See also:
 *          ECMA-262 v5, 9.12
 *
 * @return true - if the value are same according to ECMA-defined SameValue algorithm,
 *         false - otherwise.
 */
bool
ecma_op_same_value (const ecma_value_t& x, /**< ecma-value */
                    const ecma_value_t& y) /**< ecma-value */
{
  const bool is_x_undefined = ecma_is_value_undefined (x);
  const bool is_x_null = ecma_is_value_null (x);
  const bool is_x_boolean = ecma_is_value_boolean (x);
  const bool is_x_number = ecma_is_value_number (x);
  const bool is_x_string = ecma_is_value_string (x);
  const bool is_x_object = ecma_is_value_object (x);

  const bool is_y_undefined = ecma_is_value_undefined (y);
  const bool is_y_null = ecma_is_value_null (y);
  const bool is_y_boolean = ecma_is_value_boolean (y);
  const bool is_y_number = ecma_is_value_number (y);
  const bool is_y_string = ecma_is_value_string (y);
  const bool is_y_object = ecma_is_value_object (y);

  const bool is_types_equal = ((is_x_undefined && is_y_undefined)
                               || (is_x_null && is_y_null)
                               || (is_x_boolean && is_y_boolean)
                               || (is_x_number && is_y_number)
                               || (is_x_string && is_y_string)
                               || (is_x_object && is_y_object));

  if (!is_types_equal)
  {
    return false;
  }

  if (is_x_undefined
      || is_x_null)
  {
    return true;
  }

  if (is_x_number)
  {
    ecma_number_t *x_num_p = ecma_get_number_from_value (x);
    ecma_number_t *y_num_p = ecma_get_number_from_value (y);

    if (ecma_number_is_nan (*x_num_p)
        && ecma_number_is_nan (*y_num_p))
    {
      return true;
    }
    else if (ecma_number_is_zero (*x_num_p)
             && ecma_number_is_zero (*y_num_p)
             && ecma_number_is_negative (*x_num_p) != ecma_number_is_negative (*y_num_p))
    {
      return false;
    }

    return (*x_num_p == *y_num_p);
  }

  if (is_x_string)
  {
    ecma_string_t* x_str_p = ecma_get_string_from_value (x);
    ecma_string_t* y_str_p = ecma_get_string_from_value (y);

    return ecma_compare_ecma_strings (x_str_p, y_str_p);
  }

  if (is_x_boolean)
  {
    return (ecma_is_value_true (x) == ecma_is_value_true (y));
  }

  JERRY_ASSERT(is_x_object);

  ecma_object_ptr_t x_obj_p, y_obj_p;
  ecma_get_object_from_value (x_obj_p, x);
  ecma_get_object_from_value (y_obj_p, y);

  return (x_obj_p == y_obj_p);
} /* ecma_op_same_value */

/**
 * ToPrimitive operation.
 *
 * See also:
 *          ECMA-262 v5, 9.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
ecma_op_to_primitive (ecma_completion_value_t &ret_value, /**< out: completion value */
                      const ecma_value_t& value, /**< ecma-value */
                      ecma_preferred_type_hint_t preferred_type) /**< preferred type hint */
{
  ecma_check_value_type_is_spec_defined (value);

  if (ecma_is_value_object (value))
  {
    ecma_object_ptr_t obj_p;
    ecma_get_object_from_value (obj_p, value);

    ecma_op_object_default_value (ret_value, obj_p, preferred_type);
  }
  else
  {
    ecma_value_t value_copy;
    ecma_copy_value (value_copy, value, true);

    ecma_make_normal_completion_value (ret_value, value_copy);
  }
} /* ecma_op_to_primitive */

/**
 * ToBoolean operation.
 *
 * See also:
 *          ECMA-262 v5, 9.2
 *
 * @return completion value
 *         Returned value is simple and so need not be freed.
 *         However, ecma_free_completion_value may be called for it, but it is a no-op.
 */
void
ecma_op_to_boolean (ecma_completion_value_t &ret_value, /**< out: completion value */
                    const ecma_value_t& value) /**< ecma-value */
{
  ecma_check_value_type_is_spec_defined (value);

  ecma_simple_value_t boolean;

  if (ecma_is_value_boolean (value))
  {
    boolean = (ecma_is_value_true (value) ?
                 ECMA_SIMPLE_VALUE_TRUE : ECMA_SIMPLE_VALUE_FALSE);
  }
  else if (ecma_is_value_undefined (value)
           || ecma_is_value_null (value))
  {
    boolean = ECMA_SIMPLE_VALUE_FALSE;
  }
  else if (ecma_is_value_number (value))
  {
    ecma_number_t *num_p = ecma_get_number_from_value (value);

    if (ecma_number_is_nan (*num_p)
        || ecma_number_is_zero (*num_p))
    {
      boolean = ECMA_SIMPLE_VALUE_FALSE;
    }
    else
    {
      boolean = ECMA_SIMPLE_VALUE_TRUE;
    }
  }
  else if (ecma_is_value_string (value))
  {
    ecma_string_t *str_p = ecma_get_string_from_value (value);

    if (ecma_string_get_length (str_p) == 0)
    {
      boolean = ECMA_SIMPLE_VALUE_FALSE;
    }
    else
    {
      boolean = ECMA_SIMPLE_VALUE_TRUE;
    }
  }
  else
  {
    JERRY_ASSERT (ecma_is_value_object (value));

    boolean = ECMA_SIMPLE_VALUE_TRUE;
  }

  ecma_make_simple_completion_value (ret_value, boolean);
} /* ecma_op_to_boolean */

/**
 * ToNumber operation.
 *
 * See also:
 *          ECMA-262 v5, 9.3
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
ecma_op_to_number (ecma_completion_value_t &ret_value, /**< out: completion value */
                   const ecma_value_t& value) /**< ecma-value */
{
  ecma_check_value_type_is_spec_defined (value);

  if (ecma_is_value_number (value))
  {
    ecma_value_t value_copy;
    ecma_copy_value (value_copy, value, true);

    ecma_make_normal_completion_value (ret_value, value_copy);
  }
  else if (ecma_is_value_string (value))
  {
    ecma_string_t *str_p = ecma_get_string_from_value (value);

    ecma_number_t *num_p = ecma_alloc_number ();
    *num_p = ecma_string_to_number (str_p);

    ecma_make_normal_completion_value (ret_value, ecma_value_t (num_p));
  }
  else if (ecma_is_value_object (value))
  {
    ECMA_TRY_CATCH (ret_value, ecma_op_to_primitive, primitive_value, value, ECMA_PREFERRED_TYPE_NUMBER);

    ecma_op_to_number (ret_value, primitive_value);

    ECMA_FINALIZE (primitive_value);
  }
  else
  {
    ecma_number_t *num_p = ecma_alloc_number ();

    if (ecma_is_value_undefined (value))
    {
      *num_p = ecma_number_make_nan ();
    }
    else if (ecma_is_value_null (value))
    {
      *num_p = ECMA_NUMBER_ZERO;
    }
    else
    {
      JERRY_ASSERT (ecma_is_value_boolean (value));

      if (ecma_is_value_true (value))
      {
        *num_p = ECMA_NUMBER_ONE;
      }
      else
      {
        *num_p = ECMA_NUMBER_ZERO;
      }
    }

    ecma_make_normal_completion_value (ret_value, ecma_value_t (num_p));
  }
} /* ecma_op_to_number */

/**
 * ToString operation.
 *
 * See also:
 *          ECMA-262 v5, 9.8
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
ecma_op_to_string (ecma_completion_value_t &ret_value, /**< out: completion value */
                   const ecma_value_t& value) /**< ecma-value */
{
  ecma_check_value_type_is_spec_defined (value);

  if (unlikely (ecma_is_value_object (value)))
  {
    ECMA_TRY_CATCH (ret_value, ecma_op_to_primitive, prim_value, value, ECMA_PREFERRED_TYPE_STRING);

    ecma_op_to_string (ret_value, prim_value);

    ECMA_FINALIZE (prim_value);

    return;
  }
  else
  {
    ecma_string_t *res_p = NULL;

    if (ecma_is_value_string (value))
    {
      res_p = ecma_get_string_from_value (value);
      res_p = ecma_copy_or_ref_ecma_string (res_p);
    }
    else if (ecma_is_value_number (value))
    {
      ecma_number_t *num_p = ecma_get_number_from_value (value);
      res_p = ecma_new_ecma_string_from_number (*num_p);
    }
    else if (ecma_is_value_undefined (value))
    {
      res_p = ecma_get_magic_string (ECMA_MAGIC_STRING_UNDEFINED);
    }
    else if (ecma_is_value_null (value))
    {
      res_p = ecma_get_magic_string (ECMA_MAGIC_STRING_NULL);
    }
    else
    {
      JERRY_ASSERT (ecma_is_value_boolean (value));

      if (ecma_is_value_true (value))
      {
        res_p = ecma_get_magic_string (ECMA_MAGIC_STRING_TRUE);
      }
      else
      {
        res_p = ecma_get_magic_string (ECMA_MAGIC_STRING_FALSE);
      }
    }

    ecma_make_normal_completion_value (ret_value, ecma_value_t (res_p));
  }
} /* ecma_op_to_string */

/**
 * ToObject operation.
 *
 * See also:
 *          ECMA-262 v5, 9.9
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
ecma_op_to_object (ecma_completion_value_t &ret_value, /**< out: completion value */
                   const ecma_value_t& value) /**< ecma-value */
{
  ecma_check_value_type_is_spec_defined (value);

  if (ecma_is_value_number (value))
  {
    ecma_op_create_number_object (ret_value, value);
  }
  else if (ecma_is_value_string (value))
  {
    ecma_op_create_string_object (ret_value, &value, 1);
  }
  else if (ecma_is_value_object (value))
  {
    ecma_value_t value_copy;
    ecma_copy_value (value_copy, value, true);

    ecma_make_normal_completion_value (ret_value, value_copy);
  }
  else
  {
    if (ecma_is_value_undefined (value)
        || ecma_is_value_null (value))
    {
      ecma_object_ptr_t exception_obj_p;
      ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
      ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
    }
    else
    {
      JERRY_ASSERT (ecma_is_value_boolean (value));

      ecma_op_create_boolean_object (ret_value, value);
    }
  }
} /* ecma_op_to_object */

/**
 * FromPropertyDescriptor operation.
 *
 * See also:
 *          ECMA-262 v5, 8.10.4
 *
 * @return constructed object
 */
void
ecma_op_from_property_descriptor (ecma_object_ptr_t &obj_p, /**< out: object pointer */
                                  const ecma_property_descriptor_t* src_prop_desc_p) /**< property descriptor */
{
  // 2.
  ecma_op_create_object_object_noarg (obj_p);

  ecma_completion_value_t completion;
  ecma_property_descriptor_t prop_desc = ecma_make_empty_property_descriptor ();
  {
    prop_desc.is_value_defined = true;

    prop_desc.is_writable_defined = true;
    prop_desc.is_writable = true;

    prop_desc.is_enumerable_defined = true;
    prop_desc.is_enumerable = true;

    prop_desc.is_configurable_defined = true;
    prop_desc.is_configurable = true;
  }

  // 3.
  if (prop_desc.is_value_defined
      || prop_desc.is_writable_defined)
  {
    JERRY_ASSERT (prop_desc.is_value_defined && prop_desc.is_writable_defined);

    // a.
    prop_desc.value = src_prop_desc_p->value;

    ecma_string_t *value_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_VALUE);
    ecma_op_object_define_own_property (completion,
                                        obj_p,
                                        value_magic_string_p,
                                        &prop_desc,
                                        false);
    ecma_deref_ecma_string (value_magic_string_p);
    JERRY_ASSERT (ecma_is_completion_value_normal_true (completion));

    // b.
    const bool is_writable = (src_prop_desc_p->is_writable);
    prop_desc.value = (ecma_value_packed_t) ecma_value_t (is_writable ? ECMA_SIMPLE_VALUE_TRUE
                                                          : ECMA_SIMPLE_VALUE_FALSE);

    ecma_string_t *writable_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_WRITABLE);
    ecma_op_object_define_own_property (completion,
                                        obj_p,
                                        writable_magic_string_p,
                                        &prop_desc,
                                        false);
    ecma_deref_ecma_string (writable_magic_string_p);
    JERRY_ASSERT (ecma_is_completion_value_normal_true (completion));
  }
  else
  {
    // 4.
    JERRY_ASSERT (prop_desc.is_get_defined && prop_desc.is_set_defined);

    // a.
    if (src_prop_desc_p->get_p == NULL)
    {
      prop_desc.value = (ecma_value_packed_t) ecma_value_t (ECMA_SIMPLE_VALUE_UNDEFINED);
    }
    else
    {
      prop_desc.value = (ecma_value_packed_t) ecma_value_t (src_prop_desc_p->get_p);
    }

    ecma_string_t *get_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_GET);
    ecma_op_object_define_own_property (completion,
                                        obj_p,
                                        get_magic_string_p,
                                        &prop_desc,
                                        false);
    ecma_deref_ecma_string (get_magic_string_p);
    JERRY_ASSERT (ecma_is_completion_value_normal_true (completion));

    // b.
    if (src_prop_desc_p->set_p == NULL)
    {
      prop_desc.value = (ecma_value_packed_t) ecma_value_t (ECMA_SIMPLE_VALUE_UNDEFINED);
    }
    else
    {
      prop_desc.value = (ecma_value_packed_t) ecma_value_t (src_prop_desc_p->set_p);
    }

    ecma_string_t *set_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_SET);
    ecma_op_object_define_own_property (completion,
                                        obj_p,
                                        set_magic_string_p,
                                        &prop_desc,
                                        false);
    ecma_deref_ecma_string (set_magic_string_p);
    JERRY_ASSERT (ecma_is_completion_value_normal_true (completion));
  }

  const bool is_enumerable = src_prop_desc_p->is_enumerable;
  prop_desc.value = (ecma_value_packed_t) ecma_value_t (is_enumerable ? ECMA_SIMPLE_VALUE_TRUE
                                                        : ECMA_SIMPLE_VALUE_FALSE);

  ecma_string_t *enumerable_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_ENUMERABLE);
  ecma_op_object_define_own_property (completion,
                                      obj_p,
                                      enumerable_magic_string_p,
                                      &prop_desc,
                                      false);
  ecma_deref_ecma_string (enumerable_magic_string_p);
  JERRY_ASSERT (ecma_is_completion_value_normal_true (completion));

  const bool is_configurable = src_prop_desc_p->is_configurable;
  prop_desc.value = (ecma_value_packed_t) ecma_value_t (is_configurable ? ECMA_SIMPLE_VALUE_TRUE
                                                                  : ECMA_SIMPLE_VALUE_FALSE);

  ecma_string_t *configurable_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_CONFIGURABLE);
  ecma_op_object_define_own_property (completion,
                                      obj_p,
                                      configurable_magic_string_p,
                                      &prop_desc,
                                      false);
  ecma_deref_ecma_string (configurable_magic_string_p);
  JERRY_ASSERT (ecma_is_completion_value_normal_true (completion));
} /* ecma_op_from_property_descriptor */

/**
 * ToPropertyDescriptor operation.
 *
 * See also:
 *          ECMA-262 v5, 8.10.5
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
ecma_op_to_property_descriptor (ecma_completion_value_t &ret_value, /**< out: completion value */
                                const ecma_value_t& obj_value, /**< object value */
                                ecma_property_descriptor_t *out_prop_desc_p) /**< out: filled property descriptor
                                                                                  if return value is normal
                                                                                  empty completion value */
{
  ecma_make_empty_completion_value (ret_value);

  // 1.
  if (!ecma_is_value_object (obj_value))
  {
    ecma_object_ptr_t exception_obj_p;
    ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
    ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
  }
  else
  {
    ecma_object_ptr_t obj_p;
    ecma_get_object_from_value (obj_p, obj_value);

    // 2.
    ecma_property_descriptor_t prop_desc = ecma_make_empty_property_descriptor ();

    // 3.
    ecma_string_t *enumerable_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_ENUMERABLE);

    if (ecma_op_object_get_property (obj_p, enumerable_magic_string_p) != NULL)
    {
      ECMA_TRY_CATCH (ret_value, ecma_op_object_get, enumerable_prop_value, obj_p, enumerable_magic_string_p);
      ECMA_TRY_CATCH (ret_value, ecma_op_to_boolean, boolean_enumerable_prop_value, enumerable_prop_value);

      prop_desc.is_enumerable_defined = true;
      if (ecma_is_value_true (boolean_enumerable_prop_value))
      {
        prop_desc.is_enumerable = true;
      }
      else
      {
        JERRY_ASSERT (ecma_is_value_boolean (boolean_enumerable_prop_value));

        prop_desc.is_enumerable = false;
      }

      ECMA_FINALIZE (boolean_enumerable_prop_value);
      ECMA_FINALIZE (enumerable_prop_value);
    }

    ecma_deref_ecma_string (enumerable_magic_string_p);

    if (!ecma_is_completion_value_throw (ret_value))
    {
      JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));

      // 4.
      ecma_string_t *configurable_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_CONFIGURABLE);

      if (ecma_op_object_get_property (obj_p, configurable_magic_string_p) != NULL)
      {
        ECMA_TRY_CATCH (ret_value, ecma_op_object_get, configurable_prop_value, obj_p, configurable_magic_string_p);
        ECMA_TRY_CATCH (ret_value, ecma_op_to_boolean, boolean_configurable_prop_value, configurable_prop_value);

        prop_desc.is_configurable_defined = true;
        if (ecma_is_value_true (boolean_configurable_prop_value))
        {
          prop_desc.is_configurable = true;
        }
        else
        {
          JERRY_ASSERT (ecma_is_value_boolean (boolean_configurable_prop_value));

          prop_desc.is_configurable = false;
        }

        ECMA_FINALIZE (boolean_configurable_prop_value);
        ECMA_FINALIZE (configurable_prop_value);
      }

      ecma_deref_ecma_string (configurable_magic_string_p);
    }

    if (!ecma_is_completion_value_throw (ret_value))
    {
      JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));

      // 5.
      ecma_string_t *value_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_VALUE);

      if (ecma_op_object_get_property (obj_p, value_magic_string_p) != NULL)
      {
        ECMA_TRY_CATCH (ret_value, ecma_op_object_get, value_prop_value, obj_p, value_magic_string_p);

        ecma_value_t value_copy;
        ecma_copy_value (value_copy, value_prop_value, true);

        prop_desc.is_value_defined = true;
        prop_desc.value = (ecma_value_packed_t) value_copy;

        ECMA_FINALIZE (value_prop_value);
      }

      ecma_deref_ecma_string (value_magic_string_p);
    }

    if (!ecma_is_completion_value_throw (ret_value))
    {
      JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));

      // 6.
      ecma_string_t *writable_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_WRITABLE);

      if (ecma_op_object_get_property (obj_p, writable_magic_string_p) != NULL)
      {
        ECMA_TRY_CATCH (ret_value, ecma_op_object_get, writable_prop_value, obj_p, writable_magic_string_p);
        ECMA_TRY_CATCH (ret_value, ecma_op_to_boolean, boolean_writable_prop_value, writable_prop_value);

        prop_desc.is_writable_defined = true;
        if (ecma_is_value_true (boolean_writable_prop_value))
        {
          prop_desc.is_writable = true;
        }
        else
        {
          JERRY_ASSERT (ecma_is_value_boolean (boolean_writable_prop_value));

          prop_desc.is_writable = false;
        }

        ECMA_FINALIZE (boolean_writable_prop_value);
        ECMA_FINALIZE (writable_prop_value);
      }

      ecma_deref_ecma_string (writable_magic_string_p);
    }

    if (!ecma_is_completion_value_throw (ret_value))
    {
      JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));

      // 7.
      ecma_string_t *get_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_GET);

      if (ecma_op_object_get_property (obj_p, get_magic_string_p) != NULL)
      {
        ECMA_TRY_CATCH (ret_value, ecma_op_object_get, get_prop_value, obj_p, get_magic_string_p);

        if (!ecma_op_is_callable (get_prop_value)
            && !ecma_is_value_undefined (get_prop_value))
        {
          ecma_object_ptr_t exception_obj_p;
          ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
          ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
        }
        else
        {
          prop_desc.is_get_defined = true;

          if (ecma_is_value_undefined (get_prop_value))
          {
            prop_desc.get_p = NULL;
          }
          else
          {
            JERRY_ASSERT (ecma_is_value_object (get_prop_value));

            ecma_object_ptr_t get_p;
            ecma_get_object_from_value (get_p, get_prop_value);
            ecma_ref_object (get_p);

            prop_desc.get_p = (ecma_object_t*) get_p;
          }
        }

        ECMA_FINALIZE (get_prop_value);
      }

      ecma_deref_ecma_string (get_magic_string_p);
    }

    if (!ecma_is_completion_value_throw (ret_value))
    {
      JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));

      // 8.

      ecma_string_t *set_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_SET);

      if (ecma_op_object_get_property (obj_p, set_magic_string_p) != NULL)
      {
        ECMA_TRY_CATCH (ret_value, ecma_op_object_get, set_prop_value, obj_p, set_magic_string_p);

        if (!ecma_op_is_callable (set_prop_value)
            && !ecma_is_value_undefined (set_prop_value))
        {
          ecma_object_ptr_t exception_obj_p;
          ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
          ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
        }
        else
        {
          prop_desc.is_set_defined = true;

          if (ecma_is_value_undefined (set_prop_value))
          {
            prop_desc.set_p = NULL;
          }
          else
          {
            JERRY_ASSERT (ecma_is_value_object (set_prop_value));

            ecma_object_ptr_t set_p;
            ecma_get_object_from_value (set_p, set_prop_value);
            ecma_ref_object (set_p);

            prop_desc.set_p = (ecma_object_t*) set_p;
          }
        }

        ECMA_FINALIZE (set_prop_value);
      }

      ecma_deref_ecma_string (set_magic_string_p);
    }

    if (!ecma_is_completion_value_throw (ret_value))
    {
      JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));

      // 9.
      if (prop_desc.is_get_defined
          || prop_desc.is_set_defined)
      {
        if (prop_desc.is_value_defined
            || prop_desc.is_writable_defined)
        {
          ecma_object_ptr_t exception_obj_p;
          ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
          ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
        }
      }
    }

    if (!ecma_is_completion_value_throw (ret_value))
    {
      JERRY_ASSERT (ecma_is_completion_value_empty (ret_value));
    }
    else
    {
      ecma_free_property_descriptor (&prop_desc);
    }

    *out_prop_desc_p = prop_desc;
  }
} /* ecma_op_to_property_descriptor */

/**
 * @}
 * @}
 */
