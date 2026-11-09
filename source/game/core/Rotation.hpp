#ifndef GAME_CORE_ROTATION_HPP
#define GAME_CORE_ROTATION_HPP

#include <string>

#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"

class Direction;

class Rotation
{
public:
    enum eTYPE
    {
        eTYPE_None = 0,
        eTYPE_Clockwise90 = 1,
        eTYPE_Clockwise180 = 2,
        eTYPE_CounterClockwise90 = 3
    };

    static Rotation* NONE;
    static Rotation* CLOCKWISE_90;
    static Rotation* CLOCKWISE_180;
    static Rotation* COUNTERCLOCKWISE_90;
    static Rotation* VALUES[4];
    Rotation(const std::wstring& name, eTYPE type);

    static Rotation* byName(std::wstring name);
    static Rotation* byType(eTYPE type);
    static Rotation* getRandom(Random random);

    eTYPE getType() const { return type; }

    Rotation* getRotated(Rotation* rotation);

    const Direction* rotate(const Direction* direction) const;
    int rotate(int value, int count) const;
    Vec3i fjRotate(const Vec3i& pos) const;

private:
    std::wstring name;
    eTYPE type;
};

#endif
