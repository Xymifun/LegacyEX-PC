#ifndef GAME_WORLD_BLOCK_DIRTBLOCK_HPP
#define GAME_WORLD_BLOCK_DIRTBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BonemealableBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Random;
class TextureAtlasSprite;

template <class T> class EnumProperty;

class DirtBlock : public Block, public BonemealableBlock
{
public:
    class Variant : public StringRepresentable
    {
    public:
        Variant(int data, const std::wstring& name, int nameId, int descriptionId, const MaterialColor* materialColor,
                const std::wstring& texture, const std::wstring& topTexture, const std::wstring& sideTexture);

        const MaterialColor* getMaterialColor();
        int getData();
        static Variant* byData(int data);

        static Variant* DIRT;
        static Variant* COARSE_DIRT;
        static Variant* PODZOL;

        TextureAtlasSprite* getTexture(const Direction* direction);
        void registerIcons(IconRegister* iconRegister);
        int getNameId();
        int getDescriptionId();
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();

    private:
        int mData;
        std::wstring mName;
        int mNameId;
        int mDescriptionId;
        const MaterialColor* mMaterialColor;
        std::wstring mTexture;
        std::wstring mTopTexture;
        std::wstring mSideTexture;
        TextureAtlasSprite* mIcon;
        TextureAtlasSprite* mTopIcon;
        TextureAtlasSprite* mSideIcon;
    };

    DirtBlock();
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    static void blockStaticCtor();

    static EnumProperty<Variant*>* VARIANT;

    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
};

#endif
