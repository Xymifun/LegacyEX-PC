#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_IOREQUESTCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_IOREQUESTCONTEXT_HPP

#include "nn/nex/EndPoint.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/UserContext.hpp"

namespace nn
{
namespace nex
{

class Buffer;

class IORequestContext : public RootObject
{
public:
    IORequestContext();
    virtual ~IORequestContext();

    Buffer* m_pResponse;
    Buffer* m_pRequest;
    EndPoint::ConnectionCallback m_pCallback;
    UserContext m_userContext;
};

}
}

#endif
