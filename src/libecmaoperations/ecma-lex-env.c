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

#include "ecma-builtins.h"
#include "ecma-exceptions.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-lex-env.h"
#include "ecma-objects.h"
#include "globals.h"

/** \addtogroup ecma ECMA
 * @{
 */

/**
 * \addtogroup lexicalenvironment Lexical environment
 * @{
 */

/**
 * HasBinding operation.
 *
 * See also: ECMA-262 v5, 10.2.1
 *
 * @return true / false
 */
bool
ecma_op_has_binding (const ecma_object_ptr_t& lex_env_p, /**< lexical environment */
                     ecma_string_t *name_p) /**< argument N */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));

  if (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
  {
    ecma_property_t *property_p = ecma_find_named_property (lex_env_p, name_p);

    return (property_p != NULL);
  }
  else
  {
    JERRY_ASSERT (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_OBJECTBOUND);

    ecma_object_ptr_t binding_obj_p;
    ecma_get_lex_env_binding_object (binding_obj_p, lex_env_p);

    return (ecma_op_object_get_property (binding_obj_p, name_p) != NULL);
  }
} /* ecma_op_has_binding */

/**
 * CreateMutableBinding operation.
 *
 * See also: ECMA-262 v5, 10.2.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value
 */
void
ecma_op_create_mutable_binding (ecma_completion_value_t &ret_value, /**< out: completion value */
                                const ecma_object_ptr_t& lex_env_p, /**< lexical environment */
                                ecma_string_t *name_p, /**< argument N */
                                bool is_deletable) /**< argument D */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));
  JERRY_ASSERT(name_p != NULL);

  if (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
  {
    ecma_create_named_data_property (lex_env_p,
                                     name_p,
                                     true, false, is_deletable);
  }
  else
  {
    JERRY_ASSERT (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_OBJECTBOUND);

    ecma_object_ptr_t binding_obj_p;
    ecma_get_lex_env_binding_object (binding_obj_p, lex_env_p);

    ecma_property_descriptor_t prop_desc = ecma_make_empty_property_descriptor ();
    {
      prop_desc.is_value_defined = true;
      prop_desc.value = (ecma_value_packed_t) ecma_value_t (ECMA_SIMPLE_VALUE_UNDEFINED);

      prop_desc.is_writable_defined = true;
      prop_desc.is_writable = true;

      prop_desc.is_enumerable_defined = true;
      prop_desc.is_enumerable = true;

      prop_desc.is_configurable_defined = true;
      prop_desc.is_configurable = is_deletable;
    }

    ecma_completion_value_t completion;
    ecma_op_object_define_own_property (completion,
                                        binding_obj_p,
                                        name_p,
                                        &prop_desc,
                                        true);

    if (ecma_is_completion_value_throw (completion))
    {
      ret_value = completion;
      return;
    }
    else
    {
      JERRY_ASSERT (ecma_is_completion_value_normal_true (completion)
                    || ecma_is_completion_value_normal_false (completion));
    }
  }

  ecma_make_empty_completion_value (ret_value);
} /* ecma_op_create_mutable_binding */

/**
 * SetMutableBinding operation.
 *
 * See also: ECMA-262 v5, 10.2.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
void
ecma_op_set_mutable_binding (ecma_completion_value_t &ret_value, /**< out: completion value */
                             const ecma_object_ptr_t& lex_env_p, /**< lexical environment */
                             ecma_string_t *name_p, /**< argument N */
                             const ecma_value_t& value, /**< argument V */
                             bool is_strict) /**< argument S */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));
  JERRY_ASSERT(name_p != NULL);

  if (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
  {
#ifndef JERRY_NDEBUG
# ifdef CONFIG_ECMA_COMPACT_PROFILE
    bool is_equal = false;

    ecma_string_t *arguments_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_ARGUMENTS);
    if (ecma_compare_ecma_strings (name_p, arguments_magic_string_p))
    {
      is_equal = true;
    }
    ecma_deref_ecma_string (arguments_magic_string_p);

    JERRY_ASSERT (!is_equal);

    if (is_equal)
    {
      ecma_object_ptr_t cp_error_obj_p;
      ecma_builtin_get (cp_error_obj_p, ECMA_BUILTIN_ID_COMPACT_PROFILE_ERROR);
      ecma_make_throw_obj_completion_value (ret_value, cp_error_obj_p);
      return;
    }
# endif /* CONFIG_ECMA_COMPACT_PROFILE */
#endif /* !JERRY_NDEBUG */

    ecma_property_t *property_p = ecma_get_named_data_property (lex_env_p, name_p);

    if (ecma_is_property_writable (property_p))
    {
      ecma_named_data_property_assign_value (lex_env_p, property_p, value);
    }
    else if (is_strict)
    {
      ecma_object_ptr_t exception_obj_p;
      ecma_new_standard_error (exception_obj_p, ECMA_ERROR_TYPE);
      ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
      return;
    }
  }
  else
  {
    JERRY_ASSERT (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_OBJECTBOUND);

    ecma_object_ptr_t binding_obj_p;
    ecma_get_lex_env_binding_object (binding_obj_p, lex_env_p);

    ecma_completion_value_t completion;
    ecma_op_object_put (completion,
                        binding_obj_p,
                        name_p,
                        value,
                        is_strict);

    if (ecma_is_completion_value_throw (completion))
    {
      ret_value = completion;
      return;
    }
    else
    {
      JERRY_ASSERT (ecma_is_completion_value_normal_true (completion)
                    || ecma_is_completion_value_normal_false (completion));
    }
  }

  ecma_make_empty_completion_value (ret_value);
} /* ecma_op_set_mutable_binding */

/**
 * GetBindingValue operation.
 *
 * See also: ECMA-262 v5, 10.2.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
void
ecma_op_get_binding_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                           const ecma_object_ptr_t& lex_env_p, /**< lexical environment */
                           ecma_string_t *name_p, /**< argument N */
                           bool is_strict) /**< argument S */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));
  JERRY_ASSERT(name_p != NULL);

  if (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
  {
#ifndef JERRY_NDEBUG
# ifdef CONFIG_ECMA_COMPACT_PROFILE
    bool is_equal = false;

    ecma_string_t *arguments_magic_string_p = ecma_get_magic_string (ECMA_MAGIC_STRING_ARGUMENTS);
    if (ecma_compare_ecma_strings (name_p, arguments_magic_string_p))
    {
      is_equal = true;
    }
    ecma_deref_ecma_string (arguments_magic_string_p);

    JERRY_ASSERT (!is_equal);

    if (is_equal)
    {
      ecma_object_ptr_t cp_error_obj_p;
      ecma_builtin_get (cp_error_obj_p, ECMA_BUILTIN_ID_COMPACT_PROFILE_ERROR);
      ecma_make_throw_obj_completion_value (ret_value, cp_error_obj_p);
      return;
    }
# endif /* CONFIG_ECMA_COMPACT_PROFILE */
#endif /* !JERRY_NDEBUG */

    ecma_property_t *property_p = ecma_get_named_data_property (lex_env_p, name_p);

    ecma_value_t prop_value;
    ecma_get_named_data_property_value (prop_value, property_p);

    /* is the binding mutable? */
    if (!ecma_is_property_writable (property_p)
        && ecma_is_value_empty (prop_value))
    {
      /* unitialized immutable binding */
      if (is_strict)
      {
        ecma_object_ptr_t exception_obj_p;
        ecma_new_standard_error (exception_obj_p, ECMA_ERROR_REFERENCE);
        ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
        return;
      }
      else
      {
        ecma_make_simple_completion_value (ret_value, ECMA_SIMPLE_VALUE_UNDEFINED);
        return;
      }
    }

    ecma_value_t prop_value_copy;
    ecma_copy_value (prop_value_copy, prop_value, true);

    ecma_make_normal_completion_value (ret_value, prop_value_copy);
  }
  else
  {
    JERRY_ASSERT (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_OBJECTBOUND);

    ecma_object_ptr_t binding_obj_p;
    ecma_get_lex_env_binding_object (binding_obj_p, lex_env_p);

    if (ecma_op_object_get_property (binding_obj_p, name_p) == NULL)
    {
      if (is_strict)
      {
        ecma_object_ptr_t exception_obj_p;
        ecma_new_standard_error (exception_obj_p, ECMA_ERROR_REFERENCE);
        ecma_make_throw_obj_completion_value (ret_value, exception_obj_p);
        return;
      }
      else
      {
        ecma_make_simple_completion_value (ret_value, ECMA_SIMPLE_VALUE_UNDEFINED);
        return;
      }
    }

    ecma_op_object_get (ret_value, binding_obj_p, name_p);
  }
} /* ecma_op_get_binding_value */

/**
 * DeleteBinding operation.
 *
 * See also: ECMA-262 v5, 10.2.1
 *
 * @return completion value
 *         Return value is simple and so need not be freed.
 *         However, ecma_free_completion_value may be called for it, but it is a no-op.
 */
void
ecma_op_delete_binding (ecma_completion_value_t &ret_value, /**< out: completion value */
                        const ecma_object_ptr_t& lex_env_p, /**< lexical environment */
                        ecma_string_t *name_p) /**< argument N */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));
  JERRY_ASSERT(name_p != NULL);


  if (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
  {
    ecma_property_t *prop_p = ecma_find_named_property (lex_env_p, name_p);
    ecma_simple_value_t ret_val;

    if (prop_p == NULL)
    {
      ret_val = ECMA_SIMPLE_VALUE_TRUE;
    }
    else
    {
      JERRY_ASSERT(prop_p->type == ECMA_PROPERTY_NAMEDDATA);

      if (!ecma_is_property_configurable (prop_p))
      {
        ret_val = ECMA_SIMPLE_VALUE_FALSE;
      }
      else
      {
        ecma_delete_property (lex_env_p, prop_p);

        ret_val = ECMA_SIMPLE_VALUE_TRUE;
      }
    }

    ecma_make_simple_completion_value (ret_value, ret_val);
  }
  else
  {
    JERRY_ASSERT (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_OBJECTBOUND);

    ecma_object_ptr_t binding_obj_p;
    ecma_get_lex_env_binding_object (binding_obj_p, lex_env_p);

    ecma_op_object_delete (ret_value, binding_obj_p, name_p, false);
  }
} /* ecma_op_delete_binding */

/**
 * ImplicitThisValue operation.
 *
 * See also: ECMA-262 v5, 10.2.1
 *
 * @return completion value
 *         Returned value must be freed with ecma_free_completion_value.
 */
void
ecma_op_implicit_this_value (ecma_completion_value_t &ret_value, /**< out: completion value */
                             const ecma_object_ptr_t& lex_env_p) /**< lexical environment */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));

  if (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
  {
    ecma_make_simple_completion_value (ret_value, ECMA_SIMPLE_VALUE_UNDEFINED);
  }
  else
  {
    JERRY_ASSERT (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_OBJECTBOUND);

    if (ecma_get_lex_env_provide_this (lex_env_p))
    {
      ecma_object_ptr_t binding_obj_p;
      ecma_get_lex_env_binding_object (binding_obj_p, lex_env_p);
      ecma_ref_object (binding_obj_p);

      ecma_make_normal_completion_value (ret_value, ecma_value_t (binding_obj_p));
    }
    else
    {
      ecma_make_simple_completion_value (ret_value, ECMA_SIMPLE_VALUE_UNDEFINED);
    }
  }
} /* ecma_op_implicit_this_value */

/**
 * CreateImmutableBinding operation.
 *
 * See also: ECMA-262 v5, 10.2.1
 */
void
ecma_op_create_immutable_binding (const ecma_object_ptr_t& lex_env_p, /**< lexical environment */
                                  ecma_string_t *name_p) /**< argument N */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));
  JERRY_ASSERT (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE);

  /*
   * Warning:
   *         Whether immutable bindings are deletable seems not to be defined by ECMA v5.
   */
  ecma_property_t *prop_p = ecma_create_named_data_property (lex_env_p,
                                                             name_p,
                                                             false, false, false);

#ifndef JERRY_NDEBUG
  ecma_value_t prop_value;
  ecma_get_named_data_property_value (prop_value, prop_p);

  JERRY_ASSERT(ecma_is_value_undefined (prop_value));
#endif /* !JERRY_NDEBUG */

  ecma_set_named_data_property_value (prop_p,
                                      ecma_value_t (ECMA_SIMPLE_VALUE_EMPTY));
} /* ecma_op_create_immutable_binding */

/**
 * InitializeImmutableBinding operation.
 *
 * See also: ECMA-262 v5, 10.2.1
 */
void
ecma_op_initialize_immutable_binding (const ecma_object_ptr_t& lex_env_p, /**< lexical environment */
                                      ecma_string_t *name_p, /**< argument N */
                                      const ecma_value_t& value) /**< argument V */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));
  JERRY_ASSERT (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE);

  ecma_property_t *prop_p = ecma_get_named_data_property (lex_env_p, name_p);

  ecma_value_t prop_value;
  ecma_get_named_data_property_value (prop_value, prop_p);

  /* The binding must be unitialized immutable binding */
  JERRY_ASSERT (!ecma_is_property_writable (prop_p)
                && ecma_is_value_empty (prop_value));

  ecma_named_data_property_assign_value (lex_env_p, prop_p, value);
} /* ecma_op_initialize_immutable_binding */

/**
 * The Global Environment constructor.
 *
 * See also: ECMA-262 v5, 10.2.3
 *
 * @return pointer to created lexical environment
 */
void
ecma_op_create_global_environment (ecma_object_ptr_t &glob_env_p, /**< out: object pointer */
                                   const ecma_object_ptr_t& glob_obj_p) /**< the Global object */
{
  ecma_object_ptr_t null_pointer;

#ifdef CONFIG_ECMA_GLOBAL_ENVIRONMENT_DECLARATIVE
  (void) glob_obj_p;
  ecma_create_decl_lex_env (glob_env_p, null_pointer);
#else /* !CONFIG_ECMA_GLOBAL_ENVIRONMENT_DECLARATIVE */
  ecma_create_object_lex_env (glob_env_p, null_pointer, glob_obj_p, false);
#endif /* !CONFIG_ECMA_GLOBAL_ENVIRONMENT_DECLARATIVE */
} /* ecma_op_create_global_environment */

/**
 * Figure out whether the lexical environment is global.
 *
 * @return true - if lexical environment is object-bound and corresponding object is global object,
 *         false - otherwise.
 */
bool
ecma_is_lexical_environment_global (const ecma_object_ptr_t& lex_env_p) /**< lexical environment */
{
  JERRY_ASSERT(lex_env_p.is_not_null ()
               && ecma_is_lexical_environment (lex_env_p));

  ecma_lexical_environment_type_t type = ecma_get_lex_env_type (lex_env_p);

  if (type == ECMA_LEXICAL_ENVIRONMENT_OBJECTBOUND)
  {
    ecma_object_ptr_t binding_obj_p;
    ecma_get_lex_env_binding_object (binding_obj_p, lex_env_p);

    return ecma_builtin_is (binding_obj_p, ECMA_BUILTIN_ID_GLOBAL);
  }
  else
  {
    return false;
  }
} /* ecma_is_lexical_environment_global */

/**
 * @}
 * @}
 */
