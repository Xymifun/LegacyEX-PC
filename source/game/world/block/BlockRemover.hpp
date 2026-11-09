#ifndef GAME_WORLD_BLOCK_BLOCKREMOVER_HPP
#define GAME_WORLD_BLOCK_BLOCKREMOVER_HPP

#include <vector>

class AABB;
class BlockPos;
class Level;

class BlockRemover
{
public:
    class Area
    {
    public:
        Area(AABB* bounds);

        AABB* mBounds;
        int mX;
        int mY;
        int mZ;
    };

    BlockRemover(Level* level, std::vector<AABB*>& areas);
    virtual ~BlockRemover();

    bool Finished();

    virtual void Tick();
    virtual bool RemoveBlocks(int areaIndex, int maxBlocks) = 0;
    virtual void CreateParticles(const BlockPos& pos, int colour);

protected:
    std::vector<Area> mAreas;
    bool mFinished;
    Level* mLevel;
    int mTicks;
    int mRemovalCount;
    int mBlocksPerRemoval;
    int mTicksPerRemoval;
};

#endif
