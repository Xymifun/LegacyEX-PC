#include "game/world/generation/BowlLayerFill.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockTransaction.hpp"

BowlLayerFill::BowlLayerFill(int offset, float (*curve)(float), int thickness)
    : mOffset(offset),
      mCurve(curve),
      mThickness(thickness)
{
}

float BowlLayerFill::BOWL(float value)
{
    return 1.0f - value;
}

float BowlLayerFill::DOME(float value)
{
    return value;
}

void BowlLayerFill::ExtrudeLayer(Random&, const Shape& shape, const Vec3i&, BlockTransaction* transaction, int* minY,
                                 int* maxY) const
{
    const float height = mOffset - 0.001f;
    for (unsigned int x = 0; x < shape.width; ++x)
    {
        for (unsigned int z = 0; z < shape.depth; ++z)
        {
            float value = shape.at(x, z);
            if (0.0f <= value && value < 1.0)
            {
                int startY = (int)std::floor(height * mCurve(value)) + 3;
                Vec3i column(x, transaction->mSize.y, z);
                for (int layer = 0; layer < mThickness; ++layer)
                {
                    Vec3i position = column.subtract(Vec3i(0, startY + layer, 0));
                    *maxY = std::max(*maxY, position.y);
                    *minY = std::min(*minY, position.y);
                    transaction->setBlock(position, 1, 0);
                }
            }
        }
    }
}
