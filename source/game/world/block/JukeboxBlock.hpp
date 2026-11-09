#ifndef GAME_WORLD_BLOCK_JUKEBOXBLOCK_HPP
#define GAME_WORLD_BLOCK_JUKEBOXBLOCK_HPP
#include <boost/shared_ptr.hpp>
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"
#include "game/world/block/BlockEntity.hpp"
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class CompoundTag;
class DataFixerUpper;
class Direction;
class IconRegister;
class Level;
class Player;
class TextureAtlasSprite;
template <class T> class AbstractProperty;
class JukeboxBlock : public BaseEntityBlock
{
public:
    class Entity : public BlockEntity
    {
    public:
        Entity();
        static BlockEntity* create();
        virtual void load(CompoundTag* tag);
        virtual CompoundTag* save(CompoundTag* tag);
        virtual unsigned long long GetType();
        virtual boost::shared_ptr<BlockEntity> clone();
        virtual not_null_ptr<ItemInstance> getRecord();
        virtual void setRecord(not_null_ptr<ItemInstance> record);

    private:
        not_null_ptr<ItemInstance> mRecord;
    };
    static AbstractProperty<bool>* HAS_RECORD;

    JukeboxBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual bool TestUse(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player);
    void dropRecording(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    void setRecord(Level* level, const BlockPos& pos, const BlockState* state, not_null_ptr<ItemInstance> record);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mTopIcon;
};
#endif
