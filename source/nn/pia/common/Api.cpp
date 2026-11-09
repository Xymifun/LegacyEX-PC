#include "nn/pia/Common.hpp"

#include <cstdarg>
#include <cstdio>
#include <new>
#include <nn/middleware.h>
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/CachedPrint.hpp"
#include "nn/pia/common/Report.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/Trace.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "platform/wiiu/pead.hpp"

namespace pead
{
namespace system
{
void PrintStringImpl(const char* text, int level);
}
}

namespace nn
{
namespace pia
{
void EmbedMiddlewareInfo();
}
}

namespace
{
unsigned char g_commonFirstInitialize = 1;
nn::pia::common::Report* g_commonReport;
void (*g_printFunction)(const char*, int);
unsigned char g_commonInitialized = 0;
unsigned char g_commonInSetup = 0;
unsigned char g_commonReportStorage[sizeof(nn::pia::common::Report)];

class DelegatePrint : public pead::IDelegate1<const pead::PrintConfig::PrintEventArg&>
{
public:
    virtual void invoke(const pead::PrintConfig::PrintEventArg& event);
};

class MyAllocCallback : public pead::IDelegate1<const pead::HeapMgr::AllocCallbackArg*>
{
public:
    virtual void invoke(const pead::HeapMgr::AllocCallbackArg* event);
};

class MyFreeCallback : public pead::IDelegate1<const pead::HeapMgr::FreeCallbackArg*>
{
public:
    virtual void invoke(const pead::HeapMgr::FreeCallbackArg* event);
};

void MyAllocCallback::invoke(const pead::HeapMgr::AllocCallbackArg*)
{
}

void MyFreeCallback::invoke(const pead::HeapMgr::FreeCallbackArg*)
{
}

void DelegatePrint::invoke(const pead::PrintConfig::PrintEventArg& event)
{
    if (g_printFunction == 0)
    {
        pead::system::PrintStringImpl(event.text, event.level);
        return;
    }
    g_printFunction(event.text, event.level);
}

void registerInternalPrintCallback()
{
    static DelegatePrint sDelegatePrint;
    pead::PrintConfig::registerFinalCallback(&sDelegatePrint);
}

void unregisterInternalPrintCallback()
{
    pead::PrintConfig::registerFinalCallback(0);
}

void destroySystemSingletons()
{
    nn::pia::common::PayloadSizeManager::DestroyInstance();
}

void createSystemSingletons()
{
    nn::pia::common::PayloadSizeManager::CreateInstance();
}
}

namespace nn
{
namespace pia
{
namespace common
{

Result Initialize(void* workMemory, unsigned int workMemorySize)
{
    nn::pia::EmbedMiddlewareInfo();
    EmbedModuleInfo();

    if (workMemory == 0)
        return ResultInvalidArgument();
    if (workMemorySize == 0)
        return ResultInvalidArgument();
    if ((reinterpret_cast<unsigned int>(workMemory) & 3) != 0)
        return ResultInvalidArgument();
    if (g_commonInitialized != 0)
        return ResultAlreadyInitialized();

    if (g_commonFirstInitialize)
    {
        g_sessionBeginMonitoringContent.Initialize();
        g_sessionStateMonitoringContent.Initialize();
        g_commonFirstInitialize = 0;
    }
    else
    {
        const unsigned long long metric11 = g_sessionBeginMonitoringContent.m_metric11;
        const unsigned char metric12 = g_sessionBeginMonitoringContent.m_metric12;
        const unsigned int metric13 = g_sessionBeginMonitoringContent.m_metric13;
        const unsigned int metric14 = g_sessionBeginMonitoringContent.m_metric14;
        const unsigned char metric15 = g_sessionBeginMonitoringContent.m_metric15;
        const unsigned char metric16 = g_sessionBeginMonitoringContent.m_metric16;
        const unsigned char metric72 = g_sessionBeginMonitoringContent.m_metric72;
        const unsigned char metric73 = g_sessionBeginMonitoringContent.m_metric73;
        g_sessionBeginMonitoringContent.Initialize();
        g_sessionStateMonitoringContent.Initialize();
        g_sessionBeginMonitoringContent.m_metric11 = metric11;
        g_sessionBeginMonitoringContent.m_metric12 = metric12;
        g_sessionBeginMonitoringContent.m_metric13 = metric13;
        g_sessionBeginMonitoringContent.m_metric14 = metric14;
        g_sessionBeginMonitoringContent.m_metric15 = metric15;
        g_sessionBeginMonitoringContent.m_metric16 = metric16;
        g_sessionBeginMonitoringContent.m_metric72 = metric72;
        g_sessionBeginMonitoringContent.m_metric73 = metric73;
    }

    HeapManager::Initialize(workMemory, workMemorySize);
    g_sessionBeginMonitoringContent.m_metric9 = workMemorySize;
    pead::SafeStringBase<char> heapName("pia common heap");
    HeapManager::Setup(cModuleType_Common, 0, heapName);
    g_commonReport = new (g_commonReportStorage) Report();
    g_currentReport = g_commonReport;
    pead::CoreInfo::configure();
    registerInternalPrintCallback();
    g_commonInitialized = 1;
    ModuleStackChecker::Register(cModuleType_Common);
    return 0;
}

Result EndSetup()
{
    if (g_commonInitialized == 0)
        return transport::cResultNotInitialized;
    if (g_commonInSetup == 0)
        return transport::cResultInvalidState;

    HeapManager::GetHeap()->adjust();
    HeapManager::ClearCurrentHeap();
    g_commonInSetup = 0;
    return 0;
}

void Finalize()
{
    if (g_commonInitialized == 0)
        return;

    if (g_commonInSetup != 0)
        EndSetup();
    unregisterInternalPrintCallback();
    Scheduler::DestroyInstance();
    WatermarkManager::DestroyInstance();
    Trace::DestroyInstance();
    CachedPrint::DestroyInstance();
    destroySystemSingletons();
    if (g_currentReport == g_commonReport)
        g_currentReport = 0;
    g_commonReport->~Report();
    g_commonReport = 0;
    HeapManager::Cleanup(cModuleType_Common);
    HeapManager::Finalize();
    g_commonInitialized = 0;
    ModuleStackChecker::Unregister(cModuleType_Common);
}

Result BeginSetup()
{
    if (g_commonInitialized == 0)
        return ResultNotInitialized();
    if (g_commonInSetup != 0)
        return ResultInvalidState();

    HeapManager::SetCurrentHeap(cModuleType_Common);
    g_commonInSetup = 1;
    createSystemSingletons();
    return 0;
}

unsigned int GetMemoryUsage()
{
    pead::Heap* heap = g_commonInitialized ? pead::HeapMgr::getRootHeap(0) : 0;
    if (heap == 0)
        return 0;
    return heap->getSize() - heap->getMaxAllocatableSize(4);
}

void SetProgramInformation(unsigned int sdkVersion, unsigned char nexMajorVersion, unsigned char nexMinorVersion,
                           unsigned char nexMicroVersion)
{
    g_sessionBeginMonitoringContent.SetSdkVersion(sdkVersion);
    g_sessionBeginMonitoringContent.SetNexVersion(nexMajorVersion, nexMinorVersion, nexMicroVersion);
}

void PrintDebugInfo()
{
    pead::Heap* heap = HeapManager::GetHeap(cModuleType_Common);
    if (heap != 0)
        heap->dump();
}

void VPrintf(const char* format, va_list args)
{
    char buffer[512];
    unsigned int length = std::vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    if (static_cast<int>(length) < 0 || length >= sizeof(buffer))
    {
        buffer[sizeof(buffer) - 1] = '\0';
        pead::system::PrintString(buffer, sizeof(buffer));
    }
    else
    {
        buffer[length] = '\0';
        pead::system::PrintString(buffer, length);
    }
}

void Printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VPrintf(format, args);
    va_end(args);
}

unsigned char IsInitialized()
{
    return g_commonInitialized;
}

unsigned char IsInSetupMode()
{
    return g_commonInSetup;
}

void serializeU64(unsigned char* buffer, unsigned long long value)
{
    buffer[0] = static_cast<unsigned char>((value >> 56) & 0xff);
    buffer[1] = static_cast<unsigned char>((value >> 48) & 0xff);
    buffer[2] = static_cast<unsigned char>((value >> 40) & 0xff);
    buffer[3] = static_cast<unsigned char>((value >> 32) & 0xff);
    buffer[4] = static_cast<unsigned char>((value >> 24) & 0xff);
    buffer[5] = static_cast<unsigned char>((value >> 16) & 0xff);
    buffer[6] = static_cast<unsigned char>((value >> 8) & 0xff);
    buffer[7] = static_cast<unsigned char>(value & 0xff);
}

unsigned long long deserializeU64(const unsigned char* buffer)
{
    unsigned long long value = 0;
    value |= static_cast<unsigned long long>(buffer[0]) << 56;
    value |= static_cast<unsigned long long>(buffer[1]) << 48;
    value |= static_cast<unsigned long long>(buffer[2]) << 40;
    value |= static_cast<unsigned long long>(buffer[3]) << 32;
    value |= static_cast<unsigned long long>(buffer[4]) << 24;
    value |= static_cast<unsigned long long>(buffer[5]) << 16;
    value |= static_cast<unsigned long long>(buffer[6]) << 8;
    value |= static_cast<unsigned long long>(buffer[7]);
    return value;
}

void serializeU32(unsigned char* buffer, unsigned int value)
{
    buffer[0] = static_cast<unsigned char>((value >> 24) & 0xff);
    buffer[3] = static_cast<unsigned char>(value & 0xff);
    buffer[1] = static_cast<unsigned char>((value >> 16) & 0xff);
    buffer[2] = static_cast<unsigned char>((value >> 8) & 0xff);
}

unsigned int deserializeU32(const unsigned char* buffer)
{
    return (static_cast<unsigned int>(buffer[0]) << 24) | (static_cast<unsigned int>(buffer[1]) << 16)
           | (static_cast<unsigned int>(buffer[2]) << 8) | buffer[3];
}

void serializeU16(unsigned char* buffer, unsigned short value)
{
    buffer[0] = static_cast<unsigned char>((value >> 8) & 0xff);
    buffer[1] = static_cast<unsigned char>(value & 0xff);
}

unsigned short deserializeU16(const unsigned char* buffer)
{
    return static_cast<unsigned short>((static_cast<unsigned int>(buffer[0]) << 8) | buffer[1]);
}

void serializeU8(unsigned char* buffer, unsigned char value)
{
    buffer[0] = value;
}

unsigned char deserializeU8(const unsigned char* buffer)
{
    return buffer[0];
}

bool isValidSourceStationIndex(StationIndex index)
{
    if (static_cast<unsigned int>(index) < cStationIndexNum || index == cStationIndexInvalid)
        return true;
    return false;
}

bool isValidDestinationStationIndex(StationIndex index)
{
    if (static_cast<unsigned int>(index) < cStationIndexNum || index == cStationIndexInvalid
        || index == cStationIndexSentinelFE || index == cStationIndexSentinelFF)
        return true;
    return false;
}

void dump(unsigned long long, const void*, unsigned int)
{
}

}
}
}
