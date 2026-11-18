#ifdef _WIN32

#include "platform/pc/PcCrashHandler.hpp"

#include <windows.h>

#include <exception>
#include <new>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>

extern char* gHeapStart;
extern char* gHeapEnd;
extern char* gHeapTop;

namespace
{
const ULONG STACK_GUARANTEE = 64 * 1024;
const DWORD EXC_STACK_BUFFER_OVERRUN = 0xC0000409;
const DWORD EXC_HEAP_CORRUPTION = 0xC0000374;

LONG reporting = 0;

#ifdef _WIN64
typedef unsigned long long Address;
#define ADDRESS "0x%08I64x"
#define CONTEXT_IP Rip
#define CONTEXT_SP Rsp
#define CONTEXT_FP Rbp
#else
typedef unsigned long Address;
#define ADDRESS "0x%08lx"
#define CONTEXT_IP Eip
#define CONTEXT_SP Esp
#define CONTEXT_FP Ebp
#endif

void Write(const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = _vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    if (len < 0)
        len = sizeof(buf) - 1;
    DWORD n = 0;
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), buf, (DWORD)len, &n, 0);
}

const char* AccessKind(ULONG_PTR kind)
{
    if (kind == 1)
        return "write";
    if (kind == 8)
        return "execute";
    return "read";
}

LONG WINAPI ReportCrash(EXCEPTION_POINTERS* ep)
{
    if (InterlockedExchange(&reporting, 1) != 0)
    {
        Sleep(INFINITE);
        return EXCEPTION_EXECUTE_HANDLER;
    }
    const EXCEPTION_RECORD* rec = ep->ExceptionRecord;
    const CONTEXT* ctx = ep->ContextRecord;
    Write("[pc] crash: exception 0x%08lx at " ADDRESS " in thread %lu\n", (unsigned long)rec->ExceptionCode,
          static_cast<Address>(reinterpret_cast<uintptr_t>(rec->ExceptionAddress)),
          (unsigned long)GetCurrentThreadId());
    Write("[pc] crash: program loaded at " ADDRESS "\n",
          static_cast<Address>(reinterpret_cast<uintptr_t>(GetModuleHandleW(0))));
    if (rec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && rec->NumberParameters >= 2)
        Write("[pc] crash: %s address " ADDRESS "\n", AccessKind(rec->ExceptionInformation[0]),
              static_cast<Address>(rec->ExceptionInformation[1]));
    if (rec->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
        Write("[pc] crash: stack overflow\n");
    Write("[pc] stack: " ADDRESS, static_cast<Address>(ctx->CONTEXT_IP));
    const uintptr_t* sp = reinterpret_cast<const uintptr_t*>(ctx->CONTEXT_SP);
    if (IsBadCodePtr(reinterpret_cast<FARPROC>(ctx->CONTEXT_IP)) && !IsBadReadPtr(sp, sizeof(uintptr_t)))
        Write(" [" ADDRESS "]", static_cast<Address>(sp[0]));
#ifdef _WIN64
    CONTEXT uw = *ctx;
    for (int d = 0; d < 48; ++d)
    {
        DWORD64 base = 0;
        PRUNTIME_FUNCTION fn = RtlLookupFunctionEntry(uw.Rip, &base, 0);
        if (fn == 0)
        {
            if (IsBadReadPtr(reinterpret_cast<const void*>(uw.Rsp), sizeof(DWORD64)))
                break;
            uw.Rip = *reinterpret_cast<const DWORD64*>(uw.Rsp);
            uw.Rsp += sizeof(DWORD64);
        }
        else
        {
            void* handlerData = 0;
            DWORD64 establisherFrame = 0;
            RtlVirtualUnwind(UNW_FLAG_NHANDLER, base, uw.Rip, fn, &uw, &handlerData,
                             &establisherFrame, 0);
        }
        if (uw.Rip == 0)
            break;
        Write(" " ADDRESS, static_cast<Address>(uw.Rip));
    }
    const uintptr_t* bp = 0;
#else
    const uintptr_t* bp = reinterpret_cast<const uintptr_t*>(ctx->CONTEXT_FP);
#endif
    for (int d = 0; d < 48 && bp != 0; ++d)
    {
        if (IsBadReadPtr(bp, sizeof(uintptr_t) * 2))
            break;
        uintptr_t ret = bp[1];
        if (ret == 0)
            break;
        Write(" " ADDRESS, static_cast<Address>(ret));
        const uintptr_t* next = reinterpret_cast<const uintptr_t*>(bp[0]);
        if (next <= bp)
            break;
        bp = next;
    }
    Write("\n");
    TerminateProcess(GetCurrentProcess(), 3);
    return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ReportFatalFirstChance(EXCEPTION_POINTERS* ep)
{
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    if (code == EXCEPTION_STACK_OVERFLOW || code == EXC_STACK_BUFFER_OVERRUN || code == EXC_HEAP_CORRUPTION)
        return ReportCrash(ep);
    return EXCEPTION_CONTINUE_SEARCH;
}

void ReportOutOfMemory()
{
    Write("[pc] crash: allocation failed; game heap %lu of %lu MB used\n",
          (unsigned long)((gHeapTop - gHeapStart) >> 20),
          (unsigned long)((gHeapEnd - gHeapStart) >> 20));
    __builtin_trap();
}

void ReportTerminate()
{
    Write("[pc] crash: std::terminate\n");
    __builtin_trap();
}

void ReportAbort(int)
{
    Write("[pc] crash: abort\n");
    __builtin_trap();
}
}

void PcCrashHandlerPrepareThread()
{
    ULONG size = STACK_GUARANTEE;
    SetThreadStackGuarantee(&size);
}

namespace
{
const DWORD HANG_SECONDS = 8;
HANDLE mainThread = 0;
volatile LONG beats = 0;

void WriteAddress(DWORD64 a)
{
    HMODULE mod = 0;
    char name[MAX_PATH] = "?";
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)a, &mod) && mod != 0)
    {
        char path[MAX_PATH];
        DWORD n = GetModuleFileNameA(mod, path, MAX_PATH);
        const char* base = path;
        for (DWORD i = 0; i < n; i++)
            if (path[i] == '\\' || path[i] == '/') base = path + i + 1;
        if (n > 0) lstrcpynA(name, base, MAX_PATH);
    }
    Write(" " ADDRESS " (%s+0x%I64x)", (Address)a, name, (unsigned long long)(a - (DWORD64)(uintptr_t)mod));
}

void WriteMainStack()
{
    CONTEXT ctx;
    ZeroMemory(&ctx, sizeof(ctx));
    ctx.ContextFlags = CONTEXT_FULL;
    if (SuspendThread(mainThread) == (DWORD)-1)
        return;
    BOOL ok = GetThreadContext(mainThread, &ctx);
    ResumeThread(mainThread);
    if (!ok)
        return;
    Write("[pc] hang: main thread stack:\n");
    WriteAddress(ctx.CONTEXT_IP);
#ifdef _WIN64
    CONTEXT uw = ctx;
    for (int d = 0; d < 40; ++d)
    {
        DWORD64 base = 0;
        PRUNTIME_FUNCTION fn = RtlLookupFunctionEntry(uw.Rip, &base, 0);
        if (fn == 0)
        {
            if (IsBadReadPtr((const void*)uw.Rsp, sizeof(DWORD64)))
                break;
            uw.Rip = *(const DWORD64*)uw.Rsp;
            uw.Rsp += sizeof(DWORD64);
        }
        else
        {
            void* handlerData = 0;
            DWORD64 frame = 0;
            RtlVirtualUnwind(UNW_FLAG_NHANDLER, base, uw.Rip, fn, &uw, &handlerData, &frame, 0);
        }
        if (uw.Rip == 0)
            break;
        Write("\n");
        WriteAddress(uw.Rip);
    }
#endif
    Write("\n");
}

DWORD WINAPI WatchdogThread(void*)
{
    LONG seen = beats;
    DWORD still = 0;
    DWORD reported = 0;
    for (;;)
    {
        Sleep(1000);
        LONG now = beats;
        if (now != seen)
        {
            if (reported != 0)
                Write("[pc] hang: main loop moving again after %lu s\n", (unsigned long)still);
            seen = now;
            still = 0;
            reported = 0;
            continue;
        }
        still++;
        if (still == HANG_SECONDS || (reported != 0 && still - reported >= 30))
        {
            Write("[pc] hang: no frame for %lu s\n", (unsigned long)still);
            WriteMainStack();
            reported = still;
        }
    }
    return 0;
}
}

void PcHangWatchdogStart()
{
    if (mainThread != 0)
        return;
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread,
                    THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, 0);
    HANDLE t = CreateThread(0, 64 * 1024, WatchdogThread, 0, 0, 0);
    if (t != 0)
        CloseHandle(t);
}

void PcHangWatchdogBeat()
{
    InterlockedIncrement(&beats);
}

void PcInstallCrashHandler()
{
    SetUnhandledExceptionFilter(ReportCrash);
    AddVectoredExceptionHandler(1, ReportFatalFirstChance);
    std::set_new_handler(ReportOutOfMemory);
    std::set_terminate(ReportTerminate);
    signal(SIGABRT, ReportAbort);
    PcCrashHandlerPrepareThread();
}

#else

#include "platform/pc/PcCrashHandler.hpp"

void PcCrashHandlerPrepareThread()
{
}

void PcInstallCrashHandler()
{
}

void PcHangWatchdogStart()
{
}

void PcHangWatchdogBeat()
{
}

#endif
