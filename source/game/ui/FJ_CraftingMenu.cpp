#include "game/ui/FJ_CraftingMenu.hpp"

#include <cstdio>
#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

std::string s_horizontalTabNames[7] = {"TabH_1", "TabH_2", "TabH_3", "TabH_4", "TabH_5", "TabH_6", "TabH_7"};
std::string s_verticalTabNames[5] = {"TabV_1", "TabV_2", "TabV_3", "TabV_4", "TabV_5"};
std::string s_horizontalTouchPanelNames[7] = {"TouchPanel_0", "TouchPanel_1", "TouchPanel_2", "TouchPanel_3",
                                              "TouchPanel_4", "TouchPanel_5", "TouchPanel_6"};
std::string s_verticalTouchPanelNames[5] = {"TouchPanel_V_0", "TouchPanel_V_1", "TouchPanel_V_2", "TouchPanel_V_3",
                                            "TouchPanel_V_4"};

static const int s_craftingIconFrames[7] = {1, 2, 3, 5, 6, 7, 1};
static const int s_otherIconFrames[7] = {1, 2, 3, 4, 5, 6, 7};

FJ_CraftingMenu::FJ_CraftingMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_SlideComplete, this, _handleSlideComplete, false);
}

void FJ_CraftingMenu::handleSlideComplete(FJ_Event* event)
{
    m_pCallbackScene->handleTouchBoxRebuild();
}

void FJ_CraftingMenu::_handleSlideComplete(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CraftingMenu*>(node)->handleSlideComplete(event);
}

FJ_Document* FJ_CraftingMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_CraftingMenu(renderNode);
}

unsigned int FJ_CraftingMenu::getType()
{
    return 11;
}

void FJ_CraftingMenu::showPanelDisplay(int panel)
{
    getChildByName("MainPanel.Ingredient1")->setVisible(panel == 2 && m_ingredients[0]);
    getChildByName("MainPanel.Ingredient2")->setVisible(panel == 2 && m_ingredients[1]);
    getChildByName("MainPanel.Ingredient3")->setVisible(panel == 2 && m_ingredients[2]);
    getChildByName("MainPanel.Ingredient4")->setVisible(panel == 2 && m_ingredients[3]);
    if (getChildByName("MainPanel.Ingredient5") != NULL)
        getChildByName("MainPanel.Ingredient5")->setVisible(panel == 2 && m_ingredients[4]);
    getChildByName("MainPanel.Ingredient1Desc")->setVisible(panel == 2 && m_ingredients[0]);
    getChildByName("MainPanel.Ingredient2Desc")->setVisible(panel == 2 && m_ingredients[1]);
    getChildByName("MainPanel.Ingredient3Desc")->setVisible(panel == 2 && m_ingredients[2]);
    getChildByName("MainPanel.Ingredient4Desc")->setVisible(panel == 2 && m_ingredients[3]);
    if (getChildByName("MainPanel.Ingredient5Desc") != NULL)
        getChildByName("MainPanel.Ingredient5Desc")->setVisible(panel == 2 && m_ingredients[4]);

    if (panel != m_panelType)
    {
        getChildByName("MainPanel.Inventory")->setVisible(panel == 0);
        getChildByName("MainPanel.HotBar")->setVisible(panel == 0);
        getChildByName("MainPanel.InventoryLabel")->setVisible(panel == 0);
        getChildByName("MainPanel.DescriptionText")->setVisible(panel == 1);
        getChildByName("MainPanel.IngredientsLabel")->setVisible(panel == 2);
        getChildByName("MainPanel.PreviewLabel")->setVisible(panel == 3);
        getChildByName("MainPanel.PreviewSlot")->setVisible(panel == 3);
        m_panelType = panel;
    }
}

void FJ_CraftingMenu::ShowDescriptionPanel(bool show)
{
    getChildByName("MainPanel.DescriptionPanel")->setVisible(show);
    getChildByName("MainPanel.DescriptionTextTop")->setVisible(show);
}

void FJ_CraftingMenu::ShowSuperCraftingSlots(int count)
{
    m_slot3Items[0]->setVisible(count == 3);
    m_slot3Items[1]->setVisible(count == 3);
    m_slot3Items[2]->setVisible(count == 3);
    m_slot2Items[0]->setVisible(count == 2);
    m_slot2Items[1]->setVisible(count == 2);
    m_panelMode = count;
    getChildByName("MainPanel.SpinnerTitle")->setVisible(count > 0);
    getChildByName("MainPanel.CraftingHSlots")->setVisible(count == 0);
    getChildByName("MainPanel.GroupName")->setVisible(count == 0);
    getChildByName("MainPanel.CraftingSelector.ScrollArrows")->setVisible(count > 0);
}

void FJ_CraftingMenu::MoveSelector(int horizontal, int vertical)
{
    FJ_FuiNode* selector = getChildByName("MainPanel.CraftingSelector");
    FJ_FuiNode* scrollArrows = selector->getChildByName("ScrollArrows");
    FJ_FuiNode* slots = getChildByName("MainPanel.CraftingHSlots");
    FJ_FuiNode* slotSelector2 = getChildByName("MainPanel.SlotSelector2");
    FJ_FuiNode* verticalSlot2 = slotSelector2->getChildByName("Crafting2VSlot2");
    FJ_FuiNode* slotSelector3 = getChildByName("MainPanel.SlotSelector3");
    FJ_FuiNode* verticalSlot3 = slotSelector3->getChildByName("Crafting3VSlot3");

    if (m_panelMode == 0)
    {
        selector->setX(slots->getX() - Math::round((selector->getWidth() - m_selectorStep) * 0.5f)
                       + horizontal * m_selectorStep);
        selector->setY(slots->getY() - Math::round((selector->getHeight() - m_selectorStep) * 0.5f));
        if (verticalSlot2 != NULL && verticalSlot3 != NULL)
        {
            slotSelector2->setVisible(vertical == 1);
            slotSelector3->setVisible(vertical == 2);
            if (slotSelector2->isVisible())
            {
                slotSelector2->setX(slots->getX() - Math::round((slotSelector2->getWidth() - m_selectorStep) * 0.5f)
                                    + horizontal * m_selectorStep);
                slotSelector2->setY(slots->getY() - Math::round((slotSelector2->getHeight() - m_selectorStep) * 0.5f)
                                    + m_selectorStep / 2);
            }
            if (slotSelector3->isVisible())
            {
                slotSelector3->setX(slots->getX() - Math::round((slotSelector3->getWidth() - m_selectorStep) * 0.5f)
                                    + horizontal * m_selectorStep);
                slotSelector3->setY(slots->getY() - Math::round((slotSelector3->getHeight() - m_selectorStep) * 0.5f));
            }
        }
        else
        {
            slotSelector2->setVisible(vertical == 1);
            slotSelector3->setVisible(vertical == 2);
            if (slotSelector2->isVisible())
            {
                slotSelector2->setX(slots->getX() - Math::round((slotSelector2->getWidth() - m_selectorStep) * 0.5f)
                                    + horizontal * m_selectorStep);
                slotSelector2->setY(slots->getY() - Math::round((slotSelector2->getHeight() - m_selectorStep) * 0.5f));
            }
            if (slotSelector3->isVisible())
            {
                slotSelector3->setX(slots->getX() - Math::round((slotSelector3->getWidth() - m_selectorStep) * 0.5f)
                                    + horizontal * m_selectorStep);
                slotSelector3->setY(slots->getY() - Math::round((slotSelector3->getHeight() - m_selectorStep) * 0.5f));
            }
        }
    }
    else
    {
        slotSelector2->setVisible(false);
        slotSelector3->setVisible(false);
        if (m_panelMode != 1)
        {
            if (m_panelMode == 2)
            {
                if (getChildByName("MainPanel.Slot2_1") != NULL && getChildByName("MainPanel.Slot2_2") != NULL)
                {
                    if (horizontal >= 2)
                        horizontal = 1;
                    selector->setX(m_slot2Items[horizontal]->getX()
                                   - Math::round((selector->getWidth() - m_selectorStep) * 0.5f));
                    selector->setY(m_slot2Items[horizontal]->getY()
                                   - Math::round((selector->getHeight() - m_selectorStep) * 0.5f));
                }
            }
            else if (m_panelMode == 3)
            {
                if (getChildByName("MainPanel.Slot3_1") != NULL && getChildByName("MainPanel.Slot3_2") != NULL
                    && getChildByName("MainPanel.Slot3_3") != NULL)
                {
                    if (horizontal >= 3)
                        horizontal = 2;
                    selector->setX(m_slot3Items[horizontal]->getX()
                                   - Math::round((selector->getWidth() - m_selectorStep) * 0.5f));
                    selector->setY(m_slot3Items[horizontal]->getY()
                                   - Math::round((selector->getHeight() - m_selectorStep) * 0.5f));
                }
            }
        }
        scrollArrows->setVisible(vertical > 0);
    }
}

void FJ_CraftingMenu::init(FJ_Event* event)
{
    m_slot2Items[0] = getChildByName("MainPanel.Slot2_1");
    m_slot2Items[1] = getChildByName("MainPanel.Slot2_2");
    m_slot3Items[0] = getChildByName("MainPanel.Slot3_1");
    m_slot3Items[1] = getChildByName("MainPanel.Slot3_2");
    m_slot3Items[2] = getChildByName("MainPanel.Slot3_3");
    m_activeSubtab = -1;
    m_verticalTabCount = 0;
    m_horizontalTabCount = 0;
    m_descriptionBaseY = 0;
    m_panelType = -1;
    m_activeTab = -1;
    m_selectorStep = getChildByName("MainPanel.CraftingHSlots")->asFJ_SlotList()->m_slotSize;
    for (int i = 0; i < 5; ++i)
        m_ingredients[i] = false;
    showPanelDisplay(1);
    ShowDescriptionPanel(false);
    ShowSuperCraftingSlots(0);
    MoveSelector(0, 0);
}

void FJ_CraftingMenu::InitCraftingMenu(int type)
{
    FJ_FuiNode* ingredientsLayout = getChildByName("MainPanel.IngredientsLayout");
    FJ_FuiNode* ingredientsLayoutSmall = getChildByName("MainPanel.IngredientsLayout_Small");
    FJ_FuiNode* craftingOutput = getChildByName("MainPanel.CraftingOutput");
    FJ_FuiNode* craftingArrow = getChildByName("MainPanel.CraftingArrow");
    FJ_Label* itemName = getChildByName("MainPanel.ItemName")->asFJ_Label();
    FJ_Label* itemNameSmall = getChildByName("MainPanel.ItemNameSmall")->asFJ_Label();
    FJ_Label* ingredient1Desc = getChildByName("MainPanel.Ingredient1Desc")->asFJ_Label();
    FJ_Label* ingredient2Desc = getChildByName("MainPanel.Ingredient2Desc")->asFJ_Label();
    FJ_Label* ingredient3Desc = getChildByName("MainPanel.Ingredient3Desc")->asFJ_Label();
    FJ_Label* ingredient4Desc = getChildByName("MainPanel.Ingredient4Desc")->asFJ_Label();
    FJ_Label* ingredient5Desc = getChildByName("MainPanel.Ingredient5Desc")->asFJ_Label();

    m_craftingType = type;
    if (type == 0)
    {
        ingredientsLayout->setVisible(false);
        ingredientsLayoutSmall->setVisible(true);
        ingredientsLayout->decX((ingredientsLayout->getWidth() - ingredientsLayoutSmall->getWidth()) * 0.5f);
        craftingOutput->decX((ingredientsLayout->getWidth() - ingredientsLayoutSmall->getWidth()) * 0.5f);
        craftingArrow->setX(
            (ingredientsLayoutSmall->getX() + ingredientsLayoutSmall->getWidth())
            + (craftingOutput->getX() - (ingredientsLayoutSmall->getX() + ingredientsLayoutSmall->getWidth())) * 0.5f
            - craftingArrow->getWidth() * 0.5f);
        itemName->setUseHtmlText(true);
    }
    else
    {
        ingredientsLayout->setVisible(true);
        ingredientsLayoutSmall->setVisible(false);
        itemName->setUseHtmlText(true);
    }
    if (itemNameSmall != NULL)
        itemNameSmall->setUseHtmlText(true);
    ingredient1Desc->setUseHtmlText(true);
    ingredient2Desc->setUseHtmlText(true);
    ingredient3Desc->setUseHtmlText(true);
    ingredient4Desc->setUseHtmlText(true);
    if (ingredient5Desc != NULL)
        ingredient5Desc->setUseHtmlText(true);
    if (itemNameSmall != NULL)
        itemNameSmall->SetLabel(L"");
    m_descriptionBaseY = (int)ingredient2Desc->getY();
}

void FJ_CraftingMenu::SetItemName(const std::wstring& name)
{
    FJ_Label* itemName = getChildByName("MainPanel.ItemName")->asFJ_Label();
    FJ_Label* itemNameSmall = getChildByName("MainPanel.ItemNameSmall")->asFJ_Label();
    itemName->SetLabel(name);
    if (itemNameSmall != NULL)
        itemNameSmall->SetLabel(L"");
    if (itemName->GetTextField()->getNumLines() > 2)
    {
        itemName->SetLabel(L"");
        if (itemNameSmall != NULL)
            itemNameSmall->SetLabel(name);
    }
}

void FJ_CraftingMenu::SetItemDescription(int index, const std::wstring& description)
{
    FJ_Label* ingredient1Desc = getChildByName("MainPanel.Ingredient1Desc")->asFJ_Label();
    FJ_Label* ingredient2Desc = getChildByName("MainPanel.Ingredient2Desc")->asFJ_Label();
    FJ_Label* ingredient3Desc = getChildByName("MainPanel.Ingredient3Desc")->asFJ_Label();
    FJ_Label* ingredient4Desc = getChildByName("MainPanel.Ingredient4Desc")->asFJ_Label();
    FJ_Label* ingredient5Desc = getChildByName("MainPanel.Ingredient5Desc")->asFJ_Label();

    switch (index)
    {
    case 0:
        ingredient1Desc->SetLabel(description);
        break;
    case 1:
        ingredient2Desc->SetLabel(description);
        ingredient2Desc->setY(m_descriptionBaseY);
        if (ingredient2Desc->GetTextField()->getNumLines() > 2)
            ingredient2Desc->incY(ingredient2Desc->GetAverageCharHeight() * 0.5f);
        break;
    case 2:
        ingredient3Desc->SetLabel(description);
        break;
    case 3:
        ingredient4Desc->SetLabel(description);
        break;
    case 4:
        if (ingredient5Desc != NULL)
            ingredient5Desc->SetLabel(description);
        break;
    }
}

void FJ_CraftingMenu::EnableTab(int index, bool enabled)
{
    FJ_FuiNode* tab = FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))
                          ->getChildByName(s_horizontalTabNames[index])
                          ->asMovieClip();
    FJ_FuiNode* tabOff = FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))
                             ->getChildByName(std::string(tab->getName()) + "_Off")
                             ->asMovieClip();
    FJ_FuiNode* touchPanel = FJ_FuiNode::asMovieClip(
        FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))->getChildByName(s_horizontalTouchPanelNames[index]));
    if (enabled)
    {
        tabOff->gotoAndStop(1);
        tab->setVisible(true);
        if (touchPanel != NULL)
            touchPanel->setVisible(true);
    }
    else
    {
        tabOff->gotoAndStop(2);
        tab->setVisible(false);
        if (touchPanel != NULL)
            touchPanel->setVisible(false);
    }
}

FJ_FuiNode* FJ_CraftingMenu::SetupIggySlot(FJ_FuiNode* node)
{
    FJ_FuiNode* iggy = node->getChildByName("iggy")->asMovieClip();
    if (iggy != NULL)
    {
        fuiRect rect;
        rect.xMin = 0.0f;
        rect.yMin = 0.0f;
        rect.xMax = iggy->getRenderNodeChildAt(0)->getWidth();
        rect.yMax = iggy->getRenderNodeChildAt(0)->getHeight();
        char name[64];
        std::sprintf(name, "iggy_%s", node->getName());
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, name);
        renderNode->setAlpha(0.25f);
        FJ_FuiNode* slot = new FJ_FuiNode(renderNode);
        renderNode->setFuiNode(slot);
        iggy->removeChildAt(0, true);
        iggy->addChildAt(slot, 0);
        return slot;
    }
    else
    {
        return node->getChildByName("iggy_" + std::string(node->getName()));
    }
}

void FJ_CraftingMenu::SetCraftingMenu(int horizontalCount, int verticalCount, bool enabled)
{
    int i;
    FJ_FuiNode* tab;
    FJ_FuiNode* tabOff;
    FJ_FuiNode* touchPanel;
    int frame;

    for (i = 0; i < 7; ++i)
    {
        tab = FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))
                  ->getChildByName(s_horizontalTabNames[i])
                  ->asMovieClip();
        tabOff = FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))
                     ->getChildByName(std::string(tab->getName()) + "_Off")
                     ->asMovieClip();
        touchPanel = FJ_FuiNode::asMovieClip(
            FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))->getChildByName(s_horizontalTouchPanelNames[i]));
        if (tab != NULL)
        {
            SetupIggySlot(tab)->setVisible(enabled);
            tab->getChildByName("staticicon")->setVisible(!enabled);
            if (m_craftingType == 0)
                FJ_FuiNode::asMovieClip(FJ_FuiNode::asMovieClip(tab->getChildByName("staticicon"))->getChildAt(0))
                    ->gotoAndStop(s_craftingIconFrames[i]);
            else
                FJ_FuiNode::asMovieClip(FJ_FuiNode::asMovieClip(tab->getChildByName("staticicon"))->getChildAt(0))
                    ->gotoAndStop(s_otherIconFrames[i]);

            if (i == 0)
                frame = 1;
            else if (i >= 1 && i <= 5)
                frame = 2;
            else
                frame = 3;
            FJ_FuiNode::asMovieClip(tab->getChildByName("HorTabOn"))->gotoAndStop(frame);

            if (touchPanel != NULL)
                touchPanel->setVisible(i < horizontalCount);
            tab->setVisible(i < horizontalCount);
            tabOff->setVisible(i < horizontalCount);
            m_horizontalTabCount = horizontalCount;
        }
        else
        {
            break;
        }
    }

    for (i = 0; i < 5; ++i)
    {
        tab =
            FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))->getChildByName(s_verticalTabNames[i])->asMovieClip();
        tabOff = FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))
                     ->getChildByName(std::string(tab->getName()) + "_Off")
                     ->asMovieClip();
        touchPanel = FJ_FuiNode::asMovieClip(
            FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))->getChildByName(s_verticalTouchPanelNames[i]));
        if (tab == NULL)
            break;

        SetupIggySlot(tab);
        if (touchPanel != NULL)
            touchPanel->setVisible(i < verticalCount);
        tab->setVisible(i < verticalCount);
        tabOff->setVisible(i < verticalCount);
        m_verticalTabCount = verticalCount;
    }
}

void FJ_CraftingMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
        setY(top);
}

void FJ_CraftingMenu::SelectVerticalItem(int index)
{
    if (m_panelMode != 0)
        return;

    FJ_FuiNode* selector = getChildByName("MainPanel.CraftingSelector");
    FJ_FuiNode* slots = getChildByName("MainPanel.CraftingHSlots");
    FJ_FuiNode* slotSelector2 = getChildByName("MainPanel.SlotSelector2");
    FJ_FuiNode* verticalSlot2 = slotSelector2->getChildByName("Crafting2VSlot2");
    FJ_FuiNode* slotSelector3 = getChildByName("MainPanel.SlotSelector3");
    FJ_FuiNode* verticalSlot3 = slotSelector3->getChildByName("Crafting3VSlot3");
    index--;
    if (verticalSlot2 != NULL && verticalSlot3 != NULL)
    {
        if (slotSelector2->isVisible() && index == -1)
            index = 0;
        selector->setY(slots->getY() - Math::round((selector->getHeight() - m_selectorStep) * 0.5f)
                       + m_selectorStep * index);
    }
}

void FJ_CraftingMenu::UpdateTabs(FJ_Event* event)
{
    int i;
    FJ_FuiNode* tab;
    FJ_FuiNode* tabOff;

    for (i = 0; i < m_horizontalTabCount; ++i)
    {
        tab = FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))
                  ->getChildByName(s_horizontalTabNames[i])
                  ->asMovieClip();
        tabOff = FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))
                     ->getChildByName(std::string(tab->getName()) + "_Off")
                     ->asMovieClip();
        if (i == m_activeTab)
        {
            tab->gotoAndStop(2);
            tabOff->setVisible(false);
        }
        else
        {
            tab->gotoAndStop(1);
            tabOff->setVisible(true);
        }
    }

    for (i = 0; i < m_verticalTabCount; ++i)
    {
        tab =
            FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))->getChildByName(s_verticalTabNames[i])->asMovieClip();
        tabOff = FJ_FuiNode::asMovieClip(getChildByName("MainPanel"))
                     ->getChildByName(std::string(tab->getName()) + "_Off")
                     ->asMovieClip();
        if (i == m_activeSubtab)
        {
            tab->gotoAndStop(2);
            tabOff->setVisible(false);
        }
        else
        {
            tab->gotoAndStop(1);
            tabOff->setVisible(true);
        }
    }
}

void FJ_CraftingMenu::SetActiveTab(int tab, int subtab)
{
    m_activeTab = tab;
    m_activeSubtab = subtab;
    UpdateTabs(NULL);
}

void FJ_CraftingMenu::ShowIngredient(int index, bool show)
{
    m_ingredients[index] = show;
}
