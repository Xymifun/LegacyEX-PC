#include "nn/pia/chat/Decoder.hpp"

#include "vendor/VCTAudio.hpp"
#include "vendor/VCTQueue.hpp"

#include <coreinit/fastmutex.h>

namespace
{
const unsigned int DECODER_QUEUE_MAX = 15;

_udp_packet* s_decoderHead = NULL;
_udp_packet* s_decoderTail = NULL;
unsigned int s_decoderCounts[DECODER_QUEUE_MAX] = {0};
}

namespace nn
{
namespace pia
{
namespace chat
{

void Decoder::Init()
{
    OSFastMutex_Lock(&g_vctDecoderMutex);
    s_decoderHead = 0;
    s_decoderTail = 0;
    for (unsigned int uiStream = 0; uiStream < DECODER_QUEUE_MAX; ++uiStream)
    {
        s_decoderCounts[uiStream] = 0;
    }
    OSFastMutex_Unlock(&g_vctDecoderMutex);
}

_udp_packet* Decoder::GetPacket(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&g_vctDecoderMutex);
    _udp_packet* pPacket = s_decoderHead;
    while (pPacket != 0)
    {
        if (pPacket->m_StreamIndex == uiStreamIndex)
        {
            OSFastMutex_Unlock(&g_vctDecoderMutex);
            return pPacket;
        }
        pPacket = pPacket->m_pNext;
    }
    OSFastMutex_Unlock(&g_vctDecoderMutex);
    return 0;
}

int Decoder::RemovePacket(_udp_packet* pPacket)
{
    OSFastMutex_Lock(&g_vctDecoderMutex);

    _udp_packet* pNext = pPacket->m_pNext;
    if (pPacket->m_pPrevious != 0)
    {
        pPacket->m_pPrevious->m_pNext = pNext;
    }
    else
    {
        s_decoderHead = pNext;
        if (pNext != 0)
        {
            pNext->m_pPrevious = 0;
        }
    }

    pNext = pPacket->m_pNext;
    _udp_packet* pPrevious = pPacket->m_pPrevious;
    int iCount;
    if (pNext != 0)
    {
        pNext->m_pPrevious = pPrevious;
        iCount = s_decoderCounts[pPacket->m_StreamIndex] - 1;
        s_decoderCounts[pPacket->m_StreamIndex] = iCount;
        OSFastMutex_Unlock(&g_vctDecoderMutex);
    }
    else
    {
        s_decoderTail = pPrevious;
        if (pPrevious != 0)
        {
            pPrevious->m_pNext = 0;
        }
        iCount = s_decoderCounts[pPacket->m_StreamIndex] - 1;
        s_decoderCounts[pPacket->m_StreamIndex] = iCount;
        OSFastMutex_Unlock(&g_vctDecoderMutex);
    }
    return iCount;
}

void Decoder::Flush(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&g_vctDecoderMutex);

    _udp_packet* pPacket = GetPacket(uiStreamIndex);
    while (pPacket != 0)
    {
        RemovePacket(pPacket);
        vct_free_packet_buffer(pPacket);
        pPacket = GetPacket(uiStreamIndex);
    }
    s_decoderCounts[uiStreamIndex] = 0;

    OSFastMutex_Unlock(&g_vctDecoderMutex);
}

void Decoder::FlushAll()
{
    OSFastMutex_Lock(&g_vctDecoderMutex);

    _udp_packet* pPacket = s_decoderHead;
    while (pPacket != 0)
    {
        _udp_packet* pFree = pPacket;
        pPacket = pPacket->m_pNext;
        vct_free_packet_buffer(pFree);
    }

    s_decoderHead = 0;
    s_decoderTail = 0;
    for (unsigned int uiStream = 0; uiStream < DECODER_QUEUE_MAX; ++uiStream)
    {
        s_decoderCounts[uiStream] = 0;
    }

    OSFastMutex_Unlock(&g_vctDecoderMutex);
}

unsigned int Decoder::CountQueue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&g_vctDecoderMutex);
    const unsigned int uiCount = s_decoderCounts[uiStreamIndex];
    OSFastMutex_Unlock(&g_vctDecoderMutex);
    return uiCount;
}

int Decoder::Insert(_udp_packet* pPacket)
{
    OSFastMutex_Lock(&g_vctDecoderMutex);

    int iCount;
    if (s_decoderHead == 0)
    {
        s_decoderHead = pPacket;
        pPacket->m_pPrevious = 0;
        s_decoderHead->m_pNext = 0;
        s_decoderTail = s_decoderHead;
        iCount = s_decoderCounts[pPacket->m_StreamIndex] + 1;
        s_decoderCounts[pPacket->m_StreamIndex] = iCount;
    }
    else
    {
        pPacket->m_pNext = 0;
        pPacket->m_pPrevious = s_decoderTail;
        s_decoderTail->m_pNext = pPacket;
        s_decoderTail = pPacket;
        iCount = s_decoderCounts[pPacket->m_StreamIndex] + 1;
        s_decoderCounts[pPacket->m_StreamIndex] = iCount;
    }

    OSFastMutex_Unlock(&g_vctDecoderMutex);
    return iCount;
}

_udp_packet* Decoder::Top()
{
    OSFastMutex_Lock(&g_vctDecoderMutex);
    _udp_packet* pPacket = s_decoderHead;
    OSFastMutex_Unlock(&g_vctDecoderMutex);
    return pPacket;
}

void Decoder::Lock()
{
    OSFastMutex_Lock(&g_vctDecoderMutex);
}

void Decoder::Unlock()
{
    OSFastMutex_Unlock(&g_vctDecoderMutex);
}

}
}
}
