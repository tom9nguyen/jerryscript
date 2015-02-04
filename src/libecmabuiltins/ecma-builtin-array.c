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
#include "ecma-conversion.h"
#include "ecma-exceptions.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-objects.h"
#include "ecma-array-object.h"
#include "ecma-try-catch-macro.h"
#include "globals.h"

#ifndef CONFIG_ECMA_COMPACT_PROFILE_DISABLE_ARRAY_BUILTIN

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

#define BUILTIN_INC_HEADER_NAME "ecma-builtin-array.inc.h"
#define BUILTIN_UNDERSCORED_ID array
#include "ecma-builtin-internal-routines-template.inc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup array ECMA Array object built-in
 * @{
 */

/**
 * The Array object's 'isArray' routine
 *
 * See also:
 *          ECMA-262 v5, 15.4.3.2
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
static void
ecma_builtin_array_object_is_array (ecma_completion_value_t &ret_value, /**< out: completion value */
                                    const ecma_value_t& this_arg __unused, /**< 'this' argument */
                                    const ecma_value_t& arg) /**< first argument */
{
  ecma_simple_value_t is_array = ECMA_SIMPLE_VALUE_FALSE;

  if (ecma_is_value_object (arg))
  {
    ecma_object_ptr_t obj_p;
    ecma_get_object_from_value (obj_p, arg);

    ecma_property_t *class_prop_p = ecma_get_internal_property (obj_p,
                                                                ECMA_INTERNAL_PROPERTY_CLASS);

    if (class_prop_p->u.internal_property.value == ECMA_MAGIC_STRING_ARRAY_UL)
    {
      is_array = ECMA_SIMPLE_VALUE_TRUE;
    }
  }

  ecma_make_simple_completion_value (ret_value, is_array);
} /* ecma_builtin_array_object_is_array */

/**
 * Handle calling [[Call]] of built-in Array object
 *
 * @return completion-value
 */
void
ecma_builtin_array_dispatch_call (ecma_completion_value_t &ret_value, /**< out: completion value */
                                  const ecma_value_t *arguments_list_p, /**< arguments list */
                                  ecma_length_t arguments_list_len) /**< number of arguments */
{
  JERRY_ASSERT (arguments_list_len == 0 || arguments_list_p != NULL);

  ecma_builtin_array_dispatch_construct (ret_value, arguments_list_p, arguments_list_len);
} /* ecma_builtin_array_dispatch_call */

/**
 * Handle calling [[Construct]] of built-in Array object
 *
 * @return completion-value
 */
void
ecma_builtin_array_dispatch_construct (ecma_completion_value_t &ret_value, /**< out: completion value */
                                       const ecma_value_t *arguments_list_p, /**< arguments list */
                                       ecma_length_t arguments_list_len) /**< number of arguments */
{
  JERRY_ASSERT (arguments_list_len == 0 || arguments_list_p != NULL);

  ecma_op_create_array_object (ret_value, arguments_list_p, arguments_list_len, true);
} /* ecma_builtin_array_dispatch_construct */

/**
 * @}
 * @}
 * @}
 */

#endif /* !CONFIG_ECMA_COMPACT_PROFILE_DISABLE_ARRAY_BUILTIN */
