#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_FILESTORAGEUNIT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_FILESTORAGEUNIT_HPP

#include "nn/nex/StorageUnit.hpp"

namespace nn
{
namespace nex
{

class FileStorageUnit : public StorageUnit
{
public:
    virtual ~FileStorageUnit() {}

    virtual unsigned int GetSize() const;

    virtual void Truncate() {}

    virtual unsigned int Read(unsigned int uiOffset, unsigned int uiSize, unsigned char* pBuffer) const;
    virtual unsigned int Write(unsigned int uiOffset, unsigned int uiSize, unsigned char* pBuffer);

    bool Close();
};

}
}

#endif
