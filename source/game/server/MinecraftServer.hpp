#ifndef GAME_SERVER_MINECRAFTSERVER_HPP
#define GAME_SERVER_MINECRAFTSERVER_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/ConsoleInputSource.hpp"
#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameType.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

class ChunkGenerator;
class ChunkSource;
class CommandDispatcher;
class ConsoleInput;
class C4JEventImpl;
class C4JThreadImpl;
class Entity;
class FillTool;
class Level;
class LevelStorageSource;
class LevelType;
class MasterGameMode;
class Player;
class PlayerList;
class ProgressRenderer;
class ServerConnection;
class ServerLevel;
class Settings;
class _NetworkGameInitData;

struct DebugFillAreaRequest;
class Level;
class MinecraftServer : public ConsoleInputSource
{
public:
    struct postProcessRequest
    {
        int x;
        int z;
        ChunkSource* chunkSource;
        ChunkGenerator* chunkGenerator;
    };

    MinecraftServer();
    virtual ~MinecraftServer();
    bool IsServerPaused() const;
    void setFlightAllowed(bool allowed);
    void setPlayers(PlayerList* players);
    void setAnimals(bool animals);
    void setNpcsEnabled(bool enabled);
    void setPvpAllowed(bool allowed);
    void setMaxBuildHeight(int height);
    int getMaxBuildHeight();
    bool isHardcore();
    bool getSpawnMonsters();
    void setLevelDifficulty(ServerLevel* level, const Difficulty* difficulty);
    void setDifficulty(const Difficulty* difficulty);
    static int runPostUpdate(void* argument);
    void postProcessTerminate(ProgressRenderer* progressRenderer);
    ServerLevel* getLevel(int dimension);
    void saveGameRules();
    void endProgress();
    bool loadLevel(LevelStorageSource* storageSource, const std::wstring& name, long long seed, LevelType* levelType,
                   _NetworkGameInitData* initData);
    bool initServer(long long seed, _NetworkGameInitData* initData, unsigned int hostSettings, bool findSeed);
    void saveAllChunks();
    void stopServer(bool didInit);
    PlayerList* getPlayers();
    void setDefaultGameMode(const GameType* gameType);
    const GameType* getDefaultGameType();
    ServerConnection* getConnection();
    bool isAnimals();
    bool isNpcsEnabled();
    bool isPvpAllowed();
    bool isFlightAllowed();
    bool isCommandBlockEnabled();
    bool isNetherEnabled();
    CommandDispatcher* getCommandDispatcher();
    int getSpawnProtectionRadius();
    bool isUnderSpawnProtection(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player);
    bool getForceGameType();
    static unsigned long long getCurrentTimeMillis();
    int getSpawnRadius(ServerLevel* level);
    void halt();
    void fjSetServerPaused(bool paused);
    void tick();
    void chunkPacketManagement_PreTick();
    void run(long long seed, void* argument);
    void DebugFillArea(Level* level, const DebugFillAreaRequest& request);
    void DebugUndoFillArea(Level* level);
    void broadcastStartSavingPacket();
    void broadcastStopSavingPacket();
    static unsigned long long main(long long seed, void* argument);
    static void HaltServer(bool primaryPlayerSignedOut);
    virtual void info(const std::wstring& message);
    virtual void warn(const std::wstring& message);
    virtual std::wstring getConsoleName();
    void setLevel(int dimension, ServerLevel* level);
    bool flagEntitiesToBeRemoved(unsigned int* entityIds);
    void addPostProcessRequest(ChunkSource* chunkSource, ChunkGenerator* chunkGenerator, int x, int z);
    static bool chunkPacketManagement_CanSendTo(INetworkPlayer* player);
#ifdef MINECRAFT_PC
    static bool chunkPacketManagement_CanSendTo(INetworkPlayer* player, int perTick);
#endif
    static void chunkPacketManagement_DidSendTo(INetworkPlayer* player);
    void Suspend();

    void saveDevData();
    void setProgress(const std::wstring& progressName, int progress);
    int getOperatorUserPermissionLevel();
    Level* getCommandSenderWorld();
    Entity* getCommandSenderEntity();
    void setForceGameType(bool force);
    int getPlayerIdleTimeout();
    void setPlayerIdleTimeout(int timeout);
    void chunkPacketManagement_PostTick();
    void handleConsoleInput(const std::wstring& input, ConsoleInputSource* source);
    void handleConsoleInputs();
    std::wstring getFile(const std::wstring& name);
    bool IsSuspending();

    int getGameModeId() const { return texturePackId; }

    static bool s_primaryPlayerSignedOut;
    static bool s_haltRequested;
    static bool s_saveOnExitAnswered;
    static bool s_setTimeAtEndOfTick;
    static long long s_setTime;
    static bool s_setDayTimeAtEndOfTick;
    static long long s_setDayTime;

    ServerConnection* mConnection;
    Settings* mSettings;
    arrayWithLength<ServerLevel*> mLevels;
    PlayerList* mPlayers;

private:
    friend class PlayerList;
    friend class GameStats;
    friend class MasterGameMode;

    static long long s_chunkPacketTickStart;
    static bool s_chunkPacketBudgetSpent;
    static std::vector<INetworkPlayer*> s_chunkPacketRecipients;
    static boost::unordered_map<std::wstring, int> s_countdownTimers;

    ConsoleSchematicFile* schematicFiles[8];
    FillTool* fillTools[8];
    Random random;
    int unusedValue0;
    bool running;
    bool initialised;

public:
    bool shuttingDown;

private:
    int tickCount;
    std::wstring progressName;
    int progress;
    CommandDispatcher* commandDispatcher;
    std::vector<ConsoleInput*> consoleInput;
    bool unusedFlag0;
    bool unusedFlag1;
    bool animals;
    bool npcsEnabled;
    bool pvpAllowed;
    bool flightAllowed;
    std::wstring motd;
    int maxBuildHeight;
    int playerIdleTimeout;
    bool forceGameType;
    const GameType* defaultGameType;

public:
    int ugcPlayersVersion;
    int texturePackId;

private:
    C4JThreadImpl* postProcessThread;
    bool postProcessTerminating;
    std::vector<postProcessRequest> postProcessRequests;
    OSFastMutex postProcessCritSec;

public:
    C4JEventImpl* serverPausedEvent;

private:
    bool serverPaused;
    long long serverPauseTime;

public:
    bool saveOnExit;

private:
    bool suspending;

public:
    MasterGameMode* mMasterGameMode;
};

extern MinecraftServer* gMinecraftServer;

#endif
