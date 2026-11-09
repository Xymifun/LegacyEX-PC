#ifndef SOURCE_VENDOR_VCT_QUEUE_HPP
#define SOURCE_VENDOR_VCT_QUEUE_HPP

const unsigned int VCT_PACKET_FLAG_SPARE = 0x40;
const unsigned int VCT_AUDIO_FRAME_SAMPLE_MAX = 1536;
const unsigned int VCT_AUDIO_PAYLOAD_MAX = 3092;

struct VCTAudioPacket
{
    unsigned int m_Signature;
    unsigned char m_Type;
    unsigned char m_Codec;
    unsigned short m_Sequence;
    unsigned int m_Timestamp;
    unsigned char m_Data[VCT_AUDIO_PAYLOAD_MAX];
};

struct VCTAudioFrame
{
    short m_Samples[VCT_AUDIO_FRAME_SAMPLE_MAX];
    VCTAudioPacket m_SparePacket;
};

struct _udp_packet
{
    _udp_packet* m_pNextFree;
    _udp_packet* m_pPrevious;
    _udp_packet* m_pNext;
    VCTAudioFrame* m_pFrame;
    long long m_ReceiveTick;
    long long m_PlayTick;
    long long m_SendTick;
    unsigned int m_Unknown28;
    unsigned int m_Sequence;
    unsigned char m_Datagram[3472];
    unsigned int m_Codec;
    unsigned short m_Cid;
    unsigned char m_StreamIndex;
    unsigned char m_Flags;
    unsigned int m_IsSpare;
};

void vct_init_audio_queue(unsigned int uiStreamIndex);
int vct_insert_audio_queue(_udp_packet* pPacket, unsigned int uiStreamIndex);
_udp_packet* vct_top_audio_queue(unsigned int uiStreamIndex);
int vct_pop_audio_queue(unsigned int uiStreamIndex);
int vct_count_audio_queue(unsigned int uiStreamIndex);
void vct_flush_audio_queue(unsigned int uiStreamIndex);
void vct_flush_all_audio_queue();

int vct_init_packet_buffer(void* pBuffer, unsigned int uiSize);
void vct_init_packet_buffer_ex();
void vct_cleanup_packet_buffer();
_udp_packet* vct_alloc_packet_buffer();
void vct_free_packet_buffer(_udp_packet* pPacket);
void vct_trace_packet_buffer(unsigned long long ullTag);

#endif
