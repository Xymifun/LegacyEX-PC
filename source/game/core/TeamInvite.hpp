#ifndef GAME_CORE_TEAMINVITE_HPP
#define GAME_CORE_TEAMINVITE_HPP

#include <string>

class TeamInvite
{
public:
    TeamInvite();
    ~TeamInvite() {}

    unsigned int m_uTeamID;
    std::wstring m_sToPlayerName;
    std::wstring m_sFromPlayerName;
    std::wstring m_sTeamName;
    unsigned int m_uTimeCreated;
};

#endif
