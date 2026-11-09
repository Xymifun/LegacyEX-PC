#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STORAGEDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STORAGEDEVICE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class StorageUnit;

class StorageDevice : public RootObject
{
public:
    virtual ~StorageDevice() {}

    virtual StorageUnit* Create(const String& rName) = 0;
    virtual StorageUnit* Open(const String& rName) = 0;
    virtual bool Close(StorageUnit* pUnit) = 0;
    virtual bool Delete(const String& rName) = 0;
};

}
}

#endif
