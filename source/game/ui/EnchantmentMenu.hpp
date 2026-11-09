#ifndef GAME_UI_ENCHANTMENTMENU_HPP
#define GAME_UI_ENCHANTMENTMENU_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"
#include "game/item/EnchantmentContainer.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"

class Inventory;

class EnchantmentMenu : public AbstractContainerMenu
{
public:
    EnchantmentMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos);
    EnchantmentMenu(boost::shared_ptr<Inventory> inventory, Level* level);

    void broadcastData(net_minecraft_world_inventory::ContainerListener* listener);
    virtual void addSlotListener(net_minecraft_world_inventory::ContainerListener* listener);
    virtual void broadcastChanges();
    virtual void setData(int id, int value);
    std::vector<EnchantmentInstance*>* getEnchantmentList(not_null_ptr<ItemInstance> item, int enchantSlot, int cost);
    virtual void slotsChanged();
    virtual bool clickMenuButton(boost::shared_ptr<Player> player, int id);
    int getGoldCount();
    virtual void removed(boost::shared_ptr<Player> player);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);

private:
    boost::shared_ptr<Container> mEnchantmentContainer;
    Level* mLevel;
    BlockPos mPosition;
    Random mRandom;
    bool mDataChanged;
    long long mEnchantmentSeed;

public:
    int mCosts[3];
    int mEnchantmentIds[3];
    int mEnchantmentLevels[3];
};

#endif
