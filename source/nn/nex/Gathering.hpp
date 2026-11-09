#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_GATHERING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_GATHERING_HPP

#include <stdint.h>

#include "nn/nex/_DDL_Gathering.hpp"

namespace nn
{
namespace nex
{

class Gathering : public _DDL_Gathering
{
public:
    Gathering();
    virtual ~Gathering();

    virtual void Trace(unsigned long long uiTraceMask) const;

    void Reset();
    void SetParticipationPolicy(unsigned int uiPolicy, unsigned int uiArgument);
    void SetDescription(const String& rDescription);
    unsigned int GetPolicyArgument() const;

    unsigned int GetParticipationPolicy() const { return m_uiParticipationPolicy; }
};

}
}

#endif
