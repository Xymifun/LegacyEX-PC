#ifndef GAME_MINIGAME_WEAPONMEDAL_HPP
#define GAME_MINIGAME_WEAPONMEDAL_HPP

#include <vector>

#include "game/minigame/DamageDealtMedal.hpp"

class DamageEvent;
class Item;

class WeaponMedal : public DamageDealtMedal
{
public:
    WeaponMedal(int name, float target, unsigned int weight, Item* item);
    WeaponMedal(int name, float target, unsigned int weight, int itemType);
    WeaponMedal(int name, float target, unsigned int weight, std::vector<int>* itemTypes);

    virtual ~WeaponMedal();

    virtual bool IncludeDamage(const DamageEvent* damageEvent) const;

private:
    Item* mItem;
    std::vector<int>* mItemTypes;
};

#endif
