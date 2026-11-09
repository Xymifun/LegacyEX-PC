#include "nn/nex/Gathering.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint32_t DEFAULT_VALUE4 = 0x200;

}

Gathering::Gathering() : _DDL_Gathering()
{
    Reset();
}

Gathering* _DDL_Gathering::Clone() const
{
    return new Gathering;
}

Gathering::~Gathering()
{
}

void Gathering::Reset()
{
    m_uiValue3 = 0;
    m_uiValue4 = DEFAULT_VALUE4;
    m_uiPolicyArgument = 0;
    m_uiParticipationPolicy = 1;
    m_uiValue5 = 0;
    m_uiShortValue1 = 0;
    m_uiValue1 = 0;
    m_uiShortValue2 = 0;
    m_uiValue2 = 0;
}

void Gathering::SetParticipationPolicy(unsigned int uiPolicy, unsigned int uiArgument)
{
    m_uiParticipationPolicy = uiPolicy;
    m_uiPolicyArgument = uiArgument;
}

void Gathering::SetDescription(const String& rDescription)
{
    m_Description = rDescription;
}

unsigned int Gathering::GetPolicyArgument() const
{
    return m_uiPolicyArgument;
}

void Gathering::Trace(unsigned long long) const
{
}

}
}
