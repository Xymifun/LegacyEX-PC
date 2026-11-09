#ifndef GAME_WORLD_BLOCK_PRISMARINEBLOCK_HPP
#define GAME_WORLD_BLOCK_PRISMARINEBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/EnumProperty.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class LevelSource;
class MaterialColor;
class TextureAtlasSprite;

class PrismarineBlock : public Block
{
public:
    class Variant : public StringRepresentable
    {
    public:
        static Variant* ROUGH;
        static Variant* BRICKS;
        static Variant* DARK;

        Variant(int data, const std::wstring& name, int nameId, int descriptionId, const std::wstring& serializedName);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        int getData();
        static Variant* byData(int data);
        TextureAtlasSprite* getTexture();
        void registerIcons(IconRegister* iconRegister);
        int getNameId();
        int getDescriptionId();

        int mData;
        std::wstring mName;
        int mNameId;
        int mDescriptionId;
        std::wstring mSerializedName;
        TextureAtlasSprite* mTexture;
    };

    static EnumProperty<Variant*>* VARIANT;
    static int ROUGH_DATA;
    static int BRICKS_DATA;
    static int DARK_DATA;

    PrismarineBlock();
    static void blockStaticCtor();
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual std::wstring getName();
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
};

#endif
