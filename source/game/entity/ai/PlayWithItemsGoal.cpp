#include "game/entity/ai/PlayWithItemsGoal.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Dolphin.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

PlayWithItemsGoal::~PlayWithItemsGoal()
{
}

bool PlayWithItemsGoal::canUse()
{
    if (!mDolphin->isInWater())
        return false;

    Level* level = mDolphin->mLevel;
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
        typeid(ItemEntity), mDolphin->mBoundingBox->grow(8.0, 8.0, 8.0));
    bool result = !entities->empty() || !mDolphin->getMainHandItem()->isEmpty();
    delete entities;
    return result;
}

bool PlayWithItemsGoal::canContinueToUse()
{
    return mDolphin->isInWater();
}

void PlayWithItemsGoal::start()
{
    Level* level = mDolphin->mLevel;
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
        typeid(ItemEntity), mDolphin->mBoundingBox->grow(8.0, 8.0, 8.0));
    if (!entities->empty())
    {
        mDolphin->getNavigation()->moveTo(entities->at(0), 1.2f);
        mDolphin->playSound(SoundEvents::DOLPHIN_PLAY, 1.0f, 1.0f);
    }
    delete entities;
}

void PlayWithItemsGoal::stop()
{
    not_null_ptr<ItemInstance> item = mDolphin->getMainHandItem();
    if (!item->isEmpty())
    {
        mDolphin->drop(item);
        mDolphin->setItemSlot(EquipmentSlot::MAINHAND, ItemInstance::EMPTY);
    }
}

void PlayWithItemsGoal::tick()
{
    not_null_ptr<ItemInstance> item = mDolphin->getMainHandItem();
    if (!item->isEmpty())
    {
        mDolphin->drop(item);
        mDolphin->setItemSlot(EquipmentSlot::MAINHAND, ItemInstance::EMPTY);
        return;
    }

    Level* level = mDolphin->mLevel;
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
        typeid(ItemEntity), mDolphin->mBoundingBox->grow(8.0, 8.0, 8.0));
    if (!entities->empty())
    {
        mDolphin->getNavigation()->moveTo(entities->at(0), 1.2f);
    }
    delete entities;
}

std::wstring PlayWithItemsGoal::getName()
{
    return L"Play with Item Goal!!";
}
