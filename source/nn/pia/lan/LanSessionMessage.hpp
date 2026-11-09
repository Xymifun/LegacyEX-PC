#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONMESSAGE_HPP

#include "nn/pia/lan/LanMessage.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanSessionMessage : public LanMessage
{
public:
    enum
    {
        cFragmentSizeMax = 800
    };

    LanSessionMessage();
    LanSessionMessage(unsigned int sessionId, unsigned short sequence, unsigned char index, unsigned char count,
                      const unsigned char* fragment, unsigned int fragmentSize);
    virtual ~LanSessionMessage() {}
    virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
    virtual Result Deserialize(const unsigned char* buffer);
    virtual unsigned int GetSerializedSize() const;

    unsigned int m_sessionId;
    unsigned short m_sequence;
    unsigned char m_index;
    unsigned char m_count;
    unsigned char m_fragment[cFragmentSizeMax];
    unsigned int m_fragmentSize;
};

}
}
}

#endif
