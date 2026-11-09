#ifndef GAME_UI_UISCENE_CREATIVEMENU_HPP
#define GAME_UI_UISCENE_CREATIVEMENU_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/ui/UIControl_Touch.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"

class AbstractContainerMenu;
class Inventory;
class Player;
class SimpleContainer;
class UILayer;

class UIScene_CreativeMenu : public UIScene_AbstractContainerMenu
{
public:
    static const int ITEM_SLOT_COUNT = 50;

    class ItemBuilder
    {
    public:
        virtual not_null_ptr<ItemInstance> build() const = 0;
    };

    enum ECreativeInventoryTabs
    {
        ECreativeInventoryTabs_Building = 0,
        ECreativeInventoryTabs_Decoration = 1,
        ECreativeInventoryTabs_Redstone = 2,
        ECreativeInventoryTabs_Transportation = 3,
        ECreativeInventoryTabs_Miscellaneous = 4,
        ECreativeInventoryTabs_Food = 5,
        ECreativeInventoryTabs_Tools = 6,
        ECreativeInventoryTabs_Combat = 7,
        ECreativeInventoryTabs_COUNT = 8
    };

    enum ECreative_Inventory_Groups
    {
        ECreative_Inventory_Groups_None = -1,
        ECreative_Inventory_Groups_BuildingBlocks,
        ECreative_Inventory_Groups_Decoration,
        ECreative_Inventory_Groups_Redstone,
        ECreative_Inventory_Groups_Transport,
        ECreative_Inventory_Groups_Materials,
        ECreative_Inventory_Groups_Food,
        ECreative_Inventory_Groups_ToolsAndCombat,
        ECreative_Inventory_Groups_BrewingIngredients,
        ECreative_Inventory_Groups_Potions,
        ECreative_Inventory_Groups_LongPotions,
        ECreative_Inventory_Groups_StrongPotions,
        ECreative_Inventory_Groups_Miscellaneous,
        ECreative_Inventory_Groups_Count = 15
    };

    class TabSpec
    {
    public:
        TabSpec(const wchar_t* title, int titleStringId, int groupCount, ECreative_Inventory_Groups* groups,
                int specialGroupCount, ECreative_Inventory_Groups* specialGroups, int extraGroupCount,
                ECreative_Inventory_Groups* extraGroups);
        ~TabSpec();
        unsigned int getPageCount();
        void populateMenu(AbstractContainerMenu* menu, int specialGroup, unsigned int page);

        const wchar_t* m_title;
        int m_titleStringId;
        int m_groupCount;
        ECreative_Inventory_Groups* m_groups;
        int m_specialGroupCount;
        ECreative_Inventory_Groups* m_specialGroups;
        int m_extraGroupCount;
        ECreative_Inventory_Groups* m_extraGroups;
        unsigned int m_pageCount;
        unsigned int m_itemsPerPage;
        unsigned int m_itemCount;
        unsigned int m_extraItemCount;
    };

    class ItemPickerMenu : public AbstractContainerMenu
    {
    public:
        ItemPickerMenu(boost::shared_ptr<SimpleContainer> container, boost::shared_ptr<Inventory> inventory);
        virtual bool stillValid(boost::shared_ptr<Player> player);
        virtual bool isOverrideResultClick(int slot, int button);
        virtual bool doQuickMoveLoop();

    private:
        boost::shared_ptr<SimpleContainer> m_container;
        boost::shared_ptr<Inventory> m_inventory;
    };

    UIScene_CreativeMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_CreativeMenu();

    virtual int getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position);
    virtual void GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, _UIVec2D* position,
                                   _UIVec2D* size);
    virtual void handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction,
                                        bool wrap);
    virtual void handleOtherClicked(int button, IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                    bool pressed);
    virtual unsigned int GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                      IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y);
    virtual unsigned int handleValidKeyPress(int key, int value, int player);
    virtual void handleOutsideClicked(int x, int y, int button);
    virtual void handleAdditionalKeyPress(int key);
    virtual void handleSlotListClicked(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int button);
    virtual int getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual bool IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int CanHaveFocus(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int overrideTooltips(IUIScene_AbstractContainerMenu::ESceneSection section,
                                          not_null_ptr<ItemInstance> slotItem, bool carriedIsItem, bool slotIsItem,
                                          bool carriedSameAsSlot, int slotSpace, EToolTipItem& toolTip0,
                                          EToolTipItem& toolTip2, EToolTipItem& toolTip3, EToolTipItem& toolTip5,
                                          EToolTipItem& toolTip10, EToolTipItem& toolTip6, EToolTipItem& toolTip7);
    void updateTabHighlightAndText(ECreativeInventoryTabs tab);
    void updateScrollCurrentPage(int page, int pageCount);
    void switchTab(ECreativeInventoryTabs tab);
    void ScrollBar(_UIVec2D position);
#ifdef MINECRAFT_PC
    virtual bool PcAllowsHotbarSwap() { return false; }
    virtual void PcWheelAndDrag(int wheelSteps, bool leftHeld);
    bool m_pcScrollDrag;
#endif
    bool getEmptyInventorySlot(not_null_ptr<ItemInstance> item, int& slot);
    static void staticCtor();
    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual UIControl* GetMainPanel();
    virtual void handleTouchBoxRebuild();
    virtual void handleTimerComplete(int timerId);
    virtual int getSceneType();

private:
    AbstractContainerMenu* m_containerMenu;
    int m_currentTab;
    int m_tabPages[8];
    int m_tabCurrentPages[8];
    UIControl_SlotList m_containerList;
    UIControl_Touch m_touchPanels[9];
    bool m_restoreSelectorPending;
    int m_savedSlotX;
    int m_savedSlotY;
    int m_emptySlotX;
    int m_emptySlotY;
    bool m_unreferencedFlag;

    static TabSpec** s_tabSpecs;

public:
    static std::vector<const ItemBuilder*> s_groupItems[ECreative_Inventory_Groups_Count];
};

#endif
