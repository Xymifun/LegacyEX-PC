#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_DECODER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_DECODER_HPP

struct _udp_packet;

namespace nn
{
namespace pia
{
namespace chat
{

class Decoder
{
public:
    static void Init();
    static _udp_packet* GetPacket(unsigned int uiStreamIndex);
    static int RemovePacket(_udp_packet* pPacket);
    static void Flush(unsigned int uiStreamIndex);
    static void FlushAll();
    static unsigned int CountQueue(unsigned int uiStreamIndex);
    static int Insert(_udp_packet* pPacket);
    static _udp_packet* Top();
    static void Lock();
    static void Unlock();
};

}
}
}

#endif
