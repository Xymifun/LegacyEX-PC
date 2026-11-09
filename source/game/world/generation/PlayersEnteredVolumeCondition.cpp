#include "game/world/generation/PlayersEnteredVolumeCondition.hpp"
#include "game/core/StringIds.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/GameStats.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/XmlValue.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"

PlayersEnteredVolumeCondition::PlayersEnteredVolumeCondition(const XmlObject& object) : IntegerCondition(object)
{
    m_volumeName = object[std::wstring(L"volumeName")].maybeString();
    m_useTargetAreas = object[std::wstring(L"useTargetAreas")].maybeBool();
    m_useRatio = object[std::wstring(L"useRatio")].maybeBool();
}

int PlayersEnteredVolumeCondition::GetValue(MasterGameMode* gameMode) const
{
    int entered = 0;
    std::vector<boost::shared_ptr<ServerPlayer> > players;
    if (CommonMasterGameMode::GetMiniGame()->GetId() == 3)
        players = gameMode->GetGlidePlayers();
    else
        players = gameMode->GetPlayers(true, false);

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players.begin(); it != players.end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        if (gameMode->mGameStats.GetEndZoneEntered(player->getSUID()))
        {
            ++entered;
        }
        else if (player->getHealth() > 0.0f)
        {
            if (m_useTargetAreas.isSet() && m_useTargetAreas.get())
            {
                std::vector<TargetAreaRuleDefinition*> areas;
                gApp.getLevelGenerationOptions()->getRequiredGameRules()->getTargetAreas(&areas);
                for (std::vector<TargetAreaRuleDefinition*>::iterator area = areas.begin(); area != areas.end(); ++area)
                {
                    TargetAreaRuleDefinition* targetArea = *area;
                    if (targetArea->mPrimary
                        && targetArea->mLaps
                               <= static_cast<int>(gameMode->mGameStats.GetLapsCompleted(player->getSUID())))
                    {
                        const bool atFinalCheckpoint =
                            (*area)->getArea()->intersects(gameMode->GetSweptVolumeFromLastKnownPosition(player))
                            && gameMode->mGameStats.GetLastCheckpointSequenceID(player->getSUID())
                                   == gameMode->mFinalCheckpointSequenceID;
                        if (atFinalCheckpoint)
                            ++entered;
                    }
                }
            }
            else
            {
                AABB* area = gApp.getLevelGenerationOptions()->getRequiredGameRules()->getNamedArea(
                    m_volumeName.otherwise(L""));
                if (area->intersects(gameMode->GetSweptVolumeFromLastKnownPosition(player)))
                    ++entered;
            }
        }
    }
    return entered;
}

int PlayersEnteredVolumeCondition::GetThresholdPlayerOverride(MasterGameMode* gameMode, int playerCount) const
{
    if (m_option >= 0)
        return CommonMasterGameMode::GetMiniGame()->HostOption_Time(static_cast<eCustomGameModeHostOption>(m_option));
    if (m_useRatio.isSet() && m_useRatio.get())
    {
        if (m_threshold == 1)
            return playerCount;
        int threshold = playerCount / m_threshold + 1;
        return threshold < m_threshold ? threshold : m_threshold;
    }
    return m_threshold;
}

unsigned int PlayersEnteredVolumeCondition::GetThreshold(MasterGameMode* gameMode) const
{
    if (CommonMasterGameMode::GetMiniGame()->GetId() == 3)
    {
        std::vector<boost::shared_ptr<ServerPlayer> > glidePlayers = gameMode->GetGlidePlayers();
        return static_cast<unsigned int>(GetThresholdPlayerOverride(gameMode, static_cast<int>(glidePlayers.size())));
    }
    else
    {
        std::vector<boost::shared_ptr<ServerPlayer> > players = gameMode->GetPlayers(true, false);
        return static_cast<unsigned int>(GetThresholdPlayerOverride(gameMode, static_cast<int>(players.size())));
    }
}

void PlayersEnteredVolumeCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    const int threshold = GetThreshold(gameMode);
    params->push_back(threshold);
    const int playersString = IDS_MG_PLAYERS;
    params->push_back(playersString);
}

bool PlayersEnteredVolumeCondition::IsUsingPrimaryTargetAreas() const
{
    return m_useTargetAreas.isSet() ? m_useTargetAreas.get() : false;
}

bool PlayersEnteredVolumeCondition::UpdateMessage(MasterGameMode* gameMode, int message) const
{
    return true;
}
