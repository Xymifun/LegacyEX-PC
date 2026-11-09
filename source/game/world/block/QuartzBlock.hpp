#ifndef GAME_WORLD_BLOCK_QUARTZBLOCK_HPP
#define GAME_WORLD_BLOCK_QUARTZBLOCK_HPP

#include <string>

#include "game/core/Direction.hpp"
#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class IconRegister;
class Level;
class LevelSource;
class LivingEntity;
class MaterialColor;
class Rotation;
class TextureAtlasSprite;

template <class T> class EnumProperty;

class QuartzBlock : public Block
{
public:
    class Variant : public StringRepresentable
    {
    public:
        static Variant* DEFAULT;
        static Variant* CHISELED;
        static Variant* LINES_Y;
        static Variant* LINES_X;
        static Variant* LINES_Z;

        Variant(int data, const std::wstring& serializedName, const std::wstring& name);

        int getData();
        static Variant* byData(int data);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring getName();

    private:
        int mData;
        std::wstring mSerializedName;
        std::wstring mName;
    };

    static EnumProperty<Variant*>* VARIANT;

    QuartzBlock();

    static void blockStaticCtor();
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mTextures[9];
};

#endif
