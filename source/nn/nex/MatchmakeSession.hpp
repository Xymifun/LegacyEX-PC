#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKESESSION_HPP

#include "nn/nex/MatchmakeSystemType.hpp"
#include "nn/nex/_DDL_MatchmakeSession.hpp"

namespace nn
{
namespace nex
{

class MatchmakeSession : public _DDL_MatchmakeSession
{
public:
    MatchmakeSession();
    virtual ~MatchmakeSession() {}

    void SetMatchmakeSystemType(MatchmakeSystemType type, unsigned int uiPolicyArgument);
    void Reset();
    void SetGameMode(unsigned int uiGameMode);
    void SetAttribute(unsigned int uiIndex, unsigned int uiValue);
    void SetApplicationBuffer(const qVector<unsigned char>& rApplicationBuffer);
    void SetOpenParticipation(bool bOpenParticipation);
    unsigned int GetAttribute(unsigned int uiIndex) const;
    bool SetProgressScore(unsigned char uiProgressScore);
};

}
}

#endif
