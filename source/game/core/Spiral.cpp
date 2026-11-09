#include "game/core/Spiral.hpp"

Spiral::Spiral(int size, int originX, int originY)
    : mSize(size),
      mOriginX(originX),
      mOriginY(originY),
      mCount(0),
      mX(0),
      mY(0),
      mStepX(0),
      mStepY(1)
{
}

bool Spiral::Step(int& x, int& y)
{
    bool done = mCount >= mSize * mSize;
    if (!done)
    {
        x = mX + mOriginX;
        y = mY + mOriginY;

        if (mX == mY || (mX < 0 && mX == -mY) || (mX > 0 && mX == 1 - mY))
        {
            int stepX = mStepX;
            mStepX = -mStepY;
            mStepY = stepX;
        }

        mX += mStepX;
        mY += mStepY;
        ++mCount;
    }

    return !done;
}

bool Spiral::GetNext(int& x, int& y)
{
    return Step(x, y);
}
