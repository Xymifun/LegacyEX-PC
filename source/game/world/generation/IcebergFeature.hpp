#ifndef GAME_WORLD_GENERATION_ICEBERGFEATURE_HPP
#define GAME_WORLD_GENERATION_ICEBERGFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class BlockState;
class Level;
class Random;

class IcebergFeature : public Feature
{
public:
    IcebergFeature();
    virtual ~IcebergFeature();

    void setSnowOnTop(bool snowOnTop);
    int heightDependentRadiusEllipse(int height, int totalHeight, int radius);
    int heightDependentRadiusRound(Random& random, int height, int totalHeight, int radius);
    int heightDependentRadiusSteep(Random& random, int height, int totalHeight, int radius);
    void initEllipse(Random& random);
    double signedDistanceEllipse(int x, int z, const BlockPos& origin, int radius, int c, double angle);
    int getEllipseC(int height, int totalHeight);
    double signedDistanceCircle(int x, int z, const BlockPos& origin, int radius, Random& random);
    void setIcebergBlock(const BlockPos& pos, Level* level, Random& random, int height, int totalHeight);
    void generateIcebergBlock(Level* level, Random& random, const BlockPos& origin, int height, int x, int y, int z,
                              int radius, int ellipseRadius);
    bool isIcebergBlock(Block* block);
    bool belowIsAir(Level* level, const BlockPos& pos);
    void smooth(Level* level, const BlockPos& origin, int radius, int height);
    void removeFloatingSnowLayer(Level* level, const BlockPos& pos);
    void carve(int radius, int y, const BlockPos& origin, Level* level, bool placeWater, double angle,
               const BlockPos& cutCentre);
    void generateCutOut(Random& random, Level* level, int radius, int height, const BlockPos& origin);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
    void setLargestOffset(int largestOffset);

private:
    bool mSnowOnTop;
    bool mIsEllipse;
    double mAngle;
    int mEllipseRadius;
    int mEllipseC;
    int mLargestOffset;
    const BlockState* mIcebergState;
};

#endif
