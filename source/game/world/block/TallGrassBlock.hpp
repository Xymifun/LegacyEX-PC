#ifndef GAME_WORLD_BLOCK_TALLGRASSBLOCK_HPP
#define GAME_WORLD_BLOCK_TALLGRASSBLOCK_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/Bush.hpp"
#include "game/core/StringRepresentable.hpp"
#include "game/world/block/BonemealableBlock.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class ItemInstance;
class Level;
class LevelSource;
class Player;
class Random;
class TextureAtlasSprite;

template <class T>
class AbstractProperty;

class TallGrassBlock : public Bush, public BonemealableBlock
{
public:
    class Variant : public StringRepresentable
    {
    public:
        Variant(int data, const std::wstring& name);

        int getData();
        static Variant* byData(int data);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

        static Variant* DEAD_BUSH;
        static Variant* GRASS;
        static Variant* FERN;

    private:
        int mData;
        std::wstring mName;
    };

    TallGrassBlock();
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual int getColor() const;
    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCountForLootBonus(int lootBonus, Random* random);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockOffsetType getOffsetType();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

    static AbstractProperty<Variant*>* VARIANT;
    static Variant* VARIANTS[3];

private:
    TextureAtlasSprite* mIcons[3];
};


#endif
