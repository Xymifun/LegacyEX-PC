#ifndef GAME_UI_ANVILMENU_HPP
#define GAME_UI_ANVILMENU_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"
#include "game/world/block/BlockPos.hpp"

class Inventory;
class Level;
class Player;
class RepairContainer;
class ResultContainer;

namespace net_minecraft_world_inventory
{
class ContainerListener;
}

class AnvilMenu : public AbstractContainerMenu
{
public:
    AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& position,
              boost::shared_ptr<Player> player);
    AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level, boost::shared_ptr<Player> player);

    void _init(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& position,
               boost::shared_ptr<Player> player);
    void createResult();
    void slotsChanged(boost::shared_ptr<Container> container);
    virtual void addSlotListener(net_minecraft_world_inventory::ContainerListener* listener);
    virtual void setData(int id, int value);
    virtual void removed(boost::shared_ptr<Player> player);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    void setItemName(const std::wstring& name, bool censored);

    boost::shared_ptr<Container> mResultSlots;
    boost::shared_ptr<Container> mInputSlots;
    Level* mLevel;
    BlockPos mPosition;
    int mRepairCost;
    int mMaterialCost;
    std::wstring mItemName;
    bool mItemNameCensored;
    boost::shared_ptr<Player> mPlayer;
};

#endif
