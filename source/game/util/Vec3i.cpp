#include "game/util/Vec3i.hpp"
#include <math.h>
#include "game/core/Mth.hpp"

Vec3i::Vec3i(double x, double y, double z)
{
    this->x = Mth::floor(x);
    this->y = Mth::floor(y);
    this->z = Mth::floor(z);
}

Vec3i Vec3i::offset(int dx, int dy, int dz) const
{
    return Vec3i(x + dx, y + dy, z + dz);
}
Vec3i Vec3i::offset(const Vec3i& other) const
{
    return offset(other.x, other.y, other.z);
}
Vec3i Vec3i::subtract(const Vec3i& other) const
{
    return offset(-other.x, -other.y, -other.z);
}
Vec3i Vec3i::multiply(int scale) const
{
    return Vec3i(x * scale, y * scale, z * scale);
}
Vec3i Vec3i::cross(const Vec3i& other) const
{
    return Vec3i(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}
double Vec3i::dist(int px, int py, int pz) const
{
    double dx = (double)(x - px), dy = (double)(y - py), dz = (double)(z - pz);
    return sqrt(dx * dx + dy * dy + dz * dz);
}
double Vec3i::dist(const Vec3i& other) const
{
    return dist(other.x, other.y, other.z);
}
double Vec3i::distSqr(double px, double py, double pz) const
{
    double dx = (double)x - px, dy = (double)y - py, dz = (double)z - pz;
    return dx * dx + dy * dy + dz * dz;
}
double Vec3i::distSqrToCenter(double px, double py, double pz) const
{
    double dx = (double)x + 0.5 - px, dy = (double)y + 0.5 - py, dz = (double)z + 0.5 - pz;
    return dx * dx + dy * dy + dz * dz;
}
double Vec3i::distSqr(const Vec3i& other) const
{
    return distSqr((double)other.x, (double)other.y, (double)other.z);
}
bool Vec3i::equals(const Vec3i& other) const
{
    return x == other.x && y == other.y && z == other.z;
}
int Vec3i::hashCode() const
{
    int value = x * 3129871;
    value = value * 31 + y;
    return value * 31 + z;
}
int Vec3i::compareTo(const Vec3i& other) const
{
    if (y != other.y)
        return y < other.y ? -1 : 1;
    if (z != other.z)
        return z < other.z ? -1 : 1;
    if (x != other.x)
        return x < other.x ? -1 : 1;
    return 0;
}
Vec3i Vec3i::offset(double dx, double dy, double dz) const
{
    return Vec3i((double)x + dx, (double)y + dy, (double)z + dz);
}
Vec3i Vec3i::relative(const Direction* direction, int steps) const
{
    return offset(direction->getStepX() * steps, direction->getStepY() * steps, direction->getStepZ() * steps);
}
Vec3i Vec3i::relative(const Direction* direction) const
{
    return relative(direction, 1);
}
Vec3i Vec3i::above(int steps) const
{
    return relative(Direction::UP, steps);
}
Vec3i Vec3i::above() const
{
    return above(1);
}
Vec3i Vec3i::below(int steps) const
{
    return relative(Direction::DOWN, steps);
}
Vec3i Vec3i::below() const
{
    return below(1);
}
Vec3i Vec3i::north(int steps) const
{
    return relative(Direction::NORTH, steps);
}
Vec3i Vec3i::north() const
{
    return north(1);
}
Vec3i Vec3i::south(int steps) const
{
    return relative(Direction::SOUTH, steps);
}
Vec3i Vec3i::south() const
{
    return south(1);
}
Vec3i Vec3i::west(int steps) const
{
    return relative(Direction::WEST, steps);
}
Vec3i Vec3i::west() const
{
    return west(1);
}
Vec3i Vec3i::east(int steps) const
{
    return relative(Direction::EAST, steps);
}
Vec3i Vec3i::east() const
{
    return east(1);
}
Vec3i Vec3i::relative(Direction::Axis* axis, int steps) const
{
    if (axis == Direction::Axis::X)
        return offset(steps, 0, 0);
    if (axis == Direction::Axis::Y)
        return offset(0, steps, 0);
    return offset(0, 0, steps);
}
