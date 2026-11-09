#ifndef GAME_CORE_SPIRAL_HPP
#define GAME_CORE_SPIRAL_HPP

class Spiral
{
public:
    Spiral(int size, int originX, int originY);

    bool GetNext(int& x, int& y);

private:
    bool Step(int& x, int& y);

    int mSize;
    int mOriginX;
    int mOriginY;
    int mCount;
    int mX;
    int mY;
    int mStepX;
    int mStepY;
};

#endif
