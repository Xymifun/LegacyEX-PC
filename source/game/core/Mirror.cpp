#include "game/core/Mirror.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Rotation.hpp"

Mirror* Mirror::NONE = new Mirror(L"no_mirror", Mirror::eTYPE_None);
Mirror* Mirror::LEFT_RIGHT = new Mirror(L"mirror_left_right", Mirror::eTYPE_LeftRight);
Mirror* Mirror::FRONT_BACK = new Mirror(L"mirror_front_back", Mirror::eTYPE_FrontBack);

Mirror* Mirror::VALUES[3] = {NONE, LEFT_RIGHT, FRONT_BACK};

Mirror::Mirror(const std::wstring& name, eTYPE type) : name(name), type(type)
{
}

Mirror* Mirror::byName(std::wstring name)
{
    int i;
    for (i = 0; i < 3; ++i)
    {
        if (VALUES[i]->name == name)
        {
            return VALUES[i];
        }
    }
    return NULL;
}

int Mirror::mirror(int rotation, int rotationCount)
{
    int half = rotationCount / 2;
    int value = rotation > half ? rotation - rotationCount : rotation;

    switch (type)
    {
    case eTYPE_LeftRight:
        return (half - value + rotationCount) % rotationCount;

    case eTYPE_FrontBack:
        return (rotationCount - value) % rotationCount;

    default:
        return rotation;
    }
}

Rotation* Mirror::getRotation(const Direction* direction)
{
    Direction::Axis* axis = direction->getAxis();

    if ((this == LEFT_RIGHT && axis == Direction::Axis::Z) || (this == FRONT_BACK && axis == Direction::Axis::X))
    {
        return Rotation::CLOCKWISE_180;
    }

    return Rotation::NONE;
}

const Direction* Mirror::mirror(const Direction* direction)
{
    switch (type)
    {
    case eTYPE_LeftRight:
        if (direction == Direction::NORTH)
        {
            return Direction::SOUTH;
        }

        if (direction == Direction::SOUTH)
        {
            return Direction::NORTH;
        }

        break;

    case eTYPE_FrontBack:
        if (direction == Direction::WEST)
        {
            return Direction::EAST;
        }

        if (direction == Direction::EAST)
        {
            return Direction::WEST;
        }

        break;

    default:
        break;
    }

    return direction;
}
