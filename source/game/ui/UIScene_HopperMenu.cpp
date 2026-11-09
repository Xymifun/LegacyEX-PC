#include "game/ui/UIScene_HopperMenu.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/HopperMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/UILayer.hpp"


namespace
{
struct HopperMenuSceneData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int playerIndex;
    bool splitscreen;
};
}

UIScene_HopperMenu::UIScene_HopperMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer),
      m_hopperSlot(),
      m_hopperLabel()
{
    initialiseMovie();

    HopperMenuSceneData* sceneData = static_cast<HopperMenuSceneData*>(data);
    if (!sceneData->container->getName().empty())
    {
        m_hopperLabel.init(UIString(sceneData->container->getName()));
    }
    else
    {
        boost::shared_ptr<InteractionObject> interaction = boost::dynamic_pointer_cast<InteractionObject>(
            sceneData->container);
        if (interaction != 0 && interaction->getTypeId(false) == ClientboundContainerOpenPacket::MINECART_HOPPER)
            m_hopperLabel.init(UIString(L10N::GetString(IDS_CONTAINER_MINECART)));
        else
            m_hopperLabel.init(UIString(L10N::GetString(IDS_CONTAINER_HOPPER)));
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[sceneData->playerIndex]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_HopperMenu, this);
        }
    }

    HopperMenu* menu = new HopperMenu(boost::dynamic_pointer_cast<Container>(sceneData->inventory),
                                      sceneData->container, minecraft->mLocalPlayers[m_playerId]);
    m_containerSize = static_cast<int>(sceneData->container->getContainerSize());
    Initialize(false, sceneData->playerIndex, menu, true, m_containerSize,
               IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_HopperMax, false);
    m_hopperSlot.addSlots(0, 9);
    delete sceneData;
}

UIScene_HopperMenu::~UIScene_HopperMenu()
{
}

int UIScene_HopperMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots:
        columns = 5;
        break;
    default:
        break;
    }
    return columns;
}

int UIScene_HopperMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar:
        rows = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_HopperMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                              IUIScene_AbstractContainerMenu::ETapState state, int* x,
                                                              int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection newSection = section;
    int xOffset = 0;

    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar
                                          : IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory;
            xOffset = -2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar;
            xOffset = -2;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots;
            xOffset = 2;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots;
            xOffset = 2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots
                                          : IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory;
            xOffset = m_smallInventory ? 2 : 0;
        }
        break;
    }

    updateSlotPosition(section, newSection, state, x, y, xOffset, 0);
    return newSection;
}

void UIScene_HopperMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots:
        position->x = static_cast<float>(m_hopperSlot.getXPos());
        position->y = static_cast<float>(m_hopperSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory:
        position->x = static_cast<float>(m_inventoryList.getXPos());
        position->y = static_cast<float>(m_inventoryList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar:
        position->x = static_cast<float>(m_hotbarList.getXPos());
        position->y = static_cast<float>(m_hotbarList.getYPos());
        break;
    default:
        break;
    }
}

void UIScene_HopperMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                           _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D controlSize;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots:
        controlSize.x = m_hopperSlot.getWidth();
        controlSize.y = m_hopperSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory:
        controlSize.x = m_inventoryList.getWidth();
        controlSize.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar:
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
    const int row = slot / columns;
    const int column = slot % columns;
    position->x = static_cast<float>(column) * size->x;
    position->y = static_cast<float>(row) * size->y;
}

void UIScene_HopperMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                int direction, bool wrap)
{
    const int highlightSlot = direction * getSectionColumns(section) + slot;
    UIControl_SlotList* list = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots:
        list = &m_hopperSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar:
        list = &m_hotbarList;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(highlightSlot);
}

unsigned int UIScene_HopperMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_hopperSlot.setupControl(this, panel, std::string("Trap"));
        m_controls.push_back(&m_hopperSlot);
    }
    {
        m_hopperLabel.setupControl(this, panel, std::string("dispenserLabel"));
        m_controls.push_back(&m_hopperLabel);
    }
    return 1;
}

std::wstring UIScene_HopperMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"HopperMenuSplit") : std::wstring(L"HopperMenu");
}

void UIScene_HopperMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, m_containerSize,
               IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_HopperMax, false);
    m_hopperSlot.addSlots(0, 9);
}

UIControl* UIScene_HopperMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots:
        control = &m_hopperSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar:
        control = &m_hotbarList;
        break;
    default:
        break;
    }
    return control;
}

void UIScene_HopperMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_HopperMenu::getSceneType()
{
    return EUIScene_HopperMenu;
}

int UIScene_HopperMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar:
        offset = 32;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory:
        offset = 5;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots:
        offset = 0;
        break;
    default:
        break;
    }
    return offset;
}
