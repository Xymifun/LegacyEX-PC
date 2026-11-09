#ifndef EDG_RUNTIME_H
#define EDG_RUNTIME_H

#include <stddef.h>

typedef unsigned char a_byte;
typedef int a_boolean;

#define FALSE 0
#define TRUE 1

typedef enum
{
    ec_none,
    ec_abort_header,
    ec_terminate_called,
    ec_terminate_returned,
    ec_already_marked_for_destruction,
    ec_main_called_more_than_once,
    ec_pure_virtual_called,
    ec_bad_cast,
    ec_bad_typeid,
    ec_array_not_from_vec_new,
    ec_terminate_called_more_than_once,
    ec_negative_vla_size,
    ec_vla_allocation_failed,
    ec_deleted_virtual_called,
    ec_last
} an_error_code;

#ifdef __cplusplus
extern "C"
{
#endif

void __abort_execution(an_error_code err_code);

#ifdef __cplusplus
}

#include <typeinfo>

typedef void (*a_void_function_ptr)(void);

typedef a_byte a_unique_id;
typedef a_unique_id* a_unique_id_ptr;
typedef short an_object_offset;
typedef a_byte a_base_class_spec_flag_set;

#define BCS_VIRTUAL 0x01
#define BCS_LAST 0x02
#define BCS_PUBLIC 0x04
#define BCS_AMBIGUOUS 0x08
#define BCS_DIRECT 0x10

typedef struct a_type_info_impl* a_type_info_impl_ptr;
typedef struct a_base_class_spec* a_base_class_spec_ptr;

typedef struct a_base_class_spec
{
    a_type_info_impl_ptr type_info;
    an_object_offset offset;
    a_base_class_spec_flag_set flags;
} a_base_class_spec;

struct a_type_info_impl : std::type_info
{
    const char* name;
    a_unique_id_ptr unique_id;
    a_base_class_spec* base_class_info;
};

#define MATCHING_TYPE_INFO(tiip1, tiip2) \
    ((tiip1) == (tiip2) || ((tiip1)->unique_id == (tiip2)->unique_id && (tiip1)->unique_id != NULL))

typedef char* an_access_flag_string;

struct a_vtbl_entry
{
    an_object_offset delta;
    union
    {
        a_void_function_ptr func;
        a_type_info_impl_ptr type_info;
    };
};
typedef struct a_vtbl_entry* a_vtbl_entry_ptr;

typedef struct a_base_class_traversal_block* a_base_class_traversal_block_ptr;
typedef void a_base_class_process_function(void* ptr, a_type_info_impl_ptr class_info,
                                           a_base_class_traversal_block_ptr bctbp,
                                           a_base_class_spec_ptr curr_base_info);
typedef a_base_class_process_function* a_base_class_process_function_ptr;

typedef struct a_base_class_traversal_block
{
    a_base_class_process_function_ptr pre_function;
    a_base_class_process_function_ptr post_function;
    a_boolean visit_indirect_bases;
    a_boolean public_bases_only;
    a_boolean done;
    a_type_info_impl_ptr dest_tiip;
    a_type_info_impl_ptr source_tiip;
    void* source_ptr;
    void* dest_ptr;
    void* result_ptr;
} a_base_class_traversal_block;

#endif

#endif
