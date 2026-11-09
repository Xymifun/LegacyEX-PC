#ifndef GAME_UI_FJ_CRAFTINGMENU_HPP
#define GAME_UI_FJ_CRAFTINGMENU_HPP

#include <string>

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_CraftingMenu : public FJ_Document
{
public:
    FJ_CraftingMenu(fuiRenderNode* renderNode);

    void handleSlideComplete(FJ_Event* event);
    static void _handleSlideComplete(FJ_FuiNode* node, FJ_Event* event);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void showPanelDisplay(int panel);
    void ShowDescriptionPanel(bool show);
    void ShowSuperCraftingSlots(int count);
    void MoveSelector(int horizontal, int vertical);
    virtual void init(FJ_Event* event);
    void InitCraftingMenu(int type);
    void SetItemName(const std::wstring& name);
    void SetItemDescription(int index, const std::wstring& description);
    void EnableTab(int index, bool enabled);
    FJ_FuiNode* SetupIggySlot(FJ_FuiNode* node);
    void SetCraftingMenu(int category, int item, bool enabled);
    void SetSafeZone(int top, int bottom, int left, int right);
    void SelectVerticalItem(int index);
    void UpdateTabs(FJ_Event* event);
    void SetActiveTab(int tab, int subtab);
    void ShowIngredient(int index, bool show);

private:
    int m_craftingType;
    FJ_FuiNode* m_slot2Items[2];
    FJ_FuiNode* m_slot3Items[3];
    int m_horizontalTabCount;
    int m_verticalTabCount;
    int m_panelMode;
    int m_panelType;
    unsigned int m_activeTab;
    unsigned int m_activeSubtab;
    int m_selectorStep;
    bool m_ingredients[5];
    int m_descriptionBaseY;
};

#endif
