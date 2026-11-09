#ifndef GAME_CORE_DIRECTION_HPP
#define GAME_CORE_DIRECTION_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/core/StringRepresentable.hpp"

class BlockPos;
class LivingEntity;
class Random;
class Vec3;
class Vec3i;
class Direction : public StringRepresentable
{
public:
    enum EDirection
    {
        eDirection_Down = 0,
        eDirection_Up = 1,
        eDirection_North = 2,
        eDirection_South = 3,
        eDirection_West = 4,
        eDirection_East = 5
    };

    class Axis;
    class AxisDirection;
    class Plane;

    class AxisDirection
    {
    public:
        static AxisDirection* POSITIVE;
        static AxisDirection* NEGATIVE;
        static AxisDirection* VALUES[2];

        AxisDirection(int step, const std::string& name, int ordinal);

        int getStep();
        int getOrdinal();
        std::string getName();
        std::string toString();

    private:
        int step;
        std::string name;
        int ordinal;
    };

    class Axis : public Predicate<const Direction*>, public StringRepresentable
    {
    public:
        enum EAxis
        {
            eAxis_X,
            eAxis_Y,
            eAxis_Z
        };

        static Axis* X;
        static Axis* Y;
        static Axis* Z;
        static Axis* VALUES[3];

        Axis(const std::wstring& name, Plane* plane, EAxis id);

        static void staticCtor();
        static Axis* byName(const std::wstring& name);

        std::wstring getName();
        EAxis getId();
        Plane* getPlane();
        bool isHorizontal();
        bool isVertical();

        virtual bool apply(const Direction* direction) const;
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

    private:
        static boost::unordered_map<std::wstring, Axis*> BY_NAME;

        std::wstring name;
        Plane* plane;
        EAxis id;
    };

    class Plane : public Predicate<const Direction*>
    {
    public:
        static Plane* HORIZONTAL;
        static Plane* VERTICAL;
        static Plane* VALUES[2];

        virtual ~Plane();

        arrayWithLength<const Direction*> getFaces();
        arrayWithLength<Axis*> getAxis();
        arrayWithLength<const Direction*> iterator();
        const Direction* getRandomDirection(Random& random);
        Axis* getRandomAxis(Random& random);

        virtual bool apply(const Direction* direction) const;
    };

    static const Direction* DOWN;
    static const Direction* UP;
    static const Direction* NORTH;
    static const Direction* SOUTH;
    static const Direction* WEST;
    static const Direction* EAST;

    Direction(int data3D, int oppositeIndex, int data2D, const std::wstring& name, AxisDirection* axisDirection,
              Axis* axis, Vec3i* normal, unsigned char ordinal);
    ~Direction();
    static void staticCtor();

    static const Direction* from3DDataValue(int data3D);
    static const Direction* from2DDataValue(int data2D);
    static const Direction* fromYRot(double yRot);
    static const Direction* fromVec3(const Vec3* vec);
    static const Direction* byName(const std::wstring& name);
    static const Direction* getNearest(float x, float y, float z);
    static const Direction* getNearest(float x, float z);
    static const Direction* getNearest(double x, double y, double z);
    static const Direction* get(AxisDirection* axisDirection, Axis* axis);
    static const Direction* getRandomDirection(Random& random);
    static const Direction* getDirectionFromLivingEntity(const BlockPos& pos,
                                                         const boost::shared_ptr<LivingEntity>& entity);

    int get3DDataValue() const;
    int get2DDataValue() const;
    unsigned char getOrdinal() const;
    int getStepX() const { return stepX; }
    int getStepY() const { return stepY; }
    int getStepZ() const { return stepZ; }
    Axis* getAxis() const;
    AxisDirection* getAxisDirection() const;
    std::wstring getName() const;

    const Direction* getOpposite() const;
    const Direction* getClockWise() const;
    const Direction* getCounterClockWise() const;
    const Direction* getClockWiseX() const;
    const Direction* getClockWiseZ() const;
    const Direction* getClockWise(Axis* axis) const;
    const Direction* getCounterClockWiseX() const;
    const Direction* getCounterClockWiseZ() const;
    const Direction* getCounterClockWise(Axis* axis) const;
    const Direction* getRelative(const Direction* direction) const;

    float toYRot() const;
    Vec3i getNormal() const;
    virtual std::wstring toString() const;
    virtual std::wstring getSerializedName() const;

public:
    static std::vector<const Direction*> VALUES;

private:
    static const Direction* BY_3D_DATA[6];
    static const Direction* BY_2D_DATA[4];
    static boost::unordered_map<std::wstring, const Direction*> BY_NAME;

    int data3D;
    int oppositeIndex;
    int data2D;
    std::wstring name;
    Axis* axis;
    AxisDirection* axisDirection;
    Vec3i* normal;
    unsigned char ordinal;
    int stepX;
    int stepY;
    int stepZ;
};

#include "game/util/Vec3i.hpp"

#endif
