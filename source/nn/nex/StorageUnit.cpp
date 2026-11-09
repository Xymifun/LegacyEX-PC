#include "nn/nex/StorageUnit.hpp"

namespace nn
{
namespace nex
{

static const unsigned int STORAGE_COPY_CHUNK_SIZE = 4096;

bool StorageUnit::AppendData(const StorageUnit* pSource, StorageUnit* pDestination)
{
    unsigned char aBuffer[STORAGE_COPY_CHUNK_SIZE];
    unsigned int uiSourceSize = pSource->GetSize();
    unsigned int uiDestinationSize = pDestination->GetSize();
    unsigned int uiOffset = 0;
    while (uiOffset < uiSourceSize)
    {
        unsigned int uiEnd = uiOffset + STORAGE_COPY_CHUNK_SIZE;
        if (uiEnd > uiSourceSize)
        {
            uiEnd = uiSourceSize;
        }
        if (!pSource->Read(uiOffset, uiEnd - uiOffset, aBuffer)
            || !pDestination->Write(uiDestinationSize + uiOffset, uiEnd - uiOffset, aBuffer))
        {
            return false;
        }
        uiOffset = uiEnd;
    }
    return true;
}

void StorageUnit::CopyData(const StorageUnit* pSource, StorageUnit* pDestination)
{
    pDestination->Truncate();
    AppendData(pSource, pDestination);
}

}
}
