#ifndef GAME_WORLD_BLOCK_WOOLCARPETBLOCK_HPP
#define GAME_WORLD_BLOCK_WOOLCARPETBLOCK_HPP

#include "game/world/block/Block.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class DyeColor;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class TextureAtlasSprite;

template <class T> class EnumProperty;

class WoolCarpetBlock : public Block
{
public:
    WoolCarpetBlock();

    static void blockStaticCtor();

    virtual void addCollisionAABB(const BlockPos& pos, const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                  const AABB* blockBox);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    bool checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual bool hasInHandRenderOffset();
    virtual bool waterSpreadCausesSpawn();
};

extern EnumProperty<const DyeColor*>* gWoolCarpetColorProperty;

#endif
