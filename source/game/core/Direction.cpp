#include "game/core/Direction.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/core/StringUtils.hpp"

const Direction* Direction::DOWN;
const Direction* Direction::UP;
const Direction* Direction::NORTH;
const Direction* Direction::SOUTH;
const Direction* Direction::WEST;
const Direction* Direction::EAST;

std::vector<const Direction*> Direction::VALUES;
const Direction* Direction::BY_3D_DATA[6];
const Direction* Direction::BY_2D_DATA[4];
boost::unordered_map<std::wstring, const Direction*> Direction::BY_NAME;

Direction::AxisDirection* Direction::AxisDirection::POSITIVE = new Direction::AxisDirection(1, "Towards positive", 0);
Direction::AxisDirection* Direction::AxisDirection::NEGATIVE = new Direction::AxisDirection(-1, "Towards negative", 1);
Direction::AxisDirection* Direction::AxisDirection::VALUES[2] = {Direction::AxisDirection::POSITIVE,
                                                                 Direction::AxisDirection::NEGATIVE};

Direction::Plane* Direction::Plane::HORIZONTAL = new Direction::Plane();
Direction::Plane* Direction::Plane::VERTICAL = new Direction::Plane();
Direction::Plane* Direction::Plane::VALUES[2] = {Direction::Plane::HORIZONTAL, Direction::Plane::VERTICAL};

Direction::Axis* Direction::Axis::X;
Direction::Axis* Direction::Axis::Y;
Direction::Axis* Direction::Axis::Z;
Direction::Axis* Direction::Axis::VALUES[3] = {Direction::Axis::X, Direction::Axis::Y, Direction::Axis::Z};

boost::unordered_map<std::wstring, Direction::Axis*> Direction::Axis::BY_NAME;

Direction::Direction(int data3D, int oppositeIndex, int data2D, const std::wstring& name, AxisDirection* axisDirection,
                     Axis* axis, Vec3i* normal, unsigned char ordinal)
{
    this->data3D = data3D;
    this->oppositeIndex = oppositeIndex;
    this->data2D = data2D;
    this->name = name;
    this->axisDirection = axisDirection;
    this->axis = axis;
    this->normal = new Vec3i(*normal);
    this->ordinal = ordinal;

    VALUES.push_back(this);

    this->stepX = normal->x;
    this->stepY = normal->y;
    this->stepZ = normal->z;
}

void Direction::staticCtor()
{
    Axis::staticCtor();

    DOWN = new Direction(0, 1, -1, L"down", AxisDirection::NEGATIVE, Axis::Y, new Vec3i(0, -1, 0), 0);
    UP = new Direction(1, 0, -1, L"up", AxisDirection::POSITIVE, Axis::Y, new Vec3i(0, 1, 0), 1);
    NORTH = new Direction(2, 3, 2, L"north", AxisDirection::NEGATIVE, Axis::Z, new Vec3i(0, 0, -1), 2);
    SOUTH = new Direction(3, 2, 0, L"south", AxisDirection::POSITIVE, Axis::Z, new Vec3i(0, 0, 1), 3);
    WEST = new Direction(4, 5, 1, L"west", AxisDirection::NEGATIVE, Axis::X, new Vec3i(-1, 0, 0), 4);
    EAST = new Direction(5, 4, 3, L"east", AxisDirection::POSITIVE, Axis::X, new Vec3i(1, 0, 0), 5);

    for (std::vector<const Direction*>::iterator it = VALUES.begin(); it != VALUES.end(); ++it)
    {
        const Direction* direction = *it;

        BY_3D_DATA[direction->data3D] = direction;

        if (direction->getAxis()->isHorizontal())
        {
            BY_2D_DATA[direction->data2D] = direction;
        }

        BY_NAME[direction->getName()] = direction;
    }
}

const Direction* Direction::from3DDataValue(int data3D)
{
    return BY_3D_DATA[Mth::abs(data3D % 6)];
}

const Direction* Direction::from2DDataValue(int data2D)
{
    return BY_2D_DATA[Mth::abs(data2D % 4)];
}

const Direction* Direction::fromYRot(double yRot)
{
    double quadrant = yRot / 90.0 + 0.5;
    int index = (int)quadrant;

    if (quadrant < (double)index)
    {
        index--;
    }

    return from2DDataValue(index & 3);
}

const Direction* Direction::fromVec3(const Vec3* vec)
{
    Vec3* absolute = Vec3::newTemp(::fabs(vec->x), ::fabs(vec->y), ::fabs(vec->z));

    if (absolute->lengthSqr() > 0.01)
    {
        if (absolute->y > absolute->x && absolute->y > absolute->z)
        {
            if (vec->y > 0.0)
            {
                return UP;
            }

            return DOWN;
        }

        if (absolute->x > absolute->y && absolute->x > absolute->z)
        {
            if (vec->x > 0.0)
            {
                return EAST;
            }

            return WEST;
        }

        if (vec->z > 0.0)
        {
            return SOUTH;
        }

        return NORTH;
    }

    return UP;
}


const Direction* Direction::get(AxisDirection* axisDirection, Axis* axis)
{
    for (std::vector<const Direction*>::iterator it = VALUES.begin(); it != VALUES.end(); ++it)
    {
        const Direction* direction = *it;

        if (direction->getAxisDirection() == axisDirection && direction->getAxis() == axis)
        {
            return direction;
        }
    }

    return NULL;
}

const Direction* Direction::getRandomDirection(Random& random)
{
    return VALUES[random.nextInt(VALUES.size())];
}

const Direction* Direction::getDirectionFromLivingEntity(const BlockPos& pos,
                                                         const boost::shared_ptr<LivingEntity>& entity)
{
    if (::fabs(entity->mX - ((float)pos.x + 0.5f)) < 2.0 && ::fabs(entity->mZ - ((float)pos.z + 0.5f)) < 2.0)
    {
        double eyeY = entity->mY + entity->getEyeHeight();

        if (eyeY - (double)pos.y > 2.0)
        {
            return UP;
        }

        if ((double)pos.y - eyeY > 0.0)
        {
            return DOWN;
        }
    }

    return entity->getDirection()->getOpposite();
}

int Direction::get3DDataValue() const
{
    return data3D;
}

int Direction::get2DDataValue() const
{
    return data2D;
}

unsigned char Direction::getOrdinal() const
{
    return ordinal;
}

Direction::Axis* Direction::getAxis() const
{
    return axis;
}

Direction::AxisDirection* Direction::getAxisDirection() const
{
    return axisDirection;
}

std::wstring Direction::getName() const
{
    return name;
}

Vec3i Direction::getNormal() const
{
    return *normal;
}

const Direction* Direction::getOpposite() const
{
    return from3DDataValue(oppositeIndex);
}

const Direction* Direction::getClockWise() const
{
    switch (get3DDataValue())
    {
    case 2:
        return EAST;

    case 5:
        return SOUTH;

    case 3:
        return WEST;

    case 4:
        return NORTH;
    }

    return NULL;
}

const Direction* Direction::getCounterClockWise() const
{
    switch (get3DDataValue())
    {
    case 2:
        return WEST;

    case 5:
        return NORTH;

    case 3:
        return EAST;

    case 4:
        return SOUTH;
    }

    return NULL;
}

float Direction::toYRot() const
{
    return (float)(data2D & 3) * 90.0f;
}

std::wstring Direction::toString() const
{
    return name;
}

std::wstring Direction::getSerializedName() const
{
    return name;
}

Direction::AxisDirection::AxisDirection(int step, const std::string& name, int ordinal)
{
    this->step = step;
    this->name = name;
    this->ordinal = ordinal;
}

int Direction::AxisDirection::getStep()
{
    return step;
}

int Direction::AxisDirection::getOrdinal()
{
    return ordinal;
}

Direction::Axis::Axis(const std::wstring& name, Plane* plane, EAxis id)
{
    this->name = name;
    this->plane = plane;
    this->id = id;
}

void Direction::Axis::staticCtor()
{
    X = new Axis(L"x", Plane::HORIZONTAL, eAxis_X);
    Y = new Axis(L"y", Plane::VERTICAL, eAxis_Y);
    Z = new Axis(L"z", Plane::HORIZONTAL, eAxis_Z);

    VALUES[0] = X;
    VALUES[1] = Y;
    VALUES[2] = Z;

    for (int i = 0; i < 3; i++)
    {
        Axis* axis = VALUES[i];

        BY_NAME[toLower(axis->getName())] = axis;
    }
}

std::wstring Direction::Axis::getName()
{
    return name;
}

Direction::Axis::EAxis Direction::Axis::getId()
{
    return id;
}

Direction::Plane* Direction::Axis::getPlane()
{
    return plane;
}

bool Direction::Axis::isHorizontal()
{
    return plane == Plane::HORIZONTAL;
}

bool Direction::Axis::isVertical()
{
    return plane == Plane::VERTICAL;
}

bool Direction::Axis::apply(const Direction* direction) const
{
    return direction != NULL && direction->getAxis() == this;
}

std::wstring Direction::Axis::toString() const
{
    return name;
}

std::wstring Direction::Axis::getSerializedName() const
{
    return name;
}

Direction::Plane::~Plane()
{
}

arrayWithLength<const Direction*> Direction::Plane::getFaces()
{
    if (this == HORIZONTAL)
    {
        static const Direction* horizontalFaces[4] = {NORTH, EAST, SOUTH, WEST};

        return arrayWithLength<const Direction*>(horizontalFaces, 4);
    }

    if (this == VERTICAL)
    {
        static const Direction* verticalFaces[2] = {UP, DOWN};

        return arrayWithLength<const Direction*>(verticalFaces, 2);
    }

    return arrayWithLength<const Direction*>();
}

arrayWithLength<const Direction*> Direction::Plane::iterator()
{
    return getFaces();
}

const Direction* Direction::Plane::getRandomDirection(Random& random)
{
    arrayWithLength<const Direction*> faces = getFaces();

    return faces.data[random.nextInt(faces.length)];
}

bool Direction::Plane::apply(const Direction* direction) const
{
    return direction != NULL && direction->getAxis()->getPlane() == this;
}
