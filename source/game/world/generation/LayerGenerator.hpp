#ifndef GAME_WORLD_GENERATION_LAYERGENERATOR_HPP
#define GAME_WORLD_GENERATION_LAYERGENERATOR_HPP

#include <vector>

class AABB;
class LayerFill;
class LayerShape;
class LayerTheme;
class Level;
class Random;
class BlockTransaction;

class LayerGenerator
{
public:
    LayerGenerator(AABB* area, const LayerShape* shape, const LayerFill* fill,
                   const std::vector<const LayerTheme*>& themes);
    ~LayerGenerator();

    AABB* getArea() const;
    int getCoreWorldMinY() const;
    BlockTransaction* Generate(Random& random, Level* level);
    void updateArea(AABB* area);
    int getCoreWorldMaxY() const;

private:
    const LayerShape* mShape;
    const LayerFill* mFill;
    std::vector<const LayerTheme*> mThemes;
    AABB* mArea;
    int mCoreWorldMinY;
    int mCoreWorldMaxY;
};

#endif
