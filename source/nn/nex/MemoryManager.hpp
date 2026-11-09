#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MEMORYMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MEMORYMANAGER_HPP

#include <cstddef>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class MutexPrimitive;

class MemoryManager : public RootObject
{
public:
    typedef void* (*Allocator)(unsigned int);
    typedef void (*Deallocator)(void*);

    enum DefaultMemoryManagerState
    {
        DEFAULT_MEMORY_MANAGER_ACTIVE = 0,
        DEFAULT_MEMORY_MANAGER_SHUTTING_DOWN = 1,
        DEFAULT_MEMORY_MANAGER_SHUT_DOWN = 2
    };

    static void SetCallbacks(Allocator allocator, Deallocator deallocator)
    {
        s_allocator = allocator;
        s_deallocator = deallocator;
    }

    virtual ~MemoryManager();

    static void* GenericMalloc(unsigned int size);
    static void* Allocate(unsigned int size);
    static void GenericFree(Deallocator deallocator, void* block);
    static void Free(void* block);
    static void* AllocateForPbPool(void* block, Deallocator deallocator, void* context);

    static void ShutdownDefaultMemoryManager();
    virtual void BeginProtection();
    virtual void EndProtection();

private:
    static DefaultMemoryManagerState s_eDefaultMemoryManagerState;
    static Allocator s_allocator;
    static Deallocator s_deallocator;
    static MemoryManager* s_pDefaultMemoryManager;
    static bool s_bDefaultMemoryManagerCreated;

    const wchar_t* m_szName;
    unsigned int m_uiMemUsage;
    unsigned int m_field08;
    unsigned int m_field0c;
    MutexPrimitive* m_pMutex;
};

}
}

#endif
