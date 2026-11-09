#include "game/world/generation/FunctionLayerFill.hpp"

#include <algorithm>

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockTransaction.hpp"

FunctionLayerFill::FunctionLayerFill(int (*function)(float)) : mFunction(function)
{
}

int FunctionLayerFill::UNIFORM(float)
{
    return 1;
}

int FunctionLayerFill::ISLAND(float value)
{
    if (value < 0.1)
        return 3;
    if (value < 0.5)
        return 2;
    return 1;
}

void FunctionLayerFill::ExtrudeLayer(Random&, const Shape& shape, const Vec3i&, BlockTransaction* transaction,
                                     int* minY, int* maxY) const
{
    for (unsigned int x = 0; x < shape.width; ++x)
    {
        for (unsigned int z = 0; z < shape.depth; ++z)
        {
            const float& value = shape.at(x, z);
            if (0.0f <= value && value < 1.0)
            {
                int top = transaction->mSize.y - 3;
                int count = mFunction(value);
                for (int layer = 0; layer < count; ++layer)
                {
                    int y = top - layer;
                    *maxY = std::max(*maxY, y);
                    *minY = std::min(*minY, y);
                    transaction->setBlock(Vec3i(x, y, z), 1, 0);
                }
            }
        }
    }
}
