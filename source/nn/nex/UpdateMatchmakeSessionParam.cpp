#include <string.h>

#include "nn/nex/UpdateMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

void UpdateMatchmakeSessionParam::Reset()
{
    m_uiValue2 = 0;
    m_uiValue1 = 0;
    unsigned int aAttributes[ATTRIBUTE_COUNT];
    memset(aAttributes, 0, sizeof(aAttributes));
    m_Attributes.assign(aAttributes, aAttributes + ATTRIBUTE_COUNT);
    m_bOpenParticipation = true;
    m_ApplicationBuffer = qVector<unsigned char>();
    m_uiProgressScore = MAX_PROGRESS_SCORE;
    m_MatchmakeParam = MatchmakeParam();
    m_strValue1 = String();
    m_StartedTime = DateTime::s_never;
    m_uiValue3 = 0;
    m_strValue2 = String();
    m_uiValue5 = 0;
    m_uiValue4 = 0;
}

}
}
