#ifndef GAME_WORLD_BLOCK_THINFENCEBLOCK_HPP
#define GAME_WORLD_BLOCK_THINFENCEBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/Material.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Material;
class Mirror;
class Random;
class TextureAtlasSprite;
template <typename T> class AbstractProperty;

extern AbstractProperty<bool>* THIN_FENCE_NORTH;
extern AbstractProperty<bool>* THIN_FENCE_EAST;
extern AbstractProperty<bool>* THIN_FENCE_SOUTH;
extern AbstractProperty<bool>* THIN_FENCE_WEST;

class ThinFenceBlock : public Block
{
public:
    ThinFenceBlock(const std::wstring& name = L"", const std::wstring& edgeTexture = L"",
                   Material* material = Material::METAL, bool extraBlock = false);

    static int indexFor(const Direction* direction);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    static int getAABBIndex(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    static bool isExceptionBlockForAttachingPlusPiston(Block* block);
    bool attachsTo(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool isSilkTouchable();
    virtual int getRenderLayer();
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getEdgeTexture();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    bool mDropsOnBreak;
    std::wstring mEdgeTexture;
    std::wstring mName;
    TextureAtlasSprite* mEdgeTextureId;
};

#endif
