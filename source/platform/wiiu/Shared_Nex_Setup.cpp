#include "platform/wiiu/Shared_Nex_Setup.hpp"
#include "nn/nex/EventLog.hpp"
#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/NgsFacade.hpp"
#include "nn/nex/TraceLog.hpp"

#include <coreinit/memdefaultheap.h>
#include <coreinit/memexpheap.h>
#include <iostream>

void checkPiaCoreToUse();

namespace
{

unsigned int sRuntimeMemorySize = 0;
unsigned int sReservedMemorySize = 0;
void* sRuntimeMemory = NULL;
MEMHeapHandle sRuntimeHeap;
void* sReservedMemory = NULL;
unsigned int sMemoryUsed = 0;
unsigned int sPeakMemoryUsed = 0;

const int cRuntimeMemoryAlignment = 8;

void* InnerAlloc(unsigned int size)
{
    unsigned int blockSize;
    checkPiaCoreToUse();
    void* block = MEMAllocFromExpHeapEx(sRuntimeHeap, size, cRuntimeMemoryAlignment);
    if (block != NULL)
    {
        blockSize = MEMGetSizeForMBlockExpHeap(block);
        sMemoryUsed += blockSize;
        if (sMemoryUsed > sPeakMemoryUsed)
        {
            sPeakMemoryUsed = sMemoryUsed;
        }
    }
    return block;
}

void InnerFree(void* block)
{
    checkPiaCoreToUse();
    sMemoryUsed -= MEMGetSizeForMBlockExpHeap(block);
    MEMFreeToExpHeap(sRuntimeHeap, block);
}

void FreeFunction(void* block)
{
    InnerFree(block);
}

void ReleaseReservedMemory()
{
    checkPiaCoreToUse();
    if (sReservedMemory != NULL)
    {
        unsigned int memoryUsed = sMemoryUsed;
        unsigned int peakMemoryUsed = sPeakMemoryUsed;
        FreeFunction(sReservedMemory);
        sMemoryUsed = memoryUsed;
        sReservedMemory = NULL;
        sPeakMemoryUsed = peakMemoryUsed;
    }
}

void* AllocFunction(unsigned int size)
{
    checkPiaCoreToUse();
    void* block = InnerAlloc(size);
    if (block == NULL)
    {
        ReleaseReservedMemory();
        block = InnerAlloc(size);
    }
    return block;
}

void AcquireReservedMemory()
{
    checkPiaCoreToUse();
    if (sReservedMemory == NULL)
    {
        unsigned int memoryUsed = sMemoryUsed;
        unsigned int peakMemoryUsed = sPeakMemoryUsed;
        sReservedMemory = AllocFunction(sReservedMemorySize);
        sMemoryUsed = memoryUsed;
        sPeakMemoryUsed = peakMemoryUsed;
    }
}

void InitializeRuntimeMemory(unsigned int runtimeSize, unsigned int reservedSize)
{
    checkPiaCoreToUse();
    unsigned int heapSize = runtimeSize + reservedSize;
    sRuntimeMemorySize = runtimeSize;
    sReservedMemorySize = reservedSize;
    sRuntimeMemory = MEMAllocFromDefaultHeapEx(heapSize, cRuntimeMemoryAlignment);
    sRuntimeHeap = MEMCreateExpHeapEx(sRuntimeMemory, heapSize, MEM_HEAP_FLAG_ZERO_ALLOCATED | MEM_HEAP_FLAG_USE_LOCK);
    nn::nex::MemoryManager::SetCallbacks(AllocFunction, FreeFunction);
    if (sReservedMemorySize != 0)
    {
        AcquireReservedMemory();
    }
}

}

static nn::nex::NgsFacade* s_ngsFacade = NULL;
static bool s_nexInitialized = false;
static unsigned int s_internalErrorCode;
static unsigned int s_lastAPIErrorCode;

static void Nex_InitializeErrorHandling()
{
    s_lastAPIErrorCode = 0;
    s_internalErrorCode = 0;
}

unsigned int Nex_InitializeNex(unsigned int runtimeMemorySize, unsigned int reservedMemorySize,
                               nn::nex::Core::ThreadMode threadMode)
{
    checkPiaCoreToUse();
    InitializeRuntimeMemory(runtimeMemorySize, reservedMemorySize);
    nn::nex::GlobalVariables::AcquireInstance();
    Nex_InitializeErrorHandling();
    nn::nex::g_threadMode = threadMode;
    s_ngsFacade = new nn::nex::NgsFacade();
    nn::nex::EventLog* eventLog = nn::nex::EventLog::GetInstance();
    eventLog->SetLogLevel(nn::nex::EventLog::Debug);
    nn::nex::TraceLog::GetInstance()->SetFlag(nn::nex::TraceLog::TRACE_ALWAYS);
    s_nexInitialized = true;
    return 0;
}

static void Nex_FinalizeErrorHandling()
{
    s_lastAPIErrorCode = 0;
    s_internalErrorCode = 0;
}

namespace
{

void FinalizeRuntimeMemory()
{
    checkPiaCoreToUse();
    if (sReservedMemorySize != 0)
    {
        ReleaseReservedMemory();
    }
    if (sMemoryUsed != 0)
    {
        MEMDumpHeap(sRuntimeHeap);
    }
    MEMDestroyExpHeap(sRuntimeHeap);
    MEMFreeToDefaultHeap(sRuntimeMemory);
    sMemoryUsed = 0;
    sPeakMemoryUsed = 0;
}

}

void Nex_FinalizeNex()
{
    checkPiaCoreToUse();
    if (s_nexInitialized)
    {
        if (s_ngsFacade != NULL)
        {
            delete s_ngsFacade;
            s_ngsFacade = NULL;
        }
        nn::nex::GlobalVariables::ReleaseInstance();
        Nex_FinalizeErrorHandling();
        FinalizeRuntimeMemory();
        s_nexInitialized = false;
    }
}

nn::nex::NgsFacade* Nex_GetNgsFacadeInstance()
{
    return s_ngsFacade;
}

unsigned int Nex_GetInternalErrorCode()
{
    return s_internalErrorCode;
}

unsigned int Nex_GetLastAPIErrorCode()
{
    return s_lastAPIErrorCode;
}

unsigned int Nex_GetPeakMemoryUsage()
{
    return sPeakMemoryUsed;
}
