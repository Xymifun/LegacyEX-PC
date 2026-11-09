#ifndef GAME_SERVER_SERVERPLAYERGAMEMODE_HPP
#define GAME_SERVER_SERVERPLAYERGAMEMODE_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/GameMode.hpp"
#include "game/world/block/BlockPos.hpp"

class AABB;
class ClientboundChatPacket;
class CombatEntry;
class Direction;
class Entity;
class GameRulesInstance;
class Level;
class MasterGameMode;
class MiniGameDef;
class ServerLevel;
class ServerPlayer;

class ServerPlayerGameMode : public GameMode
{
    friend class PlayerList;
    friend class ServerPlayer;

public:
    ServerPlayerGameMode(Level* level, MasterGameMode* masterGameMode);
    virtual ~ServerPlayerGameMode();

    void setLevel(ServerLevel* level);
    void updateGameMode(const GameType* gameType);
    void setGameRules(GameRulesInstance* gameRules);
    void nullifyGameRules();
    void setLastCheckpoint(int checkpoint);
    unsigned int getLastCheckpoint();
    void RestoreFromOld(ServerPlayerGameMode* oldMode);
    boost::shared_ptr<ServerPlayer> getServerPlayer() const;
    int superDestroyBlock(const BlockPos& pos);
    int destroyBlock(const BlockPos& pos);
    void stopDestroyBlock(const BlockPos& pos);
    void abortDestroyBlock();
    InteractionResult::EInteractionResult useItemOn(boost::shared_ptr<Player> player, Level* level,
                                                    not_null_ptr<ItemInstance> item,
                                                    InteractionHand::EInteractionHand hand, const BlockPos& pos,
                                                    const Direction* direction, float hitX, float hitY, float hitZ,
                                                    bool simulate, bool* result);
    virtual bool startDestroyBlock(const BlockPos& pos, const Direction* direction);
    virtual InteractionResult::EInteractionResult useItem(boost::shared_ptr<Player> player, Level* level,
                                                          InteractionHand::EInteractionHand hand, bool simulate);
    virtual void SetGameType(const GameType* gameType);
    virtual const GameType* GetGameType();
    bool IsNearCentreOf(const AABB* box, ServerPlayer* player, double x, double y, double z);
    virtual void tick();
    virtual void OnDeath(boost::shared_ptr<ServerPlayer> player);
    virtual unsigned int GetDeaths();
    virtual void SetDeaths(int deaths);
    virtual unsigned int GetKills();
    virtual void SetKills(int kills);
    virtual void RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                              boost::shared_ptr<ClientboundChatPacket> packet);
    virtual void RecordDeath(boost::shared_ptr<Entity> entity);
    virtual void RecordItem(not_null_ptr<ItemInstance> item);
    virtual void RecordContainer(int type, int slot, int count, int data);
    virtual void RecordWalk(int distance, bool sprinting, bool sneaking);
    virtual void RecordSwim(int distance, bool sprinting);
    virtual void RecordClimb(int distance);
    virtual void RecordUseItem(not_null_ptr<ItemInstance> item);
    virtual void RecordJump();
    virtual void RecordGainPoints(int points);
    virtual void ResetTempPoints();
    virtual bool RecordSecondaryTargetReached(const std::wstring& name, int target,
                                              GlideRingGenerator::eGlideRingSize size);
    virtual bool RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third);
    virtual bool RecordLapCompleted();
    virtual void RecordThermalEntered();
    virtual bool RecordProgress(double first, double second, double third, double fourth);
    virtual void RecordSpawnedAtCheckpoint(int checkpoint);
    virtual void RecordBlockPlaced(const BlockPos& pos, int blockId);
    virtual void RecordBlockDestroyed(const BlockPos& pos, int blockId);
    virtual void RecordBlockPresent(const BlockPos& pos, int blockId);
    virtual void RecordLayerTime();
    virtual void RecordSnowballHit();
    virtual void RecordHitBySnowball();
    virtual void RecordLanding(const BlockPos& pos);
    virtual const MiniGameDef* GetMiniGame();
    virtual boost::shared_ptr<Player> GetPlayer();
    virtual Level* GetLevel();

private:
    Level* mLevel;
    ServerPlayer* mPlayer;
    bool mDestroying;
    int mDestroyStartTick;
    BlockPos mDestroyPos;
    int mGameModeTicks;
    bool mIsDestroying;
    BlockPos mPreviousDestroyPos;
    int mSecondaryDestroyStartTick;
    int mLastSentProgress;
    GameRulesInstance* mGameRules;
    MasterGameMode* mMasterGameMode;
    int mUnknown4C;
    long long mLayerTimeMs;
    int mDeaths;
    int mLastCheckpoint;
};

#endif
