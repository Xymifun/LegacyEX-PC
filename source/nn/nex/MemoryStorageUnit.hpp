#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MEMORYSTORAGEUNIT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MEMORYSTORAGEUNIT_HPP

#include "nn/nex/StorageUnit.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class MemoryStorageUnit : public StorageUnit
{
public:
    MemoryStorageUnit() {}

    virtual ~MemoryStorageUnit() {}

    virtual unsigned int GetSize() const;
    virtual void Truncate();
    virtual unsigned int Read(unsigned int uiOffset, unsigned int uiSize, unsigned char* pBuffer) const;
    virtual unsigned int Write(unsigned int uiOffset, unsigned int uiSize, unsigned char* pBuffer);

private:
    qVector<unsigned char> m_data;
};

}
}

#endif
