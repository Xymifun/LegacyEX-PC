#ifndef GAME_WORLD_GENERATION_WALLGENERATOR_HPP
#define GAME_WORLD_GENERATION_WALLGENERATOR_HPP

#include <vector>

#include "game/world/generation/MapGenerator.hpp"

class AABB;
class BlockState;
class BlockTransaction;
class Level;
class Random;

class WallGenerator : public MapGenerator
{
public:
    class Wall
    {
    public:
        Wall(AABB* area, bool internal, bool boundary);
        ~Wall();

        AABB* mArea;
        bool mInternal;
        bool mBoundary;
    };

    WallGenerator(Random random, std::vector<AABB*>* areas, std::vector<AABB*>* walls);
    virtual ~WallGenerator();

    void GetInternalWalls(std::vector<AABB*>* walls);
    bool IsPointInternal(int x, int z, int size);
    virtual void StartGeneration(Level* level);
    virtual AABB* GetSpawnArea() const;
    virtual void CleanUp(Level* level);
    virtual AABB* GetLayerBB(unsigned int layer);
    virtual int GetNumberOfLayers();
    virtual void GetLayerExtents(unsigned int layer, float* minimum, float* maximum);

    void SetArea(Level* level, BlockTransaction* transaction, AABB* area, const BlockState* state);

private:
    std::vector<AABB*> mAreas;
    std::vector<Wall*> mWalls;
    AABB* mBounds;
};

#endif
