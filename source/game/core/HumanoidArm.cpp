#include "game/core/HumanoidArm.hpp"

const HumanoidArm* HumanoidArm::LEFT = new HumanoidArm();
const HumanoidArm* HumanoidArm::RIGHT = new HumanoidArm();

HumanoidArm::HumanoidArm()
{
}

const HumanoidArm* HumanoidArm::getOpposite() const
{
    if (this == LEFT)
    {
        return RIGHT;
    }

    return LEFT;
}
