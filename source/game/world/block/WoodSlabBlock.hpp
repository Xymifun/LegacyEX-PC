#ifndef GAME_WORLD_BLOCK_WOODSLABBLOCK_HPP
#define GAME_WORLD_BLOCK_WOODSLABBLOCK_HPP

#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Property;
class Random;
class TextureAtlasSprite;

class WoodSlabBlock : public HalfSlabBlock
{
public:
    WoodSlabBlock();
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getAuxName(int auxValue);
    virtual Property* getTypeProperty();
    virtual Boxed* getTypeForItem(not_null_ptr<ItemInstance> itemInstance);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int getDescriptionId(int auxValue);
    virtual ~WoodSlabBlock() {}
    virtual void DerivedInit();
    static void blockStaticCtor();

    static EnumProperty<PlanksBlock::Variant*>* VARIANT;
};

#endif
