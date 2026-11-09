#ifndef GAME_CORE_SEAPICKLE_HPP
#define GAME_CORE_SEAPICKLE_HPP

#include "game/core/Bush.hpp"
#include "game/world/block/BonemealableBlock.hpp"

#include <string>

template <class T> class AbstractProperty;

class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Level;
class LevelSource;
class Random;
class Block;
class BlockEntity;
class Item;
class ItemInstance;

class SeaPickle : public Bush, public BonemealableBlock
{
public:
    SeaPickle();

    virtual bool isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool);
    virtual bool isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*);
    virtual void performBonemeal(Level*, Random*, const BlockPos&, const BlockState*);
    bool performBonemeal(bool, Level*, Random*, const BlockPos&, const BlockState*);
    virtual int getColor(const BlockState*);
    virtual int getColor(LevelSource*, const BlockPos&, const BlockState*);
    virtual const MaterialColor* getMapColor(const BlockState*, LevelSource*, const BlockPos&);
    virtual EBlockRenderShape getRenderShape(const BlockState*);
    virtual Item* getResource(const BlockState*, Random*, int);
    virtual void spawnResources(Level*, const BlockPos&, const BlockState*, float, int);
    virtual int getLightEmission(const BlockState*);
    virtual void playerDestroy(Level*, boost::shared_ptr<Player>, const BlockPos&, const BlockState*,
                               boost::shared_ptr<BlockEntity>, not_null_ptr<ItemInstance>);
    virtual bool canSurvive(Level*, const BlockPos&, const BlockState*);
    virtual const AABB* getShape(const BlockState*, LevelSource*, const BlockPos&);
    virtual bool mayReplaceWithPlace(LevelSource*, const BlockPos&);
    virtual bool canContainLiquid(const BlockState*) const;
    virtual bool waterSpreadCausesSpawn();
    virtual std::wstring getTileItemIconName();
    virtual int convertBlockStateToLegacyData(const BlockState*);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder&);
    virtual const BlockState* getBlockState(int);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool isSilkTouchable();
    virtual bool mayPlaceOn(Level*, const BlockPos&, const BlockState*);
    virtual void onPlace(Level*, const BlockPos&, const BlockState*);
    virtual void checkAlive(Level*, const BlockPos&, const BlockState*);
    virtual void neighborChanged(const BlockState*, Level*, const BlockPos&, Block*, const BlockPos&);
    virtual void tick(Level*, const BlockPos&, const BlockState*, Random*);
    virtual bool canProvideSupport(const BlockState*, const Direction*, EBlockSupportType);

    static AbstractProperty<int>* COUNT;
    static AbstractProperty<int>* FLANGE;
};

#endif
