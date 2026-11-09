#include "nn/nex/MemoryStorageDevice.hpp"

#include "nn/nex/MemoryStorageUnit.hpp"

namespace nn
{
namespace nex
{

MemoryStorageDevice::~MemoryStorageDevice()
{
    while (m_units.begin() != m_units.end())
    {
        Delete(m_units.begin()->first);
    }
}

StorageUnit* MemoryStorageDevice::Create(const String& rName)
{
    UnitMap::iterator it = m_units.find(rName);
    if (it == m_units.end())
    {
        MemoryStorageUnit* pUnit = new MemoryStorageUnit;
        m_units[rName] = pUnit;
        return pUnit;
    }
    return 0;
}

StorageUnit* MemoryStorageDevice::Open(const String& rName)
{
    UnitMap::iterator it = m_units.find(rName);
    if (it != m_units.end())
    {
        return it->second;
    }
    return 0;
}

bool MemoryStorageDevice::Close(StorageUnit*)
{
    return true;
}

bool MemoryStorageDevice::Delete(const String& rName)
{
    UnitMap::iterator it = m_units.find(rName);
    if (it != m_units.end())
    {
        delete it->second;
        m_units.erase(it);
        return true;
    }
    return false;
}

}
}
