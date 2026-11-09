#include <cafe/libsedg/edg_runtime.h>

namespace std
{

type_info::~type_info()
{
}

bool type_info::before(const type_info& rhs) const
{
    a_unique_id_ptr this_id = (a_unique_id_ptr)this;
    a_unique_id_ptr rhs_id = (a_unique_id_ptr)&rhs;

    if (((a_type_info_impl_ptr)this)->unique_id != NULL)
    {
        this_id = ((a_type_info_impl_ptr)this)->unique_id;
    }
    if (((a_type_info_impl_ptr)&rhs)->unique_id != NULL)
    {
        rhs_id = ((a_type_info_impl_ptr)&rhs)->unique_id;
    }
    return this_id < rhs_id;
}

bool type_info::operator==(const type_info& rhs) const
{
    return MATCHING_TYPE_INFO((a_type_info_impl_ptr)this, (a_type_info_impl_ptr)&rhs);
}

bool type_info::operator!=(const type_info& rhs) const
{
    return !MATCHING_TYPE_INFO((a_type_info_impl_ptr)this, (a_type_info_impl_ptr)&rhs);
}

const char* type_info::name() const
{
    return ((a_type_info_impl_ptr)this)->name;
}

size_t type_info::hash_code() const
{
    a_unique_id_ptr id = ((a_type_info_impl_ptr)this)->unique_id;

    if (id == NULL)
    {
        id = (a_unique_id_ptr)this;
    }
    return (size_t)id;
}

}
