#include "game/core/AreaHint.hpp"

#include "game/core/Minecraft.hpp"
#include "game/util/AABB.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

namespace
{
const int AREA_HINT_TYPE_VALUE = 5;
const TutorialHint::eHintType AREA_HINT_TYPE = static_cast<TutorialHint::eHintType>(AREA_HINT_TYPE_VALUE);
const eTutorial_State AREA_HINT_ANY_STATE = eTutorial_State_Completed;
const eTutorial_State AREA_HINT_NO_COMPLETION_STATE = eTutorial_State_Invalid;
}

AreaHint::AreaHint(eTutorial_Hint hint, Tutorial* tutorial, eTutorial_State state, eTutorial_State completionState,
                   int descriptionId, double x0, double y0, double z0, double x1, double y1, double z1,
                   bool repeatMessage, bool insideArea)
    : TutorialHint(hint, tutorial, descriptionId, AREA_HINT_TYPE, repeatMessage, true, -1),
      area(AABB::newPermanent(x0, y0, z0, x1, y1, z1)),
      insideArea(insideArea),
      state(state),
      completionState(completionState)
{
}

AreaHint::~AreaHint()
{
    delete area;
}

int AreaHint::tick()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if ((state == AREA_HINT_ANY_STATE || tutorial->currentState == state) && active)
    {
        Vec3* pos = minecraft->mPlayer->getPos(1.0f);
        if (area->contains(pos) == insideArea)
        {
            if (completionState == AREA_HINT_NO_COMPLETION_STATE)
            {
                active = false;
                return descriptionId;
            }
            if (tutorial->isStateCompleted(completionState))
            {
                active = false;
                return -1;
            }
            return descriptionId;
        }
    }
    return -1;
}
