#include "nn/nex/PlayingSession.hpp"

#include "nn/nex/MatchmakeSession.hpp"

namespace nn
{
namespace nex
{

MatchmakeSession* PlayingSession::GetMatchmakeSession() const
{
    if (m_Gathering.GetObject() != 0 && m_Gathering.GetObject()->IsAKindOf(String(L"MatchmakeSession")))
        return static_cast<MatchmakeSession*>(m_Gathering.GetObject());
    return 0;
}

}
}
