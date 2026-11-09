#ifndef GAME_WORLD_BLOCK_SANDBLOCK_HPP
#define GAME_WORLD_BLOCK_SANDBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/BonemealableBlock.hpp"
#include "game/world/block/FallingBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class TextureAtlasSprite;
template <class T> class AbstractProperty;

class SandBlock : public FallingBlock, public BonemealableBlock
{
public:
    class Variant : public StringRepresentable
    {
    public:
        static Variant* SAND;
        static Variant* RED_SAND;

        Variant(int data, const std::wstring& name, int nameId, int descriptionId, const MaterialColor* materialColor,
                int dustColor);

        const MaterialColor* getMaterialColor();
        int getData();
        static Variant* byData(int data);
        int getDustColor();
        std::wstring getName();
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        int getNameId();
        int getDescriptionId();

    private:
        int mData;
        std::wstring mName;
        const MaterialColor* mMaterialColor;
        int mNameId;
        int mDescriptionId;
        int mDustColor;
    };

    typedef Variant Type;
    static AbstractProperty<Variant*>* TYPE;

    SandBlock();
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    static void blockStaticCtor();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual unsigned int getDustColor(const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mTextures[2];
};

#endif
