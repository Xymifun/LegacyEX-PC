#ifndef GAME_WORLD_BLOCK_HUGEMUSHROOMBLOCK_HPP
#define GAME_WORLD_BLOCK_HUGEMUSHROOMBLOCK_HPP

#include <string>

#include "game/world/block/Block.hpp"
#include "game/core/Direction.hpp"

class BlockState;
class BlockStateDefinition;
template <typename T>
class EnumProperty;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Mirror;
class Random;
class Rotation;
class TextureAtlasSprite;

class HugeMushroomBlock : public Block
{
public:
    class Variant : public StringRepresentable
    {
    public:
        Variant(int data, const std::wstring& name);
        int getData();
        static Variant* byData(int data);

        static Variant* NORTH_WEST;
        static Variant* NORTH;
        static Variant* NORTH_EAST;
        static Variant* WEST;
        static Variant* CENTER;
        static Variant* EAST;
        static Variant* SOUTH_WEST;
        static Variant* SOUTH;
        static Variant* SOUTH_EAST;
        static Variant* STEM;
        static Variant* ALL_INSIDE;
        static Variant* ALL_OUTSIDE;
        static Variant* ALL_STEM;
        static Variant* VALUES[13];

        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

    private:
        int mData;
        std::wstring mName;
    };

    HugeMushroomBlock(Material* material, const MaterialColor* materialColor, Block* otherBlock);

    static void blockStaticCtor();
    static EnumProperty<Variant*>* VARIANT;
    static const std::wstring TEXTURE_STEM;
    static const std::wstring TEXTURE_INSIDE;
    static const std::wstring TEXTURE_TYPE[2];

    virtual int getResourceCount(Random* random);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    Block* mOtherBlock;
    int mIsOtherBlock;
    TextureAtlasSprite* mTextures[2];
    TextureAtlasSprite* mStemTexture;
    TextureAtlasSprite* mInsideTexture;
};

int getMushroomDataValue(HugeMushroomBlock::Variant* variant);

#endif
