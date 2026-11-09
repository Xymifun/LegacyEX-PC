#ifndef GAME_WORLD_BLOCK_DEADBUSHBLOCK_HPP
#define GAME_WORLD_BLOCK_DEADBUSHBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Bush.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockEntity;
class BlockPos;
class BlockState;
class Level;
class LevelSource;
class MaterialColor;
class Random;
class ItemInstance;
class Player;

class DeadBushBlock : public Bush
{
public:
    DeadBushBlock();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
};

#endif
