#ifndef GAME_WORLD_GENERATION_FUNCTIONLAYERFILL_HPP
#define GAME_WORLD_GENERATION_FUNCTIONLAYERFILL_HPP

class Random;
class Shape;
class Vec3i;
class BlockTransaction;

class LayerFill
{
public:
    virtual void ExtrudeLayer(Random& random, const Shape& shape, const Vec3i& origin, BlockTransaction* transaction,
                              int* minY, int* maxY) const = 0;
};

class FunctionLayerFill : public LayerFill
{
public:
    FunctionLayerFill(int (*function)(float));

    static int UNIFORM(float value);
    static int ISLAND(float value);

    virtual void ExtrudeLayer(Random& random, const Shape& shape, const Vec3i& origin, BlockTransaction* transaction,
                              int* minY, int* maxY) const;

private:
    int (*mFunction)(float);
};

#endif
