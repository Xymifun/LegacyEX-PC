#ifndef GAME_ENTITY_MOB_ENTITYCATEGORY_HPP
#define GAME_ENTITY_MOB_ENTITYCATEGORY_HPP

class EntityCategory
{
public:
    enum E_EntityCategory
    {
        eEntityCategory_Undefined = 0,
        eEntityCategory_Player = 0x1,
        eEntityCategory_Mob = 0x2,
        eEntityCategory_Monster = 0x4,
        eEntityCategory_Humanoid = 0x8,
        eEntityCategory_Animal = 0x10,
        eEntityCategory_Rideable = 0x100,
        eEntityCategory_Item = 0x400,
        eEntityCategory_Bat = 0x800,
        eEntityCategory_Villager = 0x1000,
        eEntityCategory_Arthropod = 0x2000,
        eEntityCategory_Undead = 0x4000,
        eEntityCategory_Zombie = 0x8000,
        eEntityCategory_Minecart = 0x10000,
        eEntityCategory_Boat = 0x20000
    };
};

#endif
