#include "game/util/GameModeUtilities.hpp"
#include "game/world/block/PressurePlateBlock.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/WeighedTreasure.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/AddItemRuleDefinition.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/world/generation/TumbleGenerator.hpp"
#include "game/world/chunk/ChunkSource.hpp"

namespace
{
struct ChestItemsInfoLess
{
    bool operator()(const MiniGameDef::ChestItemsInfo& left, const MiniGameDef::ChestItemsInfo& right) const
    {
        return left.mKpScore < right.mKpScore;
    }
};
}

const std::wstring SPECTATOR_TEAM_NAME = L"s";

GameModeUtilities::Position::Position() : x(0.0), z(0.0)
{
}

GameModeUtilities::Position::Position(double xValue, double zValue) : x(xValue), z(zValue), y(Math::random() * 360.0)
{
}

void GameModeUtilities::Position::randomize(Random* random, double minX, double minZ, double maxX, double maxZ)
{
    x = Mth::nextDouble(random, minX, maxX);
    z = Mth::nextDouble(random, minZ, maxZ);
}

bool GameModeUtilities::Position::isSafeInRange(Level* level, int minY, int maxY)
{
    int blockX = Mth::floor(x);
    int blockZ = Mth::floor(z);
    int top = std::min(Level::maxBuildHeight, maxY);
    int bottom = std::max(minY, 2);

    for (int y = top; y > bottom; --y)
    {
        BlockPos floorPos = BlockPos(blockX, y, blockZ).below();
        const BlockState* floor = level->getBlockState(floorPos);
        const BlockState* head = level->getBlockState(floorPos.above());
        if (floor != NULL && floor->getBlock() != Blocks::air)
        {
            return floor->isSolidBlockingCube() && head->getMaterial() != Material::FIRE
                   && dynamic_cast<PressurePlateBlock*>(head->getBlock()) == NULL;
        }
    }
    return false;
}

void GameModeUtilities::Position::randomize(Random* random, std::vector<long long>* positions)
{
    z = x = 0.0;
    if (positions != NULL && !positions->empty())
    {
        int index = Mth::nextInt(random, 0, positions->size() - 1);
        long long packed = positions->at(index);
        int packedX = (int)(packed >> 32);
        int packedZ = (int)(packed & 0xFFFFFFFF);
        x = packedX;
        z = packedZ;
    }
}

void GameModeUtilities::Position::set(double xValue, double zValue)
{
    x = xValue;
    z = zValue;
}

double GameModeUtilities::Position::dist(Position* other)
{
    double dx = x - other->x;
    double dz = z - other->z;
    return std::sqrt(dx * dx + dz * dz);
}

int GameModeUtilities::Position::getSpawnY(Level* level)
{
    int ix = Mth::floor(x);
    int iz = Mth::floor(z);
    for (int y = 256; y > 0; --y)
    {
        Block* block = level->getBlockState(BlockPos(ix, y, iz))->getBlock();
        if (block != NULL && block != Blocks::air)
        {
            if (block == Blocks::lava || block == Blocks::flowing_lava)
                return y + 50;
            return y + 1;
        }
    }
    return 257;
}

bool GameModeUtilities::DistributeItemsSpread(const std::vector<boost::shared_ptr<Container> >* containers,
                                              const std::vector<not_null_ptr<ItemInstance> >* items)
{
    std::vector<not_null_ptr<ItemInstance> > remaining(*items);
    delete items;
    bool result = false;
    if (!remaining.empty() && !containers->empty())
    {
        Random random;
        unsigned int containerCount = (unsigned int)containers->size();
        unsigned int itemCount = (unsigned int)remaining.size();
        unsigned int baseCount = itemCount / containerCount;
        unsigned int remainder = itemCount - baseCount * containerCount;
        std::vector<unsigned int> counts(containerCount, baseCount);
        for (unsigned int i = 0; i < remainder; ++i)
        {
            unsigned int index;
            do
            {
                index = (unsigned int)random.nextInt((int)containerCount);
            } while (counts[index] != baseCount);
            ++counts[index];
        }
        for (unsigned int ci = 0; ci < containerCount; ++ci)
        {
            boost::shared_ptr<Container> container = containers->at(ci);
            std::vector<not_null_ptr<ItemInstance> > batch;
            for (unsigned int j = 0; j < counts[ci]; ++j)
            {
                unsigned int index = (unsigned int)random.nextInt((int)remaining.size());
                batch.push_back(remaining.at(index));
                remaining.erase(remaining.begin() + index);
            }
            while (!batch.empty())
            {
                unsigned int index = (unsigned int)random.nextInt((int)batch.size());
                AddItemRuleDefinition::addItemToContainer(container, batch.at(index), -1, true);
                batch.erase(batch.begin() + index);
            }
        }
        result = true;
    }
    return result;
}

int GameModeUtilities::SelectRandomKit(const std::vector<MiniGameDef::ChestKitInfo>* kits, unsigned int* weights,
                                       Random* random)
{
    if (kits == NULL || kits->empty())
        return -1;
    std::vector<unsigned int> localWeights;
    if (weights == NULL)
    {
        localWeights.resize(kits->size());
        for (unsigned int i = 0; i < kits->size(); ++i)
            localWeights[i] = (unsigned int)kits->at(i).mWeight;
        weights = &localWeights[0];
    }
    unsigned int total = 0;
    for (unsigned int i = 0; i < kits->size(); ++i)
        total += weights[i];
    if (total == 0)
        return 0;
    unsigned int value = (unsigned int)random->nextInt((int)total);
    for (unsigned int i = 0; i < kits->size(); ++i)
    {
        if (value < weights[i])
        {
            weights[i] >>= 1;
            return (int)i;
        }
        value -= weights[i];
    }
    return (int)kits->size() - 1;
}

void GameModeUtilities::GenerateContainerContentsGreedy(std::vector<not_null_ptr<ItemInstance> >* output,
                                                        const std::vector<MiniGameDef::ChestItemsInfo>* items,
                                                        Random* random, unsigned int playerCount, unsigned int score)
{
    if (output == NULL || items == NULL)
        return;
    std::vector<MiniGameDef::ChestItemsInfo> sorted(*items);
    std::sort(sorted.begin(), sorted.end(), ChestItemsInfoLess());
    unsigned int currentScore = score;
    unsigned int index = 0;
    while (index < sorted.size())
    {
        while (index < sorted.size() && (unsigned int)sorted[index].mKpScore + currentScore > playerCount)
            ++index;
        if (index == sorted.size())
            break;
        unsigned int end = index + 1;
        while (end < sorted.size() && sorted[end].mKpScore == sorted[index].mKpScore)
            ++end;
        unsigned int selected = index + (unsigned int)random->nextInt((int)(end - index));
        output->push_back(sorted[selected].mItem->copy());
        currentScore += (unsigned int)sorted[selected].mKpScore;
    }
}

void GameModeUtilities::GenerateContainerContentsRandom(std::vector<not_null_ptr<ItemInstance> >* output,
                                                        const std::vector<MiniGameDef::ChestItemsInfo>* items,
                                                        Random* random, unsigned int playerCount, unsigned int score)
{
    if (output == NULL || items == NULL || items->empty())
        return;
    unsigned int total = score;
    while (total < playerCount)
    {
        const MiniGameDef::ChestItemsInfo& item = items->at((unsigned int)random->nextInt((int)items->size()));
        output->push_back(item.mItem->copy());
        total += (unsigned int)item.mKpScore;
    }
}

bool GameModeUtilities::DistributeItemsKitsAndKP(const std::vector<boost::shared_ptr<Container> >* containers,
                                                 const std::vector<MiniGameDef::ChestItemsInfo>* items,
                                                 const std::vector<MiniGameDef::ChestKitInfo>* kits,
                                                 unsigned int playerCount, bool greedy)
{
    Random random;
    std::vector<not_null_ptr<ItemInstance> > contents;
    unsigned int* weights = new unsigned int[kits->size()];
    for (unsigned int i = 0; i < kits->size(); ++i)
        weights[i] = (unsigned int)kits->at(i).mWeight;

    unsigned int score = 0;
    for (std::vector<boost::shared_ptr<Container> >::const_iterator it = containers->begin(); it != containers->end();
         ++it)
    {
        boost::shared_ptr<Container> container = *it;
        unsigned int size = container->getContainerSize();
        unsigned int slot = 0;
        if (!kits->empty())
        {
            int kitIndex = SelectRandomKit(kits, weights, &random);
            if (kitIndex >= 0)
            {
                const MiniGameDef::ChestKitInfo& kit = kits->at((unsigned int)kitIndex);
                score = (unsigned int)kit.mKpScore;
                for (std::vector<not_null_ptr<ItemInstance> >::const_iterator item = kit.mItems.begin();
                     item != kit.mItems.end() && slot < size; ++item, ++slot)
                    AddItemRuleDefinition::addItemToContainer(container, (*item)->copy(), -1, true);
            }
        }
        if (score < playerCount && !items->empty())
        {
            if (greedy)
                GenerateContainerContentsGreedy(&contents, items, &random, playerCount, score);
            else
                GenerateContainerContentsRandom(&contents, items, &random, playerCount, score);
        }
        for (std::vector<not_null_ptr<ItemInstance> >::iterator item = contents.begin();
             item != contents.end() && slot < size; ++item, ++slot)
            AddItemRuleDefinition::addItemToContainer(container, *item, -1, true);
        contents.clear();
    }
    delete[] weights;
    return true;
}

bool GameModeUtilities::DistributeItemsKitsFixed(const std::vector<boost::shared_ptr<Container> >* containers,
                                                 const std::vector<MiniGameDef::ChestKitInfo>* kits)
{
    if (!kits->empty())
    {
        const MiniGameDef::ChestKitInfo& kit = kits->at(0);
        for (std::vector<boost::shared_ptr<Container> >::const_iterator container = containers->begin();
             container != containers->end(); ++container)
        {
            for (std::vector<not_null_ptr<ItemInstance> >::const_iterator item = kit.mItems.begin();
                 item != kit.mItems.end(); ++item)
                AddItemRuleDefinition::addItemToContainer(*container, (*item)->copy(), -1, false);
        }
    }
    return true;
}

bool GameModeUtilities::DistributeItemsWeightedRandom(const std::vector<boost::shared_ptr<Container> >* containers,
                                                      arrayWithLength<WeighedTreasure*> treasures, unsigned int score)
{
    Random random;
    for (unsigned int i = 0; i < containers->size(); ++i)
        WeighedTreasure::addChestItems(&random, treasures, containers->at(i), (int)score, true);
    for (unsigned int i = 0; i < treasures.length; ++i)
        if (treasures.data[i] != NULL)
            delete treasures.data[i];
    delete[] treasures.data;
    return true;
}

void GameModeUtilities::SpawnWithinArea(boost::shared_ptr<ServerPlayer> player, AABB* area)
{
    Random random;
    double x = area->mMinX + (area->mMaxX - area->mMinX) * random.nextDouble();
    double y = area->mMinY + (area->mMaxY - area->mMinY) * random.nextDouble();
    double z = area->mMinZ + (area->mMaxZ - area->mMinZ) * random.nextDouble();
    player->absMoveTo(x, y, z, 0.0f, 0.0f);
    player->teleportTo(player->mX, player->mY, player->mZ);
}

void GameModeUtilities::DistributePlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                          std::vector<GameRuleDefinition*>* rules, arrayWithLength<unsigned char>* data)
{
    ServerLevel* level = gMinecraftServer->getLevel(0);
    ChunkSource* chunks = level->getChunkSource();
    std::vector<int> availableRules;
    for (unsigned int index = 0; index < rules->size(); ++index)
        availableRules.push_back(static_cast<int>(index));

    Random random;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        int ruleIndex;
        while (true)
        {
            if (availableRules.empty())
            {
                ruleIndex = random.nextInt(static_cast<int>(rules->size()));
                break;
            }

            const int candidate = random.nextInt(static_cast<int>(availableRules.size()));
            ruleIndex = availableRules.at(static_cast<unsigned int>(candidate));
            if (data != NULL && ruleIndex >= 0 && static_cast<unsigned int>(ruleIndex) < data->length
                && data->data[ruleIndex] != static_cast<unsigned char>(-1))
            {
                availableRules.erase(availableRules.begin() + candidate);
                continue;
            }
            break;
        }

#if defined(_WIN32)
        const char* referenceSpawn = std::getenv("MC_BATTLE_REFERENCE_SPAWN");
        if (referenceSpawn != NULL && rules->size() == 16 && players->size() == 1)
        {
            const int requestedRule = std::atoi(referenceSpawn);
            if (requestedRule == 1 || requestedRule == 12)
                ruleIndex = requestedRule;
        }
#endif

        if (data != NULL && ruleIndex >= 0 && static_cast<unsigned int>(ruleIndex) < data->length)
            data->data[ruleIndex] = player->mPlayerIndex;

        rules->at(static_cast<unsigned int>(ruleIndex))->postProcessPlayer(player);

        const int chunkX = static_cast<int>(player->mX) >> 4;
        const int chunkZ = static_cast<int>(player->mZ) >> 4;
        if (!chunks->hasChunk(chunkX, chunkZ))
            chunks->create(chunkX, chunkZ);

        player->doTick(true, true, true);
        player->mConnection->teleport(player->mX, player->mY, player->mZ, player->mYRot, player->mXRot, true, true);
        if (Minecraft::InMiniGame(eMiniGameId_Battle, false))
            player->ResetSpawnInvulnerableTime();
    }
}

std::vector<GameModeUtilities::Position*>* GameModeUtilities::createInitialPositions(Random* random, int count,
                                                                                     double minX, double minZ,
                                                                                     double maxX, double maxZ)
{
    std::vector<Position*>* positions = new std::vector<Position*>();
    positions->reserve(count);
    for (int i = 0; i < count; ++i)
    {
        Position* position = new Position();
        position->randomize(random, minX, minZ, maxX, maxZ);
        positions->push_back(position);
    }
    return positions;
}

int GameModeUtilities::getNumberOfTeams(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    return 0;
}

int GameModeUtilities::spreadPositions(double distance, Level* level, Random* random, const AABB* area,
                                       std::vector<Position*>* positions, bool teams)
{
    std::vector<long long> candidates;
    for (int gx = (int)std::floor(area->mMinX); gx < (int)std::ceil(area->mMaxX); ++gx)
    {
        for (int gz = (int)std::floor(area->mMinZ); gz < (int)std::ceil(area->mMaxZ); ++gz)
        {
            Position cell((double)gx, (double)gz);
            if (cell.isSafeInRange(level, (int)std::floor(area->mMinY), (int)std::ceil(area->mMaxY)))
            {
                long long packed = ((long long)(unsigned int)gx << 32) | (unsigned int)gz;
                candidates.push_back(packed);
            }
        }
    }

    for (unsigned int i = 0; i < positions->size(); ++i)
        positions->at(i)->randomize(random, &candidates);

    int placed = 0;
    unsigned int i = 0;
    for (; i < positions->size(); ++i)
    {
        Position* position = positions->at(i);
        double savedX = position->x;
        double savedZ = position->z;
        position->randomize(random, &candidates);
        int roundedX = (int)position->x;
        int roundedZ = (int)position->z;
        for (unsigned int c = 0; c < candidates.size();)
        {
            long long packed = candidates[c];
            int candX = (int)(packed >> 32);
            int candZ = (int)packed;
            double dx = (double)roundedX - (double)candX;
            double dz = (double)roundedZ - (double)candZ;
            double len = Vec3::newTemp(dx, 0.0, dz)->length();
            if (distance <= len)
                break;
            candidates.erase(candidates.begin() + c);
        }
        if (candidates.empty())
        {
            position->set(savedX, savedZ);
            break;
        }
        ++placed;
    }

    double centerX = (area->mMinX + area->mMaxX) * 0.5;
    double centerZ = (area->mMinZ + area->mMaxZ) * 0.5;
    for (unsigned int j = 0; j < positions->size(); ++j)
    {
        Position* position = positions->at(j);
        double yaw = Mth::atan2((int)(position->z - centerZ), (int)(position->x - centerX));
        position->y = (float)(yaw * 57.29577951308232 + 90.0);
    }

    return placed;
}

double GameModeUtilities::setPlayerPositions(std::vector<boost::shared_ptr<ServerPlayer> >* players, Level* level,
                                             std::vector<Position*>* positions, bool, TumbleGenerator* generator)
{
    double totalDistance = 0.0;
    unsigned int playerIndex = 0;
    unsigned int positionIndex = 0;
    while (playerIndex < players->size())
    {
        boost::shared_ptr<ServerPlayer> player = players->at(playerIndex);
        Position* position = positions->at(positionIndex++);
        double spawnY;
        bool foundSpawnY = false;
        if (generator == NULL || level->mIsClientSide)
        {
            spawnY = (double)position->getSpawnY(level);
        }
        else
        {
            int layer = gMinecraftServer->mMasterGameMode->GetSpawnLayerIndex();
            AABB* spawnArea = generator->GetSpawnArea((unsigned int)layer);
            spawnY = spawnArea->mMinY;
            for (double y = spawnY; y <= spawnArea->mMaxY; y += 1.0)
            {
                const BlockState* state = level->getBlockState(BlockPos(position->x, y, position->z));
                if (state->getMaterial()->isSolid())
                {
                    foundSpawnY = true;
                    spawnY = y + 1.0;
                }
            }
            if (!foundSpawnY)
            {
                position->randomize(level->mRandom, position->x - 16.0, position->z - 16.0, position->x + 16.0,
                                    position->z + 16.0);
                BlockPos below(BlockPos(position->x, spawnY, position->z).below());
                level->setBlock(below, Blocks::sponge->defaultBlockState(), Block::UPDATE_ALL, false);
                generator->AddAdditionalBlock(below);
            }
        }
        int x = (int)position->x;
        if (position->x < (double)x)
            --x;
        int z = (int)position->z;
        if (position->z < (double)z)
            --z;
        player->teleportTo((double)((float)x + 0.5f), spawnY, (double)z + 0.5, 0.0f, (float)position->y);
        double nearest = 1.7976931348623157e+308;
        for (unsigned int otherIndex = 0; otherIndex < positions->size(); ++otherIndex)
        {
            Position* other = positions->at(otherIndex);
            if (position != other)
            {
                double distance = position->dist(other);
                if (distance < nearest)
                    nearest = distance;
            }
        }
        totalDistance += nearest;
        ++playerIndex;
    }
    return totalDistance / (double)playerIndex;
}

void GameModeUtilities::SpreadPlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players, const AABB* area,
                                      double distance, Level* level, bool teams, TumbleGenerator* generator)
{
    if (players == NULL || area == NULL)
        return;
    Random random;
    unsigned int count = teams ? (unsigned int)getNumberOfTeams(players) : (unsigned int)players->size();
    std::vector<Position*>* positions = createInitialPositions(&random, (int)count, area->mMinX, area->mMinZ,
                                                               area->mMaxX, area->mMaxZ);
    spreadPositions(distance, level, &random, area, positions, teams);
    setPlayerPositions(players, level, positions, teams, generator);
    for (unsigned int i = 0; i < positions->size(); ++i)
        delete positions->at(i);
    delete positions;
}

void GameModeUtilities::DistributePlayersInTeams(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                                 Scoreboard* scoreboard)
{
    std::vector<PlayerTeam*>* teams = scoreboard->getPlayerTeams();
    if (!teams->empty())
    {
        if (!players->empty())
        {
            for (unsigned int i = 0; i < players->size(); ++i)
            {
                PlayerTeam* smallest = NULL;
                for (unsigned int t = 0; t < teams->size(); ++t)
                {
                    PlayerTeam* team = teams->at(t);
                    if (team->getName() != SPECTATOR_TEAM_NAME)
                    {
                        if (smallest == NULL || team->getPlayerCount() < smallest->getPlayerCount())
                            smallest = team;
                    }
                }
                boost::shared_ptr<ServerPlayer> player = players->at(i);
                scoreboard->addPlayerToTeam(player->mXuid, smallest);
            }
        }
        delete teams;
    }
}
