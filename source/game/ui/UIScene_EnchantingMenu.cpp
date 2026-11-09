#include "game/ui/UIScene_EnchantingMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstdio>
#include <cstring>

#include <boost/shared_ptr.hpp>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/ui/UILayer.hpp"


struct UIScene_EnchantingMenuData
{
    boost::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int playerIndex;
    int alignment;
    std::wstring name;
};

UIScene_EnchantingMenu::UIScene_EnchantingMenu(int pad, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(pad, layer),
      m_ingredientSlot(),
      m_lapisSlot(),
      m_enchantLabel(),
      m_enchantmentButtons(),
      m_enchantmentBook()
{
    UIScene_EnchantingMenuData* sceneData = static_cast<UIScene_EnchantingMenuData*>(data);
    initialiseMovie();
    m_enchantmentButtons[0].init(0);
    m_enchantmentButtons[1].init(1);
    m_enchantmentButtons[2].init(2);
    m_enchantmentButtons[0].SetEnchantmentLevel(1);
    m_enchantmentButtons[1].SetEnchantmentLevel(2);
    m_enchantmentButtons[2].SetEnchantmentLevel(3);
    m_enchantLabel.init(
        UIString(sceneData->name.empty() ? std::wstring(L10N::GetString(IDS_ENCHANT)) : sceneData->name));
    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[pad]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_EnchantingMenu, this);
        }
    }
    EnchantmentMenu* menu = new EnchantmentMenu(sceneData->inventory, sceneData->level,
                                                BlockPos(sceneData->x, sceneData->y, sceneData->z));
    Initialize(false, sceneData->playerIndex, menu, true, 2,
               IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_EnchantMax, false);
    m_ingredientSlot.addSlots(0, 1);
    m_lapisSlot.addSlots(1, 1);
    gApp.SetRichPresenceContext(m_playerId, 10);
    delete sceneData;
}

UIScene_EnchantingMenu::~UIScene_EnchantingMenu()
{
}

bool UIScene_EnchantingMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        return true;
    default:
        return false;
    }
}

int UIScene_EnchantingMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        columns = 1;
        break;
    default:
        break;
    }
    return columns;
}

int UIScene_EnchantingMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_EnchantingMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                                  IUIScene_AbstractContainerMenu::ETapState state,
                                                                  int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection newSection = section;
    int xOffset = 0;

    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x > 2)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3;
            else if (*x == 2)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
            else
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x > 2)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
            else if (*x > 1)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
            else
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_smallInventory)
            {
                if (*x > 2)
                    newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3;
                else if (*x == 2)
                    newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
                else
                    newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
            }
            else
            {
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory;
            }
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar
                                          : IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar
                                          : IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory;
            xOffset = -2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar;
            xOffset = -2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar;
            xOffset = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar
                                          : IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory;
            xOffset = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem;
        break;
    }

    updateSlotPosition(section, newSection, state, x, y, xOffset, 0);
    return newSection;
}

void UIScene_EnchantingMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                  _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        position->x = static_cast<float>(m_ingredientSlot.getXPos());
        position->y = static_cast<float>(m_ingredientSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        position->x = static_cast<float>(m_lapisSlot.getXPos());
        position->y = static_cast<float>(m_lapisSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        position->x = static_cast<float>(m_inventoryList.getXPos());
        position->y = static_cast<float>(m_inventoryList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        position->x = static_cast<float>(m_hotbarList.getXPos());
        position->y = static_cast<float>(m_hotbarList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        position->x = static_cast<float>(m_enchantmentButtons[0].getXPos());
        position->y = static_cast<float>(m_enchantmentButtons[0].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        position->x = static_cast<float>(m_enchantmentButtons[1].getXPos());
        position->y = static_cast<float>(m_enchantmentButtons[1].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        position->x = static_cast<float>(m_enchantmentButtons[2].getXPos());
        position->y = static_cast<float>(m_enchantmentButtons[2].getYPos());
        break;
    default:
        break;
    }
}

void UIScene_EnchantingMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                               _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D extent;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        extent.x = static_cast<float>(m_ingredientSlot.getWidth());
        extent.y = static_cast<float>(m_ingredientSlot.getHeight());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        extent.x = static_cast<float>(m_lapisSlot.getWidth());
        extent.y = static_cast<float>(m_lapisSlot.getHeight());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        extent.x = static_cast<float>(m_inventoryList.getWidth());
        extent.y = static_cast<float>(m_inventoryList.getHeight());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        extent.x = static_cast<float>(m_hotbarList.getWidth());
        extent.y = static_cast<float>(m_hotbarList.getHeight());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        extent.x = static_cast<float>(m_enchantmentButtons[0].getWidth());
        extent.y = static_cast<float>(m_enchantmentButtons[0].getHeight());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        extent.x = static_cast<float>(m_enchantmentButtons[1].getWidth());
        extent.y = static_cast<float>(m_enchantmentButtons[1].getHeight());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        extent.x = static_cast<float>(m_enchantmentButtons[2].getWidth());
        extent.y = static_cast<float>(m_enchantmentButtons[2].getHeight());
        break;
    default:
        break;
    }
    if (IsSectionSlotList(section))
    {
        const int rows = getSectionRows(section);
        const int columns = getSectionColumns(section);
        const float cellWidth = extent.x / static_cast<float>(columns);
        const float cellHeight = extent.y / static_cast<float>(rows);
        size->x = cellWidth;
        size->y = cellHeight;
        position->x = static_cast<float>(slot % columns) * cellWidth;
        position->y = static_cast<float>(slot / columns) * cellHeight;
    }
    else
    {
        GetPositionOfSection(section, position);
        size->x = extent.x;
        size->y = extent.y;
    }
}

void UIScene_EnchantingMenu::handleOtherClicked(int pad, IUIScene_AbstractContainerMenu::ESceneSection section, int,
                                                bool)
{
    int buttonId = -1;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        buttonId = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        buttonId = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        buttonId = 2;
        break;
    default:
        break;
    }
    Minecraft* minecraft = Minecraft::GetInstance();
    if (buttonId >= 0
        && m_menu->clickMenuButton(boost::dynamic_pointer_cast<Player>(minecraft->mLocalPlayers[pad]), buttonId))
        minecraft->mGameModes[pad]->handleInventoryButtonClick(m_menu->m_containerId, buttonId);
}

int UIScene_EnchantingMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        offset = 29;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        offset = 2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        offset = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        offset = 1;
        break;
    default:
        break;
    }
    return offset;
}

void UIScene_EnchantingMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                    int direction, bool wrap)
{
    const int columns = getSectionColumns(section);
    const int index = direction * columns + slot;
    UIControl_SlotList* list = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        list = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        list = &m_lapisSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        list = &m_hotbarList;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(index);
}

std::vector<HtmlString>* UIScene_EnchantingMenu::GetSectionHoverText(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int index = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        index = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        index = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        index = 2;
        break;
    default:
        return NULL;
    }

    const bool instabuild = Minecraft::GetInstance()->mLocalPlayers[getPad()]->mAbilities.mInstabuild;
    const int lapisCount = getMenu()->getGoldCount();
    const int cost = getMenu()->mCosts[index];
    Enchantment* enchantment = Enchantment::byId(getMenu()->mEnchantmentIds[index]);
    const int level = getMenu()->mEnchantmentLevels[index];
    const int lapisCost = index + 1;
    if (cost > 0 && level >= 0 && enchantment != NULL)
    {
        std::vector<HtmlString>* result = new std::vector<HtmlString>();
        result->push_back(HtmlString(enchantment->getFullname(level).mText + L"...?", eMinecraftColour_HTMLColor_White,
                                     true, false, 0));
        if (!instabuild)
        {
            result->push_back(HtmlString(L"", static_cast<eMinecraftColour>(0), false, false, 0));
            if (Minecraft::GetInstance()->mLocalPlayers[getPad()]->mExperienceLevel < cost)
            {
                std::wstring requirement = L10N::GetString(IDS_ENCHANT_LEVEL_REQUIREMENT);
                requirement = replaceAll(requirement, L"{*LEVEL*}", _toString(cost));
                result->push_back(HtmlString(requirement, eMinecraftColour_HTMLColor_c, false, false, 0));
            }
            else
            {
                std::wstring text = L"";
                if (lapisCost == 1)
                {
                    text = L10N::GetString(IDS_ENCHANT_LAPIS_ONE);
                }
                else
                {
                    text = L10N::GetString(IDS_ENCHANT_LAPIS_MANY);
                    text = replaceAll(text, L"{*COST*}", _toString(lapisCost));
                }
                if (lapisCount >= lapisCost)
                    result->push_back(HtmlString(text, eMinecraftColour_HTMLColor_7, false, false, 0));
                else
                    result->push_back(HtmlString(text, eMinecraftColour_HTMLColor_c, false, false, 0));

                if (lapisCost == 1)
                {
                    text = L10N::GetString(IDS_ENCHANT_LEVEL_ONE);
                }
                else
                {
                    text = L10N::GetString(IDS_ENCHANT_LEVEL_MANY);
                    text = replaceAll(text, L"{*COST*}", _toString(lapisCost));
                }
                result->push_back(HtmlString(text, eMinecraftColour_HTMLColor_7, false, false, 0));
            }
        }
        return result;
    }
    return NULL;
}

bool UIScene_EnchantingMenu::IsActiveButton(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    bool active = false;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        if (!static_cast<UIControl_EnchantmentButton*>(getSection(section))->IsInactive())
            active = true;
        break;
    default:
        break;
    }
    return active;
}

unsigned int UIScene_EnchantingMenu::overrideTooltips(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                      not_null_ptr<ItemInstance>, bool, bool, bool, int,
                                                      EToolTipItem& first, EToolTipItem&, EToolTipItem&, EToolTipItem&,
                                                      EToolTipItem&, EToolTipItem&, EToolTipItem&)
{
    unsigned int count = 0;
    if (IsActiveButton(section))
    {
        count = 1;
        first = eToolTipSelect;
    }
    return count;
}

unsigned int UIScene_EnchantingMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_ingredientSlot.setupControl(this, panel, std::string("ingredient"));
        m_controls.push_back(&m_ingredientSlot);
    }
    {
        m_lapisSlot.setupControl(this, panel, std::string("lapis"));
        m_controls.push_back(&m_lapisSlot);
    }
    {
        m_enchantmentButtons[0].setupControl(this, panel, std::string("Button1"));
        m_controls.push_back(&m_enchantmentButtons[0]);
    }
    {
        m_enchantmentButtons[1].setupControl(this, panel, std::string("Button2"));
        m_controls.push_back(&m_enchantmentButtons[1]);
    }
    {
        m_enchantmentButtons[2].setupControl(this, panel, std::string("Button3"));
        m_controls.push_back(&m_enchantmentButtons[2]);
    }
    {
        m_enchantLabel.setupControl(this, panel, std::string("enchantLabel"));
        m_controls.push_back(&m_enchantLabel);
    }
    {
        m_enchantmentBook.setupControl(this, panel, std::string("iggy_EnchantmentBook"));
        m_controls.push_back(&m_enchantmentBook);
    }
    return 1;
}

std::wstring UIScene_EnchantingMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"EnchantingMenuSplit") : std::wstring(L"EnchantingMenu");
}

void UIScene_EnchantingMenu::handleReload()
{
    static_cast<IUIScene_AbstractContainerMenu*>(this)->Initialize(
        true, m_playerId, m_menu, true, 2, IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar,
        IUIScene_AbstractContainerMenu::ESceneSection_EnchantMax, false);
    m_enchantmentButtons[0].SetEnchantmentLevel(1);
    m_enchantmentButtons[1].SetEnchantmentLevel(2);
    m_enchantmentButtons[2].SetEnchantmentLevel(3);
    m_ingredientSlot.addSlots(0, 1);
    m_lapisSlot.addSlots(1, 1);
}

UIControl* UIScene_EnchantingMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantItem:
        control = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantLapis:
        control = &m_lapisSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton1:
        control = &m_enchantmentButtons[0];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton2:
        control = &m_enchantmentButtons[1];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_EnchantButton3:
        control = &m_enchantmentButtons[2];
        break;
    default:
        break;
    }
    return control;
}

EnchantmentMenu* UIScene_EnchantingMenu::getMenu()
{
    return static_cast<EnchantmentMenu*>(m_menu);
}

void UIScene_EnchantingMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == 0 || minecraft->mGameModes[m_playerId] == 0)
        return;
    if (std::strcmp(name, "EnchantmentBook") == 0)
    {
        m_enchantmentBook.render(rect);
        return;
    }
    int button = -1;
    std::sscanf(name, "slot_Button%d", &button);
    if (button >= 0)
    {
        m_enchantmentButtons[button - 1].render(rect);
        return;
    }
    UIScene_AbstractContainerMenu::customDraw(name, rect);
}

void UIScene_EnchantingMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_EnchantingMenu::getSceneType()
{
    return EUIScene_EnchantingMenu;
}
