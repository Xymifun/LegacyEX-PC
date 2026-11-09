#include "game/ui/UIScene_BeaconMenu.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Item.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BeaconMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/UILayer.hpp"

#include <cstdio>

struct BeaconMenuData
{
    boost::shared_ptr<Container> inventory;
    boost::shared_ptr<Container> beacon;
    int playerIndex;
    bool splitscreen;
};

UIScene_BeaconMenu::UIScene_BeaconMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer),
      m_activatorSlot(),
      m_activatorList(),
      m_primaryPowerLabel(),
      m_secondaryPowerLabel(),
      m_powerButtons(),
      m_confirmButton()
{
    BeaconMenuData* sceneData = static_cast<BeaconMenuData*>(data);
    m_beaconContainer = boost::shared_ptr<Container>();
    m_primaryEffect = -1;
    m_beaconLevel = -1;
    m_secondaryEffect = -1;
    initialiseMovie();
    m_primaryPowerLabel.init(UIString(0xb3c82373));
    m_secondaryPowerLabel.init(UIString(0x7b8c1d5d));
    int index;
    for (index = 0; index < 7; ++index)
        m_powerButtons[index].setVisible(false);
    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[type]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_BeaconMenu, this);
        }
    }
    m_beaconContainer = sceneData->beacon;
    BeaconMenu* menu = new BeaconMenu(sceneData->inventory, sceneData->beacon);
    Initialize(false, sceneData->playerIndex, menu, true, 1, IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_BeaconMax, false);
    m_activatorSlot.addSlots(0, 1);
    AbstractContainerMenu* currentMenu = m_menu;
    unsigned int slotCount = currentMenu->m_slots.size();
    m_activatorList.addSlots(slotCount, 4);
    delete sceneData;
}

bool UIScene_BeaconMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        return true;
    default:
        return false;
    }
}

int UIScene_BeaconMenu::GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
        return m_powerButtons[0].GetData();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
        return m_powerButtons[1].GetData();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
        return m_powerButtons[2].GetData();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        return m_powerButtons[3].GetData();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        return m_powerButtons[4].GetData();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        return m_powerButtons[5].GetData();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        return m_powerButtons[6].GetData();
    default:
        return 0;
    }
}

unsigned int UIScene_BeaconMenu::IsVisible(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2
        && GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) == 0)
        return 0;
    return 1;
}

int UIScene_BeaconMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        columns = 1;
        break;
    default:
        break;
    }
    return columns;
}

int UIScene_BeaconMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        rows = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_BeaconMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                              IUIScene_AbstractContainerMenu::ETapState state, int* x,
                                                              int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection newSection = section;
    int xOffset = 0;

    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x < 4)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5;
            else if (*x < 7)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator;
            else
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x < 2)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1;
            else if (*x < 5)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2;
            else if (*x > 8)
                newSection = GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) > 0
                                 ? IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2
                                 : IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
            else
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_smallInventory)
            {
                if (*x < 4)
                    newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5;
                else if (*x < 7)
                    newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator;
                else
                    newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm;
            }
            else
            {
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory;
            }
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar
                                          : IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory;
            xOffset = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left
                 || state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
            xOffset = -1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left
                 || state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
            xOffset = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left
                 || state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
        {
            if (GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) > 0)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2;
            else
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar
                                          : IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory;
            xOffset = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
            xOffset = -7;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            if (GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) > 0)
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2;
            else
                newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar;
            xOffset = -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            newSection = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar
                                          : IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory;
            xOffset = -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left
                 || state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            newSection = IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator;
        }
        break;
    }

    updateSlotPosition(section, newSection, state, x, y, xOffset, 0);
    return newSection;
}

void UIScene_BeaconMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        position->x = static_cast<float>(m_activatorSlot.getXPos());
        position->y = static_cast<float>(m_activatorSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        position->x = static_cast<float>(m_inventoryList.getXPos());
        position->y = static_cast<float>(m_inventoryList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        position->x = static_cast<float>(m_hotbarList.getXPos());
        position->y = static_cast<float>(m_hotbarList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
        position->x = static_cast<float>(m_powerButtons[0].getXPos());
        position->y = static_cast<float>(m_powerButtons[0].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
        position->x = static_cast<float>(m_powerButtons[1].getXPos());
        position->y = static_cast<float>(m_powerButtons[1].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
        position->x = static_cast<float>(m_powerButtons[2].getXPos());
        position->y = static_cast<float>(m_powerButtons[2].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        position->x = static_cast<float>(m_powerButtons[3].getXPos());
        position->y = static_cast<float>(m_powerButtons[3].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        position->x = static_cast<float>(m_powerButtons[4].getXPos());
        position->y = static_cast<float>(m_powerButtons[4].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        position->x = static_cast<float>(m_powerButtons[5].getXPos());
        position->y = static_cast<float>(m_powerButtons[5].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        position->x = static_cast<float>(m_powerButtons[6].getXPos());
        position->y = static_cast<float>(m_powerButtons[6].getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm:
        position->x = static_cast<float>(m_confirmButton.getXPos());
        position->y = static_cast<float>(m_confirmButton.getYPos());
        break;
    }
}

void UIScene_BeaconMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                           _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D extent;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        extent.x = m_activatorSlot.getWidth();
        extent.y = m_activatorSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        extent.x = m_inventoryList.getWidth();
        extent.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        extent.x = m_hotbarList.getWidth();
        extent.y = m_hotbarList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
        extent.x = m_powerButtons[0].getWidth();
        extent.y = m_powerButtons[0].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
        extent.x = m_powerButtons[1].getWidth();
        extent.y = m_powerButtons[1].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
        extent.x = m_powerButtons[2].getWidth();
        extent.y = m_powerButtons[2].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        extent.x = m_powerButtons[3].getWidth();
        extent.y = m_powerButtons[3].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        extent.x = m_powerButtons[4].getWidth();
        extent.y = m_powerButtons[4].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        extent.x = m_powerButtons[5].getWidth();
        extent.y = m_powerButtons[5].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        extent.x = m_powerButtons[6].getWidth();
        extent.y = m_powerButtons[6].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm:
        extent.x = m_confirmButton.getWidth();
        extent.y = m_confirmButton.getHeight();
        break;
    }
    if (IsSectionSlotList(section))
    {
        int rows = getSectionRows(section);
        int columns = getSectionColumns(section);
        float cellWidth = extent.x / columns;
        float cellHeight = extent.y / rows;
        int column = slot % columns;
        int row = slot / columns;
        size->y = cellHeight;
        size->x = cellWidth;
        position->x = static_cast<float>(column) * cellWidth;
        position->y = static_cast<float>(row) * cellHeight;
    }
    else
    {
        GetPositionOfSection(section, position);
        size->x = extent.x;
        size->y = extent.y;
    }
}

unsigned int UIScene_BeaconMenu::GetPowerButtonTier(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        return 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        return 2;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        return 3;
    default:
        return 0;
    }
}

MobEffect* UIScene_BeaconMenu::GetPowerButtonEffect(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char powerButtonColumns[7] = {0, 1, 0, 1, 0, 0, 0};
    int column = 0;
    int tier = GetPowerButtonTier(section);
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1
        && section <= IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2)
        column = powerButtonColumns[section - IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1];
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2)
        return MobEffect::byId(m_beaconContainer->getContainerData(1));
    return BeaconBlockEntity::sBeaconEffects[tier][column];
}

unsigned char UIScene_BeaconMenu::IsPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
        return m_powerButtons[0].IsButtonSelected();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
        return m_powerButtons[1].IsButtonSelected();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
        return m_powerButtons[2].IsButtonSelected();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        return m_powerButtons[3].IsButtonSelected();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        return m_powerButtons[4].IsButtonSelected();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        return m_powerButtons[5].IsButtonSelected();
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        return m_powerButtons[6].IsButtonSelected();
    default:
        return 0;
    }
}

void UIScene_BeaconMenu::SetPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl_BeaconEffectButton* button;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        button = m_powerButtons;
        button[0].SetButtonSelected(false);
        button[1].SetButtonSelected(false);
        button[2].SetButtonSelected(false);
        button[3].SetButtonSelected(false);
        button[4].SetButtonSelected(false);
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        m_powerButtons[5].SetButtonSelected(false);
        m_powerButtons[6].SetButtonSelected(false);
        break;
    default:
        break;
    }
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
        m_powerButtons[0].SetButtonSelected(true);
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
        m_powerButtons[1].SetButtonSelected(true);
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
        m_powerButtons[2].SetButtonSelected(true);
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        m_powerButtons[3].SetButtonSelected(true);
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        m_powerButtons[4].SetButtonSelected(true);
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        m_powerButtons[5].SetButtonSelected(true);
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        m_powerButtons[6].SetButtonSelected(true);
        break;
    default:
        break;
    }
}

void UIScene_BeaconMenu::AddPowerButton(int id, int icon, int tier, int column, bool active, bool selected)
{
    switch (tier)
    {
    case 0:
        if (column == 0)
        {
            m_powerButtons[0].SetData(id, icon, active, selected);
            m_powerButtons[0].setVisible(true);
        }
        else
        {
            m_powerButtons[1].SetData(id, icon, active, selected);
            m_powerButtons[1].setVisible(true);
        }
        break;
    case 1:
        if (column == 0)
        {
            m_powerButtons[2].SetData(id, icon, active, selected);
            m_powerButtons[2].setVisible(true);
        }
        else
        {
            m_powerButtons[3].SetData(id, icon, active, selected);
            m_powerButtons[3].setVisible(true);
        }
        break;
    case 2:
        m_powerButtons[4].SetData(id, icon, active, selected);
        m_powerButtons[4].setVisible(true);
        break;
    case 3:
        if (column == 0)
        {
            m_powerButtons[5].SetData(id, icon, active, selected);
            m_powerButtons[5].setVisible(true);
        }
        else
        {
            m_powerButtons[6].SetData(id, icon, active, selected);
            m_powerButtons[6].setVisible(true);
        }
        break;
    }
}

void UIScene_BeaconMenu::handleOtherClicked(int, IUIScene_AbstractContainerMenu::ESceneSection section, int, bool)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
    {
        if (IsPowerButtonSelected(section))
            return;
        int tier = GetPowerButtonTier(section);
        int effectId = MobEffect::getId(GetPowerButtonEffect(section));
        if (m_beaconContainer->getContainerData(0) <= tier)
            return;
        if (tier < 3)
        {
            m_beaconContainer->setContainerData(1, effectId);
            if (IsPowerButtonSelected(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2))
                m_beaconContainer->setContainerData(2, effectId);
        }
        else
        {
            m_beaconContainer->setContainerData(2, effectId);
        }
        SetPowerButtonSelected(section);
        if (m_beaconContainer->getContainerData(1) > 0)
        {
            int primaryId = m_beaconContainer->getContainerData(1);
            bool active = true;
            bool selected = false;
            if (m_beaconContainer->getContainerData(0) > 3)
                selected = primaryId == m_beaconContainer->getContainerData(2);
            else
                active = false;
            int buttonId = m_nextButtonId;
            AddPowerButton(buttonId, MobEffect::byId(m_beaconContainer->getContainerData(1))->getIcon(), 3, 1, active,
                           selected);
        }
        return;
    }
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm:
    {
        int primaryId = m_beaconContainer->getContainerData(1);
        int secondaryId = m_beaconContainer->getContainerData(2);
        bool unchanged = true;
        if (!m_beaconContainer->getItem(0)->isEmpty()
            && (m_primaryEffect != primaryId || m_secondaryEffect != secondaryId))
            unchanged = false;
        if (unchanged)
            return;

        ByteArrayOutputStream bytes;
        DataOutputStream output(&bytes);
        output.writeInt(primaryId);
        output.writeInt(secondaryId);
        Minecraft::GetInstance()->getConnection(getPad())->send(
            boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(
                ServerboundCustomPayloadPacket::SET_BEACON_PACKET, bytes.toByteArray())));
        m_primaryEffect = primaryId;
        m_secondaryEffect = secondaryId;
        m_beaconContainer->setContainerData(1, primaryId);
        return;
    }
    default:
        return;
    }
}

int UIScene_BeaconMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        offset = 0x1c;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        offset = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        offset = 0;
        break;
    default:
        break;
    }
    return offset;
}

void UIScene_BeaconMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                int direction, bool wrap)
{
    int columns = getSectionColumns(section);
    int index = direction * columns + slot;
    UIControl_SlotList* list = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        list = &m_activatorSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        list = &m_hotbarList;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(index);
}

std::vector<HtmlString>* UIScene_BeaconMenu::GetSectionHoverText(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    std::vector<HtmlString>* text = 0;
    std::wstring suffix(L"");
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        if (GetPowerButtonId(IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2) == 0)
            break;
        suffix = L" II";
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
    {
        MobEffect* effect = GetPowerButtonEffect(section);
        text = new std::vector<HtmlString>;
        std::wstring description(L10N::GetString(effect->getDescriptionId(-1)));
        description += suffix;
        HtmlString line(description, eMinecraftColour_HTMLColor_White, false, false, 0);
        text->push_back(line);
    }
    break;
    default:
        break;
    }
    return text;
}

unsigned int UIScene_BeaconMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_activatorSlot.setupControl(this, panel, std::string("ActivatorSlot"));
        m_controls.push_back(&m_activatorSlot);
    }
    {
        m_activatorList.setupControl(this, panel, std::string("ActivatorList"));
        m_controls.push_back(&m_activatorList);
    }
    {
        m_powerButtons[0].setupControl(this, panel, std::string("Primary_Slot_01"));
        m_controls.push_back(&m_powerButtons[0]);
    }
    {
        m_powerButtons[1].setupControl(this, panel, std::string("Primary_Slot_02"));
        m_controls.push_back(&m_powerButtons[1]);
    }
    {
        m_powerButtons[2].setupControl(this, panel, std::string("Primary_Slot_03"));
        m_controls.push_back(&m_powerButtons[2]);
    }
    {
        m_powerButtons[3].setupControl(this, panel, std::string("Primary_Slot_04"));
        m_controls.push_back(&m_powerButtons[3]);
    }
    {
        m_powerButtons[4].setupControl(this, panel, std::string("Primary_Slot_05"));
        m_controls.push_back(&m_powerButtons[4]);
    }
    {
        m_powerButtons[5].setupControl(this, panel, std::string("Secondary_Slot_01"));
        m_controls.push_back(&m_powerButtons[5]);
    }
    {
        m_powerButtons[6].setupControl(this, panel, std::string("Secondary_Slot_02"));
        m_controls.push_back(&m_powerButtons[6]);
    }
    {
        m_confirmButton.setupControl(this, panel, std::string("ConfirmButton"));
        m_controls.push_back(&m_confirmButton);
    }
    {
        m_primaryPowerLabel.setupControl(this, panel, std::string("PrimaryPowerLabel"));
        m_controls.push_back(&m_primaryPowerLabel);
    }
    {
        m_secondaryPowerLabel.setupControl(this, panel, std::string("SecondaryPowerLabel"));
        m_controls.push_back(&m_secondaryPowerLabel);
    }
    return 1;
}

std::wstring UIScene_BeaconMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return L"BeaconMenuSplit";
    return L"BeaconMenu";
}

void UIScene_BeaconMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, 1, IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_BeaconMax, false);
    m_activatorSlot.addSlots(0, 1);
    unsigned int slotCount = m_menu->m_slots.size();
    m_activatorList.addSlots(slotCount, 4);
}

void UIScene_BeaconMenu::SetConfirmButtonEnabled(bool enabled)
{
    m_confirmButton.SetButtonActive(enabled);
}

void UIScene_BeaconMenu::handleTick()
{
    int level = m_beaconContainer->getContainerData(0);
    MobEffect* primary = MobEffect::byId(m_beaconContainer->getContainerData(1));
    MobEffect* secondary = MobEffect::byId(m_beaconContainer->getContainerData(2));
    if (level != m_beaconLevel)
    {
        m_nextButtonId = 100;
        MobEffect* effects[] = {MobEffects::MOVEMENT_SPEED, MobEffects::DIG_SPEED, MobEffects::DAMAGE_RESISTANCE,
                                MobEffects::JUMP, MobEffects::DAMAGE_BOOST};
        int tiers[] = {0, 0, 1, 1, 2};
        int columns[] = {0, 1, 0, 1, 0};
        int index;
        for (index = 0; index < 5; ++index)
        {
            bool active = tiers[index] < level;
            bool selected = active && effects[index] == primary;
            if (selected && m_primaryEffect == -1)
                m_primaryEffect = m_beaconContainer->getContainerData(1);
            AddPowerButton(m_nextButtonId++, effects[index]->getIcon(), tiers[index], columns[index], active, selected);
        }
        bool selected = level >= 4 && MobEffects::REGENERATION == secondary;
        if (selected && m_secondaryEffect == -1)
            m_secondaryEffect = m_beaconContainer->getContainerData(2);
        AddPowerButton(m_nextButtonId++, MobEffects::REGENERATION->getIcon(), 3, 0, level >= 4, selected);
        if (primary != 0)
        {
            selected = level >= 4 && primary == secondary;
            if (selected && m_secondaryEffect == -1)
                m_secondaryEffect = m_beaconContainer->getContainerData(2);
            AddPowerButton(m_nextButtonId++, primary->getIcon(), 3, 1, level >= 4, selected);
        }
    }
    m_beaconLevel = level;
    int primaryId = m_beaconContainer->getContainerData(1);
    int secondaryId = m_beaconContainer->getContainerData(2);
    bool enabled = false;
    if (!m_beaconContainer->getItem(0)->isEmpty() && (m_primaryEffect != primaryId || m_secondaryEffect != secondaryId))
        enabled = true;
    SetConfirmButtonEnabled(enabled);
}

void UIScene_BeaconMenu::tick()
{
    UIScene_AbstractContainerMenu::tick();
    handleTick();
}

UIControl* UIScene_BeaconMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconActivator:
        return &m_activatorSlot;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconInventory:
        return &m_inventoryList;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconHotbar:
        return &m_hotbarList;
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary1:
        return &m_powerButtons[0];
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary2:
        return &m_powerButtons[1];
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary3:
        return &m_powerButtons[2];
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary4:
        return &m_powerButtons[3];
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconPrimary5:
        return &m_powerButtons[4];
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary1:
        return &m_powerButtons[5];
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconSecondary2:
        return &m_powerButtons[6];
    case IUIScene_AbstractContainerMenu::ESceneSection_BeaconConfirm:
        control = &m_confirmButton;
    }
    return control;
}

void UIScene_BeaconMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == NULL || minecraft->mGameModes[m_playerId] == NULL)
        return;

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int slotId = -1;
    std::sscanf(name, "slot_%d", &slotId);
    if (slotId >= 0 && static_cast<unsigned int>(slotId) >= m_menu->m_slots.size())
    {
        switch (slotId - m_menu->m_slots.size())
        {
        case 0:
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::emerald));
            break;
        case 1:
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::diamond));
            break;
        case 2:
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::goldIngot));
            break;
        case 3:
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::ironIngot));
            break;
        }
        if (!item->isEmpty())
            customDrawSlotControlFui(rect, m_playerId, item, 1.0f, item->getItem()->isFoil(item), true, false);
    }
    else
    {
        UIScene_AbstractContainerMenu::customDraw(name, rect);
    }
}

void UIScene_BeaconMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_BeaconMenu::getSceneType()
{
    return EUIScene_BeaconMenu;
}

unsigned int UIScene_BeaconMenu::GetBaseSlotCount()
{
    return 4;
}
