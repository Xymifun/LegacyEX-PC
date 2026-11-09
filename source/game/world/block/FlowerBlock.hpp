#ifndef GAME_WORLD_BLOCK_FLOWERBLOCK_HPP
#define GAME_WORLD_BLOCK_FLOWERBLOCK_HPP

#include <string>
#include <utility>

#include "game/core/Bush.hpp"
#include "game/core/StringRepresentable.hpp"
#include "game/world/block/BonemealableBlock.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Predicate.hpp"

class BlockTooltipDataHolder;
class IconRegister;
class MutableBlockPos;
class TextureAtlasSprite;

class FlowerBlock : public Bush, public BonemealableBlock
{
public:
    class FlowerCategory;

    class Flower : public StringRepresentable
    {
    public:
        enum EFlower
        {
            FLOWER_DANDELION = 0,
            FLOWER_POPPY = 1,
            FLOWER_BLUE_ORCHID = 2,
            FLOWER_ALLIUM = 3,
            FLOWER_HOUSTONIA = 4,
            FLOWER_RED_TULIP = 5,
            FLOWER_ORANGE_TULIP = 6,
            FLOWER_WHITE_TULIP = 7,
            FLOWER_PINK_TULIP = 8,
            FLOWER_OXEYE_DAISY = 9
        };

        enum
        {
            FLOWER_COUNT = 10,
            YELLOW_FLOWER_COUNT = 1,
            RED_FLOWER_COUNT = 9
        };

        Flower(FlowerCategory* category, int data, const std::wstring& name, int nameId, int descriptionId,
               const std::wstring& iconName, EFlower id);

        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

        std::wstring getName() const;
        FlowerCategory* getCategory() const;
        int getData() const;
        EFlower getId() const;
        int getNameId() const;
        int getDescriptionId() const;
        TextureAtlasSprite* getTexture() const;
        void registerIcons(IconRegister* iconRegister);

        static void staticCtor();
        static std::pair<Flower**, int> byCategory(const FlowerCategory* category);
        static Flower* byData(const FlowerCategory* category, int data);

        static Flower* FLOWERS[FLOWER_COUNT];
        static Flower* YELLOW_FLOWERS[YELLOW_FLOWER_COUNT];
        static Flower* RED_FLOWERS[RED_FLOWER_COUNT];
        static Flower* DANDELION;
        static Flower* POPPY;
        static Flower* BLUE_ORCHID;
        static Flower* ALLIUM;
        static Flower* HOUSTONIA;
        static Flower* RED_TULIP;
        static Flower* ORANGE_TULIP;
        static Flower* WHITE_TULIP;
        static Flower* PINK_TULIP;
        static Flower* OXEYE_DAISY;

        FlowerCategory* mCategory;
        int mData;
        std::wstring mName;
        int mNameId;
        int mDescriptionId;
        std::wstring mIconName;
        TextureAtlasSprite* mIcon;
        EFlower mId;
    };

    class FlowerCategory
    {
    public:
        Block* getBlock();

        static FlowerCategory* YELLOW;
        static FlowerCategory* RED;
    };

    class FlowerCategoryPredicate : public Predicate<FlowerBlock::Flower*>
    {
    public:
        FlowerCategoryPredicate(FlowerBlock* block);

        virtual bool apply(FlowerBlock::Flower* flower) const;

    private:
        FlowerBlock* mBlock;
    };

    FlowerBlock();

    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual ~FlowerBlock() {}
    virtual void DerivedInit();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockOffsetType getOffsetType();
    virtual FlowerCategory* getFlowerCategory() = 0;
    virtual AbstractProperty<Flower*>* getFlowerProperty();
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);

    bool performBonemeal(bool checkOnly, Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    bool randomWalk(Level* level, MutableBlockPos& pos, int distance) const;

    AbstractProperty<Flower*>* mTypeProperty;
};

#endif
