#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CTelemetryManager.hpp"

#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"

CTelemetryManager::~CTelemetryManager()
{
}

unsigned int CTelemetryManager::Init()
{
    return 0;
}

unsigned int CTelemetryManager::Tick()
{
    return 0;
}

unsigned int CTelemetryManager::Flush()
{
    return 0;
}

unsigned int CTelemetryManager::RecordPlayerSessionStart(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordPlayerSessionExit(int playerId, int disconnectReason)
{
    return 1;
}

unsigned int CTelemetryManager::RecordHeartBeat(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordLevelStart(int playerId, ESen_FriendOrMatch friendOrMatch,
                                                 ESen_CompeteOrCoop competeOrCoop, int difficultyLevel,
                                                 int localPlayerCount, int onlinePlayerCount)
{
    if (playerId == gProfile->GetPrimaryPad())
        mLevelStarted = true;

    ++mLevelInstanceId;
    mLevelStartTime[playerId] = gApp.getAppTime();
    return 1;
}

unsigned int CTelemetryManager::RecordLevelExit(int playerId, ESen_LevelExitStatus exitStatus)
{
    return 1;
}

unsigned int CTelemetryManager::RecordLevelSaveOrCheckpoint(int playerId, int saveSlot, int saveSize)
{
    return 1;
}

unsigned int CTelemetryManager::RecordLevelResume(int playerId, ESen_FriendOrMatch friendOrMatch,
                                                  ESen_CompeteOrCoop competeOrCoop, int difficultyLevel,
                                                  int localPlayerCount, int onlinePlayerCount, int secondsPlayed)
{
    return 1;
}

unsigned int CTelemetryManager::RecordPauseOrInactive(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordUnpauseOrActive(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordMenuShown(int playerId, EUIScene scene, int previousScene)
{
    return 1;
}

unsigned int CTelemetryManager::RecordAchievementUnlocked(int playerId, int achievementId, int gamerScore)
{
    return 1;
}

unsigned int CTelemetryManager::RecordMediaShareUpload(int playerId, ESen_MediaDestination destination,
                                                       ESen_MediaType mediaType)
{
    return 1;
}

unsigned int CTelemetryManager::RecordUpsellPresented(int playerId, ESen_UpsellID upsellId, int upsellContext)
{
    return 1;
}

unsigned int CTelemetryManager::RecordUpsellResponded(int playerId, ESen_UpsellID upsellId, int upsellContext,
                                                      ESen_UpsellOutcome outcome)
{
    return 1;
}

unsigned int CTelemetryManager::RecordPlayerDiedOrFailed(int playerId, int locationX, int locationY, int locationZ,
                                                         int enemyTypeId, int playerWeaponId, int causeOfDeathId,
                                                         ETelemetryChallenges challenges)
{
    return 1;
}

unsigned int CTelemetryManager::RecordEnemyKilledOrOvercome(int playerId, int locationX, int locationY, int locationZ,
                                                            int enemyTypeId, int playerWeaponId, int causeOfDeathId,
                                                            ETelemetryChallenges challenges)
{
    return 1;
}

unsigned int CTelemetryManager::RecordTexturePackLoaded(int playerId, int texturePackId, bool isDefault)
{
    return 1;
}

unsigned int CTelemetryManager::RecordSkinChanged(int playerId, int skinId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordBanLevel(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordUnBanLevel(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::PrepareMiniGameHostOptions(int playerId, const LaunchMoreOptionsMenuInitData* initData,
                                                           TelemetryJoinMethod joinMethod)
{
    return 1;
}

unsigned int CTelemetryManager::RecordMiniGameHostOptions(int playerId, EMiniGameId miniGameId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordPlayerSettings(int playerId)
{
    return 1;
}

unsigned int CTelemetryManager::RecordBlockPlaced(boost::shared_ptr<LocalPlayer> player, int blockId, int blockData,
                                                  unsigned long long position)
{
    return 1;
}

unsigned int CTelemetryManager::RecordBlockBroken(boost::shared_ptr<LocalPlayer> player, int blockId, int blockData,
                                                  unsigned long long position)
{
    return 1;
}

unsigned int CTelemetryManager::RecordItemAcquired(boost::shared_ptr<LocalPlayer> player, int itemId, int itemData,
                                                   int count, unsigned long long position, const Vec3* location)
{
    return 1;
}

unsigned int CTelemetryManager::RecordItemUsed(boost::shared_ptr<LocalPlayer> player, int itemId, int itemData,
                                               unsigned long long position, int count, const Vec3* location)
{
    return 1;
}

unsigned int CTelemetryManager::RecordGameModeRoundStart(const std::wstring& gameModeName,
                                                         const std::wstring& roundName)
{
    return 1;
}

unsigned int CTelemetryManager::RecordGameModeRoundFinished()
{
    return 1;
}

unsigned int CTelemetryManager::RecordSavesInformation(int playerId, const SAVE_DETAILS* saves, unsigned int saveCount)
{
    return 1;
}

int CTelemetryManager::GetMultiplayerInstanceID()
{
    return mMultiplayerInstanceId;
}

int CTelemetryManager::GenerateMultiplayerInstanceId()
{
    return 0;
}

void CTelemetryManager::SetMultiplayerInstanceId(int instanceId)
{
    mMultiplayerInstanceId = instanceId;
}

void CTelemetryManager::RecordTransitionToLobby()
{
}

unsigned int CTelemetryManager::GetMode(unsigned int playerId)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == NULL)
        return 0;

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        return 4;

    if (playerId < 4 && minecraft->mLocalPlayers[playerId] != NULL)
    {
        const GameType* gameType = NULL;
        if (minecraft->mLocalPlayers[playerId]->mLevel != NULL
            && minecraft->mLocalPlayers[playerId]->mLevel->getLevelData() != NULL)
            gameType = minecraft->mLocalPlayers[playerId]->mLevel->getLevelData()->getGameType();

        if (gameType == NULL && gApp.GetGameHostOption(eGameHostOption_HostCanFly))
            gameType = minecraft->mLocalPlayers[playerId]->GetGameType();

        if (gameType == GameType::SURVIVAL)
            return 1;
        if (gameType == GameType::CREATIVE)
            return 2;
        if (gameType == GameType::ADVENTURE)
            return 3;
    }

    return 0;
}

unsigned int CTelemetryManager::GetLicense()
{
    if (gProfile->IsFullVersion())
        return 2;

    return 1;
}

unsigned int CTelemetryManager::GetAudioSettings(unsigned int playerId)
{
    unsigned int settings = 0;

    if (playerId == (unsigned int)gProfile->GetPrimaryPad())
    {
        int volume = GameSettings::GetGameSettings(playerId, eGameSetting_SoundVolume);
        if (volume == 0)
        {
            settings = 1;
        }
        else
        {
            settings = 3;
            if (volume == 100)
                settings = 2;
        }
    }

    return settings;
}
