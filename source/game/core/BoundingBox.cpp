#include "game/core/BoundingBox.hpp"

#include <algorithm>
#include <climits>
#include <cwchar>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Math.hpp"
#include "game/data/IntArrayTag.hpp"

BoundingBox::BoundingBox() : x0(0), y0(0), z0(0), x1(0), y1(0), z1(0)
{
}

BoundingBox::BoundingBox(int x0, int y0, int z0, int x1, int y1, int z1)
    : x0(x0),
      y0(y0),
      z0(z0),
      x1(x1),
      y1(y1),
      z1(z1)
{
}

BoundingBox::BoundingBox(BoundingBox* other)
    : x0(other->x0),
      y0(other->y0),
      z0(other->z0),
      x1(other->x1),
      y1(other->y1),
      z1(other->z1)
{
}

BoundingBox::BoundingBox(int x0, int z0, int x1, int z1) : x0(x0), y0(1), z0(z0), x1(x1), y1(512), z1(z1)
{
}

BoundingBox* BoundingBox::getUnknownBox()
{
    return new BoundingBox(INT_MAX, INT_MAX, INT_MAX, INT_MIN, INT_MIN, INT_MIN);
}

BoundingBox* BoundingBox::orientBox(int x, int y, int z, int xOffset, int yOffset, int zOffset, int xSize, int ySize,
                                    int zSize, const Direction* direction)
{
    int face = direction->get3DDataValue();
    if (face == 2)
    {
        return new BoundingBox(x + xOffset, y + yOffset, z - zSize + zOffset + 1, x + xSize - 1 + xOffset,
                               y + ySize - 1 + yOffset, z + zOffset);
    }
    if (face == 4)
    {
        return new BoundingBox(x - zSize + zOffset + 1, y + yOffset, z + xOffset, x + zOffset, y + ySize - 1 + yOffset,
                               z + xSize - 1 + xOffset);
    }
    if (face == 5)
    {
        return new BoundingBox(x + zOffset, y + yOffset, z + xOffset, x + zSize - 1 + zOffset, y + ySize - 1 + yOffset,
                               z + xSize - 1 + xOffset);
    }
    return new BoundingBox(x + xOffset, y + yOffset, z + zOffset, x + xSize - 1 + xOffset, y + ySize - 1 + yOffset,
                           z + zSize - 1 + zOffset);
}

BoundingBox* BoundingBox::createProper(int firstX, int firstY, int firstZ, int secondX, int secondY, int secondZ)
{
    int minX = firstX;
    if (secondX < firstX)
        minX = secondX;
    int minY = firstY;
    if (secondY < firstY)
        minY = secondY;
    int minZ = firstZ;
    if (secondZ < firstZ)
        minZ = secondZ;
    int maxX = firstX;
    if (firstX < secondX)
        maxX = secondX;
    int maxY = firstY;
    if (firstY < secondY)
        maxY = secondY;
    int maxZ = secondZ;
    if (secondZ < firstZ)
        maxZ = firstZ;
    return new BoundingBox(minX, minY, minZ, maxX, maxY, maxZ);
}

bool BoundingBox::intersects(BoundingBox* box)
{
    return !(x1 < box->x0 || x0 > box->x1 || z1 < box->z0 || z0 > box->z1 || y1 < box->y0 || y0 > box->y1);
}

bool BoundingBox::intersects(Vec3i min, Vec3i max)
{
    return !(x1 < min.x || x0 > max.x || z1 < min.z || z0 > max.z || y1 < min.y || y0 > max.y);
}

bool BoundingBox::intersects(int minX, int minZ, int maxX, int maxZ)
{
    return !(x1 < minX || x0 > maxX || z1 < minZ || z0 > maxZ);
}

void BoundingBox::expand(BoundingBox* box)
{
    x0 = Math::_min(x0, box->x0);
    y0 = Math::_min(y0, box->y0);
    z0 = Math::_min(z0, box->z0);
    x1 = Math::_max(x1, box->x1);
    y1 = Math::_max(y1, box->y1);
    z1 = Math::_max(z1, box->z1);
}

void BoundingBox::move(int x, int y, int z)
{
    x0 += x;
    y0 += y;
    z0 += z;
    x1 += x;
    y1 += y;
    z1 += z;
}

bool BoundingBox::isInside(Vec3i pos)
{
    return pos.x >= x0 && pos.x <= x1 && pos.z >= z0 && pos.z <= z1 && pos.y >= y0 && pos.y <= y1;
}

int BoundingBox::getXSpan()
{
    return x1 - x0 + 1;
}

int BoundingBox::getYSpan()
{
    return y1 - y0 + 1;
}

int BoundingBox::getZSpan()
{
    return z1 - z0 + 1;
}

BlockPos BoundingBox::getCenter()
{
    return BlockPos(x0 + (x1 - x0 + 1) / 2, y0 + (y1 - y0 + 1) / 2, z0 + (z1 - z0 + 1) / 2);
}

void BoundingBox::write(DataOutputStream* output)
{
    output->writeInt(x0);
    output->writeInt(y0);
    output->writeInt(z0);
    output->writeInt(x1);
    output->writeInt(y1);
    output->writeInt(z1);
}

void BoundingBox::read(DataInputStream* input)
{
    x0 = input->readInt();
    y0 = input->readInt();
    z0 = input->readInt();
    x1 = input->readInt();
    y1 = input->readInt();
    z1 = input->readInt();
}
