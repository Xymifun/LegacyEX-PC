#include "game/world/generation/WarpedLayerFill.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Shape.hpp"
#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockTransaction.hpp"

WarpedLayerFill::WarpedLayerFill(double noiseScale, int offset, int thickness)
    : mNoiseScale(noiseScale),
      mOffset(offset),
      mThickness(thickness)
{
}

void WarpedLayerFill::ExtrudeLayer(Random& random, const Shape& shape, const Vec3i&, BlockTransaction* transaction,
                                   int* minY, int* maxY) const
{
    SimplexNoise noise(&random);

    for (unsigned int x = 0; x < shape.width; ++x)
    {
        for (unsigned int z = 0; z < shape.depth; ++z)
        {
            float value = shape.at(x, z);
            if (value >= 0.0f && value < 1.0)
            {
                double noiseValue = noise.getValue((double)x / mNoiseScale, (double)z / mNoiseScale);
                int startY = (int)std::floor(mOffset * (float)((noiseValue + 1.0) * 0.5));
                int y = startY + 3;
                Vec3i position(x, transaction->mSize.y, z);
                for (int layer = 0; layer < mThickness; ++layer)
                {
                    Vec3i blockPos = position.subtract(Vec3i(0, y + layer, 0));
                    *maxY = std::max(*maxY, blockPos.y);
                    *minY = std::min(*minY, blockPos.y);
                    transaction->setBlock(blockPos, 1, 0);
                }
            }
        }
    }
}
