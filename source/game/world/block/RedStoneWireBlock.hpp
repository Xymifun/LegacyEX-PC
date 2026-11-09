#ifndef GAME_WORLD_BLOCK_REDSTONEWIREBLOCK_HPP
#define GAME_WORLD_BLOCK_REDSTONEWIREBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"

#include <string>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>

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
class TextureAtlasSprite;

class RedStoneWireBlock : public Block
{
public:
    static const std::wstring TEXTURE_CROSS;
    static const std::wstring TEXTURE_LINE;
    static const std::wstring TEXTURE_CROSS_OVERLAY;
    static const std::wstring TEXTURE_LINE_OVERLAY;
    static AbstractProperty<int>* POWER;

    class ConnectingSide : public StringRepresentable
    {
    public:
        explicit ConnectingSide(const std::wstring& name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

        std::wstring mName;
    };

    static AbstractProperty<ConnectingSide*>* NORTH;
    static AbstractProperty<ConnectingSide*>* EAST;
    static AbstractProperty<ConnectingSide*>* SOUTH;
    static AbstractProperty<ConnectingSide*>* WEST;
    static ConnectingSide* UP;
    static ConnectingSide* SIDE;
    static ConnectingSide* NONE;

    RedStoneWireBlock();
    static void blockStaticCtor();
    int getAABBIndex(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    static bool shouldConnectTo(const BlockState* state, const Direction* direction);
    static bool shouldConnectTo(const BlockState* state);
    ConnectingSide* getConnectingSide(LevelSource* levelSource, const BlockPos& pos, const Direction* direction);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    int checkTarget(Level* level, const BlockPos& pos, int strength);
    const BlockState* updatePowerStrength(Level* level, const BlockPos& pos, const BlockPos& sourcePos,
                                          const BlockState* state);
    const BlockState* updatePowerStrength(Level* level, const BlockPos& pos, const BlockState* state);
    void checkCornerChangeAt(Level* level, const BlockPos& pos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    static bool shouldConnectTo(LevelSource* levelSource, const BlockPos& pos);
    bool isPowerSourceAt(LevelSource* levelSource, const BlockPos& pos, const Direction* direction);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual bool isSignalSource(const BlockState* state);
    static int getColorForData(int data);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);
    static TextureAtlasSprite* getTexture(const std::wstring& name);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mCrossIcon;
    TextureAtlasSprite* mLineIcon;
    TextureAtlasSprite* mCrossOverlayIcon;
    TextureAtlasSprite* mLineOverlayIcon;
    bool mSignalSource;
    boost::unordered_set<BlockPos, boost::hash<const BlockPos>, std::equal_to<const BlockPos>,
                         std::allocator<BlockPos> >
        mUpdatedPositions;
};

#endif
