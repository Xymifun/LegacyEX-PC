#ifndef GAME_CORE_INPUT_HPP
#define GAME_CORE_INPUT_HPP

class LocalPlayer;

namespace phys
{
class Vec2;
}

class Input
{
public:
    Input();
    virtual ~Input();

    virtual void tick(LocalPlayer* player);
    virtual phys::Vec2* getMoveVector();

    float mMoveStrafe;
    float mMoveForward;
    bool mWasJumping;
    bool mJumping;
    bool mSneaking;
    bool mUp;
    bool mDown;
    bool mLeft;
    bool mRight;
    bool mSwimming;
    bool mMovementReady;
    bool mLookReady;
};

#endif
