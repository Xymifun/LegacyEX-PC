#ifndef GAME_WORLD_GENERATION_BOWLLAYERFILL_HPP
#define GAME_WORLD_GENERATION_BOWLLAYERFILL_HPP

#include "game/world/generation/FunctionLayerFill.hpp"

class Random;
class Shape;
class Vec3i;
class BlockTransaction;

class BowlLayerFill : public LayerFill
{
public:
    BowlLayerFill(int offset, float (*curve)(float), int thickness);

    static float BOWL(float value);
    static float DOME(float value);

    virtual void ExtrudeLayer(Random& random, const Shape& shape, const Vec3i& origin, BlockTransaction* transaction,
                              int* minY, int* maxY) const;

    int mOffset;
    float (*mCurve)(float);
    int mThickness;
};

#endif
