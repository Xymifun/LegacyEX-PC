#ifndef GAME_WORLD_BLOCK_SANDSTONEBLOCK_HPP
#define GAME_WORLD_BLOCK_SANDSTONEBLOCK_HPP

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"

#include <string>

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class LevelSource;
class MaterialColor;
class TextureAtlasSprite;

class SandStoneBlock : public Block
{
public:
    class Variant : public StringRepresentable
    {
    public:
        Variant(int data, const std::wstring& name, int translationKey);
        int getData();
        static Variant* byData(int data);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        int getTranslationKey();
        std::wstring getName();

    private:
        int mData;
        std::wstring mName;
        int mTranslationKey;
    };

    typedef Variant Type;
    static AbstractProperty<Variant*>* TYPE;
    static Variant* SANDSTONE;
    static Variant* CHISELED;
    static Variant* SMOOTH;

    SandStoneBlock();
    static void blockStaticCtor();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mTextures[3];
    TextureAtlasSprite* mTopTexture;
    TextureAtlasSprite* mBottomTexture;
};

#endif
