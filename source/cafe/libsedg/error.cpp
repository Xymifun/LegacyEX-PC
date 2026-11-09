#include <stdio.h>
#include <stdlib.h>

#include <cafe/libsedg/edg_runtime.h>

#ifdef __cplusplus
extern "C"
{
#endif

static char* error_text(an_error_code err_code)
{
    char* text = NULL;

    switch (err_code)
    {
    case ec_abort_header:
        text = "C++ runtime abort";
        break;
    case ec_terminate_called:
        text = "terminate() called by the exception handling mechanism";
        break;
    case ec_terminate_returned:
        text = "returned from a user-defined terminate() routine";
        break;
    case ec_already_marked_for_destruction:
        text = "internal error: static object marked for destruction more than once";
        break;
    case ec_main_called_more_than_once:
        text = "main() called more than once";
        break;
    case ec_pure_virtual_called:
        text = "a pure virtual function was called";
        break;
    case ec_bad_cast:
        text = "invalid dynamic cast";
        break;
    case ec_bad_typeid:
        text = "invalid typeid operation";
        break;
    case ec_array_not_from_vec_new:
        text = "freeing array not allocated by an array new operation";
        break;
    case ec_terminate_called_more_than_once:
        text = "terminate() called itself recursively";
        break;
    case ec_negative_vla_size:
        text = "negative size for variable-length array";
        break;
    case ec_vla_allocation_failed:
        text = "VLA allocation failed";
        break;
    case ec_deleted_virtual_called:
        text = "a deleted virtual function was called";
        break;
    }
    return text;
}

static inline void display_abort_message(an_error_code err_code)
{
    fputs(error_text(ec_abort_header), stderr);
    fputs(": ", stderr);
    fputs(error_text(err_code), stderr);
    fputs("\n", stderr);
}

void __abort_execution(an_error_code err_code)
{
    display_abort_message(err_code);
    abort();
}

#ifdef __cplusplus
}
#endif
