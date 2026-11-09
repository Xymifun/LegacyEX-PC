#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MEMORYSTORAGEDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MEMORYSTORAGEDEVICE_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/StorageDevice.hpp"

namespace nn
{
namespace nex
{

class MemoryStorageUnit;

class MemoryStorageDevice : public StorageDevice
{
public:
    MemoryStorageDevice() {}

    virtual ~MemoryStorageDevice();

    virtual StorageUnit* Create(const String& rName);
    virtual StorageUnit* Open(const String& rName);
    virtual bool Close(StorageUnit* pUnit);
    virtual bool Delete(const String& rName);

private:
    typedef std::map<String, MemoryStorageUnit*, std::less<String>,
                     MemAllocator<std::pair<const String, MemoryStorageUnit*> > >
        UnitMap;

    UnitMap m_units;
};

}
}

#endif
