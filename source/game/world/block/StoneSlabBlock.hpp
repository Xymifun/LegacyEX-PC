#ifndef GAME_WORLD_BLOCK_STONESLABBLOCK_HPP
#define GAME_WORLD_BLOCK_STONESLABBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/HalfSlabBlock.hpp"

class BlockState;
template <typename T>
class EnumProperty;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Random;
class TextureAtlasSprite;

class StoneSlabBlock : public HalfSlabBlock
{
public:
    class Variant : public StringRepresentable
    {
    public:
        static Variant* STONE;
        static Variant* SAND;
        static Variant* WOOD;
        static Variant* COBBLESTONE;
        static Variant* BRICK;
        static Variant* SMOOTHBRICK;
        static Variant* NETHERBRICK;
        static Variant* QUARTZ;

        Variant(int data, const MaterialColor* materialColor, const std::wstring& name, int translationKey);
        int getData();
        int getTranslationKey();
        static Variant* byData(int data);
        const MaterialColor* getColor();
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();

    private:
        int mData;
        std::wstring mName;
        int mTranslationKey;
        const MaterialColor* mColor;
    };

    StoneSlabBlock();
    static void blockStaticCtor();
    static EnumProperty<Variant*>* VARIANT;
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual ~StoneSlabBlock() {}
    virtual void DerivedInit();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getAuxName(int auxValue);
    virtual Property* getTypeProperty();
    virtual Boxed* getTypeForItem(not_null_ptr<ItemInstance> itemInstance);

private:
    TextureAtlasSprite* mSideTexture;
};

#endif
