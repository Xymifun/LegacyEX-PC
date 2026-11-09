#ifndef GAME_WORLD_BLOCK_NEWSTONESLABBLOCK_HPP
#define GAME_WORLD_BLOCK_NEWSTONESLABBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/HalfSlabBlock.hpp"

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Random;
class TextureAtlasSprite;

class NewStoneSlabBlock : public HalfSlabBlock
{
public:
    class Variant : public StringRepresentable
    {
    public:
        Variant(int data, const std::wstring& name, int translationKey, const MaterialColor* materialColor);
        static Variant* byData(int data);

        static Variant* RED_SANDSTONE;
        int getData();
        const MaterialColor* getMaterialColor();
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        int getTranslationKey();

    private:
        int mData;
        std::wstring mName;
        int mTranslationKey;
        const MaterialColor* mMaterialColor;
    };

    NewStoneSlabBlock();
    virtual ~NewStoneSlabBlock() {}
    virtual void DerivedInit();
    static void blockStaticCtor();

    static EnumProperty<Variant*>* VARIANT;
    virtual std::wstring getName();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getAuxName(int auxValue);
    virtual Property* getTypeProperty();
    virtual Boxed* getTypeForItem(not_null_ptr<ItemInstance> itemInstance);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mSideTexture;
};

#endif
