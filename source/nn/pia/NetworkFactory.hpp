#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NETWORKFACTORY_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn
{
namespace pia
{

namespace common
{
class IPacketInput;
class IPacketOutput;
class MonitoringDataSender;
class StepSequenceJob;
class String;
}

namespace session
{
class ICommunityInfoList;
class IMatchmakeSession;
class ISessionInfoList;
class MeshLayerController;
}

namespace transport
{

class ConnectStationJob;
class DisconnectStationJob;
class MissingStationHandler;
class PacketHandler;

template <typename T> inline T* CreateObject()
{
    void* storage = pead::allocateFromSeadHeap(sizeof(T), common::HeapManager::GetHeap());
    if (storage == 0)
        return 0;
    return ::new (storage) T();
}

template <typename T> inline void DestroyObject(T* object)
{
    if (object != 0)
    {
        object->~T();
        pead::freeToSeadHeap(object);
    }
}

class NetworkFactory
{
public:
    NetworkFactory();
    virtual ~NetworkFactory();
    virtual unsigned int GetMaxStationNum() const = 0;
    virtual unsigned int GetHeaderSize() const = 0;
    virtual unsigned int GetMtuSizeMax() const = 0;
    virtual unsigned int GetMtuSizeMin() const = 0;
    virtual int GetDefaultRttInitialPulseInterval() const = 0;
    virtual bool IsClientRelayConnectionPermitted() const;
    virtual bool IsStarConnection() const;
    virtual unsigned int IsSupportedProtocol(unsigned char protocolType) const;
    virtual bool IsBitRateCheckPermitted() const;
    virtual NetworkFactory* CreateSelf() = 0;
    virtual void DeleteSelf(NetworkFactory* factory) = 0;
    virtual Result CreateProtocols();
    virtual PacketHandler* CreatePacketHandler();
    virtual MissingStationHandler* CreateMissingStationHandler();
    virtual void DeletePacketHandler(PacketHandler* handler);
    virtual void DeleteMissingStationHandler(MissingStationHandler* handler);
    virtual common::IPacketInput* CreateInputStream() = 0;
    virtual common::IPacketOutput* CreateOutputStream() = 0;
    virtual void DeleteInputStream(common::IPacketInput* stream) = 0;
    virtual void DeleteOutputStream(common::IPacketOutput* stream) = 0;
    virtual ConnectStationJob* CreateConnectStationJob();
    virtual DisconnectStationJob* CreateDisconnectStationJob();
    virtual void DeleteConnectStationJob(ConnectStationJob* job);
    virtual void DeleteDisconnectStationJob(DisconnectStationJob* job);
    virtual common::MonitoringDataSender* CreateMonitoringDataSender();
    virtual void DeleteMonitoringDataSender(common::MonitoringDataSender* sender);
};

}
}
}

#endif
