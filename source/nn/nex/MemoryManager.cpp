#include "nn/nex/MemoryManager.hpp"

#include <cstdlib>

#include "nn/nex/MutexPrimitive.hpp"

namespace nn
{
namespace nex
{

MemoryManager::DefaultMemoryManagerState MemoryManager::s_eDefaultMemoryManagerState = DEFAULT_MEMORY_MANAGER_ACTIVE;
MemoryManager::Allocator MemoryManager::s_allocator = 0;
MemoryManager::Deallocator MemoryManager::s_deallocator = 0;
MemoryManager* MemoryManager::s_pDefaultMemoryManager = 0;
bool MemoryManager::s_bDefaultMemoryManagerCreated = false;

MemoryManager::~MemoryManager()
{
}

void* MemoryManager::GenericMalloc(unsigned int size)
{
    if (s_allocator != 0)
        return s_allocator(size);
    return std::malloc(size);
}

void* MemoryManager::Allocate(unsigned int size)
{
    const unsigned int alignedSize = (size + 3u) & 0xfffffffcU;
    unsigned char* allocation = static_cast<unsigned char*>(GenericMalloc(alignedSize + 8u));
    *reinterpret_cast<Deallocator*>(allocation) = s_deallocator;
    return allocation + 8;
}

void MemoryManager::GenericFree(Deallocator deallocator, void* block)
{
    if (deallocator != 0)
    {
        deallocator(block);
        return;
    }
    std::free(block);
}

void MemoryManager::Free(void* block)
{
    if (block == 0)
        return;

    unsigned char* allocation = static_cast<unsigned char*>(block) - 8;
    GenericFree(*reinterpret_cast<Deallocator*>(allocation), allocation);
}

void* MemoryManager::AllocateForPbPool(void* block, Deallocator deallocator, void* context)
{
    void** header = static_cast<void**>(block);
    *reinterpret_cast<Deallocator*>(header) = deallocator;
    header[1] = context;
    return header + 2;
}

void MemoryManager::ShutdownDefaultMemoryManager()
{
    s_eDefaultMemoryManagerState = DEFAULT_MEMORY_MANAGER_SHUTTING_DOWN;
    delete s_pDefaultMemoryManager->m_pMutex;
    s_pDefaultMemoryManager->m_pMutex = 0;
    delete s_pDefaultMemoryManager;
    s_pDefaultMemoryManager = 0;
    s_bDefaultMemoryManagerCreated = false;
    s_eDefaultMemoryManagerState = DEFAULT_MEMORY_MANAGER_SHUT_DOWN;
}

void MemoryManager::BeginProtection()
{
}

void MemoryManager::EndProtection()
{
}

}
}
