#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_RECEIVED_DATA_CONTAINER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_RECEIVED_DATA_CONTAINER_HPP
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/transport/UnreliableProtocol.hpp"
namespace nn
{
namespace pia
{
namespace transport
{
class ReceivedDataContainer : public common::PooledList<UnreliableProtocol::Payload>
{
};
}
}
}
#endif
