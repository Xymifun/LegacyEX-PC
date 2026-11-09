#ifndef GAME_UTIL_VEC3I_HPP
#define GAME_UTIL_VEC3I_HPP

class Vec3i;
#include "game/core/Direction.hpp"

class Vec3i
{
public:
    int x;
    int y;
    int z;
    Vec3i(int x, int y, int z) : x(x), y(y), z(z) {}
    Vec3i(double x, double y, double z);
    int getX() const { return x; }
    int getY() const { return y; }
    int getZ() const { return z; }
    Vec3i offset(int dx, int dy, int dz) const;
    Vec3i offset(const Vec3i& other) const;
    Vec3i offset(double dx, double dy, double dz) const;
    Vec3i subtract(const Vec3i& other) const;
    Vec3i multiply(int scale) const;
    Vec3i cross(const Vec3i& other) const;
    double dist(int x, int y, int z) const;
    double dist(const Vec3i& other) const;
    double distSqr(double x, double y, double z) const;
    double distSqrToCenter(double x, double y, double z) const;
    double distSqr(const Vec3i& other) const;
    bool equals(const Vec3i& other) const;
    int hashCode() const;
    int compareTo(const Vec3i& other) const;
    Vec3i relative(const Direction* direction, int steps) const;
    Vec3i above(int steps) const;
    Vec3i above() const;
    Vec3i below(int steps) const;
    Vec3i below() const;
    Vec3i north(int steps) const;
    Vec3i north() const;
    Vec3i south(int steps) const;
    Vec3i south() const;
    Vec3i west(int steps) const;
    Vec3i west() const;
    Vec3i east(int steps) const;
    Vec3i east() const;
    Vec3i relative(const Direction* direction) const;
    Vec3i relative(Direction::Axis* axis, int steps) const;
};

#endif
