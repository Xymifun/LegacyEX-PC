#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STORAGEUNIT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STORAGEUNIT_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class StorageUnit : public RootObject
{
public:
    virtual ~StorageUnit() {}

    virtual unsigned int GetSize() const = 0;
    virtual void Truncate() = 0;
    virtual unsigned int Read(unsigned int uiOffset, unsigned int uiSize, unsigned char* pBuffer) const = 0;
    virtual unsigned int Write(unsigned int uiOffset, unsigned int uiSize, unsigned char* pBuffer) = 0;

    virtual unsigned int Reserve(unsigned int) { return 0; }

    virtual unsigned int GetReservedSize() const { return 0; }

    static bool AppendData(const StorageUnit* pSource, StorageUnit* pDestination);
    static void CopyData(const StorageUnit* pSource, StorageUnit* pDestination);
};

}
}

#endif
