#include "game/world/generation/TerracottaLayerTheme.hpp"

#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

Block* getColour(unsigned char colour);

TerracottaLayerTheme::TerracottaLayerTheme(const Pattern& pattern, TerracottaLayerThemeFunction function)
    : mPattern(pattern),
      mFunction(function)
{
}

unsigned int TerracottaLayerTheme::CONCENTRIC_RINGS(int x, int z)
{
    int value = (x < 0 ? -x : x) + (z < 0 ? -z : z);
    return (unsigned int)(value % 16) & 255u;
}

unsigned int TerracottaLayerTheme::OFFSET_RINGS(int x, int z)
{
    return CONCENTRIC_RINGS(x + 3, z + 4);
}

unsigned int TerracottaLayerTheme::HASHED(int x, int z)
{
    unsigned int xHash = boost::hash_value(x);
    unsigned int zHash = boost::hash_value(z);
    return (xHash ^ zHash) & 15u;
}

unsigned char TerracottaLayerTheme::TARTAN(int x, int z)
{
    unsigned char values[5][5] = {{7, 1, 2, 1, 7}, {1, 1, 2, 1, 1}, {2, 2, 3, 2, 2}, {1, 1, 2, 1, 1}, {7, 1, 2, 1, 7}};
    unsigned char xIndex = (x + 1275u) % 5;
    unsigned char zIndex = (z + 1275u) % 5;
    return values[xIndex][zIndex];
}

unsigned int TerracottaLayerTheme::STRIPES(int x, int z)
{
    return (unsigned int)(x % 16) & 255u;
}

unsigned int TerracottaLayerTheme::UNIFORM(int x, int z)
{
    return 0;
}

void TerracottaLayerTheme::Apply(Random& random, BlockTransaction* transaction, int& width, int& height) const
{
    unsigned char randomColour = (unsigned char)random.nextInt(16);
    int xOffset = transaction->mSize.x / mPattern.size / 2;
    int zOffset = transaction->mSize.z / mPattern.size / 2;
    int x;
    int z;
    int y;
    for (x = 0; x < transaction->mSize.x; ++x)
    {
        for (z = 0; z < transaction->mSize.z; ++z)
        {
            const Direction* direction = mPattern.values[(x % mPattern.size) * 4 + z % mPattern.size];
            unsigned char colour =
                (unsigned char)(randomColour + mFunction(x / mPattern.size - xOffset, z / mPattern.size - zOffset));
            const BlockState* state = getColour(colour)->defaultBlockState()->setValue(
                HorizontalDirectionalBlock::FACING, direction);
            for (y = 0; y < transaction->mSize.y; ++y)
            {
                Vec3i position(x, y, z);
                if (transaction->getBlock(position) != 0)
                {
                    transaction->setBlock(position, state);
                }
            }
        }
    }
}
