#ifndef GAME_CORE_POSITION_HPP
#define GAME_CORE_POSITION_HPP

class Position
{
public:
    virtual double getX() = 0;
    virtual double getY() = 0;
    virtual double getZ() = 0;

    virtual ~Position() {}
};

#endif
