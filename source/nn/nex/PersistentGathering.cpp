#include "nn/nex/PersistentGathering.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int GATHERING_ATTRIBUTE_COUNT = 6;
const unsigned int GATHERING_FLAG_PERSISTENT = 1;
const unsigned int PARTICIPATION_POLICY_97 = 97;

}

PersistentGathering::PersistentGathering()
{
    Reset();
}

Gathering* _DDL_PersistentGathering::Clone() const
{
    return new PersistentGathering;
}

void PersistentGathering::Reset()
{
    m_uiPersistentGatheringType = 0;
    m_strPassword = L"";
    m_Attributes.assign(GATHERING_ATTRIBUTE_COUNT, 0);
    m_ApplicationBuffer.clear();
    m_uiMatchmakeSessionCount = 0;
    m_uiParticipationCount = 0;
    m_ParticipationStartDate = DateTime::s_never;
    m_ParticipationEndDate = DateTime::s_never;
    Gathering::Reset();
    m_uiValue4 |= GATHERING_FLAG_PERSISTENT;
    SetParticipationPolicy(PARTICIPATION_POLICY_97, 0);
}

void PersistentGathering::SetPersistentGatheringType(PersistentGatheringType::PersistentGatheringType type)
{
    m_uiPersistentGatheringType = type;
}

unsigned int PersistentGathering::GetPersistentGatheringType() const
{
    return m_uiPersistentGatheringType;
}

void PersistentGathering::SetAttribute(unsigned int uiIndex, unsigned int uiValue)
{
    if (uiIndex < m_Attributes.size())
        m_Attributes[uiIndex] = uiValue;
}

unsigned int PersistentGathering::GetAttribute(unsigned int uiIndex) const
{
    if (uiIndex >= m_Attributes.size())
        return 0;
    return m_Attributes[uiIndex];
}

}
}
