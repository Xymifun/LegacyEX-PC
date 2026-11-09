extern "C" void __init_wut_newlib();
extern "C" void __init_wut_devoptab();
extern "C" void __attribute__((weak)) __init_wut_socket();

extern "C"
{
extern void (*__preinit_array_start[])();
extern void (*__preinit_array_end[])();
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();
}

extern "C" void _init()
{
}

extern "C" void _fini()
{
}

static void RunInitArray(void (**first)(), void (**last)())
{
    void (**entry)() = first;
    for (; entry != last; ++entry)
    {
        unsigned int slot = (unsigned int)(*entry);
        if (slot == 0 || slot == 0xffffffffu)
            continue;
        (*entry)();
    }
}

static void MarkInitArrayDone(void (**first)(), void (**last)())
{
    void (**entry)() = first;
    for (; entry != last; ++entry)
        *entry = (void (*)())0xffffffffu;
}

extern "C" void __init_wut()
{
    __init_wut_newlib();
    __init_wut_devoptab();
    if (&__init_wut_socket)
        __init_wut_socket();
    RunInitArray(__preinit_array_start, __preinit_array_end);
    _init();
    RunInitArray(__init_array_start, __init_array_end);
    MarkInitArrayDone(__init_array_start, __init_array_end);
}
