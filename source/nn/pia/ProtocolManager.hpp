#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_PROTOCOLMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_PROTOCOLMANAGER_HPP

#include "nn/pia/Assert.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/Reconstruction.hpp"

#include <new>

namespace nn { namespace pia { namespace transport {

class ProtocolManager
{
public:
    ProtocolManager();
    virtual ~ProtocolManager();
    virtual void Trace(unsigned long long value) const;
    void Finalize();
    Result Initialize(NetworkFactory* networkFactory);
    Result Startup(PacketHandler* packetHandler);
    Result StartupProtocols();
    Result Dispatch();
    Result UpdateProtocolEvent(const ProtocolEvent& event);
    void Cleanup();
    void CleanupProtocols();
    void CleanupProtocolsWithStationAddress();
    Result StartupProtocolsWithStationAddress(const common::StationAddress& address);
    void* AllocProtocol(unsigned int size);
    unsigned short CreateProtocolImpl(Protocol* protocol, ProtocolId id);
    Protocol* SearchProtocol(ProtocolId id, unsigned char type);
    void DestroyProtocol(unsigned short id);

    template <typename T>
    unsigned short CreateProtocol(unsigned char port)
    {
        if (m_started)
            return 0;
        void* memptr = AllocProtocol(sizeof(T));
        Protocol* pProtocol = new (memptr) T();
        PIA_ASSERT(pProtocol == memptr);
        ProtocolId id(T::cProtocolType, port);
        unsigned short handle = CreateProtocolImpl(pProtocol, id);
        return handle;
    }

    template <typename T>
    T* GetProtocol(unsigned int handle)
    {
        ProtocolId id(static_cast<unsigned short>(handle));
        return static_cast<T*>(SearchProtocol(id, T::cProtocolType));
    }

private:
    NetworkFactory* m_networkFactory;
    common::OffsetList m_protocols;
public:
    unsigned char m_started;
};

extern ProtocolManager* g_piaProtocolManager;

}}}

#endif
