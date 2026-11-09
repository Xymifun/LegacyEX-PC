#include "nn/pia/common/IPacketOutput.hpp"

namespace nn
{
namespace pia
{
namespace common
{

unsigned int IPacketOutput::GetMulticastDestinationNum() const
{
    return 1;
}

unsigned int IPacketOutput::IsBroadcastSupported() const
{
    return 0;
}

}
}
}
