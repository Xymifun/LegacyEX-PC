#include "game/ui/UIScene_ClassicCraftingMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/CraftingMenu.hpp"
#include "game/ui/FireworksMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UILayer.hpp"

UIScene_ClassicCraftingMenu::UIScene_ClassicCraftingMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer),
      m_resultSlot(),
      m_craftingSlots(),
      m_titleLabel()
{
    initialiseMovie();

    ClassicCraftingMenuInitData* sceneData = static_cast<ClassicCraftingMenuInitData*>(data);
    m_fireworks = sceneData->fireworks;
    AbstractContainerMenu* menu;
    eTutorial_State tutorialState;
    if (m_fireworks)
    {
        m_titleLabel.init(UIString(L10N::GetString(IDS_HOW_TO_PLAY_MENU_FIREWORKS)));
        menu = new FireworksMenu(sceneData->localPlayer->mInventory, sceneData->localPlayer->mLevel,
                                 BlockPos(sceneData->x, sceneData->y, sceneData->z));
        tutorialState = eTutorial_State_FireworksMenu;
    }
    else
    {
        m_titleLabel.init(UIString(L10N::GetString(IDS_CONTROLS_CRAFTING)));
        menu = new CraftingMenu(sceneData->localPlayer->mInventory, sceneData->localPlayer->mLevel,
                                BlockPos(sceneData->x, sceneData->y, sceneData->z));
        tutorialState = eTutorial_State_SurvivalCrafting;
    }

    if (Minecraft::GetInstance()->mGameModes[sceneData->player] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[sceneData->player]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(tutorialState, this);
        }
    }

    Initialize(false, sceneData->player, menu, true, CraftingMenu::INV_SLOT_START,
               IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingMax, false);
    m_resultSlot.addSlots(CraftingMenu::RESULT_SLOT, 1);
    m_craftingSlots.addSlots(CraftingMenu::CRAFT_SLOT_START, 9);

    delete sceneData;
    gApp.SetRichPresenceContext(m_playerId, 5);
}

UIScene_ClassicCraftingMenu::~UIScene_ClassicCraftingMenu()
{
}

int UIScene_ClassicCraftingMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid:
        columns = 3;
        break;
    default:
        break;
    }
    return columns;
}

int UIScene_ClassicCraftingMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid:
        rows = 3;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_ClassicCraftingMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section, IUIScene_AbstractContainerMenu::ETapState state, int* x,
    int* y)
{
    unsigned int newSection = section;
    int xOffset = 0;
    int yOffset = 0;

    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory;
            xOffset = -1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar;
            xOffset = -1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left
                 || state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory;
            xOffset = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar;
            xOffset = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid;
            yOffset = -1;
            *x = getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid);
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid;
            *x = 0;
            yOffset = -1;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x < 6)
            {
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid;
                xOffset = 1;
            }
            else
            {
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult;
            }
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x < 6)
            {
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid;
                xOffset = 1;
            }
            else
            {
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult;
            }
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory;
        }
        break;
    }

    updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(newSection), state, x, y,
                       xOffset, yOffset);
    return newSection;
}

void UIScene_ClassicCraftingMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                       _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid:
        position->x = m_craftingSlots.getXPos();
        position->y = m_craftingSlots.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult:
        position->x = m_resultSlot.getXPos();
        position->y = m_resultSlot.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory:
        position->x = m_inventoryList.getXPos();
        position->y = m_inventoryList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar:
        position->x = m_hotbarList.getXPos();
        position->y = m_hotbarList.getYPos();
        break;
    }
}

void UIScene_ClassicCraftingMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                    _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D sectionSize;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid:
        sectionSize.x = m_craftingSlots.getWidth();
        sectionSize.y = m_craftingSlots.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult:
        sectionSize.x = m_resultSlot.getWidth();
        sectionSize.y = m_resultSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory:
        sectionSize.x = m_inventoryList.getWidth();
        sectionSize.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar:
        sectionSize.x = m_hotbarList.getWidth();
        sectionSize.y = m_hotbarList.getHeight();
        break;
    default:
        break;
    }
    int rows = getSectionRows(section);
    int columns = getSectionColumns(section);
    size->x = sectionSize.x / static_cast<float>(columns);
    size->y = sectionSize.y / static_cast<float>(rows);
    int column = slot % columns;
    int row = slot / columns;
    position->x = static_cast<float>(column) * size->x;
    position->y = static_cast<float>(row) * size->y;
}

void UIScene_ClassicCraftingMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_ClassicCraftingMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar:
        offset = 37;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory:
        offset = 10;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult:
        offset = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid:
        offset = 1;
        break;
    default:
        break;
    }
    return offset;
}

void UIScene_ClassicCraftingMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                         int slot, int direction, bool wrap)
{
    int columns = getSectionColumns(section);
    int index = direction * columns + slot;
    UIControl_SlotList* list = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid:
        list = &m_craftingSlots;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult:
        list = &m_resultSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar:
        list = &m_hotbarList;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(index);
}

std::wstring UIScene_ClassicCraftingMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"ClassicCraftingMenuSplit");
    return std::wstring(L"ClassicCraftingMenu");
}

unsigned int UIScene_ClassicCraftingMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_resultSlot.setupControl(this, panel, std::string("Result"));
        m_controls.push_back(&m_resultSlot);
    }
    {
        m_craftingSlots.setupControl(this, panel, std::string("CraftingList"));
        m_controls.push_back(&m_craftingSlots);
    }
    {
        m_titleLabel.setupControl(this, panel, std::string("TitleLabel"));
        m_controls.push_back(&m_titleLabel);
    }
    return 1;
}

void UIScene_ClassicCraftingMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, CraftingMenu::INV_SLOT_START,
               IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingMax, false);
    m_resultSlot.addSlots(CraftingMenu::RESULT_SLOT, 1);
    m_craftingSlots.addSlots(CraftingMenu::CRAFT_SLOT_START, 9);
}

UIControl* UIScene_ClassicCraftingMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingGrid:
        control = &m_craftingSlots;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingResult:
        control = &m_resultSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ClassicCraftingHotbar:
        control = &m_hotbarList;
        break;
    default:
        break;
    }
    return control;
}

int UIScene_ClassicCraftingMenu::getSceneType()
{
    return EUIScene_ClassicCraftingMenu;
}
