#include "nn/nex/QuazalCRT.hpp"

#include <string.h>

#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/QuazalZlib.hpp"

extern "C"
{

int* QuazalCRTAlloc(int iSize)
{
    if (iSize != 0)
    {
        int* pBlock = static_cast<int*>(nn::nex::MemoryManager::Allocate(iSize + sizeof(int)));
        if (pBlock != 0)
        {
            memset(pBlock, 0, iSize + sizeof(int));
            *pBlock = iSize;
            return pBlock + 1;
        }
    }

    return 0;
}

void QuazalCRTFree(int iAddress)
{
    if (iAddress == 0)
        return;

    nn::nex::MemoryManager::Free(reinterpret_cast<void*>(iAddress - sizeof(int)));
}

int QuazalCRTCalloc(int iCount, int iSize)
{
    if (iCount == 0 || iSize == 0)
        return 0;

    int iAddress = reinterpret_cast<int>(QuazalCRTAlloc(iCount * iSize));
    if (iAddress != 0)
        memset(reinterpret_cast<void*>(iAddress), 0, iCount * iSize);

    return iAddress;
}

voidpf QuazalCZlibAlloc(voidpf opaque, uInt items, uInt size)
{
    return reinterpret_cast<voidpf>(QuazalCRTCalloc(items, size));
}

void QuazalCZlibFree(voidpf opaque, voidpf address)
{
    QuazalCRTFree(reinterpret_cast<int>(address));
}
}
