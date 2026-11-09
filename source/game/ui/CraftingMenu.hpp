#ifndef GAME_UI_CRAFTINGMENU_HPP
#define GAME_UI_CRAFTINGMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"
#include "game/world/block/BlockPos.hpp"

class CraftingContainer;
class Inventory;
class Level;
class Player;
class Recipe;
class ResultContainer;

class CraftingMenu : public AbstractContainerMenu
{
public:
    static const int RESULT_SLOT;
    static const int CRAFT_SLOT_START;
    static const int INV_SLOT_START;

    CraftingMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos);

    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual bool canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot);
    virtual void removed(boost::shared_ptr<Player> player);
    virtual void slotsChanged();
    virtual bool stillValid(boost::shared_ptr<Player> player);

    int getX();
    int getY();
    int getZ();

    boost::shared_ptr<CraftingContainer> m_craftSlots;

private:
    boost::shared_ptr<ResultContainer> m_resultSlots;
    Level* m_level;
    BlockPos m_playerPos;
};

#endif
