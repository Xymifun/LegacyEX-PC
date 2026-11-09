#ifndef GAME_CORE_SEAGRASS_HPP
#define GAME_CORE_SEAGRASS_HPP

#include "game/core/Bush.hpp"
#include "game/world/block/BonemealableBlock.hpp"

template <class T> class AbstractProperty;
class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;
class ItemInstance;
class BlockEntity;
class Player;

class SeaGrass : public Bush, public BonemealableBlock
{
public:
    enum EType
    {
        TYPE_SINGLE = 0,
        TYPE_TALL_TOP = 1,
        TYPE_TALL_BOTTOM = 2
    };

    static AbstractProperty<int>* TYPE;
    static const AABB* SHAPE;

    SeaGrass();
    virtual bool isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool);
    virtual bool isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*);
    virtual void performBonemeal(Level*, Random*, const BlockPos&, const BlockState*);
    virtual void tick(Level*, const BlockPos&, const BlockState*, Random*);
    virtual void registerIcons(IconRegister*);
    virtual TextureAtlasSprite* getTexture(const Direction*, const BlockState*);
    virtual EBlockRenderShape getRenderShape(const BlockState*);
    virtual int getColor(const BlockState*);
    virtual int getColor(LevelSource*, const BlockPos&, const BlockState*);
    virtual const MaterialColor* getMapColor(const BlockState*, LevelSource*, const BlockPos&);
    virtual Item* getResource(const BlockState*, Random*, int);
    virtual const BlockState* getBlockState(int);
    virtual int convertBlockStateToLegacyData(const BlockState*);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder&);
    virtual EBlockOffsetType getOffsetType();
    virtual const AABB* getShape(const BlockState*, LevelSource*, const BlockPos&);
    virtual void neighborChanged(const BlockState*, Level*, const BlockPos&, Block*, const BlockPos&);
    bool mayPlace(Level*, const BlockPos&, bool);
    bool canSurvive(Level*, const BlockPos&, const BlockState*, bool);
    virtual bool canSurvive(Level*, const BlockPos&, const BlockState*);
    virtual bool mayPlace(Level*, const BlockPos&);
    virtual void onPlace(Level*, const BlockPos&, const BlockState*);
    virtual void playerDestroy(Level*, boost::shared_ptr<Player>, const BlockPos&, const BlockState*,
                               boost::shared_ptr<BlockEntity>, not_null_ptr<ItemInstance>);
    static bool trySpawnSeaGrass(bool, Level*, Random*, const BlockPos&, const BlockState*);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool isSilkTouchable();

private:
    TextureAtlasSprite* mDoubleTallTopIcon;
    TextureAtlasSprite* mDoubleTallBottomIcon;
};

#endif
