#include "game/world/generation/PlayerTeam.hpp"

#include "game/core/Scoreboard.hpp"

PlayerTeam::PlayerTeam(Scoreboard* pScoreboard, const std::wstring& name, unsigned int teamIndex) : mPlayers(11)
{
    mScoreboard = pScoreboard;
    mName = name;
    mDeathMessageVisibility = ALWAYS;
    mNameTagVisibility = ALWAYS;
    mSeeFriendlyInvisibles = true;
    mTeamIndex = teamIndex;
    mAllowFriendlyFire = true;
}

Scoreboard* PlayerTeam::GetScoreboard()
{
    return mScoreboard;
}

std::wstring PlayerTeam::getName()
{
    return mName;
}

std::wstring PlayerTeam::getFormattedName(const std::wstring& name)
{
    return name;
}

bool PlayerTeam::canSeeFriendlyInvisibles()
{
    return mSeeFriendlyInvisibles;
}

bool PlayerTeam::isAllowFriendlyFire()
{
    return mAllowFriendlyFire;
}

Team::Visibility PlayerTeam::getNameTagVisibility()
{
    return mNameTagVisibility;
}

unsigned int PlayerTeam::getTeamIndex()
{
    return mTeamIndex;
}

Team::Visibility PlayerTeam::getDeathMessageVisibility()
{
    return mDeathMessageVisibility;
}

boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >* PlayerTeam::getPlayers()
{
    return &mPlayers;
}

unsigned int PlayerTeam::getPlayerCount()
{
    return mPlayers.size();
}

void PlayerTeam::setAllowFriendlyFire(bool allowFriendlyFire)
{
    mAllowFriendlyFire = allowFriendlyFire;
    mScoreboard->onTeamChanged(this);
}

void PlayerTeam::setSeeFriendlyInvisibles(bool seeFriendlyInvisibles)
{
    mSeeFriendlyInvisibles = seeFriendlyInvisibles;
    mScoreboard->onTeamChanged(this);
}

void PlayerTeam::setNameTagVisibility(Visibility visibility)
{
    mNameTagVisibility = visibility;
    mScoreboard->onTeamChanged(this);
}

void PlayerTeam::setDeathMessageVisibility(Visibility visibility)
{
    mDeathMessageVisibility = visibility;
    mScoreboard->onTeamChanged(this);
}

int PlayerTeam::packOptions()
{
    int options = 0;

    if (isAllowFriendlyFire())
    {
        options |= 1;
    }

    if (canSeeFriendlyInvisibles())
    {
        options |= 2;
    }

    return options | (mTeamIndex << 28);
}

void PlayerTeam::unpackOptions(int options)
{
    setAllowFriendlyFire((options & 1) > 0);
    setSeeFriendlyInvisibles((options & 2) > 0);
    mTeamIndex = (options >> 28) & 0xFF;
}

void PlayerTeam::syncChanges()
{
    mScoreboard->syncTeamChanges(this);
}

std::wstring PlayerTeam::formatNameForTeam(Team* pTeam, const std::wstring& name)
{
    if (pTeam == NULL)
    {
        return name;
    }

    return pTeam->getFormattedName(name);
}

std::wstring PlayerTeam::formatNameForTeam(PlayerTeam* pTeam)
{
    return formatNameForTeam(pTeam, pTeam->getName());
}
