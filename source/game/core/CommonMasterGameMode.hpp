#ifndef GAME_CORE_COMMONMASTERGAMEMODE_HPP
#define GAME_CORE_COMMONMASTERGAMEMODE_HPP
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/core/CriticalSection.hpp"
#include "game/core/RoundInfo.hpp"
#include "game/data/StringTable.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/network/GameModePacket.hpp"
class AABB;
class CheckpointRuleDefinition;
class Level;
class LevelGenerationOptions;
class MapGenerator;
class MiniGameDef;
class Player;
class PlayersEnteredVolumeCondition;
class PowerupRuleDefinition;
class Scoreboard;
class ServerGamePacketListenerImpl;
class StatsUID;
class TargetAreaRuleDefinition;
class Team;
class Voteables;
class ClientPacketListener;
class CommonMasterGameMode
{
    friend class GameStats;
    friend class PlayerStats;
    friend class PlayersEnteredVolumeCondition;
    friend class ServerGamePacketListenerImpl;
    friend class UIScene_MapVoteMenu;

public:
    CommonMasterGameMode();
    virtual ~CommonMasterGameMode();
    static CommonMasterGameMode* GetMasterOrClient();
    static const wchar_t* GetStateName(EGameModeState state);
    virtual void Setup(EMiniGameId miniGameId);
    virtual void Tick();
    static const MiniGameDef* GetMiniGame();
    void SetupMiniGameInstance(const MiniGameDef& definition, int variant);
    virtual void OnExitedGame();
    virtual long long RestartMapGenerator();
    virtual EGameModeState GetGameModeState() const = 0;
    virtual Level* GeneratorTargetLevel() const = 0;
    virtual void setChosenThemeWordId(int id);
    virtual bool IsRoundPlaying() const = 0;
    virtual Scoreboard* getScoreboard() = 0;
    virtual void ResetMapDistances();
    virtual void CacheMapDistance();
    virtual AABB* GetSweptVolumeFromLastKnownPosition(boost::shared_ptr<Player> player) = 0;
    virtual unsigned int GetLastCheckpointID(const StatsUID& uid) = 0;
    virtual void OnProgressMade(const boost::shared_ptr<Player>& player, double progress);
    virtual void OnLapCompleted(const boost::shared_ptr<Player>& player);
    virtual void OnCheckpointEnter(const boost::shared_ptr<Player>& player, unsigned int checkpoint,
                                   CheckpointRuleDefinition* rule);
    virtual void OnTargetEnter(const boost::shared_ptr<Player>& player, unsigned int target,
                               TargetAreaRuleDefinition* rule);
    virtual void OnPrimaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule);
    virtual void OnSecondaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule);
    virtual void OnPowerupEnter(const boost::shared_ptr<Player>& player, unsigned int powerup,
                                PowerupRuleDefinition* rule);
    EMiniGameId GetNextGameMode() const;
    AABB* GetTeamArea(Team* team) const;
    Team* GetTeamAtPosition(double x, double y, double z, double margin) const;
    bool HasFallenOffLayer(const double& previousY, const double& currentY);
    bool IsOnGeneratedLayer(const int& layer, const double& y);
    int GetLayer(const double& y);
    unsigned int GetLayerCount();
    AABB* GetLayerAABB(const int& layer);
    void UpdateGeneratorSeed(long long seed, float scale);
    void RemoveMapGenerator();
    void CleanUpMapGenerator();
    const std::wstring& getChosenThemeWord();
    void clearBuildOffVotes();
    std::wstring getThemeWordEnglish(int id);
    StringTable* getLocalizedStringTable() const { return mLocalizedStringTable; }
    StringTable* getEnglishStringTable() const { return mEnglishStringTable; }
    LevelGenerationOptions* getCurrentLevelGenOptions() const { return mCurrentLevelGenOptions; }
    LevelGenerationOptions* getRoundLevelGenOptions() const;
    bool isUsingTrialContent();
    unsigned int CheckpointIDtoSequenceID(int id) const;
    void setCheckpoints(std::vector<CheckpointRuleDefinition*> checkpoints);
    double getTotalDistance();
    std::vector<double>* getCheckpointDistances();
    void CheckProgress(boost::shared_ptr<Player> player, const std::vector<CheckpointRuleDefinition*>* rules,
                       const std::vector<AABB*>* areas);
    void CheckProgress(const std::vector<boost::shared_ptr<Player> >& players);
    void CheckCheckpoints(const std::vector<boost::shared_ptr<Player> >& players);
    void CheckTargets(const std::vector<boost::shared_ptr<Player> >& players);
    void CheckPowerups(const std::vector<boost::shared_ptr<Player> >& players);
    template <class Rule> void CheckIntersectsNamedAreas(
        const std::vector<boost::shared_ptr<Player> >& players, std::vector<Rule*>& rules,
        void (CommonMasterGameMode::*callback)(const boost::shared_ptr<Player>&, unsigned int, Rule*));

protected:
    friend class ClientPacketListener;
    LevelGenerationOptions* mNextLevelGenOptions;
    LevelGenerationOptions* mCurrentLevelGenOptions;
    StringTable* mLocalizedStringTable;
    StringTable* mEnglishStringTable;
    RoundInfo mRoundInfo;
    long long mGeneratorSeed;
    float mGeneratorSeedScale;
    MapGenerator* mMapGenerator;
    OSFastMutex mGeneratorMutex;
    std::vector<AABB*> mActiveChunkAreas;
    std::vector<int> mThemeWordIds;
    int mChosenThemeWordId;
    Voteables* mVoteables;
    std::vector<double> mSequenceDistances;
    std::vector<double> mCheckpointDistances;
    std::vector<AABB*> mPrimaryTargetAreas;
    std::vector<CheckpointRuleDefinition*> mCheckpoints;
    int mRequiredLaps;
    double mTotalDistance;
    bool mDistancesCached;
    int mFinalCheckpointSequenceID;
    int mSecondaryTargetCounts[3];
};

#endif
