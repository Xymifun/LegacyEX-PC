#include "nn/nex/RootObject.hpp"

#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/PacketBufferManager.hpp"

namespace nn
{
namespace nex
{

void* RootObject::operator new(std::size_t size)
{
    return MemoryManager::Allocate(static_cast<unsigned int>(size));
}

void* RootObject::operator new(std::size_t size, TargetPool pool)
{
    if (pool != TARGET_POOL_PACKET_MANAGEMENT)
        return 0;

    return PacketBufferManager::GetInstance()->AllocateManagementObject(static_cast<unsigned int>(size));
}

void RootObject::operator delete(void* memory)
{
    if (memory == 0)
        return;

    MemoryManager::Free(memory);
}

void* RootObject::operator new[](std::size_t size)
{
    return MemoryManager::Allocate(static_cast<unsigned int>(size));
}

void RootObject::operator delete[](void* memory)
{
    MemoryManager::Free(memory);
}

}
}
