#include "game/ui/UIScene_BrewingStandMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Container.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/BrewingStandMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/UILayer.hpp"

extern CConsoleMinecraftApp gApp;

struct UIScene_BrewingStandMenuData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int playerIndex;
    bool splitscreen;
};

UIScene_BrewingStandMenu::UIScene_BrewingStandMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer),
      m_container(),
      m_bottleSlots(),
      m_ingredientSlot(),
      m_brewingPlate(),
      m_brewingStandLabel(),
      m_brewingArrow(),
      m_brewingBubbles(),
      m_brewingPlateBar()
{
    UIScene_BrewingStandMenuData* sceneData = static_cast<UIScene_BrewingStandMenuData*>(data);
    initialiseMovie();
    m_brewingArrow.init(UIString(L""), 0, 0, 400, 0);
    m_brewingBubbles.init(UIString(L""), 0, 0, 30, 0);
    m_brewingPlateBar.init(UIString(L""), 0, 0, 20, 0);

    m_container = sceneData->container;
    if (!m_container->getName().empty())
        m_brewingStandLabel.init(UIString(m_container->getName()));
    else
        m_brewingStandLabel.init(UIString(L10N::GetString(IDS_TILE_BREWINGSTAND)));

    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[type]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_BrewingMenu, this);
        }
    }

    BrewingStandMenu* menu = new BrewingStandMenu(sceneData->inventory, sceneData->container);
    Initialize(false, sceneData->playerIndex, menu, true, 5,
               IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_BrewingMax, false);
    m_ingredientSlot.addSlots(3, 1);
    m_brewingPlate.addSlots(4, 1);
    for (int i = 0; i < 3; ++i)
        m_bottleSlots[i].addSlots(i, 1);

    delete sceneData;
    gApp.SetRichPresenceContext(type, 11);
}

UIScene_BrewingStandMenu::~UIScene_BrewingStandMenu()
{
}

int UIScene_BrewingStandMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        columns = 1;
        break;
    default:
        break;
    }
    return columns;
}

int UIScene_BrewingStandMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_BrewingStandMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section, IUIScene_AbstractContainerMenu::ETapState state, int* x,
    int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int dx = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar
                                      : IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
            dx = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar
                                      : IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
            dx = -4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar
                                      : IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
            dx = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar;
            dx = 5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left
                 || state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left
                 || state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x <= 1)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel;
            else if (*x <= 3)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1;
            else if (*x <= 4)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_smallInventory)
            {
                if (*x <= 1)
                    target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel;
                else if (*x <= 3)
                    target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1;
                else if (*x <= 4)
                    target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2;
                else
                    target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3;
            }
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x <= 1)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        }
        break;
    default:
        break;
    }
    updateSlotPosition(section, target, state, x, y, dx, 0);
    return target;
}

void UIScene_BrewingStandMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                    _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        position->x = static_cast<float>(m_bottleSlots[0].getXPos());
        position->y = static_cast<float>(m_bottleSlots[0].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        position->x = static_cast<float>(m_bottleSlots[1].getXPos());
        position->y = static_cast<float>(m_bottleSlots[1].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        position->x = static_cast<float>(m_bottleSlots[2].getXPos());
        position->y = static_cast<float>(m_bottleSlots[2].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        position->x = static_cast<float>(m_ingredientSlot.getXPos());
        position->y = static_cast<float>(m_ingredientSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        position->x = static_cast<float>(m_brewingPlate.getXPos());
        position->y = static_cast<float>(m_brewingPlate.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        position->x = static_cast<float>(m_inventoryList.getXPos());
        position->y = static_cast<float>(m_inventoryList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        position->x = static_cast<float>(m_hotbarList.getXPos());
        position->y = static_cast<float>(m_hotbarList.getYPos());
        break;
    default:
        break;
    }
}

void UIScene_BrewingStandMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                 _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D controlSize;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        controlSize.x = m_bottleSlots[0].getWidth();
        controlSize.y = m_bottleSlots[0].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        controlSize.x = m_bottleSlots[1].getWidth();
        controlSize.y = m_bottleSlots[1].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        controlSize.x = m_bottleSlots[2].getWidth();
        controlSize.y = m_bottleSlots[2].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        controlSize.x = m_ingredientSlot.getWidth();
        controlSize.y = m_ingredientSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        controlSize.x = m_brewingPlate.getWidth();
        controlSize.y = m_brewingPlate.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        controlSize.x = m_inventoryList.getWidth();
        controlSize.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        controlSize.x = m_hotbarList.getWidth();
        controlSize.y = m_hotbarList.getHeight();
        break;
    default:
        break;
    }
    const int rows = getSectionRows(section);
    const int columns = getSectionColumns(section);
    size->x = controlSize.x / static_cast<float>(columns);
    size->y = controlSize.y / static_cast<float>(rows);
    int column = slot % columns;
    int row = slot / columns;
    position->x = static_cast<float>(column) * size->x;
    position->y = static_cast<float>(row) * size->y;
}

void UIScene_BrewingStandMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

void UIScene_BrewingStandMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                      int direction, bool wrap)
{
    const int columns = getSectionColumns(section);
    const int highlightSlot = direction * columns + slot;
    UIControl_SlotList* list = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        list = &m_bottleSlots[0];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        list = &m_bottleSlots[1];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        list = &m_bottleSlots[2];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        list = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        list = &m_brewingPlate;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        list = &m_hotbarList;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(highlightSlot);
}

int UIScene_BrewingStandMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        offset = 32;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        offset = 5;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        offset = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        offset = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        offset = 2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        offset = 3;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        offset = 4;
        break;
    default:
        break;
    }
    return offset;
}

unsigned int UIScene_BrewingStandMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_bottleSlots[0].setupControl(this, currentNode, std::string("Bottle1"));
            m_controls.push_back(&m_bottleSlots[0]);
        }
        {
            m_bottleSlots[1].setupControl(this, currentNode, std::string("Bottle2"));
            m_controls.push_back(&m_bottleSlots[1]);
        }
        {
            m_bottleSlots[2].setupControl(this, currentNode, std::string("Bottle3"));
            m_controls.push_back(&m_bottleSlots[2]);
        }
        {
            m_ingredientSlot.setupControl(this, currentNode, std::string("Ingredient"));
            m_controls.push_back(&m_ingredientSlot);
        }
        {
            m_brewingPlate.setupControl(this, currentNode, std::string("BrewingPlate"));
            m_controls.push_back(&m_brewingPlate);
        }
        {
            m_brewingStandLabel.setupControl(this, currentNode, std::string("BrewingStandText"));
            m_controls.push_back(&m_brewingStandLabel);
        }
        {
            m_brewingArrow.setupControl(this, currentNode, std::string("BrewingArrow"));
            m_controls.push_back(&m_brewingArrow);
        }
        {
            m_brewingBubbles.setupControl(this, currentNode, std::string("BrewingBubbles"));
            m_controls.push_back(&m_brewingBubbles);
        }
        {
            m_brewingPlateBar.setupControl(this, currentNode, std::string("BrewingPlateBar"));
            m_controls.push_back(&m_brewingPlateBar);
        }
        currentNode = parentNode;
    }
    return 1;
}

std::wstring UIScene_BrewingStandMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"BrewingStandMenuSplit") : std::wstring(L"BrewingStandMenu");
}

void UIScene_BrewingStandMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, 5, IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_BrewingMax, false);
    m_ingredientSlot.addSlots(3, 1);
    m_brewingPlate.addSlots(4, 1);
    for (int i = 0; i < 3; ++i)
        m_bottleSlots[i].addSlots(i, 1);
    m_brewingPlateBar.setProgress(0);
}

void UIScene_BrewingStandMenu::tick()
{
    m_brewingArrow.setProgress(m_container->getContainerData(0));
    m_brewingPlateBar.setProgress(m_container->getContainerData(1));
    int bubbles = 0;
    int frame = m_container->getContainerData(0) / 2 % 7;
    switch (frame)
    {
    case 0:
        bubbles = 0;
        break;
    case 1:
        bubbles = 30;
        break;
    case 2:
        bubbles = 25;
        break;
    case 3:
        bubbles = 20;
        break;
    case 4:
        bubbles = 15;
        break;
    case 5:
        bubbles = 10;
        break;
    case 6:
        bubbles = 5;
        break;
    }
    m_brewingBubbles.setProgress(bubbles);
    UIScene_AbstractContainerMenu::tick();
}

UIControl* UIScene_BrewingStandMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        control = &m_bottleSlots[0];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        control = &m_bottleSlots[1];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        control = &m_bottleSlots[2];
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        control = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        control = &m_brewingPlate;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        control = &m_hotbarList;
        break;
    default:
        break;
    }
    return control;
}

int UIScene_BrewingStandMenu::getSceneType()
{
    return EUIScene_BrewingStandMenu;
}
