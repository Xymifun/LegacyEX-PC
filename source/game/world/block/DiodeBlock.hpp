#ifndef GAME_WORLD_BLOCK_DIODEBLOCK_HPP
#define GAME_WORLD_BLOCK_DIODEBLOCK_HPP

#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include <boost/shared_ptr.hpp>

class AABB;
class Block;
class BlockPos;
class BlockState;
class Direction;
class Level;
class LevelSource;
class LivingEntity;
class Random;
class TextureAtlasSprite;
class ItemInstance;
template <class T> class not_null_ptr;

class DiodeBlock : public HorizontalDirectionalBlock
{
public:
    DiodeBlock(bool on);
    virtual ~DiodeBlock() {}
    virtual bool isCubeShaped(const BlockState*);
    virtual EBlockRenderShape getRenderShape(const BlockState*);
    virtual const AABB* getShape(const BlockState*, LevelSource*, const BlockPos&);
    virtual bool shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*);
    virtual bool isSolidRender(const BlockState*) const;
    virtual void randomTick(Level*, const BlockPos&, const BlockState*, Random*);
    virtual void tick(Level*, const BlockPos&, const BlockState*, Random*);
    virtual void destroy(Level*, const BlockPos&, const BlockState*);
    virtual void neighborChanged(const BlockState*, Level*, const BlockPos&, Block*, const BlockPos&);
    virtual void onPlace(Level*, const BlockPos&, const BlockState*);
    virtual int getRenderLayer();
    virtual bool mayPlace(Level*, const BlockPos&);
    virtual const BlockState* getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int,
                                                   boost::shared_ptr<LivingEntity>);
    virtual int getSignal(const BlockState*, LevelSource*, const BlockPos&, const Direction*);
    virtual bool isSignalSource(const BlockState*);
    virtual int getDirectSignal(const BlockState*, LevelSource*, const BlockPos&, const Direction*);
    virtual bool canSurvive(Level*, const BlockPos&, const BlockState*);
    virtual void setPlacedBy(Level*, const BlockPos&, const BlockState*, boost::shared_ptr<LivingEntity>,
                             not_null_ptr<ItemInstance>);
    virtual bool isMatching(Block*);
    virtual TextureAtlasSprite* getTexture(const Direction*, const BlockState*);
    virtual bool isOn(const BlockState*);
    virtual void checkTickOnNeighbor(Level*, const BlockPos&, const BlockState*);
    virtual bool isLocked(LevelSource*, const BlockPos&, const BlockState*);
    virtual bool shouldTurnOn(Level*, const BlockPos&, const BlockState*);
    virtual int getInputSignal(Level*, const BlockPos&, const BlockState*);
    virtual int getAlternateSignal(LevelSource*, const BlockPos&, const BlockState*);
    virtual int getAlternateSignalAt(LevelSource*, const BlockPos&, const Direction*);
    virtual void updateNeighborsInFront(Level*, const BlockPos&, const BlockState*);
    virtual bool isAlternateInput(const BlockState*);
    virtual int getOutputSignal(LevelSource*, const BlockPos&, const BlockState*);
    virtual bool isSameDiode(const BlockState*);
    virtual bool shouldPrioritize(Level*, const BlockPos&, const BlockState*);
    virtual int getTurnOffDelay(const BlockState*);
    virtual Item* getResource(const BlockState*, Random*, int) = 0;
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level*, const BlockPos&, const BlockState*) = 0;
    virtual int getTurnOnDelay(const BlockState*) = 0;
    virtual const BlockState* getOnBlock(const BlockState*) = 0;
    virtual const BlockState* getOffBlock(const BlockState*) = 0;

    static bool isDiode(const BlockState*);

protected:
    bool mOn;
};

#endif
