#ifndef GAME_WORLD_BLOCK_DETECTORRAILBLOCK_HPP
#define GAME_WORLD_BLOCK_DETECTORRAILBLOCK_HPP

#include <typeinfo>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BaseRailBlock.hpp"

template <typename T>
class EnumProperty;

class AABB;
class BlockPos;
class BlockState;
class BooleanProperty;
class BlockStateDefinition;
class Direction;
class Entity;
class IconRegister;
class Level;
class LevelSource;
class Mirror;
class Property;
class Random;
class Rotation;
class TextureAtlasSprite;

template <class T> class Predicate;

class DetectorRailBlock : public BaseRailBlock
{
public:
    DetectorRailBlock();
    static void blockStaticCtor();
    virtual int getTickDelay(Level* level);
    virtual bool isSignalSource(const BlockState* state);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual void randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    AABB* getSearchBB(const BlockPos& pos);
    std::vector<boost::shared_ptr<Entity> >* getInteractingMinecartOfType(
        Level* level, const BlockPos& pos, const std::type_info& type,
        const Predicate<boost::shared_ptr<Entity> >* predicate);
    virtual void checkPressed(Level* level, const BlockPos& pos, const BlockState* state);
    void updatePowerToConnected(Level* level, const BlockPos& pos, const BlockState* state, bool powered);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const AbstractProperty<BaseRailBlock::RailShape*>* getShapeProperty();
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

    static EnumProperty<BaseRailBlock::RailShape*>* SHAPE;
    static BooleanProperty* POWERED;

private:
    TextureAtlasSprite* mIcons[2];
};

#endif
