#ifndef GAME_WORLD_BLOCK_COMMANDBLOCK_HPP
#define GAME_WORLD_BLOCK_COMMANDBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class Block;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LivingEntity;
class MaterialColor;
class Mirror;
class Player;
class Random;
class Rotation;
class TextureAtlasSprite;
class ItemInstance;

class CommandBlock : public BaseEntityBlock
{
public:
    enum Mode
    {
        IMPULSE = 0,
        REPEAT = 1,
        CHAIN = 2
    };

    static AbstractProperty<const Direction*>* FACING;
    static AbstractProperty<bool>* CONDITIONAL;

    CommandBlock(const MaterialColor* materialColor, Mode mode);
    unsigned int getMode();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void blockStaticCtor();
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual int getTickDelay(Level* level);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual int getResourceCount(Random* random);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    static bool isCommandBlock(Block* block);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mFrontIcon;
    TextureAtlasSprite* mBackIcon;
    TextureAtlasSprite* mConditionalIcon;
    Mode mMode;
};

#endif
