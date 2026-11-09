#include "game/ui/IUIScene_AbstractContainerMenu.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/render/Renderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Slot.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ArmorRecipes.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionBrewing.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/util/D3DXVECTOR3.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockItem.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/Rarity.hpp"
#include "platform/wiiu/CInput.hpp"

static const int s_toolTipStringIds[] = {IDS_TOOLTIPS_PICKUPPLACE,
                                         (int)IDS_TOOLTIPS_EXIT,
                                         IDS_TOOLTIPS_PICKUP_GENERIC,
                                         0x1d6a6d67,
                                         0x23a35725,
                                         (int)0x987f8d08,
                                         (int)IDS_TOOLTIPS_PLACE_ONE,
                                         (int)IDS_TOOLTIPS_DISTRIBUTE,
                                         (int)IDS_TOOLTIPS_PLACE_ALL,
                                         (int)0x92835202,
                                         (int)IDS_TOOLTIPS_DROP_ONE,
                                         (int)IDS_TOOLTIPS_DROP_ALL,
                                         (int)IDS_TOOLTIPS_SWAP,
                                         IDS_TOOLTIPS_QUICK_MOVE,
                                         (int)IDS_TOOLTIPS_QUICK_MOVE_INGREDIENT,
                                         IDS_TOOLTIPS_QUICK_MOVE_FUEL,
                                         0x2fccdffb,
                                         (int)IDS_TOOLTIPS_EQUIP,
                                         (int)IDS_TOOLTIPS_CLEAR_QUICK_SELECT,
                                         IDS_TOOLTIPS_QUICK_MOVE_TOOL,
                                         (int)IDS_TOOLTIPS_QUICK_MOVE_ARMOR,
                                         (int)IDS_TOOLTIPS_QUICK_MOVE_WEAPON,
                                         IDS_TOOLTIPS_DYE,
                                         IDS_TOOLTIPS_REPAIR,
                                         IDS_TOOLTIPS_TAKE_EVERYTHING,
                                         (int)IDS_TOOLTIPS_SELECT};

IUIScene_AbstractContainerMenu::IUIScene_AbstractContainerMenu()
{
    m_menu = NULL;
    m_autoDeleteMenu = false;
    m_pointerLabelSlot = NULL;
    m_pointerPos.x = 0.0f;
    m_previousTutorialState = eTutorial_State_Gameplay;
    m_takeAllEnabled = false;
    m_pointerPos.y = 0.0f;
    m_smallInventory = false;
    m_quickCraftDisabled = false;
}

IUIScene_AbstractContainerMenu::~IUIScene_AbstractContainerMenu()
{
    if (m_autoDeleteMenu)
        delete m_menu;
}

bool IUIScene_AbstractContainerMenu::canTakeAll() const
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return m_takeAllEnabled && Minecraft::InMiniGame(eMiniGameId_Any, true) && miniGame->IsTakeEverythingEnabled()
           && CGameNetworkManager::sInstance.IsInActiveCoopOrCompetitive(-1);
}

bool IUIScene_AbstractContainerMenu::canQuickCraft() const
{
    return !m_quickCraftDisabled && !Minecraft::InMiniGame(eMiniGameId_Any, true);
}

int IUIScene_AbstractContainerMenu::GetSectionDimensions(ESceneSection section, int* columns, int* rows)
{
    int count;
    if (IsSectionSlotList(section))
    {
        *rows = getSectionRows(section);
        *columns = getSectionColumns(section);
        count = *rows * *columns;
    }
    else
    {
        count = 0;
        *rows = 0;
        *columns = 0;
    }
    return count;
}

void IUIScene_AbstractContainerMenu::updateSlotPosition(ESceneSection section, ESceneSection newSection,
                                                        ETapState tapState, int* slotX, int* slotY, int xOffset,
                                                        int yOffset)
{
    int columns;
    int rows;
    GetSectionDimensions(newSection, &columns, &rows);

    if (newSection != section)
    {
        int baseY;
        if (tapState == ETapState_Up)
            baseY = rows - 1;
        else if (tapState == ETapState_Down)
            baseY = 0;
        else
            baseY = *slotY;
        int newY = baseY - yOffset;

        if (newY < 0)
            *slotY = 0;
        else if (newY >= rows)
            *slotY = rows - 1;
        else
            *slotY = newY;

        int newX = *slotX - xOffset;
        if (newX < 0)
        {
            *slotX = 0;
        }
        else
        {
            if (newX >= columns)
                newX = columns - 1;
            *slotX = newX;
        }
    }
    else
    {
        int newX = *slotX - xOffset;
        if (newX < 0)
            *slotX = columns - 1;
        else if (newX < columns)
            *slotX = newX;
        else
            *slotX = 0;
    }
}

IUIScene_AbstractContainerMenu::ETapState IUIScene_AbstractContainerMenu::GetTapInputType(float inputX, float inputY)
{
    if (std::fabs(inputX) < 0.3f && std::fabs(inputY) < 0.3f)
        return ETapState_None;
    if (inputX < -0.3f && std::fabs(inputY) < 0.3f)
        return ETapState_Left;
    if (inputX > 0.3f && std::fabs(inputY) < 0.3f)
        return ETapState_Right;
    if (inputY < -0.3f && std::fabs(inputX) < 0.3f)
        return ETapState_Down;
    if (inputY > 0.3f && std::fabs(inputX) < 0.3f)
        return ETapState_Up;
    return ETapState_Cancelled;
}

void IUIScene_AbstractContainerMenu::UpdateTooltips()
{
    unsigned char pad = getPad();
    for (int button = 0; button < eToolTipButton_Count; ++button)
    {
        if (m_toolTips[button] == eToolTipNone)
        {
            gConsoleUIController.ShowTooltip(pad, button, false);
        }
        else
        {
            gConsoleUIController.SetTooltipText(pad, button, s_toolTipStringIds[m_toolTips[button]]);
            gConsoleUIController.ShowTooltip(pad, button, true);
        }
    }
}

void IUIScene_AbstractContainerMenu::SetToolTip(EToolTipButton button, EToolTipItem item)
{
    if (m_toolTips[button] != item)
    {
        m_toolTips[button] = item;
        UpdateTooltips();
    }
}

void IUIScene_AbstractContainerMenu::Initialize(bool navigateBack, int pad, AbstractContainerMenu* menu,
                                                bool autoDeleteMenu, int startIndex, ESceneSection firstSection,
                                                ESceneSection maxSection, bool navigateBackOnClose)
{
    m_menu = menu;
    m_autoDeleteMenu = autoDeleteMenu;

    bool smallInventory = false;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true)
        && Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled())
    {
        smallInventory = true;
    }
    m_smallInventory = smallInventory;

#ifdef MINECRAFT_PC
    Minecraft::GetInstance()->mLocalPlayers[pad]->PcCloseHiddenMenu();
#endif
    Minecraft::GetInstance()->mLocalPlayers[pad]->mContainerMenu = menu;
    m_menu->m_uiQuickCraftActive = false;
    m_menu->m_uiQuickCraftSlots.clear();

    for (int button = 0; button < eToolTipButton_Count; ++button)
        m_toolTips[button] = eToolTipNone;

    SetToolTip((EToolTipButton)1, eToolTipExit);
    SetToolTip((EToolTipButton)0, eToolTipNone);
    SetToolTip((EToolTipButton)2, eToolTipNone);
    SetToolTip((EToolTipButton)3, eToolTipNone);

    m_navigateBack = navigateBackOnClose;
    m_consecutiveInputTicks = 0;
    m_currentSection = firstSection;
    m_firstSection = firstSection;
    m_pointerOutsideMenu = false;
    m_currentSlotY = 0;
    m_tapState = ETapState_None;
    m_currentSlotX = 0;
    m_maxSection = maxSection;

    PlatformInitialize(navigateBack, pad, startIndex);
}

std::vector<HtmlString>* IUIScene_AbstractContainerMenu::GetItemDescription(Slot* slot)
{
    if (slot == NULL)
        return NULL;

    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];
    std::vector<HtmlString>* description = slot->getItem()->getTooltipLines(player, false);
    if (!description->empty())
    {
        description->at(0).mColour = slot->getItem()->getRarity()->colour;
        if (slot->getItem()->hasCustomHoverName())
            description->at(0).mColour = eMinecraftColour_Color_RenamedItemTitle;
    }
    return description;
}

bool IUIScene_AbstractContainerMenu::IsSameItemAs(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second)
{
    if (first->isEmpty() || second->isEmpty())
        return false;

    return first->getItem() == second->getItem()
           && (!second->isStackedByData() || second->getAuxValue() == first->getAuxValue())
           && ItemInstance::tagMatches(second, first);
}

int IUIScene_AbstractContainerMenu::GetEmptyStackSpace(Slot* slot)
{
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        if (item->isStackable())
        {
            int count = item->getCount();
            int space = std::min(item->getMaxStackSize(), slot->getMaxStackSize()) - count;
            return std::max(space, 0);
        }
    }
    return 0;
}

void IUIScene_AbstractContainerMenu::recalculateQuickCraftRemaining()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];
    not_null_ptr<ItemInstance> carried = ItemInstance::EMPTY;
    if (player.get() != NULL)
        carried = player->mInventory->getCarried();

    if (carried->isEmpty() || !m_menu->m_uiQuickCraftActive)
        return;

    m_menu->m_uiQuickCraftRemaining = carried->getCount();

    AbstractContainerMenu::SlotSet::iterator it = m_menu->m_uiQuickCraftSlots.begin();
    while (it != m_menu->m_uiQuickCraftSlots.end())
    {
        Slot* slot = *it;
        if (!slot->getItem()->isEmpty() && !IsSameItemAs(slot->getItem(), carried))
            it = m_menu->m_uiQuickCraftSlots.erase(it);
        else
            ++it;
    }

    for (it = m_menu->m_uiQuickCraftSlots.begin(); it != m_menu->m_uiQuickCraftSlots.end(); ++it)
    {
        Slot* slot = *it;
        not_null_ptr<ItemInstance> item = carried->copy();
        int existingCount = slot->getItem()->isEmpty() ? 0 : slot->getItem()->getCount();
        AbstractContainerMenu::getQuickCraftSlotCount(&m_menu->m_uiQuickCraftSlots, 0, item, existingCount);
        if (item->getCount() > item->getMaxStackSize())
            item->setCount(item->getMaxStackSize());
        if (item->getCount() > slot->getMaxStackSize(item))
            item->setCount(slot->getMaxStackSize(item));
        m_menu->m_uiQuickCraftRemaining -= item->getCount() - existingCount;
    }
}

#ifdef MINECRAFT_PC
bool PcInputMenuPointerActive();
int PcInputConsumeContainerHotbarKey();
int PcInputConsumeContainerWheel();
bool PcInputContainerLeftHeld();

void IUIScene_AbstractContainerMenu::PcContainerExtras()
{
    PcWheelAndDrag(PcInputConsumeContainerWheel(), PcInputContainerLeftHeld());
    const int hotbarSlot = PcInputConsumeContainerHotbarKey();
    if (hotbarSlot < 0 || !PcAllowsHotbarSwap() || m_pointerOutsideMenu || !IsSectionSlotList(m_currentSection))
        return;
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];
    if (player == NULL || !player->mInventory->getCarried()->isEmpty())
        return;
    slotClicked(getCurrentIndex(m_currentSection), hotbarSlot, ClickType::eClickType_Swap);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}
#endif

void IUIScene_AbstractContainerMenu::onMouseTick()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[getPad()] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[getPad()]->getTutorial();
        if (tutorial != NULL && gConsoleUIController.IsTutorialVisible(getPad()) && !tutorial->popupPending
            && !minecraft->mGameModes[getPad()]->getGameModeConstraints()->isInputAllowed(EControllerActions_MenuUp))
        {
            return;
        }
    }
#ifdef MINECRAFT_PC
    PcContainerExtras();
#endif

    _UIVec2D newPointerPos = m_pointerPos;
    float pointerX = m_pointerPos.x + m_pointerImageOffset.x;
    float pointerY = m_pointerPos.y + m_pointerImageOffset.y;

    int pad = getPad();
    bool pointerMoved = false;
    float inputX = (float)(g_pInput->GetJoypadStick_Menu_LX(pad)
                           * ((float)GameSettings::GetGameSettings(pad, eGameSetting_InterfaceSensitivity) / 100.0f));
    float inputY = (float)(g_pInput->GetJoypadStick_Menu_LY(pad)
                           * ((float)GameSettings::GetGameSettings(pad, eGameSetting_InterfaceSensitivity) / 100.0f));
    float xDirection = 0.0f;
    float yDirection = 0.0f;

    ESceneSection section = ESceneSection_None;
    int slotIndex = -1;
    bool onSlot = false;

    if (std::fabs(inputX) >= 0.01f || std::fabs(inputY) >= 0.01f)
    {
        if (inputX > 0.0f)
            xDirection = 1.0f;
        else if (inputX < 0.0f)
            xDirection = -1.0f;
        if (inputY > 0.0f)
            yDirection = 1.0f;
        else if (inputY < 0.0f)
            yDirection = -1.0f;

        ETapState tapState = GetTapInputType(inputX, inputY);
        if (m_tapState == ETapState_None)
            m_tapState = tapState;
        else if (m_tapState < ETapState_NoMove && tapState != m_tapState && tapState != ETapState_None)
            m_tapState = ETapState_Cancelled;

        float moveX = inputX * inputX * xDirection * 13.0f;
        float moveY = inputY * inputY * yDirection * 13.0f;
        float speed = 1.0f;
        if (m_consecutiveInputTicks < 7)
        {
            ++m_consecutiveInputTicks;
            speed = (float)m_consecutiveInputTicks / 7.0f;
        }
        else if (m_consecutiveInputTicks < 15)
        {
            ++m_consecutiveInputTicks;
        }
        else
        {
            m_tapState = ETapState_Cancelled;
        }
        if (!gRenderer->IsHiDef() || gApp.GetLocalPlayerCount() > 1)
            speed *= 0.6f;

        pointerX = moveX * speed + pointerX;
        pointerY = pointerY - moveY * speed;
        if (pointerX < m_pointerMinX)
            pointerX = m_pointerMinX;
        else if (pointerX > m_pointerMaxX)
            pointerX = m_pointerMaxX;
        if (pointerY < m_pointerMinY)
            pointerY = m_pointerMinY;
        else if (pointerY > m_pointerMaxY)
            pointerY = m_pointerMaxY;
        pointerMoved = true;
    }
    else
    {
        m_consecutiveInputTicks = 0;
    }
#ifdef MINECRAFT_PC
    if (PcInputMenuPointerActive())
    {
        pointerMoved = true;
        m_tapState = ETapState_Cancelled;
    }
#endif

    D3DXVECTOR3 snapPos;
    int slotX = -1;
    int slotY = -1;
    for (int eSection = m_firstSection; eSection < m_maxSection; ++eSection)
    {
        ESceneSection candidate = static_cast<ESceneSection>(eSection);
        if (m_tapState == ETapState_NoMove)
        {
            section = m_currentSection;
            continue;
        }
        if (section != ESceneSection_None)
            continue;

        _UIVec2D sectionPos;
        GetPositionOfSection(candidate, &sectionPos);
        if (!IsSectionSlotList(candidate))
        {
            _UIVec2D itemPos;
            _UIVec2D itemSize;
            GetItemScreenData(candidate, 0, &itemPos, &itemSize);
            if (sectionPos.x <= pointerX && pointerX <= itemSize.x + itemPos.x && sectionPos.y <= pointerY
                && pointerY <= itemSize.y + itemPos.y)
            {
                snapPos.x = itemSize.x * 0.5f + itemPos.x;
                snapPos.y = itemSize.y * 0.5f + itemPos.y;
                if (!doesSectionTreeHaveFocus(candidate))
                    setSectionFocus(candidate, getPad());
                onSlot = false;
                if (candidate != m_currentSection || slotX != m_currentSlotX || slotY != m_currentSlotY)
                    m_tapState = ETapState_Cancelled;
                m_currentSection = candidate;
                section = candidate;
            }
            continue;
        }

        int columns;
        int rows;
        int itemCount = GetSectionDimensions(candidate, &columns, &rows);
        for (int item = 0; item < itemCount; ++item)
        {
            _UIVec2D itemPos;
            _UIVec2D itemSize;
            GetItemScreenData(candidate, item, &itemPos, &itemSize);
            itemPos.x += sectionPos.x;
            itemPos.y += sectionPos.y;
            if (itemPos.x <= pointerX && pointerX <= itemSize.x + itemPos.x && itemPos.y <= pointerY
                && pointerY <= itemSize.y + itemPos.y)
            {
                slotY = item / columns;
                slotX = item - slotY * columns;
                snapPos.x = itemSize.x * 0.5f + itemPos.x;
                snapPos.y = itemSize.y * 0.5f + itemPos.y;
                if (!doesSectionTreeHaveFocus(candidate))
                    setSectionFocus(candidate, getPad());
                setSectionSelectedSlot(candidate, slotX, slotY, true);
                onSlot = true;
                if (candidate != m_currentSection || slotX != m_currentSlotX || slotY != m_currentSlotY)
                    m_tapState = ETapState_Cancelled;
                m_currentSection = candidate;
                m_currentSlotX = slotX;
                m_currentSlotY = slotY;
                slotIndex = item;
                section = candidate;
                break;
            }
        }
    }

    if (!IsVisible(section))
    {
        setFocusToPointer(getPad());
        section = ESceneSection_None;
        m_tapState = ETapState_Cancelled;
        m_currentSection = ESceneSection_None;
        m_currentSlotX = -1;
        m_currentSlotY = -1;
    }
    else if (section == ESceneSection_None)
    {
        setFocusToPointer(getPad());
        m_tapState = ETapState_Cancelled;
        m_currentSection = ESceneSection_None;
        m_currentSlotX = -1;
        m_currentSlotY = -1;
    }
    else if (!pointerMoved)
    {
        int targetSlotX;
        int targetSlotY;
        int columns;
        int rows;
        switch (m_tapState)
        {
        case ETapState_Up:
            targetSlotX = m_currentSlotX;
            targetSlotY = m_currentSlotY - 1;
            break;
        case ETapState_Down:
            targetSlotX = m_currentSlotX;
            targetSlotY = m_currentSlotY + 1;
            break;
        case ETapState_Left:
            targetSlotX = m_currentSlotX - 1;
            targetSlotY = m_currentSlotY;
            break;
        case ETapState_Right:
            targetSlotX = m_currentSlotX + 1;
            targetSlotY = m_currentSlotY;
            break;
        case ETapState_NoMove:
            targetSlotX = m_currentSlotX;
            targetSlotY = m_currentSlotY;
            break;
        default:
            targetSlotX = -1;
            targetSlotY = -1;
            break;
        }
        GetSectionDimensions(section, &columns, &rows);

        if (m_tapState != ETapState_Cancelled && m_tapState != ETapState_None
            && (!IsSectionSlotList(section) || targetSlotX < 0 || targetSlotX >= columns || targetSlotY < 0
                || targetSlotY >= rows))
        {
            section = static_cast<ESceneSection>(
                GetSectionAndSlotInDirection(section, m_tapState, &targetSlotX, &targetSlotY));
            if (!IsSectionSlotList(section))
                onSlot = false;
            GetSectionDimensions(section, &columns, &rows);
        }

        if (!IsSectionSlotList(section)
            || (targetSlotX >= 0 && targetSlotX < columns && targetSlotY >= 0 && targetSlotY < rows))
        {
            _UIVec2D sectionPos;
            _UIVec2D itemPos;
            _UIVec2D itemSize;
            GetPositionOfSection(section, &sectionPos);
            slotIndex = targetSlotY * columns + targetSlotX;
            GetItemScreenData(section, slotIndex, &itemPos, &itemSize);
            if (IsSectionSlotList(section))
            {
                itemPos.x += sectionPos.x;
                itemPos.y += sectionPos.y;
            }
            snapPos.x = itemSize.x * 0.5f + itemPos.x;
            snapPos.y = itemSize.y * 0.5f + itemPos.y;
            m_currentSection = section;
            m_currentSlotX = targetSlotX;
            m_currentSlotY = targetSlotY;
        }

        m_tapState = ETapState_None;
        if (CanHaveFocus(section))
        {
            pointerX = snapPos.x;
            pointerY = snapPos.y;
        }
    }

    if (pointerX < m_pointerMinX)
        pointerX = m_pointerMinX;
    else if (pointerX > m_pointerMaxX)
        pointerX = m_pointerMaxX;
    newPointerPos.x = pointerX;
    if (pointerY < m_pointerMinY)
        pointerY = m_pointerMinY;
    else if (pointerY > m_pointerMaxY)
        pointerY = m_pointerMaxY;
    newPointerPos.y = pointerY;

    bool pointerOutsideMenu = false;
    if (pointerX < m_panelMinX || pointerX > m_panelMaxX || pointerY < m_panelMinY || pointerY > m_panelMaxY)
        pointerOutsideMenu = true;

    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];
    not_null_ptr<ItemInstance> carried = ItemInstance::EMPTY;
    if (player.get() != NULL)
        carried = player->mInventory->getCarried();

    not_null_ptr<ItemInstance> slotItem = ItemInstance::EMPTY;
    Slot* slot = NULL;
    int slotId = 0;
    if (onSlot)
    {
        slotId = slotIndex + getSectionStartOffset(section);
        slot = m_menu->getSlot(slotId);
        if (slot == NULL)
            onSlot = false;
    }

    bool carriedIsItem = !carried->isEmpty();
    int carriedCount = 0;
    bool carriedSameAsSlot = false;
    if (carriedIsItem)
        carriedCount = carried->getCount();

    bool slotIsItem = false;
    bool canPlaceOne = false;
    bool slotItemIsWearable = false;
    bool canCombine = false;
    int slotSpace = 0;
    bool canPlaceMultiple = false;
    bool mayPlace = false;
    int slotCount = 0;
    if (onSlot)
    {
        slotItem = slot->getItem();
        slotIsItem = !slotItem->isEmpty();
        if (slotIsItem)
        {
            slotCount = slotItem->getCount();
            if (carriedIsItem)
            {
                carriedSameAsSlot = IsSameItemAs(carried, slotItem);
                canCombine = m_menu->mayCombine(slot, carried);
                slotItemIsWearable = canCombine
                                     && (dynamic_cast<ArmorItem*>(slot->getItem()->getItem()) != NULL
                                         || dynamic_cast<ShulkerBoxBlockItem*>(slot->getItem()->getItem()) != NULL);
                if (carriedSameAsSlot)
                    slotSpace = GetEmptyStackSpace(m_menu->getSlot(slotId));
            }
        }
        if (carriedIsItem)
        {
            mayPlace = slot->mayPlace(carried);
            if (slotIsItem)
                slotSpace = GetEmptyStackSpace(slot);
            else
                slotSpace = slot->getMaxStackSize();
            if (mayPlace && slotSpace > 0)
            {
                canPlaceOne = true;
                if (slotSpace > 1 && carried->getCount() > 1)
                    canPlaceMultiple = true;
            }
        }
    }

    if (onSlot && slotIsItem)
    {
        std::vector<HtmlString>* description = GetItemDescription(slot);
        SetPointerText(description, slot != m_pointerLabelSlot);
        m_pointerLabelSlot = slot;
        delete description;
    }
    else if (section != ESceneSection_None && !IsSectionSlotList(section))
    {
        std::vector<HtmlString>* description = GetSectionHoverText(section);
        SetPointerText(description, false);
        m_pointerLabelSlot = NULL;
        delete description;
    }
    else
    {
        SetPointerText(NULL, false);
        m_pointerLabelSlot = NULL;
    }

    EToolTipItem toolTip0 = eToolTipNone;
    EToolTipItem toolTip2 = eToolTipNone;
    EToolTipItem toolTip3 = eToolTipNone;
    EToolTipItem toolTip4 = eToolTipNone;
    EToolTipItem toolTip5 = eToolTipNone;
    EToolTipItem toolTip10 = eToolTipNone;
    EToolTipItem toolTip6 = eToolTipNone;
    EToolTipItem toolTip7 = eToolTipNone;

    MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[pad];
    bool tutorialActive = gameMode != NULL && gameMode->getTutorial() != NULL;

    bool describeSection = false;
    bool checkTutorial = false;
    if (onSlot)
    {
        if (!m_menu->m_uiQuickCraftActive)
        {
            if (!carriedIsItem)
            {
                if (slotIsItem)
                {
                    m_pointerOutsideMenu = false;
                    if (slotCount == 1)
                    {
                        toolTip0 = eToolTipTake;
                    }
                    else
                    {
                        toolTip0 = eToolTipTakeAll;
                        if (m_currentSection != ESceneSection_InventoryResult
                            && m_currentSection != ESceneSection_ClassicCraftingResult)
                            toolTip2 = eToolTipTakeHalf;
                    }
                    checkTutorial = true;
                }
            }
            else if (slotIsItem)
            {
                m_pointerOutsideMenu = false;
                if (!carriedSameAsSlot)
                {
                    if (mayPlace)
                        toolTip0 = eToolTipSwap;
                }
                else if (slotSpace != 0)
                {
                    if (slotSpace == 1)
                    {
                        toolTip0 = eToolTipPlace;
                    }
                    else if (canPlaceMultiple)
                    {
                        toolTip4 = eToolTipDistribute;
                        toolTip0 = eToolTipPlaceAll;
                        toolTip2 = eToolTipPlaceOne;
                    }
                    else if (canPlaceOne)
                    {
                        if (carriedCount < 2)
                        {
                            toolTip0 = eToolTipPlace;
                        }
                        else
                        {
                            toolTip4 = eToolTipDistribute;
                            toolTip0 = eToolTipPlaceOne;
                        }
                    }
                }
                if (slotItemIsWearable)
                    toolTip2 = eToolTipDye;
                else if (canCombine)
                    toolTip2 = eToolTipRepair;
                checkTutorial = true;
            }
            else if (mayPlace)
            {
                if (carriedCount != 1)
                {
                    if (canPlaceMultiple)
                    {
                        toolTip4 = eToolTipDistribute;
                        toolTip0 = eToolTipPlaceAll;
                        toolTip2 = eToolTipPlaceOne;
                    }
                    else if (canPlaceOne)
                    {
                        toolTip4 = eToolTipDistribute;
                        toolTip0 = eToolTipPlaceOne;
                    }
                }
                else
                {
                    toolTip0 = eToolTipPlace;
                }
            }
        }
        else
        {
            toolTip4 = eToolTipDistribute;
            if (slotIsItem)
            {
                m_pointerOutsideMenu = false;
                describeSection = true;
            }
        }
    }

    if (checkTutorial)
    {
        if (tutorialActive)
            toolTip5 = eToolTipWhatIsThis;
        describeSection = true;
    }

    if (describeSection)
    {
        if (section == ESceneSection_InventoryHotbar || section == ESceneSection_InventoryInventory)
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            switch (ArmorRecipes::GetArmorType(item->getItem()))
            {
            case 0:
            {
                const EquipmentSlot* equipmentSlot = Mob::getEquipmentSlotForItem(item);
                if (equipmentSlot->getType() == EquipmentSlot::ARMOR && isSlotEmpty(ESceneSection_InventoryArmor, 0))
                    toolTip3 = eToolTipEquip;
                else
                    toolTip3 = eToolTipQuickMove;
                break;
            }
            case 1:
                toolTip3 = isSlotEmpty(ESceneSection_InventoryArmor, 0) ? eToolTipEquip : eToolTipQuickMove;
                break;
            case 2:
                toolTip3 = isSlotEmpty(ESceneSection_InventoryArmor, 1) ? eToolTipEquip : eToolTipQuickMove;
                break;
            case 3:
                toolTip3 = isSlotEmpty(ESceneSection_InventoryArmor, 2) ? eToolTipEquip : eToolTipQuickMove;
                break;
            case 4:
                toolTip3 = isSlotEmpty(ESceneSection_InventoryArmor, 3) ? eToolTipEquip : eToolTipQuickMove;
                break;
            default:
                toolTip3 = eToolTipQuickMove;
                break;
            }
        }
        else if (section == ESceneSection_FurnaceHotbar || section == ESceneSection_FurnaceInventory)
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            bool isFuel = FurnaceBlockEntity::isFuel(item);
            bool canSmelt = !FurnaceRecipes::getInstance()->getResult(item)->isEmpty();
            if (canSmelt)
            {
                if (isSlotEmpty(ESceneSection_FurnaceIngredient, 0))
                {
                    toolTip3 = eToolTipQuickMoveIngredient;
                }
                else
                {
                    not_null_ptr<ItemInstance> ingredient = getSlotItem(ESceneSection_FurnaceIngredient, 0);
                    if (ingredient->getItem() == item->getItem()
                        || !FurnaceRecipes::getInstance()->getResult(item)->isEmpty())
                    {
                        toolTip3 = eToolTipQuickMoveIngredient;
                    }
                    else
                    {
                        toolTip3 = eToolTipQuickMove;
                    }
                }
            }
            else if (!isFuel)
            {
                toolTip3 = eToolTipQuickMove;
            }
            else if (!isSlotEmpty(ESceneSection_FurnaceFuel, 0))
            {
                not_null_ptr<ItemInstance> fuel = getSlotItem(ESceneSection_FurnaceFuel, 0);
                toolTip3 = fuel->getItem() == item->getItem() ? eToolTipQuickMoveFuel : eToolTipQuickMove;
            }
            else
            {
                toolTip3 = eToolTipQuickMoveFuel;
            }
        }
        else if (section == ESceneSection_BrewingHotbar || section == ESceneSection_BrewingInventory)
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            bool isIngredient = PotionBrewing::isIngredient(item);
            bool notBlazePowder = item->getItem() != Items::blazePowder;
            bool isBottle = item->getItem() == Items::potion || item->getItem() == Items::splashPotion
                            || item->getItem() == Items::lingeringPotion || item->getItem() == Items::glassBottle;
            if (isIngredient)
            {
                if (isSlotEmpty(ESceneSection_BrewingIngredient, 0))
                {
                    toolTip3 = eToolTipQuickMoveIngredient;
                }
                else if (notBlazePowder)
                {
                    toolTip3 = eToolTipQuickMove;
                }
                else if (isSlotEmpty(ESceneSection_BrewingFuel, 0))
                {
                    toolTip3 = eToolTipQuickMoveFuel;
                }
                else
                {
                    not_null_ptr<ItemInstance> fuel = getSlotItem(ESceneSection_BrewingFuel, 0);
                    toolTip3 = fuel->getCount() < fuel->getMaxStackSize() ? eToolTipQuickMoveFuel : eToolTipNone;
                }
            }
            else if (!isBottle)
            {
                toolTip3 = eToolTipQuickMove;
            }
            else if (isSlotEmpty(ESceneSection_BrewingBottle1, 0) || isSlotEmpty(ESceneSection_BrewingBottle2, 0)
                     || isSlotEmpty(ESceneSection_BrewingBottle3, 0))
            {
                toolTip3 = eToolTipQuickMoveIngredient;
            }
            else
            {
                toolTip3 = eToolTipNone;
            }
        }
        else if (section == ESceneSection_EnchantHotbar || section == ESceneSection_EnchantInventory)
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            unsigned int id = (unsigned int)Item::getId(item->getItem());
            if (!item->getItem()->isEnchantable(item) || !isSlotEmpty(ESceneSection_EnchantItem, 0))
            {
                toolTip3 = eToolTipQuickMove;
            }
            else if (id == 0x105 || id == 0x10b || id == 0x10c || id == 0x110 || id == 0x114)
            {
                toolTip3 = eToolTipQuickMoveWeapon;
            }
            else if (id >= 0x12a && id < 0x13e)
            {
                toolTip3 = eToolTipQuickMoveArmor;
            }
            else if (id == 0x154)
            {
                toolTip3 = eToolTipQuickMove;
            }
            else
            {
                toolTip3 = eToolTipQuickMoveTool;
            }
        }
        else if (dynamic_cast<ShulkerBoxMenu*>(m_menu) == NULL)
        {
            toolTip3 = eToolTipQuickMove;
        }
        else
        {
            not_null_ptr<ItemInstance> item = getSlotItem(section, slotIndex);
            if (dynamic_cast<ShulkerBoxBlockItem*>(item->getItem()) == NULL)
                toolTip3 = eToolTipQuickMove;
        }
    }

    if (pointerOutsideMenu)
    {
        m_pointerOutsideMenu = true;
        if (carriedIsItem)
        {
            if (!m_menu->m_uiQuickCraftActive)
            {
                if (carriedCount < 2)
                {
                    toolTip0 = eToolTipDrop;
                }
                else
                {
                    toolTip0 = eToolTipDropAll;
                    toolTip2 = eToolTipDropOne;
                }
            }
            else
            {
                toolTip0 = eToolTipDropAll;
            }
        }
    }
    else
    {
        m_pointerOutsideMenu = false;
    }

    not_null_ptr<ItemInstance> pointerItem = ItemInstance::EMPTY;
    if (onSlot && slotIsItem)
        pointerItem = getSlotItem(section, slotIndex);

    if (Minecraft::InMiniGame(eMiniGameId_Any, true)
        && Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled())
    {
        if (toolTip3 != eToolTipEquip && player->mContainerMenu == player->mInventoryMenu)
            toolTip3 = eToolTipNone;

        if (pointerItem.get() != NULL && pointerItem->getItem() != NULL && pointerItem->getItem()->IsArmor()
            && section != ESceneSection_ContainerHotbar)
        {
            int armorType = pointerItem->getItem()->GetArmorType();
            const EquipmentSlot* equipmentSlot = NULL;
            if (armorType == 1)
                equipmentSlot = EquipmentSlot::HEAD;
            else if (armorType == 2)
                equipmentSlot = EquipmentSlot::CHEST;
            else if (armorType == 3)
                equipmentSlot = EquipmentSlot::LEGS;
            else if (armorType == 4)
                equipmentSlot = EquipmentSlot::FEET;
            if (player->getItemSlot(equipmentSlot)->isEmpty())
                toolTip3 = eToolTipEquip;
            if (section == ESceneSection_InventoryArmor)
                toolTip3 = eToolTipQuickMoveArmor;
        }
    }

    if (canTakeAll())
        toolTip6 = eToolTipTakeEverything;

    overrideTooltips(section, pointerItem, carriedIsItem, slotIsItem, carriedSameAsSlot, slotSpace, toolTip0, toolTip2,
                     toolTip3, toolTip5, toolTip10, toolTip6, toolTip7);

    if (canTakeAll())
        toolTip5 = eToolTipNone;
    if (!canQuickCraft())
        toolTip4 = eToolTipNone;

    SetToolTip((EToolTipButton)0, toolTip0);
    SetToolTip((EToolTipButton)2, toolTip2);
    SetToolTip((EToolTipButton)3, toolTip3);
    SetToolTip((EToolTipButton)4, toolTip4);
    SetToolTip((EToolTipButton)5, toolTip5);
    SetToolTip((EToolTipButton)10, toolTip10);
    SetToolTip((EToolTipButton)6, toolTip6);
    SetToolTip((EToolTipButton)7, toolTip7);

    newPointerPos.x -= m_pointerImageOffset.x;
    newPointerPos.y -= m_pointerImageOffset.y;
    if (xDirection != 0.0f)
    {
        if (xDirection == 1.0f)
            newPointerPos.x += 0.999999f;
        else
            newPointerPos.x -= 0.999999f;
    }
    if (yDirection != 0.0f)
    {
        if (yDirection == 1.0f)
            newPointerPos.y += 0.999999f;
        else
            newPointerPos.y -= 0.999999f;
    }
    newPointerPos.x = floorf(newPointerPos.x);
    newPointerPos.x += (float)((int)newPointerPos.x % 2);
    newPointerPos.y = floorf(newPointerPos.y);
    newPointerPos.y += (float)((int)newPointerPos.y % 2);
    m_pointerPos = newPointerPos;
    adjustPointerForSafeZone();

    if (m_menu->m_uiQuickCraftActive)
    {
        bool addSlot = false;
        if (slot != NULL && m_menu->m_uiQuickCraftSlots.size() < (unsigned int)carried->getCount() && canPlaceOne
            && AbstractContainerMenu::canItemQuickReplace(
                slot, minecraft->mLocalPlayers[getPad()]->mInventory->getCarried(), true)
            && m_menu->canDragTo(slot))
        {
            addSlot = true;
        }
        if (addSlot && m_menu->m_uiQuickCraftSlots.find(slot) == m_menu->m_uiQuickCraftSlots.end())
            m_menu->m_uiQuickCraftSlots.insert(slot);
        recalculateQuickCraftRemaining();
    }
}

int IUIScene_AbstractContainerMenu::getCurrentIndex(ESceneSection section)
{
    int columns;
    int rows;
    GetSectionDimensions(section, &columns, &rows);
    return m_currentSlotY * columns + m_currentSlotX + getSectionStartOffset(section);
}

void IUIScene_AbstractContainerMenu::slotClicked(int slotId, int buttonNum, ClickType::eClickType clickType)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mGameModes[getPad()]->handleInventoryMouseClick(m_menu->m_containerId, slotId, buttonNum, clickType,
                                                               minecraft->mLocalPlayers[getPad()]);
}

void IUIScene_AbstractContainerMenu::handleQuickCraft()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[getPad()];

    not_null_ptr<ItemInstance> carried = ItemInstance::EMPTY;
    if (player.get() != NULL)
        carried = player->mInventory->getCarried();

    if (!carried->isEmpty() && carried->getCount() > 1)
    {
        m_menu->m_uiQuickCraftActive = !m_menu->m_uiQuickCraftActive;
        if (!m_menu->m_uiQuickCraftActive)
        {
            if (!m_menu->m_uiQuickCraftSlots.empty())
            {
                slotClicked(-999, AbstractContainerMenu::getQuickcraftMask(0, 0), ClickType::eClickType_QuickCraft);
                for (AbstractContainerMenu::SlotSet::iterator it = m_menu->m_uiQuickCraftSlots.begin();
                     it != m_menu->m_uiQuickCraftSlots.end(); ++it)
                {
                    Slot* slot = *it;
                    slotClicked(slot->mIndex, AbstractContainerMenu::getQuickcraftMask(1, 0),
                                ClickType::eClickType_QuickCraft);
                }
                slotClicked(-999, AbstractContainerMenu::getQuickcraftMask(2, 0), ClickType::eClickType_QuickCraft);
            }
            m_menu->m_uiQuickCraftSlots.clear();
        }
    }
}

bool IUIScene_AbstractContainerMenu::handleKeyDown(int pad, int action, bool repeat)
{
    bool handled = false;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[getPad()] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[getPad()]->getTutorial();
        if (tutorial != NULL)
        {
            tutorial->handleUIInput(action);
            if (gConsoleUIController.IsTutorialVisible(getPad()) && !tutorial->popupPending
                && !minecraft->mGameModes[getPad()]->getGameModeConstraints()->isInputAllowed(action))
            {
                return false;
            }
        }
    }

    int buttonNum = 0;
    int quickKeyHeld = 0;
    int validKeyPress = 0;
    switch (action)
    {
    case EControllerActions_MenuA:
        if (!repeat && (!m_menu->m_uiQuickCraftActive || m_pointerOutsideMenu))
        {
            m_menu->m_uiQuickCraftActive = false;
            m_menu->m_uiQuickCraftSlots.clear();
            validKeyPress = 1;
            buttonNum = 0;
            quickKeyHeld = 0;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        }
        break;
    case EControllerActions_MenuX:
        if (!repeat && !m_menu->m_uiQuickCraftActive)
        {
            validKeyPress = 1;
            buttonNum = 1;
            quickKeyHeld = 0;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        }
        break;
    case EControllerActions_MenuY:
        if (!repeat && (!m_menu->m_uiQuickCraftActive || !m_pointerOutsideMenu))
        {
            validKeyPress = 1;
            buttonNum = 0;
            quickKeyHeld = 1;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        }
        break;
    case EControllerActions_MenuLeftScroll:
        if (!repeat && canTakeAll())
        {
            handleTakeAll(0);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        }
        break;
    case EControllerActions_MenuB:
    case EControllerActions_MenuStart:
        gConsoleUIController.SetTooltips(pad, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                         false);
        if (m_navigateBack)
        {
            gConsoleUIController.NavigateBack(pad, false, EUIScene_None, EUILayer_Count);
            return false;
        }
        gConsoleUIController.CloseUIScenes(pad, false);
        return false;
    case EControllerActions_MenuLeft:
        m_tapState = ETapState_Left;
        break;
    case EControllerActions_MenuRight:
        m_tapState = ETapState_Right;
        break;
    case EControllerActions_MenuUp:
        m_tapState = ETapState_Up;
        break;
    case EControllerActions_MenuDown:
        m_tapState = ETapState_Down;
        break;
    case EControllerActions_MenuPageUp:
        if (canQuickCraft())
            handleQuickCraft();
        handled = true;
        break;
    case EControllerActions_MenuPageDown:
        if (IsSectionSlotList(m_currentSection))
        {
            int currentIndex = getCurrentIndex(m_currentSection);
            int slotIndex = currentIndex - getSectionStartOffset(m_currentSection);
            if (!isSlotEmpty(m_currentSection, slotIndex))
            {
                MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[pad];
                if (gameMode != NULL)
                {
                    Tutorial* tutorial = gameMode->getTutorial();
                    if (tutorial != NULL)
                    {
                        not_null_ptr<ItemInstance> item = getSlotItem(m_currentSection, slotIndex)->copy();
                        Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails;
                        details->titleId = item->getUseDescriptionId();
                        if (item->getItem() != NULL)
                            details->itemDescription = item->getHoverName();
                        details->itemDescriptionId = item->getDescriptionId(-1);
                        details->itemInstance = item;
                        details->itemId = Item::VALIDgetId(item->getItem());
                        details->itemAuxValue = item->getAuxValue();
                        details->force = true;
                        details->allowFade = true;
                        tutorial->setMessage(NULL, details);
                    }
                    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                }
            }
        }
        handled = true;
        break;
    default:
        break;
    }

    if (validKeyPress == 1)
    {
        if (!handleValidKeyPress(pad, buttonNum, quickKeyHeld))
        {
            if (IsSectionSlotList(m_currentSection))
            {
                handleSlotListClicked(m_currentSection, buttonNum, quickKeyHeld);
            }
            else if (m_pointerOutsideMenu)
            {
                handleOutsideClicked(pad, buttonNum, quickKeyHeld);
            }
            else
            {
                handleOtherClicked(pad, m_currentSection, buttonNum, quickKeyHeld != 0);
            }
        }
        handled = true;
    }
    else
    {
        handleAdditionalKeyPress(action);
    }
    UpdateTooltips();
    return handled;
}

unsigned int IUIScene_AbstractContainerMenu::handleValidKeyPress(int pad, int buttonNum, int quickKeyHeld)
{
    return 0;
}

void IUIScene_AbstractContainerMenu::handleOutsideClicked(int pad, int buttonNum, int quickKeyHeld)
{
    slotClicked(-999, buttonNum, (ClickType::eClickType)(quickKeyHeld != 0));
}

void IUIScene_AbstractContainerMenu::handleOtherClicked(int pad, ESceneSection section, int buttonNum, bool quickKey)
{
}

void IUIScene_AbstractContainerMenu::handleAdditionalKeyPress(int action)
{
}

void IUIScene_AbstractContainerMenu::handleSlotListClicked(ESceneSection section, int buttonNum, int quickKeyHeld)
{
    int index = getCurrentIndex(section);
    bool quick = quickKeyHeld != 0;
    slotClicked(index, buttonNum, (ClickType::eClickType)quick);
    handleSectionClick(section);
}

void IUIScene_AbstractContainerMenu::handleTakeAll(int buttonNum)
{
    int rows;
    int columns;
    GetSectionDimensions(ESceneSection_ContainerChest, &columns, &rows);
    int startIndex = getSectionStartOffset(ESceneSection_ContainerChest);
    int count = rows * columns;

    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mGameModes[getPad()]->handleTakeAllInvenetory(m_menu->m_containerId, buttonNum, startIndex, count,
                                                             minecraft->mLocalPlayers[getPad()]);
    handleSectionClick(ESceneSection_ContainerChest);
}

std::vector<HtmlString>* IUIScene_AbstractContainerMenu::GetSectionHoverText(ESceneSection section)
{
    return NULL;
}

int IUIScene_AbstractContainerMenu::GetInventoryRows()
{
    return m_smallInventory ? 0 : 3;
}

bool IUIScene_AbstractContainerMenu::IsSectionSlotList(ESceneSection section)
{
    return section != ESceneSection_None;
}

unsigned int IUIScene_AbstractContainerMenu::CanHaveFocus(ESceneSection section)
{
    return true;
}

unsigned int IUIScene_AbstractContainerMenu::IsVisible(ESceneSection section)
{
    return true;
}

unsigned int IUIScene_AbstractContainerMenu::overrideTooltips(
    ESceneSection section, not_null_ptr<ItemInstance> slotItem, bool carriedIsItem, bool slotIsItem,
    bool carriedSameAsSlot, int slotSpace, EToolTipItem& toolTip0, EToolTipItem& toolTip2, EToolTipItem& toolTip3,
    EToolTipItem& toolTip5, EToolTipItem& toolTip10, EToolTipItem& toolTip6, EToolTipItem& toolTip7)
{
    return false;
}
