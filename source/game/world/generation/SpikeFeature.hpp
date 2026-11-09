#ifndef GAME_WORLD_GENERATION_SPIKEFEATURE_HPP
#define GAME_WORLD_GENERATION_SPIKEFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class AABB;
class BlockPos;

class SpikeFeature : public Feature
{
public:
    class EndSpike
    {
    public:
        EndSpike();
        EndSpike(const EndSpike& other);
        EndSpike(int centerX, int centerZ, int radius, int height, bool guarded);
        ~EndSpike();

        unsigned int getRadius();
        unsigned int getHeight();
        unsigned char isGuarded();
        unsigned int getCenterX();
        unsigned int getCenterZ();
        AABB* getTopBoundingBox();
        bool startsInChunk(const BlockPos& chunkPos);

    private:
        int mCenterX;
        int mCenterZ;
        int mRadius;
        int mHeight;
        bool mGuarded;
        AABB* mTopBoundingBox;
    };

    SpikeFeature();
    virtual ~SpikeFeature();

    void setSpike(EndSpike* spike);
    void setCrystalInvulnerable(bool invulnerable);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
    void setCrystalBeamTarget(const BlockPos& pos);

private:
    bool mCrystalInvulnerable;
    EndSpike* mSpike;
    BlockPos* mCrystalBeamTarget;
};

#endif
