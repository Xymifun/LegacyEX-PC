#include "nn/nex/Stream.hpp"

#include "nn/nex/StreamSettings.hpp"

namespace nn
{
namespace nex
{

Stream::Stream(Type type) : m_Type(type)
{
    g_streamSettings[type].Get().SetStreamType(m_Type);
}

Stream::~Stream()
{
}

bool Stream::IsDuplicateReorderingPacket(Packet*)
{
    return false;
}

}
}
