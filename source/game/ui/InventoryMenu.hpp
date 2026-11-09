#ifndef GAME_UI_INVENTORYMENU_HPP
#define GAME_UI_INVENTORYMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"
#include "game/item/not_null_ptr.hpp"

class Container;
class CraftingContainer;
class EquipmentSlot;
class Inventory;
class ItemInstance;
class Player;
class ResultContainer;

class InventoryMenu : public AbstractContainerMenu
{
public:
    class OffhandSlot : public Slot
    {
    public:
        OffhandSlot(boost::shared_ptr<Container> container, int index, int x, int y);
        virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    };

    static const int RESULT_SLOT;
    static const int CRAFT_SLOT_START;
    static const int CRAFT_SLOT_END;
    static const int ARMOR_SLOT_START;
    static const int ARMOR_SLOT_END;
    static const int INV_SLOT_START;
    static const int INV_SLOT_END = 36;
    static const int USE_ROW_SLOT_START;
    static const int USE_ROW_SLOT_END;

    InventoryMenu(boost::shared_ptr<Inventory> inventory, bool active, Player* player);

    static void staticCtor();
    void _init(boost::shared_ptr<Inventory> inventory, bool active);
    virtual void slotsChanged();
    virtual void removed(boost::shared_ptr<Player> player);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual bool mayCombine(Slot* slot, not_null_ptr<ItemInstance> item);
    virtual bool canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot);
    virtual not_null_ptr<ItemInstance> clicked(int slot, int button, ClickType::eClickType type,
                                               boost::shared_ptr<Player> player);

private:
    static const EquipmentSlot* SLOT_IDS[4];

    Player* mPlayer;

public:
    boost::shared_ptr<CraftingContainer> mCraftSlots;

private:
    boost::shared_ptr<ResultContainer> mResultSlots;
    bool mActive;
};

#endif
