#ifndef GAME_CORE_TEAM_HPP
#define GAME_CORE_TEAM_HPP

#include <functional>
#include <string>

#include <boost/unordered_set.hpp>

#include "game/world/generation/PlayerUID.hpp"

class Team
{
public:
    enum Visibility
    {
        ALWAYS,
        NEVER,
        HIDE_FOR_OTHER_TEAMS,
        HIDE_FOR_OWN_TEAM
    };

    virtual ~Team() {}

    virtual bool isAlliedTo(Team* other);

    virtual std::wstring getName() = 0;
    virtual std::wstring getFormattedName(const std::wstring& name) = 0;
    virtual bool canSeeFriendlyInvisibles() = 0;
    virtual bool isAllowFriendlyFire() = 0;
    virtual Visibility getNameTagVisibility() = 0;
    virtual unsigned int getTeamIndex() = 0;
    virtual Visibility getDeathMessageVisibility() = 0;
    virtual boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >* getPlayers() = 0;
};

#endif
