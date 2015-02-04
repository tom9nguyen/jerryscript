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

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmahelpers Helpers for operations with ECMA data types
 * @{
 */

#ifndef JERRY_ECMA_HELPERS_H
#define JERRY_ECMA_HELPERS_H

#include "ecma-globals.h"
#include "ecma-compressed-pointers.h"
#include "ecma-value.h"

/* ecma-helpers-string.c */
extern ecma_string_t* ecma_new_ecma_string (const ecma_char_t *string_p);
extern ecma_string_t* ecma_new_ecma_string_from_uint32 (uint32_t uint_number);
extern ecma_string_t* ecma_new_ecma_string_from_number (ecma_number_t number);
extern void ecma_new_ecma_string_on_stack_from_lit_index (ecma_string_t *string_p,
                                                          literal_index_t lit_index);
extern ecma_string_t* ecma_new_ecma_string_from_lit_index (literal_index_t lit_index);
extern void ecma_new_ecma_string_on_stack_from_magic_string_id (ecma_string_t *string_p,
                                                                ecma_magic_string_id_t id);
extern ecma_string_t* ecma_new_ecma_string_from_magic_string_id (ecma_magic_string_id_t id);
extern ecma_string_t* ecma_concat_ecma_strings (ecma_string_t *string1_p, ecma_string_t *string2_p);
extern ecma_string_t* ecma_copy_or_ref_ecma_string (ecma_string_t *string_desc_p);
extern void ecma_deref_ecma_string (ecma_string_t *string_p);
extern void ecma_check_that_ecma_string_need_not_be_freed (const ecma_string_t *string_p);
extern ecma_number_t ecma_string_to_number (const ecma_string_t *str_p);
extern ssize_t ecma_string_to_zt_string (const ecma_string_t *string_desc_p,
                                         ecma_char_t *buffer_p,
                                         ssize_t buffer_size);
extern bool ecma_compare_ecma_strings_equal_hashes (const ecma_string_t *string1_p,
                                                    const ecma_string_t *string2_p);
extern bool ecma_compare_ecma_strings (const ecma_string_t *string1_p,
                                       const ecma_string_t *string2_p);
extern bool ecma_compare_ecma_strings_relational (const ecma_string_t *string1_p,
                                                  const ecma_string_t *string2_p);
extern int32_t ecma_string_get_length (const ecma_string_t *string_p);
extern ecma_char_t ecma_string_get_char_at_pos (const ecma_string_t *string_p, uint32_t index);
extern bool ecma_compare_zt_strings (const ecma_char_t *string1_p, const ecma_char_t *string2_p);
extern bool ecma_compare_zt_strings_relational (const ecma_char_t *string1_p, const ecma_char_t *string2_p);
extern ecma_char_t*
ecma_copy_zt_string_to_buffer (const ecma_char_t *string_p,
                               ecma_char_t *buffer_p,
                               ssize_t buffer_size);
extern ecma_length_t ecma_zt_string_length (const ecma_char_t *string_p);

extern void ecma_strings_init (void);
extern const ecma_char_t* ecma_get_magic_string_zt (ecma_magic_string_id_t id);
extern ecma_string_t* ecma_get_magic_string (ecma_magic_string_id_t id);
extern bool ecma_is_string_magic (const ecma_string_t *string_p, ecma_magic_string_id_t *out_id_p);
extern bool ecma_is_zt_string_magic (const ecma_char_t *zt_string_p, ecma_magic_string_id_t *out_id_p);
extern ecma_string_hash_t ecma_string_hash (const ecma_string_t *string_p);
extern ecma_string_hash_t ecma_chars_buffer_calc_hash_last_chars (const ecma_char_t *chars, ecma_length_t length);

/* ecma-helpers-number.c */
extern const ecma_number_t ecma_number_relative_eps;

extern ecma_number_t ecma_number_make_nan (void);
extern ecma_number_t ecma_number_make_infinity (bool sign);
extern bool ecma_number_is_nan (ecma_number_t num);
extern bool ecma_number_is_negative (ecma_number_t num);
extern bool ecma_number_is_zero (ecma_number_t num);
extern bool ecma_number_is_infinity (ecma_number_t num);
extern int32_t
ecma_number_get_fraction_and_exponent (ecma_number_t num,
                                       uint64_t *out_fraction_p,
                                       int32_t *out_exponent_p);
extern ecma_number_t
ecma_number_make_normal_positive_from_fraction_and_exponent (uint64_t fraction,
                                                             int32_t exponent);
extern ecma_number_t
ecma_number_make_from_sign_mantissa_and_exponent (bool sign,
                                                  uint64_t mantissa,
                                                  int32_t exponent);
extern ecma_number_t ecma_number_get_prev (ecma_number_t num);
extern ecma_number_t ecma_number_get_next (ecma_number_t num);
extern ecma_number_t ecma_number_negate (ecma_number_t num);
extern ecma_number_t ecma_number_trunc (ecma_number_t num);
extern ecma_number_t ecma_number_add (ecma_number_t left_num, ecma_number_t right_num);
extern ecma_number_t ecma_number_substract (ecma_number_t left_num, ecma_number_t right_num);
extern ecma_number_t ecma_number_multiply (ecma_number_t left_num, ecma_number_t right_num);
extern ecma_number_t ecma_number_divide (ecma_number_t left_num, ecma_number_t right_num);
extern ecma_number_t ecma_number_sqrt (ecma_number_t num);
extern ecma_number_t ecma_number_abs (ecma_number_t num);
extern ecma_number_t ecma_number_ln (ecma_number_t num);
extern ecma_number_t ecma_number_exp (ecma_number_t num);

/* ecma-helpers-values-collection.c */

extern ecma_collection_header_t *ecma_new_values_collection (const ecma_value_t values_buffer[],
                                                             ecma_length_t values_number,
                                                             bool do_ref_if_object);
extern void ecma_free_values_collection (ecma_collection_header_t* header_p, bool do_deref_if_object);
extern ecma_collection_header_t *ecma_new_strings_collection (ecma_string_t* string_ptrs_buffer[],
                                                              ecma_length_t strings_number);

/**
 * Context of ecma-values' collection iterator
 */
typedef struct
{
  ecma_collection_header_t *header_p; /**< collection header */
  uint16_t next_chunk_cp; /**< compressed pointer to next chunk */
  ecma_length_t current_index; /**< index of current element */
  const ecma_value_packed_t *current_value_p; /**< pointer to current element */
  const ecma_value_packed_t *current_chunk_beg_p; /**< pointer to beginning of current chunk's data */
  const ecma_value_packed_t *current_chunk_end_p; /**< pointer to place right after the end of current chunk's data */
} ecma_collection_iterator_t;

extern void
ecma_collection_iterator_init (ecma_collection_iterator_t *iterator_p,
                               ecma_collection_header_t *collection_p);
extern bool
ecma_collection_iterator_next (ecma_collection_iterator_t *iterator_p);

/* ecma-helpers.c */
extern void
ecma_create_object (ecma_object_ptr_t &ret_val,
                    const ecma_object_ptr_t& prototype_object_p,
                    bool is_extensible,
                    ecma_object_type_t type);
extern void
ecma_create_decl_lex_env (ecma_object_ptr_t &ret_val,
                          const ecma_object_ptr_t& outer_lexical_environment_p);
extern void
ecma_create_object_lex_env (ecma_object_ptr_t &ret_val,
                            const ecma_object_ptr_t& outer_lexical_environment_p,
                            const ecma_object_ptr_t& binding_obj_p,
                            bool provide_this);
extern bool __attribute_pure__ ecma_is_lexical_environment (const ecma_object_ptr_t& object_p);
extern bool __attribute_pure__ ecma_get_object_extensible (const ecma_object_ptr_t& object_p);
extern void ecma_set_object_extensible (const ecma_object_ptr_t& object_p, bool is_extensible);
extern ecma_object_type_t __attribute_pure__ ecma_get_object_type (const ecma_object_ptr_t& object_p);
extern void ecma_set_object_type (const ecma_object_ptr_t& object_p, ecma_object_type_t type);
extern void
ecma_get_object_prototype (ecma_object_ptr_t &ret_val,
                           const ecma_object_ptr_t& object_p);
extern bool __attribute_pure__ ecma_get_object_is_builtin (const ecma_object_ptr_t& object_p);
extern void ecma_set_object_is_builtin (const ecma_object_ptr_t& object_p,
                                        bool is_builtin);
extern ecma_lexical_environment_type_t __attribute_pure__
ecma_get_lex_env_type (const ecma_object_ptr_t& object_p);
extern void
ecma_get_lex_env_outer_reference (ecma_object_ptr_t &ret_val,
                                  const ecma_object_ptr_t& object_p);
extern ecma_property_t* __attribute_pure__ ecma_get_property_list (const ecma_object_ptr_t& object_p);
extern void
ecma_get_lex_env_binding_object (ecma_object_ptr_t &ret_val,
                                 const ecma_object_ptr_t& object_p);
extern bool __attribute_pure__ ecma_get_lex_env_provide_this (const ecma_object_ptr_t& object_p);

extern ecma_property_t* ecma_create_internal_property (const ecma_object_ptr_t& object_p,
                                                       ecma_internal_property_id_t property_id);
extern ecma_property_t* ecma_find_internal_property (const ecma_object_ptr_t& object_p,
                                                     ecma_internal_property_id_t property_id);
extern ecma_property_t* ecma_get_internal_property (const ecma_object_ptr_t& object_p,
                                                    ecma_internal_property_id_t property_id);

extern ecma_property_t *ecma_create_named_data_property (const ecma_object_ptr_t& obj_p,
                                                         ecma_string_t *name_p,
                                                         bool is_writable,
                                                         bool is_enumerable,
                                                         bool is_configurable);
extern ecma_property_t *ecma_create_named_accessor_property (const ecma_object_ptr_t& obj_p,
                                                             ecma_string_t *name_p,
                                                             const ecma_object_ptr_t& get_p,
                                                             const ecma_object_ptr_t& set_p,
                                                             bool is_enumerable,
                                                             bool is_configurable);
extern ecma_property_t *ecma_find_named_property (const ecma_object_ptr_t& obj_p,
                                                  ecma_string_t *name_p);
extern ecma_property_t *ecma_get_named_property (const ecma_object_ptr_t& obj_p,
                                                 ecma_string_t *name_p);
extern ecma_property_t *ecma_get_named_data_property (const ecma_object_ptr_t& obj_p,
                                                      ecma_string_t *name_p);

extern void ecma_free_property (const ecma_object_ptr_t& obj_p, ecma_property_t *prop_p);

extern void ecma_delete_property (const ecma_object_ptr_t& obj_p, ecma_property_t *prop_p);

extern void ecma_get_named_data_property_value (ecma_value_t &ret, const ecma_property_t *prop_p);
extern void ecma_set_named_data_property_value (ecma_property_t *prop_p, const ecma_value_t& value);
extern void ecma_named_data_property_assign_value (const ecma_object_ptr_t& obj_p,
                                                   ecma_property_t *prop_p,
                                                   const ecma_value_t& value);

extern bool ecma_is_property_writable (ecma_property_t* prop_p);
extern void ecma_set_property_writable_attr (ecma_property_t* prop_p, bool is_writable);
extern bool ecma_is_property_enumerable (ecma_property_t* prop_p);
extern void ecma_set_property_enumerable_attr (ecma_property_t* prop_p, bool is_enumerable);
extern bool ecma_is_property_configurable (ecma_property_t* prop_p);
extern void ecma_set_property_configurable_attr (ecma_property_t* prop_p, bool is_configurable);

extern bool ecma_is_property_lcached (ecma_property_t *prop_p);
extern void ecma_set_property_lcached (ecma_property_t *prop_p,
                                       bool is_lcached);

extern ecma_property_descriptor_t ecma_make_empty_property_descriptor (void);
extern void ecma_free_property_descriptor (ecma_property_descriptor_t *prop_desc_p);

/* ecma-helpers-conversion.c */
extern ecma_number_t ecma_zt_string_to_number (const ecma_char_t *str_p);
extern ssize_t ecma_uint32_to_string (uint32_t value, ecma_char_t *out_buffer_p, ssize_t buffer_size);
extern uint32_t ecma_number_to_uint32 (ecma_number_t value);
extern int32_t ecma_number_to_int32 (ecma_number_t value);
extern ecma_number_t ecma_int32_to_number (int32_t value);
extern ecma_number_t ecma_uint32_to_number (uint32_t value);
extern ecma_length_t ecma_number_to_zt_string (ecma_number_t num, ecma_char_t *buffer_p, ssize_t buffer_size);

#endif /* !JERRY_ECMA_HELPERS_H */

/**
 * @}
 * @}
 */
