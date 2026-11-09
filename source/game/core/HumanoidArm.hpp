#ifndef GAME_CORE_HUMANOIDARM_HPP
#define GAME_CORE_HUMANOIDARM_HPP

class HumanoidArm
{
public:
    static const HumanoidArm* LEFT;
    static const HumanoidArm* RIGHT;

    HumanoidArm();

    const HumanoidArm* getOpposite() const;
};

#endif
