#ifndef GAME_WORLD_BLOCK_SKULLBLOCK_HPP
#define GAME_WORLD_BLOCK_SKULLBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

template <class T> class AbstractProperty;
class AABB;
class BlockEntity;
class BlockPattern;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class DirectionProperty;
class IconRegister;
class Level;
class LevelSource;
class LivingEntity;
class Mirror;
class Player;
class Random;
class Rotation;
class SkullBlockEntity;
class TextureAtlasSprite;
class Item;
class ItemInstance;
class WitherBoss;

class SkullBlock : public BaseEntityBlock
{
public:
    static DirectionProperty* FACING;
    static AbstractProperty<bool>* NODROP;

    SkullBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    BlockPattern* getOrCreateWitherFull();
    void checkMobSpawn(Level* level, const BlockPos& pos, boost::shared_ptr<SkullBlockEntity> skull,
                       boost::shared_ptr<Player> player);
    static void blockStaticCtor();
    virtual std::wstring getName();
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool hasCustomBreakingProgress(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    BlockPattern* getOrCreateWitherBase();
    bool canSpawnMob(Level* level, const BlockPos& pos, not_null_ptr<ItemInstance> item);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual std::wstring getTileItemIconName();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    BlockPattern* mWitherBase;
    BlockPattern* mWitherFull;
};

#endif
