#include "platform/pc/PcFunctionHooks.hpp"

#include "platform/pc/PcNativeMods.hpp"

#include <stdio.h>
#include <string.h>
#include <map>
#include <string>

#ifdef _WIN32
#include <windows.h>

namespace
{
const unsigned char ENTRY_NOP[4] = {0x0F, 0x1F, 0x44, 0x00};
const int STUB_SIZE = 16;
const int BLOCK_SIZE = 65536;

std::map<char*, void*> hooked;
char* stubBlock = 0;
int stubUsed = 0;

bool Near(char* from, char* to)
{
    long long d = to - from;
    return d > -0x7FFF0000LL && d < 0x7FFF0000LL;
}

char* NewBlock()
{
    char* base = (char*)GetModuleHandleW(0);
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);
    char* end = base + ((nt->OptionalHeader.SizeOfImage + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1));
    for (long long off = 0; off < 0x40000000LL; off += BLOCK_SIZE)
    {
        if ((unsigned long long)base > (unsigned long long)off + BLOCK_SIZE + 0x10000)
        {
            void* p = VirtualAlloc(base - off - BLOCK_SIZE, BLOCK_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (p != 0)
                return (char*)p;
        }
        void* p = VirtualAlloc(end + off, BLOCK_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (p != 0)
            return (char*)p;
    }
    return 0;
}

char* NewStub(char* fn, void* dest)
{
    if (stubBlock == 0 || stubUsed + STUB_SIZE > BLOCK_SIZE || !Near(fn, stubBlock))
    {
        stubBlock = NewBlock();
        stubUsed = 0;
        if (stubBlock == 0 || !Near(fn, stubBlock))
            return 0;
    }
    char* stub = stubBlock + stubUsed;
    stubUsed += STUB_SIZE;
    DWORD old;
    VirtualProtect(stubBlock, BLOCK_SIZE, PAGE_READWRITE, &old);
    unsigned char code[STUB_SIZE];
    memset(code, 0xCC, sizeof(code));
    if (sizeof(void*) == 8)
    {
        code[0] = 0xFF;
        code[1] = 0x25;
        memset(code + 2, 0, 4);
        memcpy(code + 6, &dest, 8);
    }
    else
    {
        int rel = (int)((char*)dest - (stub + 5));
        code[0] = 0xE9;
        memcpy(code + 1, &rel, 4);
    }
    memcpy(stub, code, STUB_SIZE);
    VirtualProtect(stubBlock, BLOCK_SIZE, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), stub, STUB_SIZE);
    return stub;
}

void WriteCode(char* at, const unsigned char* code, int len)
{
    DWORD old;
    VirtualProtect(at, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy(at, code, len);
    VirtualProtect(at, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), at, len);
}

void Fail(const char* name, const std::string& why)
{
    printf("[pc] native %s: could not hook \"%s\": %s\n", PcNativeModsCurrentId(), name != 0 ? name : "", why.c_str());
    fflush(stdout);
}
}

int PcHookFunction(const char* name, void* replacement, void** original)
{
    std::string found, err;
    char* fn = (char*)PcNativeModsFindFunction(name, &found, &err);
    if (fn == 0)
    {
        Fail(name, err);
        return 0;
    }
    if (replacement == 0)
    {
        Fail(name, "no replacement function given");
        return 0;
    }
    void* next;
    bool chained = false;
    std::map<char*, void*>::iterator it = hooked.find(fn);
    if (it != hooked.end())
    {
        next = it->second;
        chained = true;
    }
    else if (memcmp(fn, ENTRY_NOP, 4) == 0)
        next = fn + 5;
    else
    {
        Fail(name, "it does not start with the hook nop (not part of the game's own code)");
        return 0;
    }
    char* stub = NewStub(fn, replacement);
    if (stub == 0)
    {
        Fail(name, "no memory for a jump near the game's code");
        return 0;
    }
    if (original != 0)
        *original = next;
    unsigned char jmp[5];
    int rel = (int)(stub - (fn + 5));
    jmp[0] = 0xE9;
    memcpy(jmp + 1, &rel, 4);
    WriteCode(fn, jmp, 5);
    hooked[fn] = replacement;
    printf("[pc] native %s: hooked %s%s\n", PcNativeModsCurrentId(), found.c_str(),
           chained ? " (on top of an earlier hook)" : "");
    fflush(stdout);
    return 1;
}

#else

int PcHookFunction(const char* name, void*, void**)
{
    printf("[pc] native: could not hook \"%s\": native mods need the Windows build\n", name != 0 ? name : "");
    return 0;
}

#endif
