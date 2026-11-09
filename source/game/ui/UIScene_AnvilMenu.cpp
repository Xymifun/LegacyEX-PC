#include "game/ui/UIScene_AnvilMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>

#include "game/client/render/Font.hpp"
#include "game/core/CensoredString.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/item/Item.hpp"
#include "game/core/Slot.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/FJ_AnvilMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "platform/wiiu/CInput.hpp"

#include <wchar.h>

extern CInput* g_pInput;
extern ConsoleUIController gConsoleUIController;

struct RepairingMenuInitData
{
    boost::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int player;
    bool splitscreen;
};

UIScene_AnvilMenu::UIScene_AnvilMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer),
      m_nameUpdateAction(this, &UIScene_AnvilMenu::updateItemName)
{
    RepairingMenuInitData* sceneData = static_cast<RepairingMenuInitData*>(data);
    m_inventory = boost::shared_ptr<Inventory>();
    m_anvilMenu = 0;
    m_itemName = boost::shared_ptr<CensoredString>(
        new CensoredString(&m_nameUpdateAction, IDS_STRINGVERIFY_AWAITING_APPROVAL, IDS_STRINGVERIFY_CENSORED));
    m_pendingName = 0;
    initialiseMovie();
    m_crossVisible = false;
    m_editName.init(UIString(m_itemName->get()), 0);
    m_editName.SetCharLimit(0x1e);
    m_anvilLabel.init(UIString(L10N::GetString(IDS_REPAIR_AND_NAME)));
    m_inventory = sceneData->inventory;

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[type] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[type]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_AnvilMenu, this);
        }
    }

    m_anvilMenu = new AnvilMenu(sceneData->inventory, sceneData->level,
                                BlockPos(sceneData->x, sceneData->y, sceneData->z), minecraft->mLocalPlayers[type]);
    m_anvilMenu->addSlotListener(this);

    Initialize(false, type, m_anvilMenu, true, 3, IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_AnvilMax, false);
    m_ingredientSlot.addSlots(0, 1);
    m_ingredientSlot2.addSlots(1, 1);
    m_resultSlot.addSlots(2, 1);
    bool costLabelSafe = false;
    std::wstring costLabel(L"");
    if (m_anvilMenu->mRepairCost > 0)
    {
        bool showCost = true;
        if (m_anvilMenu->mRepairCost >= 40)
        {
            if (!minecraft->mLocalPlayers[type]->mAbilities.mInstabuild)
            {
                costLabel = L10N::GetString(IDS_REPAIR_EXPENSIVE);
                costLabelSafe = true;
                showCost = false;
            }
        }
        if (showCost && m_anvilMenu->getSlot(2)->hasItem())
        {
            wchar_t label[256];
            swprintf(label, 0x100, L10N::GetString(IDS_REPAIR_COST), m_anvilMenu->mRepairCost);
            costLabel = label;
            if (!m_anvilMenu->getSlot(2)->mayPickup(
                    boost::dynamic_pointer_cast<Player>(m_inventory->m_player->shared_from_this())))
                costLabelSafe = true;
        }
    }
    setCostLabel(costLabel, costLabelSafe);

    delete sceneData;
    m_ignoreInput = false;
    gApp.SetRichPresenceContext(type, 0xc);
}

bool UIScene_AnvilMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        return true;
    default:
        return false;
    }
}

int UIScene_AnvilMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char columns[] = {9, 9, 1, 1, 1};
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        return columns[section - IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar];
    default:
        return 0;
    }
}

int UIScene_AnvilMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        return 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        return GetInventoryRows();
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        return 1;
    default:
        return 0;
    }
}

unsigned int UIScene_AnvilMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                             IUIScene_AbstractContainerMenu::ETapState state, int* x,
                                                             int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int horizontalOffset = 0;

    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ? IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar
                                           : IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory;
            horizontalOffset = 3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ? IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar
                                           : IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory;
            horizontalOffset = -4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ? IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar
                                           : IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory;
            horizontalOffset = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilName:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar;
            horizontalOffset = -4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x <= 3)
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1;
            else if (*x <= 4)
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_smallInventory != 0)
            {
                if (*x <= 3)
                    target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1;
                else if (*x <= 4)
                    target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2;
                else
                    target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult;
            }
            else
            {
                target = IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory;
            }
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = *x <= 3 ? IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1
                             : IUIScene_AbstractContainerMenu::ESceneSection_AnvilName;
        }
        break;
    }

    updateSlotPosition(section, target, state, x, y, horizontalOffset, 0);
    return target;
}

void UIScene_AnvilMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
        position->x = m_ingredientSlot.getXPos();
        position->y = m_ingredientSlot.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
        position->x = m_ingredientSlot2.getXPos();
        position->y = m_ingredientSlot2.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        position->x = m_resultSlot.getXPos();
        position->y = m_resultSlot.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilName:
        position->x = m_editName.getXPos();
        position->y = m_editName.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        position->x = m_inventoryList.getXPos();
        position->y = m_inventoryList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        position->x = m_hotbarList.getXPos();
        position->y = m_hotbarList.getYPos();
        break;
    default:
        break;
    }
}

void UIScene_AnvilMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                          _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D itemSize;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
        itemSize.x = m_ingredientSlot.getWidth();
        itemSize.y = m_ingredientSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
        itemSize.x = m_ingredientSlot2.getWidth();
        itemSize.y = m_ingredientSlot2.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        itemSize.x = m_resultSlot.getWidth();
        itemSize.y = m_resultSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilName:
        itemSize.x = m_editName.getWidth();
        itemSize.y = m_editName.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        itemSize.x = m_inventoryList.getWidth();
        itemSize.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        itemSize.x = m_hotbarList.getWidth();
        itemSize.y = m_hotbarList.getHeight();
        break;
    default:
        break;
    }
    if (IsSectionSlotList(section))
    {
        int rows = getSectionRows(section);
        int columns = getSectionColumns(section);
        float cellHeight = itemSize.y / static_cast<float>(rows);
        float cellWidth = itemSize.x / static_cast<float>(columns);
        size->y = cellHeight;
        size->x = cellWidth;
        position->x = static_cast<float>(slot % columns) * cellWidth;
        position->y = static_cast<float>(slot / columns) * cellHeight;
        return;
    }
    GetPositionOfSection(section, position);
    size->x = itemSize.x;
    size->y = itemSize.y;
}

void UIScene_AnvilMenu::addPendingNameChange(const PendingNameInfo& info)
{
    delete m_pendingName;
    m_pendingName = new PendingNameInfo(info);
}

int UIScene_AnvilMenu::KeyboardCompleteCallback(void* data, bool accepted)
{
    unsigned int callbackId = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(data));
    UIScene_AnvilMenu* scene = static_cast<UIScene_AnvilMenu*>(gConsoleUIController.GetSceneFromCallbackId(callbackId));
    if (scene == 0)
        return 0;

    scene->m_ignoreInput = false;
    if (!accepted)
        return 0;

    PendingNameInfo info;
    info.itemName = Minecraft::GetInstance()->mLocalPlayers[scene->m_playerId]->getUUID();

    wchar_t buffer[128];
    std::memset(buffer, 0, sizeof(buffer));
    g_pInput->GetText(buffer, 0x7f);
    info.name = buffer;

    scene->addPendingNameChange(info);
    return 0;
}

void UIScene_AnvilMenu::handleEditNamePressed()
{
    m_ignoreInput = true;
    not_null_ptr<ItemInstance> item = m_anvilMenu->getSlot(0)->getItem();
    if (item->getItem() != Items::nameTag && item->getItem() != Items::spawnEgg)
    {
        g_pInput->RequestKeyboard(L10N::GetString(IDS_TITLE_RENAME), m_editName.getLabel(), m_playerId, 0x1e,
                                  KeyboardCompleteCallback, reinterpret_cast<void*>(GetCallbackUniqueId()),
                                  C_4JInput::EKeyboardMode_0);
    }
    else
    {
        std::wstring sanitized = Minecraft::GetInstance()->mFont->sanitize(std::wstring(m_editName.getLabel()), true);
        g_pInput->RequestKeyboard(L10N::GetString(IDS_TITLE_RENAME), sanitized.c_str(), m_playerId, 0x1e,
                                  KeyboardCompleteCallback, reinterpret_cast<void*>(GetCallbackUniqueId()),
                                  static_cast<C_4JInput::EKeyboardMode>(3));
    }
    m_ignoreInput = false;
}

void UIScene_AnvilMenu::handleOtherClicked(int, IUIScene_AbstractContainerMenu::ESceneSection section, int, bool)
{
    if (section != IUIScene_AbstractContainerMenu::ESceneSection_AnvilName)
        return;
    handleEditNamePressed();
}

int UIScene_AnvilMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
        return 1;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        return 2;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        return 3;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        return 0x1e;
    default:
        return 0;
    }
}

void UIScene_AnvilMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                               int direction, bool wrap)
{
    const int columns = getSectionColumns(section);
    const int target = direction * columns + slot;
    UIControl_SlotList* list = static_cast<UIControl_SlotList*>(0);
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
        list = &m_ingredientSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
        list = &m_ingredientSlot2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        list = &m_resultSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        list = &m_hotbarList;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(target);
}

UIScene_AnvilMenu::~UIScene_AnvilMenu()
{
    delete m_pendingName;
    m_pendingName = 0;
}

void UIScene_AnvilMenu::refreshContainer(AbstractContainerMenu* container, std::vector<not_null_ptr<ItemInstance> >*)
{
    slotChanged(container, 0, container->getSlot(0)->getItem());
}

void UIScene_AnvilMenu::slotChanged(AbstractContainerMenu*, int slot, not_null_ptr<ItemInstance> item)
{
    if (slot == 0)
    {
        if (!item->isEmpty())
        {
            m_itemName->set(item->getHoverNameUnfiltered(), item->getOwnerUUID(), item->wasCreatedByRestrictedPlayer(),
                            item->wasCreatedOnHost());
        }
        else
        {
            m_itemName->set(std::wstring(L""), std::wstring(L""), false, true);
        }
    }
}

void UIScene_AnvilMenu::setContainerData(AbstractContainerMenu*, int, int)
{
}

void UIScene_AnvilMenu::setAllContainerData(AbstractContainerMenu*, boost::shared_ptr<Container>)
{
}

void UIScene_AnvilMenu::setCostLabel(const std::wstring& label, bool safe)
{
    static_cast<FJ_AnvilMenu*>(m_movie->getRootNode()->getFuiNode())->SetCostLabel(label, safe);
}

unsigned int UIScene_AnvilMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_ingredientSlot.setupControl(this, panel, std::string("Ingredient"));
        m_controls.push_back(&m_ingredientSlot);
    }
    {
        m_ingredientSlot2.setupControl(this, panel, std::string("Ingredient2"));
        m_controls.push_back(&m_ingredientSlot2);
    }
    {
        m_resultSlot.setupControl(this, panel, std::string("Result"));
        m_controls.push_back(&m_resultSlot);
    }
    {
        m_anvilLabel.setupControl(this, panel, std::string("AnvilText"));
        m_controls.push_back(&m_anvilLabel);
    }
    {
        m_editName.setupControl(this, panel, std::string("AnvilTextInput"));
        m_controls.push_back(&m_editName);
    }
    return 1;
}

std::wstring UIScene_AnvilMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"AnvilMenuSplit") : std::wstring(L"AnvilMenu");
}

void UIScene_AnvilMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, 3, IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_AnvilMax, false);
    m_ingredientSlot.addSlots(0, 1);
    m_ingredientSlot2.addSlots(1, 1);
    m_resultSlot.addSlots(2, 1);
}

void UIScene_AnvilMenu::showCross(bool show)
{
    if (m_crossVisible != show)
    {
        static_cast<FJ_AnvilMenu*>(m_movie->getRootNode()->getFuiNode())->ShowRedCross(show);
        m_crossVisible = show;
    }
}

void UIScene_AnvilMenu::handleTick()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    bool safe = true;
    std::wstring label = L"";

    if (m_pendingName != 0)
    {
        m_itemName->set(m_pendingName->name, m_pendingName->itemName, gApp.IsConsoleChatCensored(),
                        CGameNetworkManager::sInstance.IsHost());
        delete m_pendingName;
        m_pendingName = 0;
    }

    if (m_anvilMenu->mRepairCost > 0)
    {
        bool showCost = true;
        if (m_anvilMenu->mRepairCost >= 40)
        {
            int pad = getPad();
            if (!minecraft->mLocalPlayers[pad]->mAbilities.mInstabuild)
            {
                label = L10N::GetString(IDS_REPAIR_EXPENSIVE);
                safe = false;
                showCost = false;
            }
        }
        if (showCost && m_anvilMenu->getSlot(2)->hasItem())
        {
            wchar_t buffer[256];
            swprintf(buffer, 0x100, L10N::GetString(IDS_REPAIR_COST), m_anvilMenu->mRepairCost);
            label = buffer;
            if (!m_anvilMenu->getSlot(2)->mayPickup(
                    boost::dynamic_pointer_cast<Player>(m_inventory->m_player->shared_from_this())))
                safe = false;
        }
    }

    setCostLabel(label, safe);

    bool crossVisible = false;
    if (m_anvilMenu->getSlot(0)->hasItem() || m_anvilMenu->getSlot(1)->hasItem())
    {
        if (!m_anvilMenu->getSlot(2)->hasItem())
            crossVisible = true;
    }
    showCross(crossVisible);
}

void UIScene_AnvilMenu::tick()
{
    UIScene_AbstractContainerMenu::tick();
    handleTick();
}

UIControl* UIScene_AnvilMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem1:
        return &m_ingredientSlot;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilItem2:
        return &m_ingredientSlot2;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilResult:
        return &m_resultSlot;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilName:
        return &m_editName;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilInventory:
        return &m_inventoryList;
    case IUIScene_AbstractContainerMenu::ESceneSection_AnvilHotbar:
        return &m_hotbarList;
    default:
        return 0;
    }
}

void UIScene_AnvilMenu::setEditNameValue(const std::wstring& value)
{
    m_editName.setLabel(UIString(value), false, false);
}

void UIScene_AnvilMenu::handleDestroy()
{
    g_pInput->DestroyKeyboard();
    UIScene_AbstractContainerMenu::handleDestroy();
}

void UIScene_AnvilMenu::updateItemName()
{
    Slot* slot = m_anvilMenu->getSlot(0);
    bool censored = gApp.IsConsoleChatCensored();
    std::wstring name = m_itemName->get();
    not_null_ptr<ItemInstance> item = slot->getItem();
    if (m_itemName->checkParentalFilter() == 0 && item.get() != NULL)
        name = item->getItem()->getName(item);
    setEditNameValue(name);
    m_anvilMenu->setItemName(m_itemName->raw(), censored);
    if (slot->hasItem())
    {
        ByteArrayOutputStream bytes;
        DataOutputStream output(&bytes);
        output.writeUTF(m_itemName->raw());
        std::wstring channel = censored ? ServerboundCustomPayloadPacket::SET_RENAME_ITEM_PACKET
                                        : ServerboundCustomPayloadPacket::SET_ITEM_NAME_PACKET;
        Minecraft::GetInstance()->getConnection(getPad())->send(boost::shared_ptr<ServerboundCustomPayloadPacket>(
            new ServerboundCustomPayloadPacket(channel, bytes.toByteArray())));
    }
}

void UIScene_AnvilMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_AnvilMenu::getSceneType()
{
    return EUIScene_AnvilMenu;
}
