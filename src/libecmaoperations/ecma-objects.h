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

#ifndef ECMA_OBJECTS_H
#define ECMA_OBJECTS_H

#include "ecma-conversion.h"
#include "ecma-globals.h"
#include "ecma-value.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmaobjectsinternalops ECMA objects' operations
 * @{
 */

extern void
ecma_op_object_get (ecma_completion_value_t &ret_value,
                    const ecma_object_ptr_t& obj_p,
                    ecma_string_t *property_name_p);
extern ecma_property_t*
ecma_op_object_get_own_property (const ecma_object_ptr_t& obj_p,
                                 ecma_string_t *property_name_p);
extern ecma_property_t*
ecma_op_object_get_property (const ecma_object_ptr_t& obj_p,
                             ecma_string_t *property_name_p);
extern void
ecma_op_object_put (ecma_completion_value_t &ret_value,
                    const ecma_object_ptr_t& obj_p,
                    ecma_string_t *property_name_p,
                    const ecma_value_t& value,
                    bool is_throw);
extern bool
ecma_op_object_can_put (const ecma_object_ptr_t& obj_p,
                        ecma_string_t *property_name_p);
extern void
ecma_op_object_delete (ecma_completion_value_t &ret_value,
                       const ecma_object_ptr_t& obj_p,
                       ecma_string_t *property_name_p,
                       bool is_throw);
extern void
ecma_op_object_default_value (ecma_completion_value_t &ret_value,
                              const ecma_object_ptr_t& obj_p,
                              ecma_preferred_type_hint_t hint);
extern void
ecma_op_object_define_own_property (ecma_completion_value_t &ret_value,
                                    const ecma_object_ptr_t& obj_p,
                                    ecma_string_t *property_name_p,
                                    const ecma_property_descriptor_t* property_desc_p,
                                    bool is_throw);
extern void
ecma_op_object_has_instance (ecma_completion_value_t &ret_value,
                             const ecma_object_ptr_t& obj_p,
                             const ecma_value_t& value);
/**
 * @}
 * @}
 */

#endif /* !ECMA_OBJECTS_H */
