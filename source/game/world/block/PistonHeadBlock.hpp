#ifndef GAME_WORLD_BLOCK_PISTONHEADBLOCK_HPP
#define GAME_WORLD_BLOCK_PISTONHEADBLOCK_HPP

#include <string>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"

class AABB;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Mirror;
class Random;
class Rotation;
class Property;

template <class T> class AbstractProperty;

class PistonHeadBlock : public Block
{
public:
    class PistonType : public StringRepresentable
    {
    public:
        PistonType(const std::wstring& name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

    private:
        std::wstring mName;
    };

    static AbstractProperty<const Direction*>* FACING;
    static AbstractProperty<PistonType*>* TYPE;
    static AbstractProperty<bool>* SHORT;

    PistonHeadBlock();

    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    const AABB* getArmShape(const BlockState* state);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual bool isTopSolidBlocking(const BlockState* state);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    virtual int getResourceCount(Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    static const Direction* getFacing(int data);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mOverrideTopTexture;
};

extern PistonHeadBlock::PistonType* pistonTypeNormal;
extern PistonHeadBlock::PistonType* pistonTypeSticky;

#endif
