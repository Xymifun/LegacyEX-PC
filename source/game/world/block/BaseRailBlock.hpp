#ifndef GAME_WORLD_BLOCK_BASERAILBLOCK_HPP
#define GAME_WORLD_BLOCK_BASERAILBLOCK_HPP

#include <string>
#include <vector>

#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

class AABB;
class BlockPos;
class BlockState;
class Direction;
class Level;
class LevelSource;
class Property;

template <typename T>
class AbstractProperty;

class BaseRailBlock : public Block
{
public:
    class RailShape : public StringRepresentable
    {
    public:
        RailShape(int data, const std::wstring& name);
        bool isAscending();
        int getData();
        virtual std::wstring toString() const;
        static RailShape* byData(int data);
        static RailShape* NORTH_SOUTH;
        static RailShape* EAST_WEST;
        static RailShape* ASCENDING_EAST;
        static RailShape* ASCENDING_WEST;
        static RailShape* ASCENDING_NORTH;
        static RailShape* ASCENDING_SOUTH;
        static RailShape* SOUTH_EAST;
        static RailShape* SOUTH_WEST;
        static RailShape* NORTH_WEST;
        static RailShape* NORTH_EAST;
        static RailShape* VALUES[10];
        virtual std::wstring getSerializedName() const;
        std::wstring getName();

    private:
        int mData;
        std::wstring mName;
    };

    class Rail
    {
    public:
        Rail(Level* level, const BlockPos& pos, const BlockState* state);
        const BlockState* getState();
        void updateConnections(RailShape* shape);
        Rail* getRail(const BlockPos& pos);
        bool hasConnection(const BlockPos& pos);
        bool connectsTo(Rail* rail);
        bool canConnectTo(Rail* rail);
        void removeSoftConnections();
        bool hasNeighborRail(const BlockPos& pos);
        void connectTo(Rail* rail);
        Rail* place(bool powered, bool forceUpdate);
        std::vector<BlockPos>* getConnections();
        bool hasRail(const BlockPos& pos);
        int countPotentialConnections();

    private:
        Level* mLevel;
        BlockPos mPos;
        Block* mBlock;
        const BlockState* mState;
        bool mPowered;
        std::vector<BlockPos> mConnections;
    };

    BaseRailBlock(bool isPowered);
    virtual ~BaseRailBlock() {}
    static bool isRail(const Block* block);
    static bool isRail(Level* level, const BlockPos& pos);
    static bool isRail(const BlockState* state);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void updateState(const BlockState* state, Level* level, const BlockPos& pos, Block* block);
    virtual const BlockState* updateDir(Level* level, const BlockPos& pos, const BlockState* state, bool forceUpdate);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual int getRenderLayer();
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const AbstractProperty<RailShape*>* getShapeProperty() = 0;
    bool usesDataBit() { return mIsPowered; }
    bool mIsPowered;
};

bool railfilter(BaseRailBlock::RailShape* shape);
bool railpredicate(BaseRailBlock::RailShape* shape);

#endif
