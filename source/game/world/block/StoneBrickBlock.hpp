#ifndef GAME_WORLD_BLOCK_STONEBRICKBLOCK_HPP
#define GAME_WORLD_BLOCK_STONEBRICKBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"

class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class LevelSource;
class TextureAtlasSprite;
template <class T> class EnumProperty;

class StoneBrickBlock : public Block
{
public:
    class Variant : public StringRepresentable
    {
    public:
        enum Data
        {
            eDataDefault = 0,
            eDataMossy = 1,
            eDataCracked = 2,
            eDataChiseled = 3
        };

        static Variant* DEFAULT;
        static Variant* MOSSY;
        static Variant* CRACKED;
        static Variant* CHISELED;
        static Variant* BY_DATA[4];

        Variant(int data, const std::wstring& name, int translationKey);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();
        int getData();
        int getTranslationKey();
        static Variant* byData(int data);

    private:
        int mData;
        std::wstring mName;
        int mTranslationKey;
    };

    static EnumProperty<Variant*>* VARIANT;
    static std::wstring TEXTURE_NAMES[4];
    static int DEFAULT_META;
    static int MOSSY_META;
    static int CRACKED_META;
    static int CHISELED_META;

    StoneBrickBlock();
    static void blockStaticCtor();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int getDescriptionId(int auxValue);

private:
    TextureAtlasSprite* mTextures[4];
};

#endif
