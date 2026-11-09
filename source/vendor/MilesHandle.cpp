#include "vendor/MilesHandle.hpp"

#include <stddef.h>
#include <string.h>

static const unsigned int HANDLE_BLOCK_OVERHEAD = 0x17;
static const unsigned int HANDLE_BLOCK_ALIGNMENT_MASK = 0xFFFFFFF0;
static const unsigned int HANDLE_BLOCK_SIZE_MASK = 0xFFFFFFFC;
static const unsigned int HANDLE_START_ALIGNMENT = 0xF;
static const unsigned int HANDLE_START_ALIGNMENT_MASK = 0xFFFFFFF0;
static const int HANDLE_TAG_END = 0;
static const int HANDLE_TAG_FREE = 1;

int HandleFreeHead = 0;
unsigned int HandleGeneration = 1;
int HandleBreakOnError = 0;
unsigned int HandleCount = 0;
unsigned char* HandleTableEnd = NULL;
unsigned char* HandleHeapStart = NULL;
unsigned char* HandleHeapCursor = NULL;
unsigned char* HandleHeapBase = NULL;
unsigned int HandleHeapSize = 0;
int HandleNeedsDefrag = 0;

void Handle_Shutdown()
{
    HandleHeapBase = NULL;
    HandleHeapSize = 0;
    HandleHeapStart = NULL;
    HandleHeapCursor = NULL;
    HandleCount = 0;
    HandleFreeHead = 0;
}

void Handle_Init(void* pMemory, int iSize)
{
    Handle_Shutdown();

    HandleHeapBase = static_cast<unsigned char*>(pMemory);
    HandleHeapSize = iSize;
    HandleHeapStart = reinterpret_cast<unsigned char*>(
        (reinterpret_cast<size_t>(HandleHeapBase) + HANDLE_START_ALIGNMENT) & ~static_cast<size_t>(HANDLE_START_ALIGNMENT));

    if (HandleHeapStart - sizeof(int) < HandleHeapBase)
    {
        HandleHeapStart = HandleHeapStart + 0x10;
    }

    reinterpret_cast<int*>(HandleHeapStart)[-1] = HANDLE_TAG_END;
    HandleHeapCursor = HandleHeapStart;
    HandleTableEnd = HandleHeapBase + HandleHeapSize;
    HandleCount = 0;
}

void Handle_Defrag()
{
    HandleEntry* pEntry;
    unsigned char* pCursor;
    int* pBlock;
    int iTag;
    unsigned int uBlockSize;

    if (HandleNeedsDefrag == 0)
    {
        return;
    }

    pCursor = HandleHeapStart;
    pBlock = reinterpret_cast<int*>(pCursor) - 1;
    iTag = *pBlock;

    while (iTag != HANDLE_TAG_END)
    {
        if (iTag == HANDLE_TAG_FREE)
        {
            pBlock = reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(pBlock)
                                            + (static_cast<unsigned int>(pBlock[1]) & HANDLE_BLOCK_SIZE_MASK));
        }
        else
        {
            pEntry = reinterpret_cast<HandleEntry*>(HandleTableEnd + iTag * sizeof(HandleEntry));
            uBlockSize = (pEntry->size + HANDLE_BLOCK_OVERHEAD) & HANDLE_BLOCK_ALIGNMENT_MASK;

            if (static_cast<unsigned char*>(pEntry->data) != pCursor)
            {
                memmove(pCursor - sizeof(int), static_cast<unsigned char*>(pEntry->data) - sizeof(int), uBlockSize);
                reinterpret_cast<HandleEntry*>(HandleTableEnd)[iTag].data = pCursor;
            }

            pBlock = reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(pBlock)
                                            + (uBlockSize & HANDLE_BLOCK_SIZE_MASK));
            pCursor = pCursor + uBlockSize;
        }

        iTag = *pBlock;
    }

    reinterpret_cast<int*>(pCursor)[-1] = HANDLE_TAG_END;
    HandleNeedsDefrag = 0;
    HandleHeapCursor = pCursor;
}

void Handle_MemStats(int* piTotal, int* piFree)
{
    if (piFree == NULL)
    {
        return;
    }

    if (piTotal == NULL)
    {
        return;
    }

    *piTotal = HandleHeapSize;
    *piFree = (HandleTableEnd - HandleHeapCursor) - HandleCount * sizeof(HandleEntry);
}
