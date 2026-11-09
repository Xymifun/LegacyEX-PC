#include "nn/nex/NATCheckMessage.hpp"

#include <arpa/inet.h>

#include "nn/nex/InetAddress.hpp"

namespace nn
{
namespace nex
{

NATCheckMessage::NATCheckMessage() : m_uiType(0), m_uiPortWord(0), m_uiAddress(0), m_uiTargetAddress(0)
{
}

NATCheckMessage::~NATCheckMessage()
{
}

void NATCheckMessage::ToNetworkByteOrder()
{
    m_uiType = htonl(m_uiType);
    m_uiPortWord = htonl(m_uiPortWord);
    m_uiAddress = htonl(m_uiAddress);
    m_uiTargetAddress = htonl(m_uiTargetAddress);
}

void NATCheckMessage::FromNetworkByteOrder()
{
    m_uiType = ntohl(m_uiType);
    m_uiPortWord = ntohl(m_uiPortWord);
    m_uiAddress = ntohl(m_uiAddress);
    m_uiTargetAddress = ntohl(m_uiTargetAddress);
}

void NATCheckMessage::Trace(unsigned long long)
{
    InetAddress address(m_uiAddress, GetPortNumber());
}

}
}
