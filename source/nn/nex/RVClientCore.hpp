#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RVCLIENTCORE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RVCLIENTCORE_HPP

#include <map>
#include <utility>

#include "nn/nex/InstanceTable.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/ThreadVariable.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class DateTime;
class GlobalNotificationEventManager;
class NotificationEventManager;
class ServerProtocol;

class RVClientCore : public RefCountedObject
{
public:
    RVClientCore();
    virtual ~RVClientCore();

    enum
    {
        ClassID = 12
    };

    static RVClientCore* GetInstance()
    {
        unsigned int uiContextId = g_usingInstantiationContext;
        if (g_usingInstantiationContext)
            uiContextId = g_instantiationContextVariable->GetRef();

        InstanceControl* pInstanceControl = g_instanceTable.FindInstance(ClassID, uiContextId);
        void* pInstance = pInstanceControl;
        if (pInstance != 0)
            pInstance = pInstanceControl->GetDelegatorInstance();

        return static_cast<RVClientCore*>(pInstance);
    }

    static void AcquireInstance();
    static void ReleaseInstance();

    unsigned int RegisterServerProtocol(unsigned short uiProtocolID, ServerProtocol* pProtocol);
    unsigned int UnregisterServerProtocol(unsigned short uiProtocolID);
    ServerProtocol* GetServerProtocol(unsigned short uiProtocolID);

    void RegisterGlobalNotificationEventManager(NotificationEventManager* pManager);
    void UnregisterGlobalNotificationEventManager(NotificationEventManager* pManager);

    unsigned int GetGameServerTime(DateTime* pTime) const;
    void SetGameServerTime(DateTime& rTime);

    ProtocolRegistry* GetProtocolRegistry() const { return m_pProtocolRegistry; }

private:
    typedef std::pair<const unsigned short, ServerProtocol*> ServerProtocolEntry;
    typedef std::map<unsigned short, ServerProtocol*, std::less<unsigned short>, MemAllocator<ServerProtocolEntry> >
        ServerProtocolMap;

    enum
    {
        NOTIFICATION_EVENT_PROTOCOL_ID = 14,
        NOTIFICATION_PROTOCOL_FLAG = 8
    };

    class GameServerClock : public RootObject
    {
    public:
        Time m_timeBase;
        Time m_epochTime;
    };

    ProtocolRegistry* m_pProtocolRegistry;
    PseudoSingleton m_instanceControl;
    GlobalNotificationEventManager* m_pGlobalNotificationEventManager;
    ServerProtocolMap m_serverProtocols;
    GameServerClock m_gameServerClock;
};

}
}

#endif
