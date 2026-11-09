#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERTHREADINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERTHREADINTERFACE_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class QueuingSocket;

class TransportBufferThreadInterface : public RootObject
{
public:
    virtual ~TransportBufferThreadInterface() {}

    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual bool InsertSocketPort(unsigned short uiPort, QueuingSocket* pSocket) = 0;
    virtual void EraseSocketPort(unsigned short uiPort) = 0;
    virtual void SetPriority(unsigned int uiPriority) = 0;
    virtual unsigned int GetPriority() const = 0;
};

}
}

#endif
