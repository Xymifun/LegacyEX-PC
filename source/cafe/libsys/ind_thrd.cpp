#include <setjmp.h>
#include <time.h>

#include <coreinit/thread.h>

enum __ghs_ThreadLocalStorage_specifier
{
    __ghs_TLS_asctime_buff,
    __ghs_TLS_tmpnam_space,
    __ghs_TLS_strtok_saved_pos,
    __ghs_TLS_Errno,
    __ghs_TLS_gmtime_temp,
    __ghs_TLS___eh_globals,
    __ghs_TLS_SignalHandlers,
    __ghs_TLS__eh_mem_manage,
    __ghs_TLS__eh_store_globals,
    __ghs_TLS__eh_store_globals_tdeh,
    __ghs_TLS__eh_init_block
};

struct __ghs_ThreadLocalStorage
{
    char tmpnam_space[32];
    char asctime_buff[30];
    char* strtok_saved_pos;
    struct tm gmtime_temp;
    void* eh_globals;
    void* eh_mem_manage[9];
    void* eh_store_globals[6];
    void* eh_store_globals_tdeh[76];
};

static inline __ghs_ThreadLocalStorage* ghsThreadLocalStorage(OSThread* thread)
{
    return reinterpret_cast<__ghs_ThreadLocalStorage*>(&thread->ghsExceptionHandling);
}

extern "C"
{

void* __ghs_GetThreadLocalStorageItem(int specifier)
{
    OSThread* thread = OSGetCurrentThread();
    void* ptlsitem = 0;
    switch (specifier)
    {
    case (int)__ghs_TLS_Errno:
        ptlsitem = &thread->context.error;
        break;
    case (int)__ghs_TLS_SignalHandlers:
        break;
    case (int)__ghs_TLS_asctime_buff:
        ptlsitem = ghsThreadLocalStorage(thread)->asctime_buff;
        break;
    case (int)__ghs_TLS_tmpnam_space:
        ptlsitem = ghsThreadLocalStorage(thread)->tmpnam_space;
        break;
    case (int)__ghs_TLS_strtok_saved_pos:
        ptlsitem = &ghsThreadLocalStorage(thread)->strtok_saved_pos;
        break;
    case (int)__ghs_TLS_gmtime_temp:
        ptlsitem = &ghsThreadLocalStorage(thread)->gmtime_temp;
        break;
    case (int)__ghs_TLS___eh_globals:
        ptlsitem = &ghsThreadLocalStorage(thread)->eh_globals;
        break;
    case (int)__ghs_TLS__eh_mem_manage:
        ptlsitem = ghsThreadLocalStorage(thread)->eh_mem_manage;
        break;
    case (int)__ghs_TLS__eh_store_globals:
        ptlsitem = ghsThreadLocalStorage(thread)->eh_store_globals;
        break;
    case (int)__ghs_TLS__eh_store_globals_tdeh:
        ptlsitem = ghsThreadLocalStorage(thread)->eh_store_globals_tdeh;
        break;
    case (int)__ghs_TLS__eh_init_block:
        ptlsitem = 0;
        break;
    default:
        break;
    }
    return ptlsitem;
}

int __ghs_SaveSignalContext(jmp_buf jmpbuf)
{
    return 0;
}

void __ghs_RestoreSignalContext(jmp_buf jmpbuf)
{
}
}
