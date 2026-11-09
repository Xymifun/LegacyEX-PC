#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATCHECKMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATCHECKMESSAGE_HPP

namespace nn
{
namespace nex
{

class NATCheckMessage
{
public:
    NATCheckMessage();
    ~NATCheckMessage();

    void ToNetworkByteOrder();
    void FromNetworkByteOrder();
    void Trace(unsigned long long uiFilter);

    unsigned short GetPortNumber() const { return static_cast<unsigned short>(m_uiPortWord); }

    unsigned int m_uiType;
    unsigned int m_uiPortWord;
    unsigned int m_uiAddress;
    unsigned int m_uiTargetAddress;
};

}
}

#endif
