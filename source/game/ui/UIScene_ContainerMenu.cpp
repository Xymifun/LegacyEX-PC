#include "game/ui/UIScene_ContainerMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/item/Container.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/ui/UILayer.hpp"

namespace
{
const int PLAYER_INVENTORY_SLOTS = 36;

struct _ContainerMenuArgs
{
    boost::shared_ptr<Container> first;
    boost::shared_ptr<Container> second;
    int player;
    bool splitscreen;
};
}

UIScene_ContainerMenu::UIScene_ContainerMenu(int playerId, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(playerId, layer),
      m_containerList(),
      m_chestLabel()
{

    _ContainerMenuArgs* args = static_cast<_ContainerMenuArgs*>(data);
    m_largeChest = args->second->getContainerSize() > 27;

    initialiseMovie();

    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<InteractionObject> interaction = boost::dynamic_pointer_cast<InteractionObject>(args->second);
    const int typeId = interaction != NULL ? interaction->getTypeId(false) : -1;
    AbstractContainerMenu* menu = NULL;

    if (!args->second->getName().empty())
    {
        m_chestLabel.init(UIString(args->second->getName()));
    }
    else if (typeId == ClientboundContainerOpenPacket::BONUS_CHEST)
        m_chestLabel.init(UIString(L10N::GetString(IDS_BONUS_CHEST)));
    else if (typeId == ClientboundContainerOpenPacket::ENDER_CHEST)
        m_chestLabel.init(UIString(L10N::GetString(IDS_TILE_ENDERCHEST)));
    else if (typeId == ClientboundContainerOpenPacket::LARGE_CHEST)
        m_chestLabel.init(UIString(L10N::GetString(IDS_CHEST_LARGE)));
    else if (typeId == ClientboundContainerOpenPacket::MINECART_CHEST)
        m_chestLabel.init(UIString(L10N::GetString(IDS_CONTAINER_MINECART)));
    else if (typeId == ClientboundContainerOpenPacket::SHULKER_BOX)
        m_chestLabel.init(UIString(L10N::GetString(IDS_CONTAINER_SHULKER_BOX)));
    else
        m_chestLabel.init(UIString(L10N::GetString(IDS_CHEST)));

    if (typeId == ClientboundContainerOpenPacket::SHULKER_BOX)
        menu = new ShulkerBoxMenu(args->first, args->second,
                                  boost::dynamic_pointer_cast<Player>(minecraft->mLocalPlayers[playerId]));
    if (menu == NULL)
        menu = new ChestMenu(args->first, args->second,
                             boost::dynamic_pointer_cast<Player>(minecraft->mLocalPlayers[playerId]));

    if (minecraft->mGameModes[playerId] != NULL)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[playerId]->getTutorial();
        if (tutorial != NULL)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_ContainerMenu, this);
        }
    }

    const int containerSlots = static_cast<int>(menu->m_slots.size()) - PLAYER_INVENTORY_SLOTS;
    Initialize(false, args->player, menu, true, containerSlots,
               IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_ContainerMax, false);
    m_containerList.addSlots(0, containerSlots);

    delete args;
    m_takeAllEnabled = true;
}

UIScene_ContainerMenu::~UIScene_ContainerMenu()
{
}

int UIScene_ContainerMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return static_cast<unsigned int>(section) <= IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest ? 9 : 0;
}

int UIScene_ContainerMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest:
    {
        AbstractContainerMenu* menu = m_menu;
        int containerSlots = static_cast<int>(menu->m_slots.size()) - PLAYER_INVENTORY_SLOTS;
        rows = containerSlots / 9U;
        break;
    }
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar:
        rows = 1;
        break;
    }
    return rows;
}

unsigned int UIScene_ContainerMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                                 IUIScene_AbstractContainerMenu::ETapState state,
                                                                 int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection newSection = section;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar
                                          : IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest
                                          : IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory;
        break;
    }
    updateSlotPosition(section, newSection, state, x, y, 0, 0);
    return newSection;
}

void UIScene_ContainerMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                 _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest:
        position->x = m_containerList.getXPos();
        position->y = m_containerList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory:
        position->x = m_inventoryList.getXPos();
        position->y = m_inventoryList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar:
        position->x = m_hotbarList.getXPos();
        position->y = m_hotbarList.getYPos();
        break;
    }
}

void UIScene_ContainerMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                              _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D controlSize;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest:
        controlSize.x = m_containerList.getWidth();
        controlSize.y = m_containerList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory:
        controlSize.x = m_inventoryList.getWidth();
        controlSize.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar:
        controlSize.x = m_hotbarList.getWidth();
        controlSize.y = m_hotbarList.getHeight();
        break;
    }
    const int rows = getSectionRows(section);
    const int columns = getSectionColumns(section);
    size->x = controlSize.x / columns;
    size->y = controlSize.y / rows;
    const int x = slot % columns;
    const int y = slot / columns;
    position->x = x * size->x;
    position->y = y * size->y;
}

void UIScene_ContainerMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                   int direction, bool wrap)
{
    const int highlightSlot = direction * getSectionColumns(section) + slot;
    UIControl_SlotList* list = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest:
        list = &m_containerList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar:
        list = &m_hotbarList;
        break;
    }
    if (wrap)
        list->setHighlightSlot(highlightSlot);
}

int UIScene_ContainerMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest:
        offset = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory:
    {
        AbstractContainerMenu* menu = m_menu;
        offset = static_cast<int>(menu->m_slots.size()) - PLAYER_INVENTORY_SLOTS;
        break;
    }
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar:
    {
        AbstractContainerMenu* menu = m_menu;
        offset = static_cast<int>(menu->m_slots.size()) - 9;
        break;
    }
    }
    return offset;
}

void UIScene_ContainerMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

unsigned int UIScene_ContainerMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_containerList.setupControl(this, currentNode, std::string("containerList"));
            m_controls.push_back(&m_containerList);
        }
        {
            m_chestLabel.setupControl(this, currentNode, std::string("chestLabel"));
            m_controls.push_back(&m_chestLabel);
        }
        currentNode = parentNode;
    }
    return 1;
}

std::wstring UIScene_ContainerMenu::getMoviePath()
{
    if (m_largeChest)
        return 1 < gApp.GetLocalPlayerCount() ? std::wstring(L"ChestLargeMenuSplit") : std::wstring(L"ChestLargeMenu");
    return 1 < gApp.GetLocalPlayerCount() ? std::wstring(L"ChestMenuSplit") : std::wstring(L"ChestMenu");
}

void UIScene_ContainerMenu::handleReload()
{
    AbstractContainerMenu* menu = m_menu;
    const int count = static_cast<int>(menu->m_slots.size()) - PLAYER_INVENTORY_SLOTS;
    Initialize(true, m_playerId, menu, true, count, IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_ContainerMax, false);
    m_containerList.addSlots(0, count);
}

UIControl* UIScene_ContainerMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest:
        control = &m_containerList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar:
        control = &m_hotbarList;
        break;
    }
    return control;
}

int UIScene_ContainerMenu::getSceneType()
{
    return EUIScene_ContainerMenu;
}
