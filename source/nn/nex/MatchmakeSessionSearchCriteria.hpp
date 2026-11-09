#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKESESSIONSEARCHCRITERIA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKESESSIONSEARCHCRITERIA_HPP

#include "nn/nex/MatchmakeSelectionMethod.hpp"
#include "nn/nex/MatchmakeSystemType.hpp"
#include "nn/nex/_DDL_MatchmakeSessionSearchCriteria.hpp"

namespace nn
{
namespace nex
{

class MatchmakeSessionSearchCriteria : public _DDL_MatchmakeSessionSearchCriteria
{
public:
    MatchmakeSessionSearchCriteria();
    MatchmakeSessionSearchCriteria(unsigned int uiGameMode, MatchmakeSystemType type)
    {
        Reset();
        SetGameMode(uiGameMode);
        SetMatchmakeSystemType(type);
    }

    virtual ~MatchmakeSessionSearchCriteria() {}

    void Reset();
    void SetGameMode(unsigned int uiGameMode);
    void SetMatchmakeSystemType(MatchmakeSystemType type);
    void SetMinParticipants(unsigned short uiMinParticipants);
    void SetMinParticipantsWithRange(unsigned short uiMinimum, unsigned short uiMaximum);
    void SetMaxParticipants(unsigned short uiMaxParticipants);
    void SetMaxParticipantsWithRange(unsigned short uiMinimum, unsigned short uiMaximum);
    void SetAttribute(unsigned int uiIndex, unsigned int uiValue);
    bool SetAttribute(unsigned int uiIndex, const qVector<unsigned int>& rValues);
    void SetAttributeWithRange(unsigned int uiIndex, unsigned int uiMinimum, unsigned int uiMaximum);
    void SetExcludeLocked(bool bExcludeLocked);
    void SetVacantOnly(bool bVacantOnly);
    void SetVacantOnly(bool bVacantOnly, unsigned short uiVacantParticipants)
    {
        m_bVacantOnly = bVacantOnly;
        m_uiVacantParticipants = uiVacantParticipants;
    }
    void SetExcludeNonHostPid(bool bExcludeNonHostPid);
    void SetExcludeUserPasswordSet(bool bExcludeUserPasswordSet);
    void SetSelectionMethod(MatchmakeSelectionMethod method);
    void SetSelectionMethod(MatchmakeSelectionMethod method, const MatchmakeParam& rParam);
};

}
}

#endif
