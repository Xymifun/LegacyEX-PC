#ifndef GAME_WORLD_GENERATION_MEGAJUNGLETREEFEATURE_HPP
#define GAME_WORLD_GENERATION_MEGAJUNGLETREEFEATURE_HPP

#include "game/world/generation/MegaTreeFeature.hpp"

class BlockPos;
class BlockState;
class BooleanProperty;
class Level;
class Random;

class MegaJungleTreeFeature : public MegaTreeFeature
{
public:
    MegaJungleTreeFeature(bool doBlockNotify, int baseHeight, int extraRandomHeight, const BlockState* trunkState,
                          const BlockState* leafState);
    virtual ~MegaJungleTreeFeature();

    void createCrown(Level* level, const BlockPos& pos, int radius);
    void placeVine(Level* level, Random& random, const BlockPos& pos, const BooleanProperty* property);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
