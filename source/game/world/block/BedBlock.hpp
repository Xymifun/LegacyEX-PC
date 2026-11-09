#ifndef GAME_WORLD_BLOCK_BEDBLOCK_HPP
#define GAME_WORLD_BLOCK_BEDBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/EntityBlock.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"

class BlockEntity;
class Block;
class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Entity;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Mirror;
class Player;
class Random;
class Rotation;
class Item;
class ItemInstance;

class BedBlock : public HorizontalDirectionalBlock, public EntityBlock
{
public:
    class Part : public StringRepresentable
    {
    public:
        Part(int index, const std::wstring& name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        int getIndex() const;

        std::wstring mName;
        int mIndex;
    };

    static AbstractProperty<const Part*>* PART;
    static AbstractProperty<bool>* OCCUPIED;

    BedBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void blockStaticCtor();
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool TestUse(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player);
    boost::shared_ptr<Player> getSleepingPlayerAt(Level* level, const BlockPos& pos);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual void fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance);
    virtual void updateEntityAfterFallOn(Level* level, boost::shared_ptr<Entity> entity);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool hasCustomBreakingProgress(const BlockState* state);
    static bool BlockIsDoor(const Block* block);
    static bool hasSolidGroundAndTwoBlocksSpaceAt(Level* level, const BlockPos& pos);
    static BlockPos* findStandUpPosition(Level* level, const BlockPos& pos, int data);
    using Block::spawnResources;
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual int getRenderLayer();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    static bool isHeadPiece(int data);
    virtual void registerIcons(IconRegister* iconRegister);
};

extern BedBlock::Part& gBedHead;
extern BedBlock::Part& gBedFoot;

#endif
