#ifndef GAME_CORE_FILLTOOL_HPP
#define GAME_CORE_FILLTOOL_HPP

#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockPos.hpp"

class AABB;
class Block;
class BlockState;
class ConsoleSchematicFile;
class DataInputStream;
class DataOutputStream;
class Level;
class Player;

class FillTool : public BlockPos
{
public:
    enum EHandleDataType
    {
        HANDLE_DATA_NONE = 0,
        HANDLE_DATA_EDGE = 1,
        HANDLE_DATA_FILL = 2
    };
    FillTool(ConsoleSchematicFile* schematic);
    ~FillTool();
    void Reset();
    int IsWaterBlock(const BlockState* state);
    int IsLavaBlock(const BlockState* state);
    int IsValidSurfaceBlock(const BlockState* state);
    void tick();
    void PopulateSurfaceBrushBlock(boost::shared_ptr<Player> player);
    void Serialise(DataOutputStream* output);
    void Deserialise(DataInputStream* input);
    static Vec3i minVec(const Vec3i& first, const Vec3i& second);
    static Vec3i maxVec(const Vec3i& first, const Vec3i& second);
    AABB* GetAABB();
    void TakeSchematicForUndo(Level* level);
    void BeginFill(Level* level);
    void UndoFill(Level* level);
    void SetPointA(double x, double y, double z);
    void SetPointB(double x, double y, double z);
    void SetEdgeBlock(Block* block);
    void SetFillBlock(Block* block);
    void SetReplaceOnlyBlock(Block* block);
    void SetHandleDataType(EHandleDataType type);
    void SetEdgeData(int data);
    void SetFillData(int data);
    void SetFillDataMask(int mask);
    void SetReplaceOnlyData(int data, int mask);
    void SetDimension(int dimension);
    void SetXDiameter(int diameter);
    void SetYDiameter(int diameter);
    void SetZDiameter(int diameter);
    void SetDiameter(int diameter);
    int GetXDiameter();
    int GetYDiameter();
    int GetZDiameter();
    int GetDiameter();
    int GetDimension();
    BlockPos GetPointA() const;
    BlockPos GetPointB() const;
    Block* GetEdgeBlock() const;
    Block* GetFillBlock() const;
    int GetEdgeData() const;
    int GetFillData() const;
    int GetMode() const;

private:
    struct FillOperation
    {
        Level* mLevel;
        AABB* mBounds;
        int mField0xAC;
        int mChunkMaxX;
        int mChunkX;
        int mChunkMinZ;
        int mChunkMaxZ;
        int mChunkZ;
        Block* mEdgeBlock;
        Block* mFillBlock;
        int mEdgeData;
        int mFillData;
        bool mIgnoreAir;
        bool mIgnoreWater;
        Block* mReplaceOnlyBlock;
        int mReplaceOnlyBlockId;
        int mReplaceOnlyData;
        int mReplaceOnlyDataMask;
        int mFillDataMask;
        int mHandleDataType;
        unsigned int mShape;
        int mMode;
        int mField0xF8;
        int mSmoothType;
        int mField0x100;
        bool mSkipSurfaceWater;
        std::vector<std::pair<const BlockState*, int> > mBrushBlocks;
    };

    BlockPos mPointB;
    Block* mEdgeBlock;
    Block* mFillBlock;
    Block* mReplaceOnlyBlock;
    int mEdgeData;
    int mFillData;
    int mReplaceOnlyData;
    int mReplaceOnlyDataMask;
    int mFillDataMask;
    int mDimension;
    bool mReplaceOnly;
    bool mHasPointB;
    int mHandleDataType;
    int mShape;
    int mMode;
    int mLiquidType;
    int mField0x50;
    float mBrushStrength;
    bool mSurfaceBrush;
    std::vector<std::pair<const BlockState*, int> > mSurfaceBlocks;
    bool mField0x6C;
    int mField0x70;
    int mField0x74;
    int mField0x78;
    int mField0x7C;
    int mField0x80;
    int mField0x84;
    int mField0x88;
    int mField0x8C;
    int mField0x90;
    int mField0x94;
    int mField0x98;
    int mField0x9C;
    ConsoleSchematicFile* mSchematic;
    FillOperation mOperation;
    bool mActive;
    int mField0x11C;
};

#endif
