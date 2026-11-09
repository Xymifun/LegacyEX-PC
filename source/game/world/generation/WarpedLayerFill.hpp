#ifndef GAME_WORLD_GENERATION_WARPEDLAYERFILL_HPP
#define GAME_WORLD_GENERATION_WARPEDLAYERFILL_HPP

#include "game/world/generation/FunctionLayerFill.hpp"

class Random;
class Shape;
class Vec3i;
class BlockTransaction;

class WarpedLayerFill : public LayerFill
{
public:
    WarpedLayerFill(double noiseScale, int offset, int thickness);

    virtual void ExtrudeLayer(Random& random, const Shape& shape, const Vec3i& origin, BlockTransaction* transaction,
                              int* minY, int* maxY) const;

    double mNoiseScale;
    int mOffset;
    int mThickness;
};

#endif
