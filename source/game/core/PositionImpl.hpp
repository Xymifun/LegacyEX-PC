#ifndef GAME_CORE_POSITIONIMPL_HPP
#define GAME_CORE_POSITIONIMPL_HPP

#include "game/core/Position.hpp"

class PositionImpl : public Position
{
public:
    PositionImpl(double x, double y, double z) : mX(x), mY(y), mZ(z) {}

    virtual double getX();
    virtual double getY();
    virtual double getZ();

    virtual ~PositionImpl();

private:
    double mX;
    double mY;
    double mZ;
};

#endif
