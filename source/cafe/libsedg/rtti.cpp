#include <string.h>

#include <cafe/libsedg/edg_runtime.h>

static inline void clear_base_class_traversal_block(a_base_class_traversal_block* bctbp)
{
    memset(bctbp, 0, sizeof(a_base_class_traversal_block));
}

static inline int is_virtual(a_base_class_spec* base_info)
{
    return (base_info->flags & BCS_VIRTUAL) != 0;
}

static void traverse_base_classes(void* ptr, a_type_info_impl* class_info, a_base_class_traversal_block* bctbp,
                                  a_base_class_spec* curr_base_info)
{
    a_base_class_spec_ptr base_info;
    a_boolean last;

    (*bctbp->pre_function)(ptr, class_info, bctbp, curr_base_info);
    last = FALSE;
    base_info = class_info->base_class_info;
    while (base_info != NULL && !last)
    {
        if ((bctbp->visit_indirect_bases || (base_info->flags & BCS_DIRECT))
            && (!bctbp->public_bases_only || (base_info->flags & BCS_PUBLIC)))
        {
            void* base_ptr = (char*)ptr + base_info->offset;
            if (is_virtual(base_info))
            {
                base_ptr = *(void**)base_ptr;
            }
            traverse_base_classes(base_ptr, base_info->type_info, bctbp, base_info);
            if (bctbp->done)
            {
                return;
            }
        }
        last = (base_info->flags & BCS_LAST) != 0;
        base_info++;
    }
    if (bctbp->post_function != NULL)
    {
        (*bctbp->post_function)(ptr, class_info, bctbp, curr_base_info);
    }
}

extern "C" a_boolean __derived_to_base_conversion(void** p_ptr, void** p_new_ptr, a_type_info_impl_ptr class_info,
                                                  a_type_info_impl_ptr base_info, an_access_flag_string* access_flags,
                                                  a_boolean use_access_flags)
{
    a_boolean found = FALSE;
    a_boolean accessible;
    a_boolean ambiguous = FALSE;
    void* ptr;
    a_base_class_spec_ptr curr_base_info;

    if (p_ptr != NULL)
    {
        ptr = *p_ptr;
    }
    else
    {
        ptr = NULL;
    }
    *p_new_ptr = NULL;
    curr_base_info = class_info->base_class_info;
    if (curr_base_info != NULL)
    {
        a_boolean last;
        do
        {
            a_type_info_impl_ptr curr_class_info = curr_base_info->type_info;
            void* base_ptr = (ptr != NULL) ? (char*)ptr + curr_base_info->offset : NULL;
            if (use_access_flags)
            {
                an_access_flag_string flags = *access_flags;
                accessible = FALSE;
                if (flags != NULL)
                {
                    *access_flags = flags + 1;
                    accessible = *flags == 'Y';
                }
            }
            else
            {
                accessible = (curr_base_info->flags & BCS_PUBLIC) != 0;
            }
            if (MATCHING_TYPE_INFO(curr_class_info, base_info))
            {
                ambiguous = (curr_base_info->flags & BCS_AMBIGUOUS) != 0;
                if (!ambiguous && accessible)
                {
                    found = TRUE;
                    if (ptr != NULL)
                    {
                        if (curr_base_info->flags & BCS_VIRTUAL)
                        {
                            base_ptr = *(void**)base_ptr;
                        }
                        *p_new_ptr = base_ptr;
                    }
                }
            }
            last = (curr_base_info->flags & BCS_LAST) || found || ambiguous;
            curr_base_info++;
        } while (!last);
        if (!found && !ambiguous)
        {
            curr_base_info = class_info->base_class_info;
            do
            {
                a_type_info_impl_ptr curr_class_info = curr_base_info->type_info;
                void* base_ptr = NULL;
                a_boolean descend;
                if (ptr != NULL)
                {
                    base_ptr = (char*)ptr + curr_base_info->offset;
                    if (curr_base_info->flags & BCS_VIRTUAL)
                    {
                        base_ptr = *(void**)base_ptr;
                    }
                }
                if (use_access_flags)
                {
                    descend = TRUE;
                }
                else
                {
                    descend = (curr_base_info->flags & BCS_PUBLIC) && !(curr_base_info->flags & BCS_AMBIGUOUS);
                }
                if (curr_class_info->base_class_info != NULL && descend)
                {
                    void* new_ptr;
                    if (__derived_to_base_conversion(&base_ptr, &new_ptr, curr_class_info, base_info, access_flags,
                                                     use_access_flags))
                    {
                        if (ptr != NULL)
                        {
                            *p_new_ptr = new_ptr;
                        }
                        found = TRUE;
                        break;
                    }
                }
                last = (curr_base_info->flags & BCS_LAST) != 0;
                curr_base_info++;
            } while (!last);
        }
    }
    return found;
}

static a_base_class_spec* find_base_class_at_addr(void* obj_ptr, void* base_ptr, a_type_info_impl* obj_info,
                                                  a_type_info_impl* base_info, int* found)
{
    a_base_class_spec_ptr result = NULL;
    a_base_class_spec_ptr curr_base_info;
    a_boolean last = FALSE;

    for (curr_base_info = obj_info->base_class_info; curr_base_info != NULL && !last; curr_base_info++)
    {
        void* ptr = (char*)obj_ptr + curr_base_info->offset;
        if (curr_base_info->flags & BCS_VIRTUAL)
        {
            ptr = *(void**)ptr;
        }
        if (ptr == base_ptr && MATCHING_TYPE_INFO(curr_base_info->type_info, base_info))
        {
            result = curr_base_info;
            if (curr_base_info->flags & BCS_PUBLIC)
            {
                *found = TRUE;
            }
            break;
        }
        if (curr_base_info->flags & BCS_PUBLIC)
        {
            result = find_base_class_at_addr(ptr, base_ptr, curr_base_info->type_info, base_info, found);
            if (result != NULL)
            {
                break;
            }
        }
        last = (curr_base_info->flags & BCS_LAST) != 0;
    }
    return result;
}

static void tbc_downcast(void* ptr, a_type_info_impl* class_info, a_base_class_traversal_block* bctbp,
                         a_base_class_spec* curr_base_info)
{
    if (MATCHING_TYPE_INFO(bctbp->dest_tiip, class_info))
    {
        bctbp->dest_ptr = ptr;
        bctbp->public_bases_only = TRUE;
    }
    else if (bctbp->dest_ptr != NULL && ptr == bctbp->source_ptr && MATCHING_TYPE_INFO(class_info, bctbp->source_tiip))
    {
        if (bctbp->result_ptr != NULL)
        {
            bctbp->result_ptr = NULL;
            bctbp->done = TRUE;
        }
        else
        {
            bctbp->result_ptr = bctbp->dest_ptr;
        }
    }
}

static void tbc_post_downcast(void* ptr, a_type_info_impl* class_info, a_base_class_traversal_block* bctbp,
                              a_base_class_spec* curr_base_info)
{
    if (MATCHING_TYPE_INFO(bctbp->dest_tiip, class_info))
    {
        bctbp->dest_ptr = NULL;
        bctbp->public_bases_only = FALSE;
    }
}

static inline void* try_downcast(void* complete_object_ptr, a_type_info_impl* object_tiip, void* source_ptr,
                                 a_type_info_impl* source_tiip, a_type_info_impl* dest_tiip)
{
    a_base_class_traversal_block bctb;

    clear_base_class_traversal_block(&bctb);
    bctb.pre_function = tbc_downcast;
    bctb.post_function = tbc_post_downcast;
    bctb.dest_tiip = dest_tiip;
    bctb.source_tiip = source_tiip;
    bctb.source_ptr = source_ptr;
    traverse_base_classes(complete_object_ptr, object_tiip, &bctb, NULL);
    return bctb.result_ptr;
}

extern "C" void* __dynamic_cast(void* class_ptr, a_vtbl_entry_ptr vtbl_ptr, a_type_info_impl_ptr dest_tiip,
                                void* source_ptr, a_type_info_impl_ptr source_tiip)
{
    void* complete_object_ptr = (char*)class_ptr - vtbl_ptr->delta;
    a_type_info_impl_ptr object_tiip = vtbl_ptr->type_info;
    void* result = complete_object_ptr;

    if (dest_tiip != NULL)
    {
        result = try_downcast(complete_object_ptr, object_tiip, source_ptr, source_tiip, dest_tiip);
    }
    if (result == NULL)
    {
        int found;
        if (object_tiip == source_tiip)
        {
            found = TRUE;
        }
        else
        {
            found = FALSE;
            find_base_class_at_addr(complete_object_ptr, source_ptr, object_tiip, source_tiip, &found);
        }
        if (found)
        {
            if (MATCHING_TYPE_INFO(object_tiip, dest_tiip))
            {
                result = complete_object_ptr;
            }
            else
            {
                void* new_ptr = NULL;
                if (__derived_to_base_conversion(&complete_object_ptr, &new_ptr, object_tiip, dest_tiip, NULL, FALSE))
                {
                    result = new_ptr;
                }
            }
        }
    }
    return result;
}

static inline void __throw_bad_cast(void)
{
    __abort_execution(ec_bad_cast);
}

extern "C" void __throw_bad_typeid(void)
{
    __abort_execution(ec_bad_typeid);
}

extern "C" void* __dynamic_cast_ref(void* class_ptr, a_vtbl_entry_ptr vtbl_ptr, a_type_info_impl_ptr dest_tiip,
                                    void* source_ptr, a_type_info_impl_ptr source_tiip)
{
    void* result = __dynamic_cast(class_ptr, vtbl_ptr, dest_tiip, source_ptr, source_tiip);
    if (result == NULL)
    {
        __throw_bad_cast();
    }
    return result;
}

extern "C" void* __get_typeid(a_vtbl_entry_ptr vtbl_ptr)
{
    if (vtbl_ptr == NULL)
    {
        __throw_bad_typeid();
    }
    return vtbl_ptr->type_info;
}
