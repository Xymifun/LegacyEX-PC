#ifndef GAME_WORLD_BLOCK_TNTBLOCK_HPP
#define GAME_WORLD_BLOCK_TNTBLOCK_HPP

#include "game/world/block/Block.hpp"

template <class T> class AbstractProperty;

class TntBlock : public Block
{
public:
    static AbstractProperty<bool>* EXPLODE;

    TntBlock();

    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void wasExploded(Level* level, const BlockPos& pos, Explosion* explosion);
    virtual void destroy(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void destroy(Level* level, const BlockPos& pos, const BlockState* state,
                         boost::shared_ptr<LivingEntity> entity);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual bool dropFromExplosion(Explosion* explosion);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mBottomIcon;
};

#endif
