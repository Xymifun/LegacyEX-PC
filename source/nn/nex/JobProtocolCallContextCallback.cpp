#include "nn/nex/JobProtocolCallContextCallback.hpp"

namespace nn
{
namespace nex
{

ProtocolCallContext* JobProtocolCallContextCallback::GetProtocolCallContext()
{
    if (m_pJobCallContext == 0)
        m_pJobCallContext = CreateCallContext();

    return static_cast<ProtocolCallContext*>(m_pJobCallContext);
}

CallContext* JobProtocolCallContextCallback::CreateCallContext()
{
    return new ProtocolCallContext();
}

}
}
