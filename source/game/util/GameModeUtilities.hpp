#ifndef GAME_UTIL_GAMEMODEUTILITIES_HPP
#define GAME_UTIL_GAMEMODEUTILITIES_HPP

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class AABB;
class Container;
class GameRuleDefinition;
class ItemInstance;
class Level;
class Random;
class Scoreboard;
class ServerPlayer;
class TumbleGenerator;
class WeighedTreasure;

extern const std::wstring SPECTATOR_TEAM_NAME;

class GameModeUtilities
{
public:
    class Position
    {
    public:
        Position();
        Position(double x, double z);
        Position(double x, double y, double z);
        void randomize(Random* random, double minX, double minZ, double maxX, double maxZ);
        bool isSafeInRange(Level* level, int minY, int maxY);
        void randomize(Random* random, std::vector<long long>* positions);
        void set(double x, double z);
        double dist(Position* other);
        int getSpawnY(Level* level);
        double getLength();
        void normalize();
        void moveAway(Position* other);
        void clamp(double minX, double minZ, double maxX, double maxZ);
        bool isSafe(Level* level);

        double x;
        double z;
        double y;
    };

    static bool DistributeItemsSpread(const std::vector<boost::shared_ptr<Container> >* containers,
                                      const std::vector<not_null_ptr<ItemInstance> >* items);
    static int SelectRandomKit(const std::vector<MiniGameDef::ChestKitInfo>* kits, unsigned int* weights,
                               Random* random);
    static void GenerateContainerContentsGreedy(std::vector<not_null_ptr<ItemInstance> >* output,
                                                const std::vector<MiniGameDef::ChestItemsInfo>* items, Random* random,
                                                unsigned int playerCount, unsigned int score);
    static void GenerateContainerContentsRandom(std::vector<not_null_ptr<ItemInstance> >* output,
                                                const std::vector<MiniGameDef::ChestItemsInfo>* items, Random* random,
                                                unsigned int playerCount, unsigned int score);
    static bool DistributeItemsKitsAndKP(const std::vector<boost::shared_ptr<Container> >* containers,
                                         const std::vector<MiniGameDef::ChestItemsInfo>* items,
                                         const std::vector<MiniGameDef::ChestKitInfo>* kits, unsigned int playerCount,
                                         bool greedy);
    static bool DistributeItemsKitsFixed(const std::vector<boost::shared_ptr<Container> >* containers,
                                         const std::vector<MiniGameDef::ChestKitInfo>* kits);
    static bool DistributeItemsWeightedRandom(const std::vector<boost::shared_ptr<Container> >* containers,
                                              arrayWithLength<WeighedTreasure*> treasures, unsigned int score);
    static void SpawnWithinArea(boost::shared_ptr<ServerPlayer> player, AABB* area);
    static void DistributePlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                  std::vector<GameRuleDefinition*>* rules, arrayWithLength<unsigned char>* data);
    static std::vector<Position*>* createInitialPositions(Random* random, int count, double minX, double minZ,
                                                          double maxX, double maxZ);
    static int getNumberOfTeams(std::vector<boost::shared_ptr<ServerPlayer> >* players);
    static int spreadPositions(double distance, Level* level, Random* random, const AABB* area,
                               std::vector<Position*>* positions, bool teams);
    static double setPlayerPositions(std::vector<boost::shared_ptr<ServerPlayer> >* players, Level* level,
                                     std::vector<Position*>* positions, bool teams, TumbleGenerator* generator);
    static void SpreadPlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players, const AABB* area, double distance,
                              Level* level, bool teams, TumbleGenerator* generator);
    static void DistributePlayersInTeams(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                         Scoreboard* scoreboard);
};

#endif
