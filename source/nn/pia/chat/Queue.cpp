#include "vendor/VCTAudio.hpp"
#include "vendor/VCTQueue.hpp"

#include <coreinit/fastmutex.h>

namespace
{
const unsigned int AUDIO_QUEUE_MAX = 15;

unsigned int audioQueueCounts[AUDIO_QUEUE_MAX] = {0};
_udp_packet* audioQueueHeads[AUDIO_QUEUE_MAX] = {0};
_udp_packet* audioQueueTails[AUDIO_QUEUE_MAX] = {0};

void flushQueue(unsigned int streamIndex)
{
    if (audioQueueHeads[streamIndex] != 0)
    {
        _udp_packet** ppHead = &audioQueueHeads[streamIndex];
        _udp_packet* packet = *ppHead;
        do
        {
            _udp_packet* freed = packet;
            packet = packet->m_pNext;
            *ppHead = packet;
            if (packet != 0)
            {
                packet->m_pPrevious = 0;
            }
            vct_free_packet_buffer(freed);
            packet = *ppHead;
        } while (packet != 0);
    }
    audioQueueTails[streamIndex] = 0;
    audioQueueCounts[streamIndex] = 0;
}
}

void vct_init_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&g_vctAudioQueueMutex);
    audioQueueHeads[uiStreamIndex] = audioQueueTails[uiStreamIndex] = 0;
    audioQueueCounts[uiStreamIndex] = 0;
    OSFastMutex_Unlock(&g_vctAudioQueueMutex);
}

int vct_insert_audio_queue(_udp_packet* pPacket, unsigned int uiStreamIndex)
{
    if (pPacket == 0 || uiStreamIndex >= AUDIO_QUEUE_MAX)
    {
        return -1;
    }

    OSFastMutex_Lock(&g_vctAudioQueueMutex);

    if (audioQueueHeads[uiStreamIndex] == 0)
    {
        audioQueueHeads[uiStreamIndex] = pPacket;
        pPacket->m_pPrevious = 0;
        audioQueueHeads[uiStreamIndex]->m_pNext = 0;
        const unsigned int uiCount = audioQueueCounts[uiStreamIndex] + 1;
        audioQueueTails[uiStreamIndex] = audioQueueHeads[uiStreamIndex];
        audioQueueCounts[uiStreamIndex] = uiCount;
        OSFastMutex_Unlock(&g_vctAudioQueueMutex);
        return static_cast<int>(uiCount);
    }

    _udp_packet* pCursor = audioQueueTails[uiStreamIndex];
    if (audioQueueHeads[uiStreamIndex]->m_pPrevious != 0 || pCursor->m_pNext != 0)
    {
        vct_trace_packet_buffer(static_cast<unsigned long long>(0x40) << 32);
        pCursor = audioQueueTails[uiStreamIndex];
    }

    while (pCursor != 0)
    {
        if (pCursor->m_Sequence < pPacket->m_Sequence)
        {
            pPacket->m_pPrevious = pCursor;
            pPacket->m_pNext = pCursor->m_pNext;
            pCursor->m_pNext = pPacket;
            if (pPacket->m_pNext != 0)
            {
                pPacket->m_pNext->m_pPrevious = pPacket;
            }
            if (pCursor == audioQueueTails[uiStreamIndex])
            {
                audioQueueTails[uiStreamIndex] = pPacket;
            }
            const unsigned int uiCount = audioQueueCounts[uiStreamIndex] + 1;
            audioQueueCounts[uiStreamIndex] = uiCount;
            OSFastMutex_Unlock(&g_vctAudioQueueMutex);
            return static_cast<int>(uiCount);
        }
        pCursor = pCursor->m_pPrevious;
    }

    pPacket->m_pPrevious = 0;
    pPacket->m_pNext = audioQueueHeads[uiStreamIndex];
    audioQueueHeads[uiStreamIndex]->m_pPrevious = pPacket;
    audioQueueHeads[uiStreamIndex] = pPacket;
    const unsigned int uiCount = audioQueueCounts[uiStreamIndex] + 1;
    audioQueueCounts[uiStreamIndex] = uiCount;
    OSFastMutex_Unlock(&g_vctAudioQueueMutex);
    return static_cast<int>(uiCount);
}

_udp_packet* vct_top_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&g_vctAudioQueueMutex);
    _udp_packet* pPacket = audioQueueHeads[uiStreamIndex];
    OSFastMutex_Unlock(&g_vctAudioQueueMutex);
    return pPacket;
}

int vct_pop_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&g_vctAudioQueueMutex);

    unsigned int uiCount = audioQueueCounts[uiStreamIndex];
    _udp_packet* pHead = audioQueueHeads[uiStreamIndex];
    if (pHead != 0)
    {
        uiCount = uiCount - 1;
        _udp_packet* pNext = pHead->m_pNext;
        audioQueueCounts[uiStreamIndex] = uiCount;
        audioQueueHeads[uiStreamIndex] = pNext;
        if (pNext != 0)
        {
            pNext->m_pPrevious = 0;
            uiCount = audioQueueCounts[uiStreamIndex];
            OSFastMutex_Unlock(&g_vctAudioQueueMutex);
        }
        else
        {
            audioQueueTails[uiStreamIndex] = 0;
            OSFastMutex_Unlock(&g_vctAudioQueueMutex);
        }
    }
    else
    {
        OSFastMutex_Unlock(&g_vctAudioQueueMutex);
    }

    return static_cast<int>(uiCount);
}

int vct_count_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&g_vctAudioQueueMutex);
    const unsigned int uiCount = audioQueueCounts[uiStreamIndex];
    OSFastMutex_Unlock(&g_vctAudioQueueMutex);
    return static_cast<int>(uiCount);
}

void vct_flush_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex* pMutex = &g_vctAudioQueueMutex;
    OSFastMutex_Lock(pMutex);
    flushQueue(uiStreamIndex);
    OSFastMutex_Unlock(pMutex);
}

void vct_flush_all_audio_queue()
{
    OSFastMutex_Lock(&g_vctAudioQueueMutex);
    for (unsigned int uiStream = 0; uiStream < AUDIO_QUEUE_MAX; ++uiStream)
    {
        flushQueue(uiStream);
    }
    OSFastMutex_Unlock(&g_vctAudioQueueMutex);
}
