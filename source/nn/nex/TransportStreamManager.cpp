#include "nn/nex/TransportStreamManager.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/Packet.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/VirtualNATDevice.hpp"

namespace nn
{
namespace nex
{

TransportStreamManager::TransportStreamManager() : m_streams(16)
{
}

TransportStreamManager::~TransportStreamManager()
{
    while (!m_streams.empty())
    {
        VirtualPort port = m_streams.front().first;
        UnregisterStream(port.GetType(), port.GetStreamID());
    }
}

int TransportStreamManager::GetNbStream()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return static_cast<int>(m_streams.size());
}

Stream* TransportStreamManager::GetStreamImpl(VirtualPort port)
{
    qSortedVector<VirtualPort, Stream*>::iterator position = m_streams.find(port);
    if (position == m_streams.end())
        return 0;

    return position->second;
}

Stream* TransportStreamManager::GetStream(VirtualPort port)
{
    if (g_transportThreadEnabled)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        return GetStreamImpl(port);
    }

    return GetStreamImpl(port);
}

bool TransportStreamManager::UnregisterStream(Stream::Type type, unsigned char uiStreamID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    VirtualPort port(type, uiStreamID);
    if (GetStream(port) != 0)
    {
        m_streams.erase(port);
        return true;
    }

    return false;
}

bool TransportStreamManager::RegisterStream(Stream* pStream, unsigned char uiStreamID, unsigned char* pAssignedID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (uiStreamID == 0)
    {
        int id;
        for (id = 15; id != 0; --id)
        {
            if (GetStream(VirtualPort(pStream->GetType(), id)) == 0)
                break;
        }
        if (id == 0)
            return false;
        uiStreamID = id;
    }
    VirtualPort port(pStream->GetType(), uiStreamID);

    if (GetStream(port) == 0)
    {
        m_streams.insert(std::pair<VirtualPort, Stream*>(port, pStream));
        if (pAssignedID != 0)
            *pAssignedID = uiStreamID;
        return true;
    }

    return false;
}

bool TransportStreamManager::IsStreamTypeRegistered(Stream::Type type)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (qSortedVector<VirtualPort, Stream*>::iterator stream = m_streams.begin(); stream != m_streams.end(); ++stream)
    {
        if (stream->second->GetType() == type)
            return true;
    }

    return false;
}

int TransportStreamManager::GetNBStreamByStreamType(Stream::Type type)
{
    int iCount = 0;
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (qSortedVector<VirtualPort, Stream*>::iterator stream = m_streams.begin(); stream != m_streams.end(); ++stream)
    {
        if (stream->second->GetType() == type)
            ++iCount;
    }

    return iCount;
}

bool TransportStreamManager::IsDuplicateReorderingPacket(Packet* pPacket)
{
    VirtualPort port(pPacket->m_uiDestination);
    Stream* pStream = GetStream(port);
    if (pStream != 0)
        return pStream->IsDuplicateReorderingPacket(pPacket);

    return false;
}

void TransportStreamManager::DemuxIncomingPacket(Packet* pPacket)
{
    VirtualPort port(pPacket->m_uiDestination);
    unsigned short uiPort = pPacket->m_uiPort;
    Stream* pStream = GetStream(port);
    if (pStream != 0)
    {
        VirtualNATDevice* pDevice = m_pTransport->m_pVirtualNATDevice;
        if (pDevice != 0)
        {
            StationURL url;
            url.SetInetAddress(&pPacket->m_address);
            bool bHandled = pDevice->ProcessIncomingAddress(&url, uiPort);
            pPacket->m_address = *url.GetInetAddress();
            if (bHandled)
            {
                pPacket->ReleaseRef();
                return;
            }
        }

        if (pStream->ReceiveIncomingPacket(uiPort, VirtualPort(pPacket->m_uiSource).GetStreamID(), pPacket))
            return;

        m_pTransport->RejectPacket(pPacket->m_pPayload, &pPacket->m_address);
        pPacket->ReleaseRef();
        return;
    }

    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (qSortedVector<VirtualPort, Stream*>::iterator stream = m_streams.begin(); stream != m_streams.end(); ++stream)
    {
    }
    pPacket->ReleaseRef();
}

void TransportStreamManager::DoWork()
{
    if (g_transportThreadEnabled)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        for (qSortedVector<VirtualPort, Stream*>::iterator stream = m_streams.begin(); stream != m_streams.end();
             ++stream)
            stream->second->DoWork();
        return;
    }

    for (qSortedVector<VirtualPort, Stream*>::iterator stream = m_streams.begin(); stream != m_streams.end(); ++stream)
        stream->second->DoWork();
}

}
}
