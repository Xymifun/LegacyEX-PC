#include "game/minigame/WeaponMedal.hpp"

#include <algorithm>

#include "game/core/DamageEvent.hpp"
#include "game/item/Item.hpp"

WeaponMedal::WeaponMedal(int name, float target, unsigned int weight, Item* item)
    : DamageDealtMedal(name, target, weight, OPERATOR_AT_LEAST, true, false),
      mItem(item),
      mItemTypes(new std::vector<int>)
{
}

WeaponMedal::WeaponMedal(int name, float target, unsigned int weight, int itemType)
    : DamageDealtMedal(name, target, weight, OPERATOR_AT_LEAST, true, false),
      mItem(NULL),
      mItemTypes(new std::vector<int>)
{
    mItemTypes->push_back(itemType);
}

WeaponMedal::WeaponMedal(int name, float target, unsigned int weight, std::vector<int>* itemTypes)
    : DamageDealtMedal(name, target, weight, OPERATOR_AT_LEAST, true, false),
      mItem(NULL),
      mItemTypes(itemTypes)
{
}

WeaponMedal::~WeaponMedal()
{
    delete mItemTypes;
}

bool WeaponMedal::IncludeDamage(const DamageEvent* damageEvent) const
{
    Item* item = damageEvent->mWeapon.GetItem();

    if (mItem != NULL || mItemTypes->empty())
    {
        return item == mItem;
    }

    bool included = false;
    if (item != NULL)
    {
        std::vector<int>::const_iterator it = std::find(mItemTypes->begin(), mItemTypes->end(),
                                                        item->getBaseItemType());
        if (it != mItemTypes->end())
        {
            included = true;
        }
    }

    return included;
}
