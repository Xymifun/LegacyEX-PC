#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIMEOUT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIMEOUT_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class Core;

class Timeout : public RootObject
{
public:
    Timeout();
    virtual ~Timeout();

    void SetRTO(unsigned int uiRTO);
    void Start();
    void SetExpirationTime(Time expirationTime);
    void SetRelativeExpirationTime(int iRelativeExpirationTime);
    Time GetAwaitedTime();
    bool IsAwaited() const;
    bool IsExpired() const;

    Time GetExpirationTime() const { return m_expirationTime; }

    unsigned int GetRTO() { return m_uiRTO; }

private:
    unsigned int m_uiRTO;
    Time m_awaitedTime;
    Time m_expirationTime;
    long long m_relativeExpirationTime;
    Core* m_pCore;
};

}
}

#endif
