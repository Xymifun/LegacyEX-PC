#ifndef GAME_WORLD_BLOCK_PRISMARINESLABBLOCK_HPP
#define GAME_WORLD_BLOCK_PRISMARINESLABBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/HalfSlabBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Boxed;
class Direction;
class IconRegister;
class Item;
class Level;
class LevelSource;
class MaterialColor;
class Property;
class Random;
class TextureAtlasSprite;
class ItemInstance;

template <class T> class EnumProperty;

class PrismarineSlabBlock : public HalfSlabBlock
{
public:
    class Variant : public StringRepresentable
    {
    public:
        static Variant* ROUGH;
        static Variant* BRICKS;
        static Variant* DARK;

        Variant(int data, const std::wstring& name, int nameId, int descriptionId, const std::wstring& serializedName);
        int getData();
        static Variant* byData(int data);
        int getNameId();
        void registerIcons(IconRegister* iconRegister);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        int getDescriptionId();
        TextureAtlasSprite* getTexture();

        int mData;
        std::wstring mName;
        int mNameId;
        int mDescriptionId;
        std::wstring mSerializedName;
        TextureAtlasSprite* mTexture;
    };

    static EnumProperty<Variant*>* VARIANT;

    PrismarineSlabBlock();
    virtual ~PrismarineSlabBlock() {}
    static void blockStaticCtor();
    virtual void DerivedInit();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual int getAuxName(int auxValue);
    virtual Property* getTypeProperty();
    virtual Boxed* getTypeForItem(not_null_ptr<ItemInstance> itemInstance);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
};

#endif
