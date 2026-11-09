#ifndef GAME_CORE_CTELEMETRYMANAGER_HPP
#define GAME_CORE_CTELEMETRYMANAGER_HPP

#include <cstddef>
#include <stdint.h>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/minigame/EMiniGameId.hpp"
#include "game/ui/UILayer.hpp"
#include "game/util/Vec3.hpp"

class LaunchMoreOptionsMenuInitData;
class LocalPlayer;
struct SAVE_DETAILS;
class CTelemetryManager;

enum ESen_FriendOrMatch
{
    eSen_FriendOrMatch_NotApplicable,
    eSen_FriendOrMatch_Friend,
    eSen_FriendOrMatch_Match
};

enum ESen_CompeteOrCoop
{
    eSen_CompeteOrCoop_NotApplicable,
    eSen_CompeteOrCoop_Compete,
    eSen_CompeteOrCoop_Cooperate,
    eSen_CompeteOrCoop_Both
};

enum ESen_LevelExitStatus
{
    eSen_LevelExitStatus_Unknown,
    eSen_LevelExitStatus_Completed,
    eSen_LevelExitStatus_Restarted,
    eSen_LevelExitStatus_Quit
};

enum ESen_MediaDestination
{
    eSen_MediaDestination_Unknown,
    eSen_MediaDestination_LocalStorage,
    eSen_MediaDestination_SocialNetwork
};

enum ESen_MediaType
{
    eSen_MediaType_Unknown,
    eSen_MediaType_Screenshot,
    eSen_MediaType_Video
};

enum ESen_UpsellID
{
    eSen_UpsellID_None,
    eSen_UpsellID_FullGame
};

enum ESen_UpsellOutcome
{
    eSen_UpsellOutcome_None,
    eSen_UpsellOutcome_Declined,
    eSen_UpsellOutcome_Cancelled,
    eSen_UpsellOutcome_Purchased
};

enum TelemetryJoinMethod
{
    eTelemetryJoinMethod_Host,
    eTelemetryJoinMethod_Matchmaking,
    eTelemetryJoinMethod_Invite,
    eTelemetryJoinMethod_FriendsList
};

enum ETelemetryChallenges
{
    eTelemetryChallenges_None = 0,
    eTelemetryChallenges_TutorialStart = 1,
    eTelemetryChallenges_CraftingChoice = 2,
    eTelemetryChallenges_TutorialWelcome = 3,
    eTelemetryChallenges_PistonChoice = 11,
    eTelemetryChallenges_CreativeModeChoice = 14,
    eTelemetryChallenges_DeathByFall = 25,
    eTelemetryChallenges_DeathByLava = 26,
    eTelemetryChallenges_DeathByFire = 27,
    eTelemetryChallenges_DeathByDrowning = 28,
    eTelemetryChallenges_DeathBySuffocation = 29,
    eTelemetryChallenges_DeathByVoid = 30,
    eTelemetryChallenges_DeathByCactus = 31,
    eTelemetryChallenges_DeathByPlayer = 32,
    eTelemetryChallenges_DeathByPlayerProjectile = 33,
    eTelemetryChallenges_DeathByTnt = 34,
    eTelemetryChallenges_DeathByCreeper = 35,
    eTelemetryChallenges_DeathByWolf = 36,
    eTelemetryChallenges_DeathByZombie = 37,
    eTelemetryChallenges_DeathBySkeleton = 38,
    eTelemetryChallenges_DeathBySpider = 39,
    eTelemetryChallenges_DeathBySlime = 40,
    eTelemetryChallenges_DeathByGhast = 41,
    eTelemetryChallenges_DeathByZombiePigman = 42,
    eTelemetryChallenges_Challenge58 = 58,
    eTelemetryChallenges_DeathByIronGolem = 59,
    eTelemetryChallenges_DeathBySnowGolem = 60,
    eTelemetryChallenges_DeathByCaveSpider = 61,
    eTelemetryChallenges_DeathByWitch = 62,
    eTelemetryChallenges_DeathByEnderman = 63,
    eTelemetryChallenges_DeathByBlaze = 64,
    eTelemetryChallenges_DeathByMagmaCube = 65,
    eTelemetryChallenges_DeathByWither = 66,
    eTelemetryChallenges_DeathBySilverfish = 67,
    eTelemetryChallenges_DeathByEnderDragon = 68,
    eTelemetryChallenges_DeathByLightning = 69,
    eTelemetryChallenges_DeathByPhantom = 70,
    eTelemetryChallenges_DeathByGuardian = 71,
    eTelemetryChallenges_DeathByEndermite = 72,
    eTelemetryChallenges_DeathByRabbit = 73,
    eTelemetryChallenges_DeathByPolarBear = 74,
    eTelemetryChallenges_DeathByShulker = 75,
    eTelemetryChallenges_DeathByLlama = 80,
    eTelemetryChallenges_DeathByEvoker = 81,
    eTelemetryChallenges_DeathByVindicator = 82,
    eTelemetryChallenges_DeathByDolphin = 83,
    eTelemetryChallenges_DeathByPufferfish = 84
};

struct TelemetryDamageSource
{
    ETelemetryChallenges challenges;
    int causeOfDeathId;
    int playerWeaponId;
};

class CTelemetryManager
{
public:
    CTelemetryManager()
        : mUnknown0(0),
          mInitialiseTime(0.0f),
          mLevelStarted(false),
          mLevelStartTime(),
          mMultiplayerInstanceId(0),
          mLevelInstanceId(0)
    {
    }
    virtual ~CTelemetryManager();

    virtual unsigned int Init();
    virtual unsigned int Tick();
    virtual unsigned int Flush();
    virtual unsigned int RecordPlayerSessionStart(int playerId);
    virtual unsigned int RecordPlayerSessionExit(int playerId, int disconnectReason);
    virtual unsigned int RecordHeartBeat(int playerId);
    virtual unsigned int RecordLevelStart(int playerId, ESen_FriendOrMatch friendOrMatch,
                                          ESen_CompeteOrCoop competeOrCoop, int difficultyLevel, int localPlayerCount,
                                          int onlinePlayerCount);
    virtual unsigned int RecordLevelExit(int playerId, ESen_LevelExitStatus exitStatus);
    virtual unsigned int RecordLevelSaveOrCheckpoint(int playerId, int saveSlot, int saveSize);
    virtual unsigned int RecordLevelResume(int playerId, ESen_FriendOrMatch friendOrMatch,
                                           ESen_CompeteOrCoop competeOrCoop, int difficultyLevel, int localPlayerCount,
                                           int onlinePlayerCount, int secondsPlayed);
    virtual unsigned int RecordPauseOrInactive(int playerId);
    virtual unsigned int RecordUnpauseOrActive(int playerId);
    virtual unsigned int RecordMenuShown(int playerId, EUIScene scene, int previousScene);
    virtual unsigned int RecordAchievementUnlocked(int playerId, int achievementId, int gamerScore);
    virtual unsigned int RecordMediaShareUpload(int playerId, ESen_MediaDestination destination,
                                                ESen_MediaType mediaType);
    virtual unsigned int RecordUpsellPresented(int playerId, ESen_UpsellID upsellId, int upsellContext);
    virtual unsigned int RecordUpsellResponded(int playerId, ESen_UpsellID upsellId, int upsellContext,
                                               ESen_UpsellOutcome outcome);
    virtual unsigned int RecordPlayerDiedOrFailed(int playerId, int locationX, int locationY, int locationZ,
                                                  int enemyTypeId, int playerWeaponId, int causeOfDeathId,
                                                  ETelemetryChallenges challenges);
    virtual unsigned int RecordEnemyKilledOrOvercome(int playerId, int locationX, int locationY, int locationZ,
                                                     int enemyTypeId, int playerWeaponId, int causeOfDeathId,
                                                     ETelemetryChallenges challenges);
    virtual unsigned int RecordTexturePackLoaded(int playerId, int texturePackId, bool isDefault);
    virtual unsigned int RecordSkinChanged(int playerId, int skinId);
    virtual unsigned int RecordBanLevel(int playerId);
    virtual unsigned int RecordUnBanLevel(int playerId);
    virtual unsigned int PrepareMiniGameHostOptions(int playerId, const LaunchMoreOptionsMenuInitData* initData,
                                                    TelemetryJoinMethod joinMethod);
    virtual unsigned int RecordMiniGameHostOptions(int playerId, EMiniGameId miniGameId);
    virtual unsigned int RecordPlayerSettings(int playerId);
    virtual unsigned int RecordBlockPlaced(boost::shared_ptr<LocalPlayer> player, int blockId, int blockData,
                                           unsigned long long position);
    virtual unsigned int RecordBlockBroken(boost::shared_ptr<LocalPlayer> player, int blockId, int blockData,
                                           unsigned long long position);
    virtual unsigned int RecordItemAcquired(boost::shared_ptr<LocalPlayer> player, int itemId, int itemData, int count,
                                            unsigned long long position, const Vec3* location);
    virtual unsigned int RecordItemUsed(boost::shared_ptr<LocalPlayer> player, int itemId, int itemData,
                                        unsigned long long position, int count, const Vec3* location);
    virtual unsigned int RecordGameModeRoundStart(const std::wstring& gameModeName, const std::wstring& roundName);
    virtual unsigned int RecordGameModeRoundFinished();
    virtual unsigned int RecordSavesInformation(int playerId, const SAVE_DETAILS* saves, unsigned int saveCount);
    virtual int GetMultiplayerInstanceID();
    virtual int GenerateMultiplayerInstanceId();
    virtual void SetMultiplayerInstanceId(int instanceId);
    virtual void RecordTransitionToLobby();

public:
    int GetSecondsSinceInitialize();
    unsigned int GetMode(unsigned int playerId);
    unsigned int GetSubMode(unsigned int playerId);
    unsigned int GetLevelId(unsigned int playerId);
    unsigned int GetSubLevelId(unsigned int playerId);
    int GetTitleBuildId();
    int GetLevelInstanceID();
    unsigned int GetSingleOrMultiplayer();
    unsigned int GetDifficultyLevel(int difficultyId);
    unsigned int GetLicense();
    unsigned int GetDefaultGameControls(unsigned int playerId);
    unsigned int GetAudioSettings(unsigned int playerId);
    int GetLevelExitProgressStat1();
    int GetLevelExitProgressStat2();

    uint32_t mUnknown0;
    float mInitialiseTime;
    bool mLevelStarted;
    float mLevelStartTime[4];
    int mMultiplayerInstanceId;
    int mLevelInstanceId;
};

#endif
