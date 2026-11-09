#include "game/world/block/ClockItem_ItemPropertyFunction.hpp"

#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/util/Math.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"

double ClockItem_ItemPropertyFunction::wobble(Level* level, double value)
{
    if (level->getGameTime() != mLastUpdate)
    {
        mLastUpdate = level->getGameTime();
        double delta = value - mRotation;
        delta = Mth::positiveModulo(delta + 0.5, 1.0) - 0.5;
        mWobble += delta * 0.1;
        mWobble *= 0.9;
        mRotation = Mth::positiveModulo(mRotation + mWobble, 1.0);
    }
    return mRotation;
}

float ClockItem_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                           boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<Entity> holder = entity ? boost::shared_ptr<Entity>(entity)
                                              : boost::static_pointer_cast<Entity>(itemInstance->getFrame());

    if (level == NULL && holder)
        level = holder->mLevel;

    if (level == NULL)
        return 0.0f;

    double value;
    if (level->mDimension->isNaturalDimension())
        value = level->getTimeOfDay(1.0f);
    else
        value = Math::random();

    return (float)Mth::positiveModulo((float)wobble(level, value), 1.0f);
}
