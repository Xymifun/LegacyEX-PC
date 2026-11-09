#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CHAINPOLICYHISTORYPACKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CHAINPOLICYHISTORYPACKET_HPP

namespace nn
{
namespace nex
{

template <class T>
class ChainPolicyHistoryPacket
{
public:
    static T& Next(T element) { return element->m_historyLink.m_pNext; }

    static T& Previous(T element) { return element->m_historyLink.m_pPrevious; }
};

}
}

#endif
