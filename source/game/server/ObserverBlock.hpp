#ifndef GAME_SERVER_OBSERVERBLOCK_HPP
#define GAME_SERVER_OBSERVERBLOCK_HPP

#include "game/world/block/DirectionalBlock.hpp"

#include <boost/shared_ptr.hpp>

class Block;
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
class Property;
class Random;
class Rotation;
class TextureAtlasSprite;
template <typename T> class AbstractProperty;

class ObserverBlock : public DirectionalBlock
{
public:
    static AbstractProperty<bool>* POWERED;

    ObserverBlock();
    virtual ~ObserverBlock() {}

    virtual bool TestUse();
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    void updateNeighborsInFront(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    void startSignal(const BlockState* state, Level* level, const BlockPos& pos);
    void observedNeighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool isSignalSource(const BlockState* state);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mFrontIcon;
    TextureAtlasSprite* mSideIcon;
    TextureAtlasSprite* mBackIcon;
    TextureAtlasSprite* mBackLitIcon;
    TextureAtlasSprite* mTopIcon;
};

#endif
