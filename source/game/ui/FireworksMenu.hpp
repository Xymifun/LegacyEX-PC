#ifndef GAME_UI_FIREWORKSMENU_HPP
#define GAME_UI_FIREWORKSMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"
#include "game/world/block/BlockPos.hpp"

class CraftingContainer;
class Inventory;
class Level;
class Player;
class Recipe;
class ResultContainer;

class FireworksMenu : public AbstractContainerMenu
{
public:
    FireworksMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos);

    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual bool canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot);
    virtual void removed(boost::shared_ptr<Player> player);
    virtual void slotsChanged();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual bool isValidIngredient(not_null_ptr<ItemInstance> item, int slot);

private:
    boost::shared_ptr<CraftingContainer> m_craftSlots;
    boost::shared_ptr<ResultContainer> m_resultSlots;
    Level* m_level;
    BlockPos m_playerPos;
    bool m_hasPaper;
    bool m_hasGunpowder;
    bool m_hasFireworkCharge;
};

#endif
