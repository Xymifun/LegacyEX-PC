#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_ISESSIONINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_ISESSIONINFO_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class ISessionInfo
{
public:
    virtual ~ISessionInfo() {}
    virtual unsigned int GetGameMode() const = 0;
    virtual unsigned int GetSessionId() const = 0;
    virtual unsigned short GetCurrentParticipantNum() const = 0;
    virtual unsigned short GetMinParticipantNum() const = 0;
    virtual unsigned short GetMaxParticipantNum() const = 0;
    virtual bool IsOpened() const = 0;
    virtual Result GetApplicationData(void* buffer, unsigned int bufferSize) const = 0;
    virtual unsigned int GetApplicationDataSize() const = 0;
    virtual void Clear() = 0;
    virtual void Trace(unsigned long long value) const = 0;
};

}
}
}

#endif
