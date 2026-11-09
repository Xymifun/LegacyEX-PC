#ifndef GAME_CORE_BOUNDINGBOX_HPP
#define GAME_CORE_BOUNDINGBOX_HPP

#include <string>

#include "game/util/Vec3i.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class Direction;
class IntArrayTag;

class BoundingBox
{
public:
    int x0;
    int y0;
    int z0;
    int x1;
    int y1;
    int z1;

    BoundingBox();
    BoundingBox(arrayWithLength<int> values);
    BoundingBox(int x0, int y0, int z0, int x1, int y1, int z1);
    BoundingBox(Vec3i min, Vec3i max);
    BoundingBox(BoundingBox* other);
    BoundingBox(int x0, int z0, int x1, int z1);

    static BoundingBox* getUnknownBox();
    static BoundingBox* orientBox(int x, int y, int z, int xOffset, int yOffset, int zOffset, int xSize, int ySize,
                                  int zSize, const Direction* direction);
    static BoundingBox* createProper(int firstX, int firstY, int firstZ, int secondX, int secondY, int secondZ);

    bool intersects(BoundingBox* box);
    bool intersects(Vec3i min, Vec3i max);
    bool intersects(int minX, int minZ, int maxX, int maxZ);
    bool intersects(int minX, int minY, int minZ, int maxX, int maxY, int maxZ);

    BoundingBox* getIntersection(BoundingBox* box);

    void expand(BoundingBox* box);
    void move(int x, int y, int z);
    void move(Vec3i offset);

    bool isInside(Vec3i pos);

    int getXSpan();
    int getYSpan();
    int getZSpan();
    Vec3i getLength();
    BlockPos getCenter();

    std::wstring toString();

    IntArrayTag* createTag(const std::wstring& name);

    void write(DataOutputStream* output);
    void read(DataInputStream* input);
};

#endif
