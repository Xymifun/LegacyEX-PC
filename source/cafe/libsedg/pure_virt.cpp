#include <cafe/libsedg/edg_runtime.h>

extern "C" void __pure_virtual_called(void)
{
    __abort_execution(ec_pure_virtual_called);
}
