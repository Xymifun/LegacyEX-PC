#ifndef GAME_WORLD_GENERATION_MEGAPINETREEFEATURE_HPP
#define GAME_WORLD_GENERATION_MEGAPINETREEFEATURE_HPP

#include "game/world/generation/MegaTreeFeature.hpp"

class BlockPos;
class BlockState;
class Level;
class Random;

class MegaPineTreeFeature : public MegaTreeFeature
{
public:
    MegaPineTreeFeature(bool doBlockNotify, bool useHeight);
    virtual ~MegaPineTreeFeature();

    static void staticCtor();
    void createCrown(Level* level, int x, int z, int y, int radius, Random& random);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
    void placePodzolAt(Level* level, const BlockPos& pos);
    void placePodzolCircle(Level* level, const BlockPos& pos);
    virtual void postPlaceTree(Level* level, Random& random, const BlockPos& pos);

protected:
    bool mUseHeight;
};

#endif
