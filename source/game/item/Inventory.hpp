#ifndef GAME_ITEM_INVENTORY_HPP
#define GAME_ITEM_INVENTORY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockState;
class CompoundTag;
class Item;
class ItemInstance;
class Level;
class ListTag;
class Minecraft;
class Player;
class ServerGamePacketListenerImpl;
class MultiPlayerGameMode;

class Inventory : public Container
{
public:
    static const int INVENTORY_SIZE;

    explicit Inventory(Player* player);
    virtual ~Inventory();

    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();

    static bool isHotbarSlot(int slot);
    const not_null_ptr<ItemInstance>& getSelected();
    static int getSelectionSize();
    bool isSameItem(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second, bool ignoreTags);
    bool hasRemainingSpaceForItem(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second, bool ignoreTags);
    int getFreeSlot();
    bool hasPickedItem();
    int findSlotMatchingItem(not_null_ptr<ItemInstance> item, bool ignoreTags);
    int getSuitableHotbarSlot();
    void pickSlot(int slot);
    void setPickedItem(not_null_ptr<ItemInstance> item);
    int findSlotMatchingUnusedItem(not_null_ptr<ItemInstance> item);
    void swapPaint(int direction);
    int clearInventory(Item* item, int auxValue, int count, CompoundTag* tag);
    int getSlotWithRemainingSpace(not_null_ptr<ItemInstance> item);
    int addResource(int slot, not_null_ptr<ItemInstance> item);
    int addResource(not_null_ptr<ItemInstance> item);
    void tick();
    bool add(int slot, not_null_ptr<ItemInstance> item);
    bool add(not_null_ptr<ItemInstance> item);
    void placeItemBackInInventory(Level* level, not_null_ptr<ItemInstance> item);

    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    void removeItem(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    float getDestroySpeed(const BlockState* block);
    ListTag* save(ListTag* tag);
    void load(ListTag* tag);

    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual int getMaxStackSize();
    bool canDestroy(const BlockState* block);
    not_null_ptr<ItemInstance> getArmor(int slot);
    void hurtArmor(float damage);
    void dropAll();
    virtual void setChanged();
    void setCarried(not_null_ptr<ItemInstance> item);
    not_null_ptr<ItemInstance> getCarried();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    void replaceWith(boost::shared_ptr<Inventory> inventory);
    void replaceWith_NONRECURSIVE(boost::shared_ptr<Inventory> inventory);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();
    int countMatches(not_null_ptr<ItemInstance> item);
    void OnRemove(int slot);
    void PrintContents();
    int getSlot(Item* item, int auxValue, CompoundTag* tag);
    bool removeResource(Item* item, bool flag);
    bool removeResource(Item* item, int auxValue, bool flag);
    void removeResources(not_null_ptr<ItemInstance> item);
    int removeResourcesWithTags(not_null_ptr<ItemInstance> item);
    not_null_ptr<ItemInstance> getResourceItem(Item* item);
    not_null_ptr<ItemInstance> getResourceItem(Item* item, int auxValue);
    not_null_ptr<ItemInstance> getResourceItem(Item* item, int auxValue, CompoundTag* tag);
    bool hasResource(Item* item);

    bool IsHeldItem();
    int getTimesChanged();
    bool contains(not_null_ptr<ItemInstance> item);
    int countMatches(Item* item);

private:
    friend class MerchantMenu;
    friend class Minecraft;
    friend class Player;
    friend class ServerGamePacketListenerImpl;
    friend class MultiPlayerGameMode;
    friend class Gui;
    friend class LocalPlayer;
    friend class MasterGameMode;
    friend class FurnaceMenu;
    friend class EnchantmentMenu;
    friend class UIScene_AnvilMenu;
    friend class CraftingMenu;
    friend class FireworksMenu;
    friend class IUIScene_CraftingMenu;
    friend class InventoryMenu;
    friend class EnderMan;

    NonNullList<not_null_ptr<ItemInstance> > m_items;
    NonNullList<not_null_ptr<ItemInstance> > m_armor;
    NonNullList<not_null_ptr<ItemInstance> > m_offhand;
    NonNullList<not_null_ptr<ItemInstance> > m_compartments[3];
    bool m_hasPickedItem;

public:
    int m_selected;

private:
    Player* m_player;
    not_null_ptr<ItemInstance> m_auxiliaryItem;
    not_null_ptr<ItemInstance> m_carried;
    int m_timesChanged;
};

#endif
