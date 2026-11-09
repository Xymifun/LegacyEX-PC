#ifndef GAME_WORLD_GENERATION_PLAYERTEAM_HPP
#define GAME_WORLD_GENERATION_PLAYERTEAM_HPP

#include <functional>
#include <string>

#include <boost/unordered_set.hpp>

#include "game/core/Team.hpp"
#include "game/world/generation/PlayerUID.hpp"

class Scoreboard;

class PlayerTeam : public Team
{
public:
    PlayerTeam(Scoreboard* pScoreboard, const std::wstring& name, unsigned int teamIndex);

    virtual std::wstring getName();
    virtual std::wstring getFormattedName(const std::wstring& name);
    virtual bool canSeeFriendlyInvisibles();
    virtual bool isAllowFriendlyFire();
    virtual Visibility getNameTagVisibility();
    virtual unsigned int getTeamIndex();
    virtual Visibility getDeathMessageVisibility();
    virtual boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >* getPlayers();

    Scoreboard* GetScoreboard();

    unsigned int getPlayerCount();

    void setAllowFriendlyFire(bool allowFriendlyFire);
    void setSeeFriendlyInvisibles(bool seeFriendlyInvisibles);
    void setNameTagVisibility(Visibility visibility);
    void setDeathMessageVisibility(Visibility visibility);

    int packOptions();
    void unpackOptions(int options);

    void syncChanges();

    static std::wstring formatNameForTeam(Team* pTeam, const std::wstring& name);
    static std::wstring formatNameForTeam(PlayerTeam* pTeam);

private:
    Scoreboard* mScoreboard;
    std::wstring mName;
    boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> > mPlayers;
    bool mAllowFriendlyFire;
    bool mSeeFriendlyInvisibles;
    Visibility mNameTagVisibility;
    Visibility mDeathMessageVisibility;
    unsigned int mTeamIndex;
};

#endif
