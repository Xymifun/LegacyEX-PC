#ifndef GAME_WORLD_GENERATION_FEATURE_HPP
#define GAME_WORLD_GENERATION_FEATURE_HPP

class BlockPos;
class BlockState;
class Level;
class Random;

class Feature
{
public:
    Feature(bool doBlockNotify = false);
    virtual ~Feature() {}

    virtual bool place(Level* level, Random& random, const BlockPos& origin) = 0;
    virtual void init();
    virtual bool setBlock(Level* level, const BlockPos& pos, const BlockState* state, int flags);

protected:
    bool mDoBlockNotify;
};

#endif
