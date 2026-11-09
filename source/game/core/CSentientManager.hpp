#ifndef GAME_CORE_CSENTIENTMANAGER_HPP
#define GAME_CORE_CSENTIENTMANAGER_HPP

#include "game/core/CTelemetryManager.hpp"

class CSentientManager
{
public:
    unsigned int Init();
    unsigned int Tick();
    unsigned int Flush();
    unsigned int RecordPlayerSessionStart(unsigned int playerId);
    unsigned int RecordPlayerSessionExit(unsigned int playerId, int disconnectReason);
    unsigned int RecordHeartBeat(unsigned int playerId);
    unsigned int RecordLevelStart(unsigned int playerId, ESen_FriendOrMatch friendOrMatch,
                                  ESen_CompeteOrCoop competeOrCoop, int difficultyLevel, unsigned int localPlayerCount,
                                  unsigned int onlinePlayerCount);
    unsigned int RecordLevelExit(unsigned int playerId, ESen_LevelExitStatus exitStatus);
    unsigned int RecordLevelSaveOrCheckpoint(unsigned int playerId, int saveSlot, int saveSize);
    unsigned int RecordLevelResume(unsigned int playerId, ESen_FriendOrMatch friendOrMatch,
                                   ESen_CompeteOrCoop competeOrCoop, int difficultyLevel, unsigned int localPlayerCount,
                                   unsigned int onlinePlayerCount, int secondsPlayed);
    unsigned int RecordPauseOrInactive(unsigned int playerId);
    unsigned int RecordUnpauseOrActive(unsigned int playerId);
    unsigned int RecordMenuShown(unsigned int playerId, int scene, int previousScene);
    unsigned int RecordAchievementUnlocked(unsigned int playerId, int achievementId, int gamerScore);
    unsigned int RecordMediaShareUpload(unsigned int playerId, ESen_MediaDestination destination,
                                        ESen_MediaType mediaType);
    unsigned int RecordUpsellPresented(unsigned int playerId, ESen_UpsellID upsellId, int upsellContext);
    unsigned int RecordUpsellResponded(unsigned int playerId, ESen_UpsellID upsellId, int upsellContext,
                                       ESen_UpsellOutcome outcome);
    unsigned int RecordPlayerDiedOrFailed(unsigned int playerId, int playerWeaponId, int causeOfDeathId,
                                          ETelemetryChallenges challenges);
    unsigned int RecordEnemyKilledOrOvercome(unsigned int playerId, int playerWeaponId, int causeOfDeathId,
                                             ETelemetryChallenges challenges);
    unsigned int RecordSkinChanged(unsigned int playerId, unsigned int skinId);
    unsigned int RecordBanLevel(unsigned int playerId);
    unsigned int RecordUnBanLevel(unsigned int playerId);
    int GetMultiplayerInstanceID();
    int GenerateMultiplayerInstanceId();
    void SetMultiplayerInstanceId(int instanceId);
};

#endif
