#ifndef GAME_WORLD_BLOCK_WALLBLOCK_HPP
#define GAME_WORLD_BLOCK_WALLBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/EnumProperty.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class TextureAtlasSprite;

class WallBlock : public Block
{
public:
    class Variant : public StringRepresentable
    {
    public:
        static Variant* COBBLESTONE;
        static Variant* MOSSY;
        static Variant* VALUES[2];

        Variant(int data, const std::wstring& name, int translationKey);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        int getData();
        static Variant* byData(int data);
        int getTranslationKey();

        int mData;
        std::wstring mName;
        int mTranslationKey;
    };

    static EnumProperty<Variant*>* VARIANT;
    static const double POST_HEIGHT;
    static const float ARM_HALF_WIDTH;
    static const float ARM_HEIGHT;
    static const float POST_HALF_WIDTH;

    WallBlock(Block* block);
    static void blockStaticCtor();
    static unsigned int getAABBIndex(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual std::wstring getName();
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool canContainLiquid(const BlockState* state) const;
    static bool isExceptionBlockForAttachingPlusPiston(Block* block);
    bool connectsTo(LevelSource* levelSource, const BlockPos& pos, const Direction* direction);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
