#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATCHECKCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATCHECKCLIENT_HPP

#include <stdint.h>

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class CallContext;

class NATCheckClient : public RootObject
{
public:
    explicit NATCheckClient(const InetAddress& rAddress);
    ~NATCheckClient();

    bool DetectFullNATProperties(CallContext* pCallContext);
    bool DetectSimplifiedNATProperties(CallContext* pCallContext);
    bool DetectNATCurrentPortMapping(CallContext* pCallContext, bool bRelayProbe);

private:
    uint32_t m_unknown0;
    InetAddress m_address;
};

}
}

#endif
