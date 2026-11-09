#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_GLOBALVARIABLES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_GLOBALVARIABLES_HPP

#include <stdint.h>
#include <functional>
#include <map>
#include <utility>

#include "nn/nex/CacheManager.hpp"
#include "nn/nex/EventLog.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/PacketBufferManager.hpp"
#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/ThreadVariable.hpp"
#include "nn/nex/TraceLog.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class ObjectThreadRoot;
class ThreadVariableList;
class GlobalVariables;

extern GlobalVariables* g_globalVariables;
extern ThreadVariableList* g_threadVariableList;
extern bool g_threadVariableListReady;

class GlobalVariables : public RefCountedObject
{
public:
    typedef std::map<String, qVector<unsigned char>, std::less<String>,
                     MemAllocator<std::pair<const String, qVector<unsigned char> > > >
        BinaryDataMap;
    typedef std::map<unsigned int, String, std::less<unsigned int>,
                     MemAllocator<std::pair<const unsigned int, String> > >
        StringMap;
    typedef std::map<unsigned int, unsigned short, std::less<unsigned int>,
                     MemAllocator<std::pair<const unsigned int, unsigned short> > >
        VersionMap;

    GlobalVariables();
    virtual ~GlobalVariables();

    static GlobalVariables* GetInstancePtr() { return g_globalVariables; }

    static void AcquireInstance();
    static void ReleaseInstance();
    static int32_t GetCount();
    static void ClearCache();
    static void* GetThreadVariableListPtr();

    ThreadVariable<unsigned int>& GetErrorCode() { return m_errorCode; }

    ThreadVariable<unsigned int>& GetErrorDetail() { return m_errorDetail; }

    ThreadVariable<ObjectThreadRoot*>& GetCurrentThread() { return m_currentThread; }

    EventLog& GetEventLog() { return m_eventLog; }

    TraceLog& GetTraceLog() { return m_traceLog; }

    CacheManager& GetCacheManager() { return m_cacheManager; }

    String& GetBroadcastAddress() { return m_broadcastAddress; }

private:
    friend class PacketBufferManager;
    friend class JobCafeLogin;
    friend class SystemError;

    String m_broadcastAddress;
    ThreadVariable<unsigned int> m_errorCode;
    ThreadVariable<unsigned int> m_errorDetail;
    ThreadVariable<ObjectThreadRoot*> m_currentThread;
    BinaryDataMap m_binaryDataMap;
    StringMap m_nexTokenMap;
    StringMap m_nexPasswordMap;
    StringMap m_nexHostMap;
    VersionMap m_nexPortMap;
    EventLog m_eventLog;
    unsigned int m_field1a4;
    TraceLog m_traceLog;
    CacheManager m_cacheManager;
    qVector<PacketBufferManager::UnitHeapParam> m_packetBufferParams;
    bool m_packetBufferPoolEnabled;
};

}
}

#endif
