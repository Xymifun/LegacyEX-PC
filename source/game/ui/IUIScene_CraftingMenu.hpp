#ifndef GAME_UI_IUISCENE_CRAFTINGMENU_HPP
#define GAME_UI_IUISCENE_CRAFTINGMENU_HPP

#include <string>
#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/Tutorial.hpp"

class AbstractContainerMenu;
class CraftingContainer;
class Inventory;
class ItemInstance;
class Player;

struct _CraftingPanelScreenInput
{
    boost::shared_ptr<Player> player;
    int craftingMode;
    bool compactInventory;
    int pad;
    int x;
    int y;
    int z;
};

class IUIScene_CraftingMenu
{
public:
    IUIScene_CraftingMenu();
    virtual ~IUIScene_CraftingMenu();

    int getGroupAtIndex(int index);
    int getCurrentGroup();
    int GetRecipeId(int group, int recipe);
    bool isSpinnerGroup();
    bool isInventoryItem();
    bool isDuplicateItem();
    bool isCustomRecipe();
    int GetRecipeCount(int group);
    int getCurrentVSlotCount();
    bool isItemSelected(int itemId);
    not_null_ptr<ItemInstance> GetBaseItemSlot(int column, int slot, bool& enabled);
    not_null_ptr<ItemInstance> GetSpinnerSlot(int spinner, int slot, bool& enabled);
    int GetCustomIngredientCount(not_null_ptr<ItemInstance> item);
    bool isValidCustomRecipeIngredient(int index, not_null_ptr<ItemInstance> item);
    int GetCustomIngredientIndex(not_null_ptr<ItemInstance> item);
    void UpdateMaintainedInventory();
    void DisplayCustomRecipeIngredients(int recipe);
    void UpdateDescriptionText(bool show);
    void CheckRecipesAvailable();
    int getNumSpinnerSlots();
    bool canAddMoreStacks();
    void UpdateHorizontalSlots();
    void UpdateVerticalSlots();
    bool stackedCraftingAllowed();
    void DisplayIngredients();
    bool canAddCustomRecipeIngredient(not_null_ptr<ItemInstance> item);
    void UpdateTooltips();
    void UpdateHighlight();
    void UpdateSlotsAndHighlight();
    void RemoveLastCustomRecipeIngredient();
    void updateAccumulatedItem();
    int getCurrentVSlotMax();
    const wchar_t* GetGroupNameText(int group);
    void refreshAllSlots();
    void ClearCustomRecipeIngredients();
    void refreshBaseItemSlots(not_null_ptr<ItemInstance> item);
    void UpdateTabs(bool force);
    void UpdateHTab();
    void RemoveStackedCraftRecipe();
    bool isCustomIngredientInBaseRecipe(not_null_ptr<ItemInstance> item);
    void AddCustomRecipeIngredient(not_null_ptr<ItemInstance> item);
    bool AddStackedCraftRecipe(bool advanceGroup);
    void CraftCurrentItem();
    void UpdateVTab();
    void NavTabUp();
    void NavTabDown();
    void ItemScrollRight();
    void ItemScrollLeft();
    void ItemScrollUp();
    void ItemScrollDown();
    bool handleKeyDown(int pad, int key, bool repeat);
    void Init(_CraftingPanelScreenInput* input);
    void HandleInventoryUpdated();

    virtual int getPad() = 0;
    virtual void hideAllHSlots() = 0;
    virtual void hideAllVSlots() = 0;
    virtual void hideAllIngredientsSlots() = 0;
    virtual void setCraftHSlotItem(int pad, int slot, not_null_ptr<ItemInstance> item, unsigned int flags) = 0;
    virtual void setSpecialSlotItem(int pad, int slot, not_null_ptr<ItemInstance> item, unsigned int flags) = 0;
    virtual void setCraftVSlotItem(int pad, int slot, not_null_ptr<ItemInstance> item, unsigned int flags) = 0;
    virtual void setCraftingOutputSlotItem(int pad, not_null_ptr<ItemInstance> item) = 0;
    virtual void setCraftingOutputSlotRedBox(bool red) = 0;
    virtual void setIngredientSlotItem(int pad, int slot, not_null_ptr<ItemInstance> item) = 0;
    virtual void setIngredientSlotRedBox(int slot, bool red) = 0;
    virtual void setIngredientSlotHighlighted(int slot) = 0;
    virtual void setInventorySlotHighlighted(int slot) = 0;
    virtual void setIngredientDescriptionItem(int pad, int slot, not_null_ptr<ItemInstance> item) = 0;
    virtual void setIngredientDescriptionRedBox(int slot, bool red) = 0;
    virtual void setIngredientDescriptionText(int slot, const wchar_t* text) = 0;
    virtual void setPreviewSlotItem(int pad, not_null_ptr<ItemInstance> item) = 0;
    virtual void setShowCraftHSlot(int slot, bool show) = 0;
    virtual void showTabHighlight(int tab, bool show) = 0;
    virtual void setHTabEnabled(int tab, bool enabled) = 0;
    virtual void setSpinnerTitle(const wchar_t* title) = 0;
    virtual void setGroupText(const wchar_t* text) = 0;
    virtual void setDescriptionText(const wchar_t* text) = 0;
    virtual void setDescriptionTextTop(const wchar_t* text) = 0;
    virtual void setItemText(const wchar_t* text) = 0;
    virtual void scrollDescriptionUp() = 0;
    virtual void scrollDescriptionDown() = 0;
    virtual void updateHighlightAndScrollPositions() = 0;
    virtual void updateVSlotPositions(int slotCount, int slot) = 0;
    virtual void UpdateMultiPanel(bool show) = 0;
    virtual void ShowDescriptionPanel(bool show) = 0;
    virtual void ShowSuperCraftingSlots(int count) = 0;
    virtual void SetCraftingMenu(int horizontalSlot, int verticalSlot, bool spinner) = 0;
    virtual void SetHTabItem(int tab, not_null_ptr<ItemInstance> item) = 0;

protected:
    static int m_craftingTabOrder2x2[4];
    static int m_craftingTabOrder3x3[4];
    static int m_groupMap2x2[5][7];
    static int m_groupMap3x3[5][7];

    int m_recipeTable[12][50];
    int m_horizontalSlot;
    int m_verticalColumn;
    int m_recipePage;
    int m_craftingMode;
    boost::shared_ptr<Player> m_player;
    int m_currentGroup;
    int m_groupCount;
    int m_verticalSlots[3];
    int* m_groupMap;
    boost::shared_ptr<CraftingContainer> m_craftingContainer;
    std::vector<int> m_baseItemSlots;
    std::vector<int> m_availableRecipes;
    std::vector<std::pair<int, bool> > m_duplicateItemSlots;
    not_null_ptr<ItemInstance> m_baseRecipeItem;
    int m_selectedInventorySlot;
    std::vector<std::pair<int, int> > m_stackedRecipes;
    not_null_ptr<ItemInstance> m_accumulatedItem;
    std::vector<std::pair<int, int> > m_usedIngredients;
    bool m_baseFromInventory;
    bool m_hasGunpowder;
    std::vector<std::pair<not_null_ptr<ItemInstance>, bool> > m_customRecipes[3];
    std::vector<std::pair<not_null_ptr<ItemInstance>, bool> > m_customRecipeIngredients;
    std::vector<not_null_ptr<ItemInstance> > m_recipeIngredients;
    NonNullList<not_null_ptr<ItemInstance> > m_customGridItems;
    int m_customRecipeSlots[3];
    bool m_customRecipeValid;
    std::vector<int> m_customIngredientSlots;
    int m_inventoryStart;
    int m_inventoryEnd;
    boost::shared_ptr<Inventory> m_maintainedInventory;
    AbstractContainerMenu* m_inventoryMenu;
    int m_horizontalSlotCount;
    int m_ingredientSlotCount;
    int m_panelDisplay;
    int m_savedPanelDisplay;
    int m_ingredientCount;
    bool m_tabsDirty;
    bool m_compactInventory;
    wchar_t m_ingredientDescriptionText[128][5];
    int m_currentTab;
    eTutorial_State m_previousTutorialState;
};

#endif
