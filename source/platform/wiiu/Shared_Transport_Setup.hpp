#ifndef SOURCE_PLATFORM_WIIU_SHARED_TRANSPORT_SETUP_HPP
#define SOURCE_PLATFORM_WIIU_SHARED_TRANSPORT_SETUP_HPP

#include "nn/pia.h"
#include "nn/pia/Transport.hpp"

nn::pia::Result Transport_InitializePiaTransport(const nn::pia::transport::Transport::Setting& setting,
                                                 unsigned short unreliablePort, unsigned int* unreliableProtocolHandle,
                                                 unsigned short reliablePort, unsigned int* reliableProtocolHandle,
                                                 unsigned int reliableSendBufferNum,
                                                 unsigned int reliableReceiveBufferNum,
                                                 nn::pia::Result (*setupCallback)(),
                                                 unsigned int unreliableReceiveBufferNum);
void Transport_FinalizePiaTransport(unsigned int unreliableProtocolHandle, unsigned int reliableProtocolHandle,
                                    void (*finalizeCallback)());

#endif
