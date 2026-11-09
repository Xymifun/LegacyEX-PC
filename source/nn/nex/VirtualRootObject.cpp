#include "nn/nex/VirtualRootObject.hpp"

#include "nn/nex/MemoryManager.hpp"

namespace nn
{
namespace nex
{

VirtualRootObject::~VirtualRootObject()
{
}

void* VirtualRootObject::operator new(std::size_t size)
{
    return MemoryManager::Allocate(static_cast<unsigned int>(size));
}

void VirtualRootObject::operator delete(void* memory)
{
    MemoryManager::Free(memory);
}

void* VirtualRootObject::operator new[](std::size_t size)
{
    return MemoryManager::Allocate(static_cast<unsigned int>(size));
}

void VirtualRootObject::operator delete[](void* memory)
{
    MemoryManager::Free(memory);
}

}
}
