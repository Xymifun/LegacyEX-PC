#include "game/ui/UIScene_CraftingMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Slot.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/CraftingMenu.hpp"
#include "game/ui/FJ_CraftingMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/GameSettings.hpp"

#include <cstdio>

UIScene_CraftingMenu::UIScene_CraftingMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer),
      IUIScene_CraftingMenu()
{
    _CraftingPanelScreenInput* input = static_cast<_CraftingPanelScreenInput*>(data);
    m_tabsDirty = false;
    m_inventoryUpdatesDeferred = true;
    m_lastPanelDisplay = -1;
    IUIScene_CraftingMenu::Init(input);
    initialiseMovie();
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->InitCraftingMenu(m_craftingMode);
    if (m_craftingMode == 0)
        m_ingredientsLayout = m_ingredientsLayoutSmall;

    SetItemDescription(0, L"");
    SetItemDescription(1, L"");
    SetItemDescription(2, L"");
    SetItemDescription(3, L"");
    m_descriptionTextTop.init(UIString(L""));
    m_descriptionText.init(UIString(L""));
    m_groupName.init(UIString(L""));
    SetItemName(L"");
    m_inventoryLabel.init(UIString(L10N::GetString(IDS_INVENTORY)));
    m_ingredientsLabel.init(UIString(L10N::GetString(IDS_INGREDIENTS)));
    m_previewLabel.init(UIString(L10N::GetString(IDS_PREVIEW)));
    m_spinnerTitle.init(UIString(L10N::GetString(IDS_BANNER_CREATE_BANNER)));
    m_touchDragging = false;
#ifdef MINECRAFT_PC
    m_pcPressControl = -1;
    m_pcPressHorizontal = -1;
    m_pcPressVertical = -1;
    m_pcPressScrolled = false;
#endif

    AbstractContainerMenu* menu;
    if (m_craftingMode == 0)
    {
        menu = m_player->mInventoryMenu;
    }
    else
    {
        menu = new CraftingMenu(m_player->mInventory, m_player->mLevel, BlockPos(input->x, input->y, input->z));
        Minecraft::GetInstance()->mLocalPlayers[m_playerId]->mContainerMenu = menu;
    }
    m_openMenu = menu;

    m_inventory.addSlots(0, 27);
    m_hotBar.addSlots(27, 9);
    m_previewSlot.addSlots(55, 1);
    m_doubleSpinnerSlots[0].addSlots(56, 1);
    m_doubleSpinnerSlots[1].addSlots(57, 1);
    m_tripleSpinnerSlots[0].addSlots(56, 1);
    m_tripleSpinnerSlots[1].addSlots(57, 1);
    m_tripleSpinnerSlots[2].addSlots(58, 1);

    if (m_craftingMode == 1)
    {
        m_groupMap = &m_groupMap3x3[0][0];
        m_ingredientSlotCount = 9;
    }
    else
    {
        m_groupMap = &m_groupMap2x2[0][0];
        m_ingredientSlotCount = 4;
    }
    m_horizontalSlotCount = 12;
    gApp.SetRichPresenceContext(m_playerId, 5);
    setGroupText(IUIScene_CraftingMenu::GetGroupNameText(IUIScene_CraftingMenu::getCurrentGroup()));

    if (Minecraft::GetInstance()->mGameModes[m_playerId] != NULL)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[m_playerId]->getTutorial();
        if (tutorial != NULL)
        {
            m_previousTutorialState = tutorial->currentState;
            if (m_craftingMode == 0)
                tutorial->changeTutorialState(eTutorial_State_CraftingMenu, this);
            else
                tutorial->changeTutorialState(eTutorial_State_CraftingTableMenu, this);
        }
    }

    for (int ingredient = 0; ingredient < 5; ++ingredient)
        m_ingredientSlots[ingredient].addSlot(36 + ingredient);
    m_craftingOutput.addSlot(41);
    m_ingredientsLayout.addSlots(42, m_ingredientSlotCount);
    m_crafting3VSlots[0].addSlot(52);
    m_crafting3VSlots[1].addSlot(53);
    m_crafting3VSlots[2].addSlot(54);
    m_crafting2VSlots[0].addSlot(53);
    m_crafting2VSlots[1].addSlot(52);
    m_crafting1VSlot.addSlot(52);
    m_craftingHSlots.addSlots(59, 12);

    IUIScene_CraftingMenu::UpdateTabs(true);
    IUIScene_CraftingMenu::CheckRecipesAvailable();
    m_verticalSlots[0] = IUIScene_CraftingMenu::getCurrentVSlotMax();
    m_verticalSlots[2] = 1;
    m_verticalSlots[1] = 0;
    IUIScene_CraftingMenu::UpdateHorizontalSlots();
    IUIScene_CraftingMenu::UpdateVerticalSlots();
    IUIScene_CraftingMenu::UpdateHighlight();

    delete input;

    gConsoleUIController.OverrideSFX(m_playerId, 0, true);
    gConsoleUIController.OverrideSFX(m_playerId, 20, true);
    gConsoleUIController.OverrideSFX(m_playerId, 11, true);
    gConsoleUIController.OverrideSFX(m_playerId, 10, true);
    gConsoleUIController.OverrideSFX(m_playerId, 7, true);
    gConsoleUIController.OverrideSFX(m_playerId, 6, true);
    gConsoleUIController.OverrideSFX(m_playerId, 4, true);
    gConsoleUIController.OverrideSFX(m_playerId, 5, true);

    for (int tab = 0; tab < 7; ++tab)
        m_tabItems[tab] = ItemInstance::EMPTY;

    IUIScene_CraftingMenu::UpdateTooltips();
    addTimer(1, 100);
    for (int panel = 0; panel < 13; ++panel)
        m_touchPanels[panel].init(panel);
    m_descriptionText.SetupTouch();
    m_descriptionTextTop.SetupTouch();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_CraftingMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
    {
        m_compactInventory = true;
        return L"CraftingMenuSplit";
    }
    m_compactInventory = false;
    return L"CraftingMenu";
}

unsigned int UIScene_CraftingMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_mainPanel.setupControl(this, currentNode, std::string("MainPanel"));
        m_controls.push_back(&m_mainPanel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_descriptionPanel.setupControl(this, currentNode, std::string("DescriptionPanel"));
            m_controls.push_back(&m_descriptionPanel);
        }
        {
            m_descriptionTextTop.setupControl(this, currentNode, std::string("DescriptionTextTop"));
            m_controls.push_back(&m_descriptionTextTop);
        }
        {
            m_craftingHSlots.setupControl(this, currentNode, std::string("CraftingHSlots"));
            m_controls.push_back(&m_craftingHSlots);
        }
        {
            m_spinnerTitle.setupControl(this, currentNode, std::string("SpinnerTitle"));
            m_controls.push_back(&m_spinnerTitle);
        }
        {
            m_doubleSpinnerSlots[0].setupControl(this, currentNode, std::string("Slot2_1"));
            m_controls.push_back(&m_doubleSpinnerSlots[0]);
        }
        {
            m_doubleSpinnerSlots[1].setupControl(this, currentNode, std::string("Slot2_2"));
            m_controls.push_back(&m_doubleSpinnerSlots[1]);
        }
        {
            m_tripleSpinnerSlots[0].setupControl(this, currentNode, std::string("Slot3_1"));
            m_controls.push_back(&m_tripleSpinnerSlots[0]);
        }
        {
            m_tripleSpinnerSlots[1].setupControl(this, currentNode, std::string("Slot3_2"));
            m_controls.push_back(&m_tripleSpinnerSlots[1]);
        }
        {
            m_tripleSpinnerSlots[2].setupControl(this, currentNode, std::string("Slot3_3"));
            m_controls.push_back(&m_tripleSpinnerSlots[2]);
        }
        {
            m_slotSelector3.setupControl(this, currentNode, std::string("SlotSelector3"));
            m_controls.push_back(&m_slotSelector3);
        }
        {
            fuiRenderNode* parentNode = currentNode;
            currentNode = m_slotSelector3.getFuiRenderNode();
            {
                m_crafting3VSlots[0].setupControl(this, currentNode, std::string("Crafting3VSlot1"));
                m_controls.push_back(&m_crafting3VSlots[0]);
            }
            {
                m_crafting3VSlots[1].setupControl(this, currentNode, std::string("Crafting3VSlot2"));
                m_controls.push_back(&m_crafting3VSlots[1]);
            }
            {
                m_crafting3VSlots[2].setupControl(this, currentNode, std::string("Crafting3VSlot3"));
                m_controls.push_back(&m_crafting3VSlots[2]);
            }
            currentNode = parentNode;
        }
        {
            m_slotSelector2.setupControl(this, currentNode, std::string("SlotSelector2"));
            m_controls.push_back(&m_slotSelector2);
        }
        {
            fuiRenderNode* parentNode = currentNode;
            currentNode = m_slotSelector2.getFuiRenderNode();
            {
                m_crafting2VSlots[0].setupControl(this, currentNode, std::string("Crafting2VSlot1"));
                m_controls.push_back(&m_crafting2VSlots[0]);
            }
            {
                m_crafting2VSlots[1].setupControl(this, currentNode, std::string("Crafting2VSlot2"));
                m_controls.push_back(&m_crafting2VSlots[1]);
            }
            currentNode = parentNode;
        }
        {
            m_craftingSelector.setupControl(this, currentNode, std::string("CraftingSelector"));
            m_controls.push_back(&m_craftingSelector);
        }
        {
            fuiRenderNode* parentNode = currentNode;
            currentNode = m_craftingSelector.getFuiRenderNode();
            {
                m_crafting1VSlot.setupControl(this, currentNode, std::string("Crafting1VSlot1"));
                m_controls.push_back(&m_crafting1VSlot);
            }
            currentNode = parentNode;
        }
        {
            m_previewLabel.setupControl(this, currentNode, std::string("PreviewLabel"));
            m_controls.push_back(&m_previewLabel);
        }
        {
            m_ingredientsLayout.setupControl(this, currentNode, std::string("IngredientsLayout"));
            m_controls.push_back(&m_ingredientsLayout);
        }
        {
            m_ingredientsLayoutSmall.setupControl(this, currentNode, std::string("IngredientsLayout_Small"));
            m_controls.push_back(&m_ingredientsLayoutSmall);
        }
        {
            m_craftingOutput.setupControl(this, currentNode, std::string("CraftingOutput"));
            m_controls.push_back(&m_craftingOutput);
        }
        {
            m_ingredientSlots[0].setupControl(this, currentNode, std::string("Ingredient1"));
            m_controls.push_back(&m_ingredientSlots[0]);
        }
        {
            m_ingredientSlots[1].setupControl(this, currentNode, std::string("Ingredient2"));
            m_controls.push_back(&m_ingredientSlots[1]);
        }
        {
            m_ingredientSlots[2].setupControl(this, currentNode, std::string("Ingredient3"));
            m_controls.push_back(&m_ingredientSlots[2]);
        }
        {
            m_ingredientSlots[3].setupControl(this, currentNode, std::string("Ingredient4"));
            m_controls.push_back(&m_ingredientSlots[3]);
        }
        {
            m_ingredientSlots[4].setupControl(this, currentNode, std::string("Ingredient5"));
            m_controls.push_back(&m_ingredientSlots[4]);
        }
        {
            m_ingredientDescriptions[0].setupControl(this, currentNode, std::string("Ingredient1Desc"));
            m_controls.push_back(&m_ingredientDescriptions[0]);
        }
        {
            m_ingredientDescriptions[1].setupControl(this, currentNode, std::string("Ingredient2Desc"));
            m_controls.push_back(&m_ingredientDescriptions[1]);
        }
        {
            m_ingredientDescriptions[2].setupControl(this, currentNode, std::string("Ingredient3Desc"));
            m_controls.push_back(&m_ingredientDescriptions[2]);
        }
        {
            m_ingredientDescriptions[3].setupControl(this, currentNode, std::string("Ingredient4Desc"));
            m_controls.push_back(&m_ingredientDescriptions[3]);
        }
        {
            m_ingredientDescriptions[4].setupControl(this, currentNode, std::string("Ingredient5Desc"));
            m_controls.push_back(&m_ingredientDescriptions[4]);
        }
        {
            m_previewSlot.setupControl(this, currentNode, std::string("PreviewSlot"));
            m_controls.push_back(&m_previewSlot);
        }
        {
            m_ingredientsLabel.setupControl(this, currentNode, std::string("IngredientsLabel"));
            m_controls.push_back(&m_ingredientsLabel);
        }
        {
            m_descriptionText.setupControl(this, currentNode, std::string("DescriptionText"));
            m_controls.push_back(&m_descriptionText);
        }
        {
            m_inventory.setupControl(this, currentNode, std::string("Inventory"));
            m_controls.push_back(&m_inventory);
        }
        {
            m_hotBar.setupControl(this, currentNode, std::string("HotBar"));
            m_controls.push_back(&m_hotBar);
        }
        {
            m_groupName.setupControl(this, currentNode, std::string("GroupName"));
            m_controls.push_back(&m_groupName);
        }
        {
            m_itemName.setupControl(this, currentNode, std::string("ItemName"));
            m_controls.push_back(&m_itemName);
        }
        {
            m_inventoryLabel.setupControl(this, currentNode, std::string("InventoryLabel"));
            m_controls.push_back(&m_inventoryLabel);
        }
        {
            m_touchPanels[0].setupControl(this, currentNode, std::string("TouchPanel_0"));
            m_controls.push_back(&m_touchPanels[0]);
        }
        {
            m_touchPanels[1].setupControl(this, currentNode, std::string("TouchPanel_1"));
            m_controls.push_back(&m_touchPanels[1]);
        }
        {
            m_touchPanels[2].setupControl(this, currentNode, std::string("TouchPanel_2"));
            m_controls.push_back(&m_touchPanels[2]);
        }
        {
            m_touchPanels[3].setupControl(this, currentNode, std::string("TouchPanel_3"));
            m_controls.push_back(&m_touchPanels[3]);
        }
        {
            m_touchPanels[4].setupControl(this, currentNode, std::string("TouchPanel_4"));
            m_controls.push_back(&m_touchPanels[4]);
        }
        {
            m_touchPanels[5].setupControl(this, currentNode, std::string("TouchPanel_5"));
            m_controls.push_back(&m_touchPanels[5]);
        }
        {
            m_touchPanels[6].setupControl(this, currentNode, std::string("TouchPanel_6"));
            m_controls.push_back(&m_touchPanels[6]);
        }
        {
            m_touchPanels[7].setupControl(this, currentNode, std::string("TouchPanel_V_0"));
            m_controls.push_back(&m_touchPanels[7]);
        }
        {
            m_touchPanels[8].setupControl(this, currentNode, std::string("TouchPanel_V_1"));
            m_controls.push_back(&m_touchPanels[8]);
        }
        {
            m_touchPanels[9].setupControl(this, currentNode, std::string("TouchPanel_V_2"));
            m_controls.push_back(&m_touchPanels[9]);
        }
        {
            m_touchPanels[10].setupControl(this, currentNode, std::string("TouchPanel_V_3"));
            m_controls.push_back(&m_touchPanels[10]);
        }
        {
            m_touchPanels[11].setupControl(this, currentNode, std::string("TouchPanel_V_4"));
            m_controls.push_back(&m_touchPanels[11]);
        }
        {
            m_touchPanels[12].setupControl(this, currentNode, std::string("TouchPanel_CraftingHSlots"));
            m_controls.push_back(&m_touchPanels[12]);
        }
        currentNode = parentNode;
    }
    return 1;
}

int UIScene_CraftingMenu::getSceneType()
{
    return m_craftingMode == 1 ? 7 : 6;
}

int UIScene_CraftingMenu::getPad()
{
    return m_playerId;
}

void UIScene_CraftingMenu::hideAllHSlots()
{
    for (int i = 0; i < 12; ++i)
    {
        m_horizontalSlotItems[i].item = ItemInstance::EMPTY;
        m_horizontalSlotItems[i].displayValue = 0x1f;
        m_horizontalSlotItems[i].visible = false;
    }
}

void UIScene_CraftingMenu::hideAllVSlots()
{
    for (int i = 0; i < 3; ++i)
    {
        m_verticalSlotItems[i].item = ItemInstance::EMPTY;
        m_verticalSlotItems[i].displayValue = 0x1f;
        m_verticalSlotItems[i].visible = false;
    }
}

void UIScene_CraftingMenu::hideAllIngredientsSlots()
{
    for (int i = 0; i < m_ingredientCount; ++i)
    {
        m_ingredientDescriptionItems[i].item = ItemInstance::EMPTY;
        m_ingredientDescriptionItems[i].displayValue = 0x1f;
        m_ingredientDescriptionItems[i].visible = false;
        SetItemDescription(i, L"");
        static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->ShowIngredient(i, false);
    }
}

void UIScene_CraftingMenu::SetItemDescription(int index, const wchar_t* text)
{
    std::wstring description(text);
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->SetItemDescription(index, description);
}

void UIScene_CraftingMenu::setCraftHSlotItem(int, int index, not_null_ptr<ItemInstance> item, unsigned int displayValue)
{
    m_horizontalSlotItems[index].item = item;
    m_horizontalSlotItems[index].visible = true;
    m_horizontalSlotItems[index].displayValue = displayValue;
}

void UIScene_CraftingMenu::setSpecialSlotItem(int, int index, not_null_ptr<ItemInstance> item,
                                              unsigned int displayValue)
{
    m_horizontalSlotItems[index].item = item;
    m_horizontalSlotItems[index].visible = true;
    m_horizontalSlotItems[index].displayValue = displayValue;
}

void UIScene_CraftingMenu::setCraftVSlotItem(int, int index, not_null_ptr<ItemInstance> item, unsigned int displayValue)
{
    m_verticalSlotItems[index].item = item;
    m_verticalSlotItems[index].visible = true;
    m_verticalSlotItems[index].displayValue = displayValue;
}

void UIScene_CraftingMenu::setCraftingOutputSlotItem(int, not_null_ptr<ItemInstance> item)
{
    m_outputSlotItem.item = item;
    m_outputSlotItem.displayValue = 0x1f;
    m_outputSlotItem.visible = !item->isEmpty();
}

void UIScene_CraftingMenu::setCraftingOutputSlotRedBox(bool show)
{
    m_craftingOutput.showSlotRedBox(0, show);
}

void UIScene_CraftingMenu::setIngredientSlotItem(int, int index, not_null_ptr<ItemInstance> item)
{
    m_ingredientSlotItems[index].item = item;
    m_ingredientSlotItems[index].displayValue = 0x1f;
    m_ingredientSlotItems[index].visible = !item->isEmpty();
}

void UIScene_CraftingMenu::setIngredientSlotRedBox(int index, bool show)
{
    m_ingredientsLayout.showSlotRedBox(index, show);
}

void UIScene_CraftingMenu::setIngredientSlotHighlighted(int index)
{
    m_ingredientsLayout.setHighlightSlot(index);
}

void UIScene_CraftingMenu::setInventorySlotHighlighted(int index)
{
    int hotBarSlot = -1;
    if (index >= 9 && index < 37)
    {
        m_inventory.setHighlightSlot(index - 9);
    }
    else
    {
        m_inventory.setHighlightSlot(-1);
        if (index >= 0 && index < 10)
            hotBarSlot = index;
    }
    m_hotBar.setHighlightSlot(hotBarSlot);
}

void UIScene_CraftingMenu::setIngredientDescriptionItem(int, int index, not_null_ptr<ItemInstance> item)
{
    m_ingredientDescriptionItems[index].item = item;
    m_ingredientDescriptionItems[index].displayValue = 0x1f;
    m_ingredientDescriptionItems[index].visible = !item->isEmpty();
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())
        ->ShowIngredient(index, m_ingredientDescriptionItems[index].visible);
}

void UIScene_CraftingMenu::setIngredientDescriptionRedBox(int index, bool show)
{
    m_ingredientSlots[index].showSlotRedBox(0, show);
}

void UIScene_CraftingMenu::setIngredientDescriptionText(int index, const wchar_t* text)
{
    SetItemDescription(index, text);
}

void UIScene_CraftingMenu::setPreviewSlotItem(int, not_null_ptr<ItemInstance> item)
{
    m_previewSlotItem.item = item;
    m_previewSlotItem.displayValue = 0x1f;
    m_previewSlotItem.visible = !item->isEmpty();
}

void UIScene_CraftingMenu::setShowCraftHSlot(int index, bool show)
{
    m_horizontalSlotItems[index].visible = show;
}

void UIScene_CraftingMenu::showTabHighlight(int index, bool show)
{
    if (show)
        static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->SetActiveTab(index, m_currentTab);
}

void UIScene_CraftingMenu::setHTabEnabled(int index, bool enabled)
{
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->EnableTab(index, enabled);
}

void UIScene_CraftingMenu::setSpinnerTitle(const wchar_t* text)
{
    m_spinnerTitle.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::setGroupText(const wchar_t* text)
{
    m_groupName.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::setDescriptionText(const wchar_t* text)
{
    m_descriptionText.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::setDescriptionTextTop(const wchar_t* text)
{
    m_descriptionTextTop.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::SetItemName(const wchar_t* text)
{
    std::wstring name(text);
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->SetItemName(name);
}

void UIScene_CraftingMenu::setItemText(const wchar_t* text)
{
    SetItemName(text);
}

void UIScene_CraftingMenu::scrollDescriptionUp()
{
}

void UIScene_CraftingMenu::scrollDescriptionDown()
{
}

void UIScene_CraftingMenu::updateHighlightAndScrollPositions()
{
    int vertical = 0;
    if (IUIScene_CraftingMenu::getCurrentVSlotCount() == 2)
    {
        static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->MoveSelector(m_horizontalSlot, 1);
        static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->SelectVerticalItem(m_verticalColumn);
    }
    else
    {
        if (IUIScene_CraftingMenu::getCurrentVSlotCount() > 2)
            vertical = 2;
        static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->MoveSelector(m_horizontalSlot, vertical);
        static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->SelectVerticalItem(m_verticalColumn);
    }
}

void UIScene_CraftingMenu::updateVSlotPositions(int, int)
{
}

void UIScene_CraftingMenu::UpdateMultiPanel(bool force)
{
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->showPanelDisplay(m_panelDisplay);
    if (force || m_lastPanelDisplay != m_panelDisplay)
    {
        gConsoleUIController.TouchBoxRebuild(this);
        m_lastPanelDisplay = m_panelDisplay;
    }
}

void UIScene_CraftingMenu::ShowDescriptionPanel(bool show)
{
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->ShowDescriptionPanel(show);
}

void UIScene_CraftingMenu::ShowSuperCraftingSlots(int count)
{
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->ShowSuperCraftingSlots(count);
}

void UIScene_CraftingMenu::SetCraftingMenu(int category, int item, bool enabled)
{
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->SetCraftingMenu(category, item, enabled);
}

void UIScene_CraftingMenu::SetHTabItem(int index, not_null_ptr<ItemInstance> item)
{
    m_tabItems[index] = item;
}

void UIScene_CraftingMenu::handleDestroy()
{
    if (Minecraft::GetInstance()->mGameModes[m_playerId] != 0)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft->mGameModes[m_playerId] != 0)
        {
            Tutorial* tutorial = minecraft->mGameModes[m_playerId]->getTutorial();
            if (tutorial != 0)
                tutorial->changeTutorialState(m_previousTutorialState, 0);
        }
    }
    if (Minecraft::GetInstance()->mLocalPlayers[m_playerId].get() != 0)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft->mLocalPlayers[m_playerId]->mContainerMenu->m_containerId == m_openMenu->m_containerId)
            Minecraft::GetInstance()->mLocalPlayers[m_playerId]->closeContainer();
    }
    gConsoleUIController.OverrideSFX(m_playerId, 0, false);
    gConsoleUIController.OverrideSFX(m_playerId, 20, false);
    gConsoleUIController.OverrideSFX(m_playerId, 11, false);
    gConsoleUIController.OverrideSFX(m_playerId, 10, false);
    gConsoleUIController.OverrideSFX(m_playerId, 7, false);
    gConsoleUIController.OverrideSFX(m_playerId, 6, false);
    gConsoleUIController.OverrideSFX(m_playerId, 4, false);
    gConsoleUIController.OverrideSFX(m_playerId, 5, false);
}

void UIScene_CraftingMenu::handleReload()
{
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->InitCraftingMenu(m_craftingMode);
    if (m_craftingMode == 0)
        m_ingredientsLayout = m_ingredientsLayoutSmall;

    m_inventory.addSlots(0, 27);
    m_hotBar.addSlots(27, 9);
    m_previewSlot.addSlots(55, 1);
    m_doubleSpinnerSlots[0].addSlots(56, 1);
    m_doubleSpinnerSlots[1].addSlots(57, 1);
    m_tripleSpinnerSlots[0].addSlots(56, 1);
    m_tripleSpinnerSlots[1].addSlots(57, 1);
    m_tripleSpinnerSlots[2].addSlots(58, 1);
    for (int ingredient = 0; ingredient < 5; ++ingredient)
        m_ingredientSlots[ingredient].addSlot(36 + ingredient);
    m_craftingOutput.addSlot(41);
    m_ingredientsLayout.addSlots(42, m_ingredientSlotCount);
    m_crafting3VSlots[0].addSlot(52);
    m_crafting3VSlots[1].addSlot(53);
    m_crafting3VSlots[2].addSlot(54);
    m_crafting2VSlots[0].addSlot(53);
    m_crafting2VSlots[1].addSlot(52);
    m_crafting1VSlot.addSlot(52);
    m_craftingHSlots.addSlots(59, 12);

    SetCraftingMenu(m_groupCount, 4, m_currentTab == 1);
    showTabHighlight(m_currentGroup, true);

    bool spinner = IUIScene_CraftingMenu::isSpinnerGroup();
    ShowSuperCraftingSlots(spinner ? (m_craftingMode == 1 ? 3 : 2) : 0);
    ShowDescriptionPanel(spinner);
    IUIScene_CraftingMenu::UpdateTabs(false);

    int panel = m_panelDisplay;
    m_panelDisplay = (panel == 0);
    UpdateMultiPanel(false);
    m_panelDisplay = panel;
    UpdateMultiPanel(false);

    m_horizontalSlot = 0;
    m_verticalColumn = 1;
    m_verticalSlots[0] = IUIScene_CraftingMenu::getCurrentVSlotMax();
    m_verticalSlots[1] = 0;
    m_verticalSlots[2] = 1;
    IUIScene_CraftingMenu::UpdateHorizontalSlots();
    IUIScene_CraftingMenu::UpdateVerticalSlots();
    IUIScene_CraftingMenu::UpdateHighlight();
    showTabHighlight(0, false);
    showTabHighlight(m_currentGroup, true);
}

void UIScene_CraftingMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0)
    {
        GetMainPanel()->UpdateControl();
        gConsoleUIController.TouchBoxRebuild(this);
        killTimer(0);
        return;
    }
    if (timerId == 1)
    {
        killTimer(1);
        m_inventoryUpdatesDeferred = false;
    }
}

UIControl* UIScene_CraftingMenu::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_CraftingMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!minecraft->mLocalPlayers[m_playerId] || minecraft->mGameModes[m_playerId] == 0)
        return;

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int index = -1;
    bool fromInventorySlot = false;
    bool drawDecorations = true;
    float partialTick = 1.0f;

    std::sscanf(name, "slot_%d", &index);
    if (index == -1)
    {
        std::sscanf(name, "TabH_%d", &index);
        if (index == -1)
        {
            std::sscanf(name, "TabV_%d", &index);
            if (index != -1)
            {
                if (index == 1)
                    item = not_null_ptr<ItemInstance>(new ItemInstance(Blocks::crafting_table, 1, 0));
                else if (index == 2)
                    item = not_null_ptr<ItemInstance>(
                        new ItemInstance(Items::banner, 1, DyeColor::WHITE->getItemData()));
                else if (index == 3)
                    item = not_null_ptr<ItemInstance>(new ItemInstance(Items::fireworks, 1));
                else if (index == 4)
                    item = not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, DyeColor::CYAN->getItemData()));
            }
        }
        else if (m_currentTab >= 1 && m_currentTab <= 3)
            item = m_tabItems[index - 1];
    }
    else if (index >= 0 && index < 27)
    {
        item = m_inventoryMenu->getSlot(index + m_inventoryStart)->getItem();
        fromInventorySlot = true;
    }
    else if (index >= 27 && index < 36)
    {
        item = m_inventoryMenu->getSlot(index - 27 + m_inventoryEnd)->getItem();
        fromInventorySlot = true;
    }
    else if (index >= 52 && index < 55)
    {
        int slot = index - 52;
        drawDecorations = false;
        if (m_verticalSlotItems[slot].visible)
        {
            item = m_verticalSlotItems[slot].item;
            partialTick = static_cast<float>(m_verticalSlotItems[slot].displayValue) / 31.0f;
        }
    }
    else if (index >= 59 && index < 71)
    {
        int slot = index - 59;
        drawDecorations = false;
        if (m_horizontalSlotItems[slot].visible)
        {
            item = m_horizontalSlotItems[slot].item;
            partialTick = static_cast<float>(m_horizontalSlotItems[slot].displayValue) / 31.0f;
        }
    }
    else if (index >= 42 && index < m_ingredientSlotCount + 42)
    {
        int slot = index - 42;
        if (m_ingredientSlotItems[slot].visible)
        {
            item = m_ingredientSlotItems[slot].item;
            partialTick = static_cast<float>(m_ingredientSlotItems[slot].displayValue) / 31.0f;
        }
    }
    else if (index >= 36 && index < 41)
    {
        int slot = index - 36;
        if (m_ingredientDescriptionItems[slot].visible)
        {
            item = m_ingredientDescriptionItems[slot].item;
            partialTick = static_cast<float>(m_ingredientDescriptionItems[slot].displayValue) / 31.0f;
        }
    }
    else if (index == 51)
    {
        if (m_previewSlotItem.visible)
        {
            item = m_previewSlotItem.item;
            partialTick = static_cast<float>(m_previewSlotItem.displayValue) / 31.0f;
        }
    }
    else if (index == 41)
    {
        if (m_outputSlotItem.visible)
        {
            item = m_outputSlotItem.item;
            partialTick = static_cast<float>(m_outputSlotItem.displayValue) / 31.0f;
        }
    }
    else if (index == 55)
    {
        drawDecorations = false;
        item = m_accumulatedItem;
    }
    else if (index == 56)
    {
        drawDecorations = false;
        _SlotInfo& slot = m_horizontalSlotItems[0];
        if (slot.visible)
        {
            item = slot.item;
            partialTick = static_cast<float>(slot.displayValue) / 31.0f;
        }
    }
    else if (index == 57)
    {
        drawDecorations = false;
        _SlotInfo& slot = m_horizontalSlotItems[1];
        if (slot.visible)
        {
            item = slot.item;
            partialTick = static_cast<float>(slot.displayValue) / 31.0f;
        }
    }
    else if (index == 58)
    {
        drawDecorations = false;
        _SlotInfo& slot = m_horizontalSlotItems[2];
        if (slot.visible)
        {
            item = slot.item;
            partialTick = static_cast<float>(slot.displayValue) / 31.0f;
        }
    }

    if (item->isEmpty())
        return;

    if (!fromInventorySlot)
    {
        if (item->getItem() == Items::clock || item->getItem() == Items::compass)
            item->setAuxValue(255);
        else if ((item->getAuxValue() & 0xFF) == 0xFF)
            item->setAuxValue(0);
    }

    customDrawSlotControlFui(rect, m_playerId, item, partialTick, item->getItem()->isFoil(item), drawDecorations,
                             false);
}

void UIScene_CraftingMenu::handleTouchBoxRebuild()
{
    addTimer(0, 100);
}

void UIScene_CraftingMenu::handleTouchInput(unsigned int, int touchX, int touchY, int control, bool pressed,
                                            bool repeat, bool released)
{
    if (pressed)
    {
#ifdef MINECRAFT_PC
        m_pcPressControl = control;
        m_pcPressHorizontal = m_horizontalSlot;
        m_pcPressVertical = m_verticalColumn;
        m_pcPressScrolled = false;
#endif
        if (control == 12)
            m_touchScrollY = touchY;

        if (!IUIScene_CraftingMenu::isSpinnerGroup())
        {
            const float px = static_cast<float>(touchX);
            if (m_mainPanel.getXPos() + m_craftingHSlots.getXPos() + m_craftingHSlots.getHeight() * m_horizontalSlot
                    < px
                && px < m_mainPanel.getXPos() + m_craftingHSlots.getXPos()
                            + m_craftingHSlots.getHeight() * (m_horizontalSlot + 1))
            {
                m_touchDragging = true;
                return;
            }
        }
        else
        {
            const float px = static_cast<float>(touchX);
            if (IUIScene_CraftingMenu::getNumSpinnerSlots() == 3)
            {
                if (m_mainPanel.getXPos() + m_tripleSpinnerSlots[m_horizontalSlot].getXPos() < px
                    && px < m_mainPanel.getXPos() + m_tripleSpinnerSlots[m_horizontalSlot].getXPos()
                                + m_tripleSpinnerSlots[m_horizontalSlot].getHeight())
                {
                    m_touchDragging = true;
                    return;
                }
            }
            else if (m_mainPanel.getXPos() + m_doubleSpinnerSlots[m_horizontalSlot].getXPos() < px
                     && px < m_mainPanel.getXPos() + m_doubleSpinnerSlots[m_horizontalSlot].getXPos()
                                 + m_doubleSpinnerSlots[m_horizontalSlot].getHeight())
            {
                m_touchDragging = true;
                return;
            }
        }

        m_touchDragging = false;
        return;
    }

    if (repeat)
    {
        if (control == 12 && m_touchDragging)
        {
            const float py = static_cast<float>(touchY);
            if (static_cast<float>(m_touchScrollY) + m_craftingHSlots.getHeight() <= py)
            {
#ifdef MINECRAFT_PC
                m_pcPressScrolled = true;
#endif
                IUIScene_CraftingMenu::ItemScrollDown();
                m_touchScrollY = touchY;
                return;
            }
            if (py <= static_cast<float>(m_touchScrollY) - m_craftingHSlots.getHeight())
            {
#ifdef MINECRAFT_PC
                m_pcPressScrolled = true;
#endif
                IUIScene_CraftingMenu::ItemScrollUp();
                m_touchScrollY = touchY;
                return;
            }
        }
        return;
    }

    if (!released)
        return;

    if (control >= 0 && control < 7)
    {
        m_currentGroup = control;
        IUIScene_CraftingMenu::UpdateHTab();
        return;
    }

    if (control >= 7 && control < 12)
    {
        m_currentTab = control - 7;
        IUIScene_CraftingMenu::UpdateVTab();
        return;
    }

    if (control != 12)
        return;

    int column;
    float py;
    if (!IUIScene_CraftingMenu::isSpinnerGroup())
    {
        column = static_cast<int>((static_cast<float>(touchX) - m_craftingHSlots.getXPos() - m_mainPanel.getXPos())
                                  / m_craftingHSlots.getHeight());
        py = static_cast<float>(touchY);
    }
    else
    {
        const float px = static_cast<float>(touchX);
        column = -1;
        if (IUIScene_CraftingMenu::getNumSpinnerSlots() == 3)
        {
            if (m_mainPanel.getXPos() + m_tripleSpinnerSlots[0].getXPos() < px
                && px <= m_mainPanel.getXPos() + m_tripleSpinnerSlots[0].getXPos() + m_tripleSpinnerSlots[0].getWidth())
                column = 0;
            else if (m_mainPanel.getXPos() + m_tripleSpinnerSlots[1].getXPos() < px
                     && px <= m_mainPanel.getXPos() + m_tripleSpinnerSlots[1].getXPos()
                                  + m_tripleSpinnerSlots[1].getWidth())
                column = 1;
            else if (m_mainPanel.getXPos() + m_tripleSpinnerSlots[2].getXPos() < px
                     && px <= m_mainPanel.getXPos() + m_tripleSpinnerSlots[2].getXPos()
                                  + m_tripleSpinnerSlots[2].getWidth())
                column = 2;
        }
        else
        {
            if (m_mainPanel.getXPos() + m_doubleSpinnerSlots[0].getXPos() < px
                && px <= m_mainPanel.getXPos() + m_doubleSpinnerSlots[0].getXPos() + m_doubleSpinnerSlots[0].getWidth())
                column = 0;
            else if (m_mainPanel.getXPos() + m_doubleSpinnerSlots[1].getXPos() < px
                     && px <= m_mainPanel.getXPos() + m_doubleSpinnerSlots[1].getXPos()
                                  + m_doubleSpinnerSlots[1].getWidth())
                column = 1;
        }

        if (column == -1)
            column = m_horizontalSlot;
        py = static_cast<float>(touchY);
    }

    if (m_mainPanel.getYPos() + m_craftingHSlots.getYPos() < py
        && py < m_mainPanel.getYPos() + m_craftingHSlots.getYPos() + m_craftingHSlots.getHeight())
    {
        const int previous = m_horizontalSlot;
        if (previous != column)
        {
            if (column < m_horizontalSlotCount)
                m_horizontalSlot = column;
            else
                m_horizontalSlot = 0;
            m_verticalColumn = 1;
            m_verticalSlots[0] = IUIScene_CraftingMenu::getCurrentVSlotMax();
            m_verticalSlots[1] = 0;
            m_verticalSlots[2] = 1;
            IUIScene_CraftingMenu::UpdateVerticalSlots();
            IUIScene_CraftingMenu::UpdateHighlight();
            if (m_recipeTable[previous][0] > 0)
                setShowCraftHSlot(previous, true);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
            return;
        }
    }

    int rows = 0;
    if (!IUIScene_CraftingMenu::isSpinnerGroup())
    {
        if (IUIScene_CraftingMenu::getCurrentVSlotCount() == 2)
            rows = 1;
        else if (IUIScene_CraftingMenu::getCurrentVSlotCount() > 2)
            rows = 2;
    }

    if (m_mainPanel.getYPos() + m_craftingHSlots.getYPos() + m_craftingHSlots.getHeight() < py && rows > 0)
        m_verticalColumn = 2;
    else if (py < m_mainPanel.getYPos() + m_craftingHSlots.getYPos() && rows > 1)
        m_verticalColumn = 0;
    else
        m_verticalColumn = 1;
    IUIScene_CraftingMenu::UpdateHighlight();
#ifdef MINECRAFT_PC
    if (m_pcPressControl == 12 && !m_pcPressScrolled && column == m_horizontalSlot
        && m_horizontalSlot == m_pcPressHorizontal && m_verticalColumn == m_pcPressVertical)
        IUIScene_CraftingMenu::handleKeyDown(m_playerId, 0, false);
#endif
}

void UIScene_CraftingMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
        sendInputToMovie(action, repeat, pressed, released);
        return;
    }
    if (pressed)
        handled = IUIScene_CraftingMenu::handleKeyDown(m_playerId, action, repeat);
}

bool UIScene_CraftingMenu::allowRepeat(int action)
{
    return action != EControllerActions_MenuX;
}

void UIScene_CraftingMenu::handleInventoryUpdated(void*)
{
    if (!m_inventoryUpdatesDeferred)
        IUIScene_CraftingMenu::HandleInventoryUpdated();
}

void UIScene_CraftingMenu::HandleMessage(EUIMessage message, void* data)
{
    if (message != 0)
        return;
    handleInventoryUpdated(data);
}

void UIScene_CraftingMenu::updateTooltips()
{
    IUIScene_CraftingMenu::UpdateTooltips();
}
