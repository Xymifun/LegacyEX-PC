#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTSTREAMMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTSTREAMMANAGER_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/VirtualPort.hpp"
#include "nn/nex/qSortedVector.hpp"

namespace nn
{
namespace nex
{

class Packet;
class RootTransport;

class TransportStreamManager : public RootObject
{
public:
    TransportStreamManager();
    virtual ~TransportStreamManager();

    bool RegisterStream(Stream* pStream, unsigned char uiStreamID, unsigned char* pAssignedID);
    bool UnregisterStream(Stream::Type type, unsigned char uiStreamID);
    Stream* GetStream(VirtualPort port);
    bool IsStreamTypeRegistered(Stream::Type type);
    int GetNBStreamByStreamType(Stream::Type type);
    int GetNbStream();
    bool IsDuplicateReorderingPacket(Packet* pPacket);
    void DemuxIncomingPacket(Packet* pPacket);
    void DoWork();

private:
    friend class RootTransport;

    Stream* GetStreamImpl(VirtualPort port);

    qSortedVector<VirtualPort, Stream*> m_streams;
    RootTransport* m_pTransport;
};

}
}

#endif
