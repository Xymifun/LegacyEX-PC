#ifndef GAME_CORE_CLIENTMASTERGAMEMODE_HPP
#define GAME_CORE_CLIENTMASTERGAMEMODE_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/CommonMasterGameMode.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/data/StringTable.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/core/GameModeTransitionParams.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/core/StatsUID.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"

class DataInputStream;
class SoundEvent;
class HtmlString;
class Player;
class Vec3;
class MiniGameDef;
class MultiPlayerLevel;
class ClientboundChatPacket;
class Scoreboard;


class ClientMasterGameMode : public CommonMasterGameMode
{
public:
    class MapSelectInfo
    {
    public:
        int uniqueId;
        std::wstring name;
        int texturePackId;
    };

    ClientMasterGameMode();
    virtual ~ClientMasterGameMode();
    bool IsRoundRestarting() const;
    void ResetReadyStates();
    void Reset();
    void PlaySound(const SoundEvent* sound, float volume, float pitch);
    void OnRoundPlay();
    void OnRoundEnding();
    void OnRoundWaitingToStart();
    void OnRoundStart();
    void OnRoundEndingToLobby();
    void OnStateChange(EGameModeState state);
    virtual void OnExitedGame();
    void ClearPlaylist();
    void AddMapToPlaylist(int uniqueId, const std::wstring& name, int texturePackId);
    void SetMapVote(int playerIndex, int vote, bool remove);
    unsigned int GetMapVote(int playerIndex, int vote);
    void GetUniquePlaylist(std::vector<MapSelectInfo>& playlist);
    void GetCurrentMapVotesText(std::vector<HtmlString>& text);
    void GetCurrentVariantVotesText(std::vector<HtmlString>& text);
    void SetKickVote(int playerIndex, unsigned char colour, bool vote);
    unsigned int GetKickVote(int playerIndex, unsigned char colour);
    void SetBuildOffScoringVote(int playerIndex, int vote, int value);
    unsigned int GetBuildOffTeamIndexToScore(int playerIndex);
    unsigned int GetBuildOffScoringVote(int playerIndex, int vote);
    void SetBuildOffThemeVote(int playerIndex, int vote, int value);
    unsigned int GetBuildOffThemeVote(int playerIndex);
    const wchar_t* GetString(const int& id);
    void GetCurrentThemeVotesText(std::vector<HtmlString>& text);
    std::wstring GetTransitionTextOverride();
    void SetPlayerReady(unsigned int playerIndex, bool ready);
    bool IsPlayerReady(unsigned int playerIndex);
    void SetCurrentTheme(int themeId, int subId);
    void SetCurrentUniqueId(int uniqueId);
    unsigned int GetCurrentThemeId() const;
    unsigned int GetCurrentUniqueId() const;
    void SetNextLevelGenerationOptions(LevelGenerationOptions* options, bool owned);
    void OnShowdown();
    void OnChestRefill();
    void OnDeath(const Vec3* position);
    void DoLobbyTickSound(int tick, const MiniGameDef::SoundSpec& sound);
    void DoRoundStartTickSound(int tick, const MiniGameDef::SoundSpec& sound);
    void OnRoundStartTimerTick(int tick);
    void DoGraceTickSound(int tick, const MiniGameDef::SoundSpec& sound);
    void OnChatPacket(boost::shared_ptr<ClientboundChatPacket> packet);
    void SetGeneratorSeed(long long seed, float scale);
    bool IsWaitingToStart() const;
    bool IsRoundStarting() const;
    bool IsWaitingForMusicStartEvent() const;
    bool IsRoundEnding() const;
    bool IsRoundPlaying() const;
    bool IsInScoringState() const;
    int IsInLobbyState();
    int IsInActiveRoundState();
    bool IsInTrasitionFromLobby();
    bool CheckLocalChunksLoaded();
    void FireGameRoundStats();
    void updateProgressRenderer();
    virtual void Tick();
    bool isNewLevelDataPending();
    bool isWaitingToTransition();
    GameModeTransitionParams* popTransition();
    void queueTransition(GameModeTransitionParams* transition);
    void dropAllTransitions();
    void onChangeCustomGameMode();
    void SetServerIsWaitingOnChunkLoading();
    void ClearServerIsWaitingOnChunkLoading();
    bool ServerIsWaitingOnUsLoading();
    void RecievedBRUP();
    void SetBRUPSToWaitOn(int count);
    bool IsValidStateForItemUse() const;
    EGameModeState GetGameModeState() const;
    Level* GeneratorTargetLevel() const;
    class Scoreboard* getScoreboard();
    double GetPlayerMapProgress(PlayerUID uid) const;
    virtual void OnProgressMade(const boost::shared_ptr<Player>& player, double progress);
    virtual void ResetMapDistances();
    int convertCheckpointIDToSpawnableID(int checkpoint);
    void SetLocalRoundTimeStart(long long value);
    void SetLocalRoundTimeStartOverride(long long value);
    unsigned long long GetLocalRoundTimeStart() const;
    AABB* GetSweptVolumeFromLastKnownPosition(boost::shared_ptr<Player> player);
    unsigned int GetLastCheckpointID(const StatsUID& uid);
    unsigned int SetLastCheckpointID(const StatsUID& uid, int checkpoint, bool notify);
    unsigned int GetBuildOffTeamScore(int team);
    void ReadStringTableFromStream(DataInputStream* stream);
    const wchar_t* GetString(const std::wstring& key);
    unsigned int GetCurrentThemeSubId() const;

private:
    bool mPlayerReady[8];
    bool mNewLevelDataPending;
    std::vector<MapSelectInfo> mPlaylist;
    StringTable mStringTable;
    EGameModeState mGameModeState;
    std::vector<GameModeTransitionParams*> mTransitions;
    boost::unordered_map<std::wstring, unsigned int> mStrings;
    boost::unordered_map<PlayerUID, double, boost::hash<PlayerUID> > mPlayerProgress;
    boost::unordered_map<StatsUID, int, boost::hash<StatsUID> > mLastCheckpoints;
    bool mLocalPlayerInRound[4];
    long long mLocalRoundTimeStart;
    bool mServerIsWaitingOnChunkLoading;
    bool mLocalChunksLoaded;
    int mBRUPsRemaining;
    int mBRUPCount;
    int mLoadedChunkCount;
    long long mSubRoundLoadStartTime;
};

#endif
