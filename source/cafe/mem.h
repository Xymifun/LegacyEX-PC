#ifndef CAFE_MEM_H
#define CAFE_MEM_H

#include <coreinit/memexpheap.h>

#include <nn/types.h>

#define MEM_HEAP_INVALID_HANDLE NULL

inline MEMHeapHandle MEMCreateExpHeap(void* startAddress, u32 size)
{
    return MEMCreateExpHeapEx(startAddress, size, 0);
}

inline void* MEMAllocFromExpHeap(MEMHeapHandle heap, u32 size)
{
    return MEMAllocFromExpHeapEx(heap, size, 4);
}

inline u32 MEMGetAllocatableSizeForExpHeap(MEMHeapHandle heap)
{
    return MEMGetAllocatableSizeForExpHeapEx(heap, 4);
}

#endif
