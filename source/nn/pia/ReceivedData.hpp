#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_RECEIVED_DATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_RECEIVED_DATA_HPP
#include "nn/pia/ListBase.hpp"
namespace nn
{
namespace pia
{
namespace transport
{
struct ReceivedData
{
    common::ListNode node;
    unsigned char payload[0x590];
};
}
}
}
#endif
