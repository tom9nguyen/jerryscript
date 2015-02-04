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

#ifndef ECMA_FUNCTION_OBJECT_H
#define ECMA_FUNCTION_OBJECT_H

#include "ecma-globals.h"
#include "ecma-value.h"
#include "interpreter.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmafunctionobject ECMA Function object related routines
 * @{
 */

extern bool ecma_op_is_callable (const ecma_value_t& value);
extern bool ecma_is_constructor (const ecma_value_t& value);

extern void
ecma_op_create_function_object (ecma_object_ptr_t &ret_val,
                                ecma_string_t* formal_parameter_list_p[],
                                ecma_length_t formal_parameters_number,
                                const ecma_object_ptr_t& scope_p,
                                bool is_strict,
                                opcode_counter_t first_opcode_idx);

extern void
ecma_op_function_call (ecma_completion_value_t &ret_value,
                       const ecma_object_ptr_t& func_obj_p,
                       const ecma_value_t& this_arg_value,
                       const ecma_value_t* arguments_list_p,
                       ecma_length_t arguments_list_len);

extern void
ecma_op_function_construct (ecma_completion_value_t &ret_value,
                            const ecma_object_ptr_t& func_obj_p,
                            const ecma_value_t* arguments_list_p,
                            ecma_length_t arguments_list_len);

extern void
ecma_op_function_has_instance (ecma_completion_value_t &ret_value,
                               const ecma_object_ptr_t& func_obj_p,
                               const ecma_value_t& value);

extern void
ecma_op_function_declaration (ecma_completion_value_t &ret_value,
                              const ecma_object_ptr_t& lex_env_p,
                              ecma_string_t *function_name_p,
                              opcode_counter_t function_code_opcode_idx,
                              ecma_string_t* formal_parameter_list_p[],
                              ecma_length_t formal_parameter_list_length,
                              bool is_strict,
                              bool is_configurable_bindings);

/**
 * @}
 * @}
 */

#endif /* !ECMA_FUNCTION_OBJECT_H */
