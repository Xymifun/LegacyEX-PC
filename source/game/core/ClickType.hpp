#ifndef GAME_CORE_CLICKTYPE_HPP
#define GAME_CORE_CLICKTYPE_HPP

class ClickType
{
public:
    enum eClickType
    {
        eClickType_Pickup = 0,
        eClickType_QuickMove = 1,
        eClickType_Swap = 2,
        eClickType_Clone = 3,
        eClickType_Throw = 4,
        eClickType_QuickCraft = 5,
        eClickType_PickupAll = 6
    };
};

#endif
