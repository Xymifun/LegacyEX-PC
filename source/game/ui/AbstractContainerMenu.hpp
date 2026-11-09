#ifndef GAME_UI_ABSTRACTCONTAINERMENU_HPP
#define GAME_UI_ABSTRACTCONTAINERMENU_HPP

#include <vector>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include "game/core/ClickType.hpp"
#include "game/core/Slot.hpp"
#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"

class BlockEntity;
class Inventory;
class ItemInstance;
class Level;
class Player;
class ServerGamePacketListenerImpl;
class ServerPlayer;

struct _PlayerKeyHash
{
    std::size_t operator()(const boost::shared_ptr<Player>& player) const
    {
        return boost::hash<Player*>()(player.get());
    }
};

struct _PlayerKeyEq
{
    bool operator()(const boost::shared_ptr<Player>& first, const boost::shared_ptr<Player>& second) const
    {
        return first.get() == second.get();
    }
};

class AbstractContainerMenu
{
    friend class ClientPacketListener;
    friend class IUIScene_AbstractContainerMenu;
    friend class LocalPlayer;
    friend class MultiplayerLocalPlayer;
    friend class ServerGamePacketListenerImpl;
    friend class ServerPlayer;
    friend class UIScene_AbstractContainerMenu;
    friend class UIScene_BeaconMenu;
    friend class UIScene_CraftingMenu;
    friend class UIScene_EnchantingMenu;

public:
    typedef std::vector<not_null_ptr<ItemInstance> > ItemList;
    typedef boost::unordered_set<Slot*> SlotSet;
    typedef boost::unordered_set<boost::shared_ptr<Player>, _PlayerKeyHash, _PlayerKeyEq> PlayerSet;

    AbstractContainerMenu();
    virtual ~AbstractContainerMenu();
    Slot* addSlot(Slot* slot);
    virtual void addSlotListener(net_minecraft_world_inventory::ContainerListener* listener);
    virtual void removeSlotListener(net_minecraft_world_inventory::ContainerListener* listener);
    virtual ItemList* getItems();
    virtual void sendData(int id, int value);
    virtual void broadcastChanges();
    virtual bool needsRendered();
    virtual bool clickMenuButton(boost::shared_ptr<Player> player, int id);
    virtual Slot* getSlotFor(boost::shared_ptr<Container> container, int index);
    virtual Slot* getSlot(int index);
    virtual ItemList getAllItems(int start, int count);
    virtual ItemList takeAll(boost::shared_ptr<Player> player, int button, int start, int count);
    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    static int getQuickcraftHeader(int flags);
    void resetQuickCraft();
    static int getQuickcraftType(int flags);
    static bool isValidQuickcraftType(int type, boost::shared_ptr<Player> player);
    static bool canItemQuickReplace(Slot* slot, not_null_ptr<ItemInstance> item, bool stacking);
    bool canDragTo(Slot* slot);
    static void getQuickCraftSlotCount(SlotSet* slots, int mode, not_null_ptr<ItemInstance> item, int slotCount);
    virtual not_null_ptr<ItemInstance> clicked(int slot, int button, ClickType::eClickType type,
                                               boost::shared_ptr<Player> player);
    virtual bool mayCombine(Slot* slot, not_null_ptr<ItemInstance> item);
    virtual bool canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot);
    virtual bool doQuickMoveLoop() { return true; }
    virtual void clearContainer(boost::shared_ptr<Player> player, Level* level, boost::shared_ptr<Container> container);
    virtual void removed(boost::shared_ptr<Player> player);
    virtual void slotsChanged();
    bool isPauseScreen();
    void setItem(unsigned int index, not_null_ptr<ItemInstance> item);
    void setAll(NonNullList<not_null_ptr<ItemInstance> > items);
    virtual void setData(int id, int value);
    virtual bool stillValid(boost::shared_ptr<Player> player) = 0;
    int backup(boost::shared_ptr<Inventory> inventory);
    bool isSynched(boost::shared_ptr<Player> player);
    void setSynched(boost::shared_ptr<Player> player, bool value);
    static bool stackAsdAsd(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second);
    bool moveItemStackTo(not_null_ptr<ItemInstance> item, int start, int end, bool reverse, bool mergeStacks,
                         bool fillEmpty);
    virtual bool isOverrideResultClick(int slot, int button);
    static int getQuickcraftMask(int header, int type);
    static int getRedstoneSignalFromContainer(boost::shared_ptr<Container> container);
    static int getRedstoneSignalFromBlockEntity(boost::shared_ptr<BlockEntity> entity);
    virtual bool isValidIngredient(not_null_ptr<ItemInstance> item, int slot);
    not_null_ptr<ItemInstance> GetQuickCraftItem(int playerIndex, int slotIndex);
    bool IsSmallInventory();
    static bool matches(AbstractContainerMenu* first, AbstractContainerMenu* second);

    ItemList m_items;
    std::vector<Slot*> m_slots;
    int m_containerId;

protected:
    short m_transactionId;
    int m_quickcraftType;
    int m_quickcraftStatus;
    SlotSet m_quickcraftSlots;
    bool m_needsRendered;
    std::vector<net_minecraft_world_inventory::ContainerListener*> m_listeners;
    bool m_uiQuickCraftActive;
    int m_uiQuickCraftRemaining;
    SlotSet m_uiQuickCraftSlots;
    PlayerSet m_synched;
};

#endif
