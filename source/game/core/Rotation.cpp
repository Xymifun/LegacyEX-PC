#include "game/core/Rotation.hpp"

#include "game/core/Direction.hpp"

Rotation* Rotation::NONE = new Rotation(L"rotate_0", Rotation::eTYPE_None);
Rotation* Rotation::CLOCKWISE_90 = new Rotation(L"rotate_90", Rotation::eTYPE_Clockwise90);
Rotation* Rotation::CLOCKWISE_180 = new Rotation(L"rotate_180", Rotation::eTYPE_Clockwise180);
Rotation* Rotation::COUNTERCLOCKWISE_90 = new Rotation(L"rotate_270", Rotation::eTYPE_CounterClockwise90);

Rotation* Rotation::VALUES[4] = {NONE, CLOCKWISE_90, CLOCKWISE_180, COUNTERCLOCKWISE_90};

Rotation::Rotation(const std::wstring& name, eTYPE type) : name(name), type(type)
{
}

Rotation* Rotation::byType(eTYPE type)
{
    switch (type)
    {
    case eTYPE_None:
        return NONE;

    case eTYPE_Clockwise90:
        return CLOCKWISE_90;

    case eTYPE_Clockwise180:
        return CLOCKWISE_180;

    case eTYPE_CounterClockwise90:
        return COUNTERCLOCKWISE_90;

    default:
        return NONE;
    }
}

Rotation* Rotation::getRandom(Random random)
{
    return VALUES[random.nextInt(4)];
}

Rotation* Rotation::byName(std::wstring name)
{
    for (int i = 0; i < 4; ++i)
    {
        if (VALUES[i]->name == name)
        {
            return VALUES[i];
        }
    }
    return NONE;
}

Rotation* Rotation::getRotated(Rotation* rotation)
{
    switch (rotation->type)
    {
    case eTYPE_None:
        return this;

    case eTYPE_Clockwise180:
        switch (type)
        {
        case eTYPE_None:
            return CLOCKWISE_180;

        case eTYPE_Clockwise90:
            return COUNTERCLOCKWISE_90;

        case eTYPE_Clockwise180:
            return NONE;

        case eTYPE_CounterClockwise90:
            return CLOCKWISE_90;

        default:
            return this;
        }

    case eTYPE_CounterClockwise90:
        switch (type)
        {
        case eTYPE_None:
            return COUNTERCLOCKWISE_90;

        case eTYPE_Clockwise90:
            return NONE;

        case eTYPE_Clockwise180:
            return CLOCKWISE_90;

        case eTYPE_CounterClockwise90:
            return CLOCKWISE_180;

        default:
            return this;
        }

    case eTYPE_Clockwise90:
        switch (type)
        {
        case eTYPE_None:
            return CLOCKWISE_90;

        case eTYPE_Clockwise90:
            return CLOCKWISE_180;

        case eTYPE_Clockwise180:
            return COUNTERCLOCKWISE_90;

        case eTYPE_CounterClockwise90:
            return NONE;

        default:
            return this;
        }

    default:
        return this;
    }
}

const Direction* Rotation::rotate(const Direction* direction) const
{
    if (direction->getAxis() != Direction::Axis::Y)
    {
        switch (type)
        {
        case eTYPE_Clockwise180:
            return direction->getOpposite();

        case eTYPE_CounterClockwise90:
            return direction->getCounterClockWise();

        case eTYPE_Clockwise90:
            return direction->getClockWise();

        default:
            break;
        }
    }

    return direction;
}

int Rotation::rotate(int value, int count) const
{
    switch (type)
    {
    case eTYPE_Clockwise90:
        return (value + count / 4) % count;

    case eTYPE_Clockwise180:
        return (value + count / 2) % count;

    case eTYPE_CounterClockwise90:
        return (value + count * 3 / 4) % count;

    default:
        return value;
    }
}

Vec3i Rotation::fjRotate(const Vec3i& pos) const
{
    switch (type)
    {
    case eTYPE_None:
        return Vec3i(pos.x, pos.y, pos.z);

    case eTYPE_Clockwise90:
        return Vec3i(-pos.z, pos.y, pos.x);

    case eTYPE_Clockwise180:
        return Vec3i(-pos.x, pos.y, -pos.z);

    case eTYPE_CounterClockwise90:
        return Vec3i(pos.z, pos.y, -pos.x);

    default:
        return pos;
    }
}
