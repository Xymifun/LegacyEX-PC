#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_FILESTORAGEDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_FILESTORAGEDEVICE_HPP

#include "nn/nex/StorageDevice.hpp"

namespace nn
{
namespace nex
{

class FileStorageDevice : public StorageDevice
{
public:
    FileStorageDevice();
    virtual ~FileStorageDevice();

    virtual StorageUnit* Create(const String& rName);
    virtual StorageUnit* Open(const String& rName);
    virtual bool Close(StorageUnit* pUnit);
    virtual bool Delete(const String& rName);

    static void AddFileNamePrefix(const String& rPrefix);
};

}
}

#endif
