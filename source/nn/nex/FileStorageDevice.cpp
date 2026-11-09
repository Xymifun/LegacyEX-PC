#include "nn/nex/FileStorageDevice.hpp"

#include "nn/nex/StorageUnit.hpp"

namespace nn
{
namespace nex
{

FileStorageDevice::~FileStorageDevice()
{
}

StorageUnit* FileStorageDevice::Create(const String&)
{
    return 0;
}

StorageUnit* FileStorageDevice::Open(const String&)
{
    return 0;
}

bool FileStorageDevice::Close(StorageUnit* pUnit)
{
    delete pUnit;
    return false;
}

bool FileStorageDevice::Delete(const String&)
{
    return false;
}

}
}
