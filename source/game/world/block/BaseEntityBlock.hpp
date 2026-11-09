#ifndef GAME_WORLD_BLOCK_BASEENTITYBLOCK_HPP
#define GAME_WORLD_BLOCK_BASEENTITYBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/EntityBlock.hpp"

class BlockPos;
class BlockState;
class Direction;
class Level;
class Material;
class MaterialColor;
class BlockEntity;
class Player;

class BaseEntityBlock : public Block, public EntityBlock
{
public:
    BaseEntityBlock(Material* material);
    BaseEntityBlock(Material* material, const MaterialColor* materialColor);
    virtual ~BaseEntityBlock() {}

    bool isCactus(Level* level, const BlockPos& pos, const Direction* direction);
    bool touchesCactus(Level* level, const BlockPos& pos);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual bool triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId, int eventParam);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
};

#endif
