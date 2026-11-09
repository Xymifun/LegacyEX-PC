#ifndef GAME_UI_UISCENE_CRAFTINGMENU_HPP
#define GAME_UI_UISCENE_CRAFTINGMENU_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/IUIScene_CraftingMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/ui/UIControl_Touch.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class UILayer;
struct fuiRect;

class UIScene_CraftingMenu : public UIScene, public IUIScene_CraftingMenu
{
public:
    class _SlotInfo
    {
    public:
        _SlotInfo() : item(), displayValue(0x1f), visible(true) {}
        ~_SlotInfo() {}
        not_null_ptr<ItemInstance> item;
        unsigned int displayValue;
        bool visible;
    };

    UIScene_CraftingMenu(int type, void* data, UILayer* layer);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual UIControl* GetMainPanel();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual bool allowRepeat(int action);
    virtual void handleInput(int player, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleDestroy();
    virtual void handleReload();
    virtual void handleTouchBoxRebuild();
    virtual void handleTouchInput(unsigned int player, int touchX, int touchY, int control, bool pressed, bool repeat,
                                  bool released);

    virtual int getPad();
    void hideAllHSlots();
    void hideAllVSlots();
    void SetItemDescription(int index, const wchar_t* text);
    void hideAllIngredientsSlots();
    virtual void setCraftHSlotItem(int, int index, not_null_ptr<ItemInstance> item, unsigned int displayValue);
    virtual void setSpecialSlotItem(int, int index, not_null_ptr<ItemInstance> item, unsigned int displayValue);
    virtual void setCraftVSlotItem(int, int index, not_null_ptr<ItemInstance> item, unsigned int displayValue);
    void setCraftingOutputSlotItem(int index, not_null_ptr<ItemInstance> item);
    void setCraftingOutputSlotRedBox(bool show);
    virtual void setIngredientSlotItem(int, int index, not_null_ptr<ItemInstance> item);
    void setIngredientSlotRedBox(int index, bool show);
    void setIngredientSlotHighlighted(int index);
    void setInventorySlotHighlighted(int index);
    virtual void setIngredientDescriptionItem(int, int index, not_null_ptr<ItemInstance> item);
    void setIngredientDescriptionRedBox(int index, bool show);
    void setIngredientDescriptionText(int index, const wchar_t* text);
    void setPreviewSlotItem(int index, not_null_ptr<ItemInstance> item);
    void setShowCraftHSlot(int index, bool show);
    void showTabHighlight(int index, bool show);
    void setHTabEnabled(int index, bool enabled);
    void setSpinnerTitle(const wchar_t* text);
    void setGroupText(const wchar_t* text);
    void setDescriptionText(const wchar_t* text);
    void setDescriptionTextTop(const wchar_t* text);
    void SetItemName(const wchar_t* text);
    void setItemText(const wchar_t* text);
    void scrollDescriptionUp();
    void scrollDescriptionDown();
    void updateHighlightAndScrollPositions();
    void updateVSlotPositions(int first, int second);
    void UpdateMultiPanel(bool show);
    void ShowDescriptionPanel(bool show);
    void ShowSuperCraftingSlots(int count);
    void SetCraftingMenu(int category, int item, bool enabled);
    void SetHTabItem(int index, not_null_ptr<ItemInstance> item);
    void handleInventoryUpdated(void* data);
    void HandleMessage(EUIMessage message, void* data);

private:
    _SlotInfo m_horizontalSlotItems[12];
    _SlotInfo m_verticalSlotItems[3];
    _SlotInfo m_ingredientSlotItems[9];
    _SlotInfo m_outputSlotItem;
    _SlotInfo m_ingredientDescriptionItems[5];
    _SlotInfo m_previewSlotItem;
    not_null_ptr<ItemInstance> m_tabItems[7];
    AbstractContainerMenu* m_openMenu;
    bool m_touchDragging;
    bool m_inventoryUpdatesDeferred;
    UIControl m_mainPanel;
    UIControl m_descriptionPanel;
    UIControl m_craftingSelector;
    UIControl m_slotSelector2;
    UIControl m_slotSelector3;
    UIControl_SlotList m_craftingHSlots;
    UIControl_SlotList m_crafting1VSlot;
    UIControl_SlotList m_crafting2VSlots[2];
    UIControl_SlotList m_crafting3VSlots[3];
    UIControl_SlotList m_ingredientsLayout;
    UIControl_SlotList m_ingredientsLayoutSmall;
    UIControl_SlotList m_craftingOutput;
    UIControl_SlotList m_ingredientSlots[5];
    UIControl_SlotList m_previewSlot;
    UIControl_SlotList m_doubleSpinnerSlots[2];
    UIControl_SlotList m_tripleSpinnerSlots[3];
    UIControl_SlotList m_inventory;
    UIControl_SlotList m_hotBar;
    UIControl_Label m_ingredientDescriptions[5];
    UIControl_HTMLLabel m_descriptionText;
    UIControl_HTMLLabel m_descriptionTextTop;
    UIControl_Label m_groupName;
    UIControl_Label m_itemName;
    UIControl_Label m_inventoryLabel;
    UIControl_Label m_ingredientsLabel;
    UIControl_Label m_previewLabel;
    UIControl_Label m_spinnerTitle;
    UIControl_Touch m_touchPanels[13];
    int m_touchScrollY;
    int m_lastPanelDisplay;
#ifdef MINECRAFT_PC
    int m_pcPressControl;
    int m_pcPressHorizontal;
    int m_pcPressVertical;
    bool m_pcPressScrolled;
#endif
};

#endif
