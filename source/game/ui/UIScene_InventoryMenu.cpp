#include "game/ui/UIScene_InventoryMenu.hpp"
#include "game/core/StringIds.hpp"
#include <cstring>

#include <cstdio>
#include <vector>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/L10N.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/core/Slot.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/FJ_InventoryMenu.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/UILayer.hpp"

namespace
{
struct InventoryMenuSceneData
{
    boost::shared_ptr<LocalPlayer> localPlayer;
    bool creative;
    int player;
    bool splitscreen;
};
}

UIScene_InventoryMenu::UIScene_InventoryMenu(int pad, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(pad, layer),
      m_armorList(),
      m_craftingList(),
      m_resultList(),
      m_shieldList(),
      m_playerControl(),
      m_craftingLabel()
{
    InventoryMenuSceneData* sceneData = static_cast<InventoryMenuSceneData*>(data);
    initialiseMovie();
    m_effectDurations = new int[static_cast<int>(MobEffect::GetEffectCount()) + 1];
    m_classicCrafting = GameSettings::GetGameSettings(pad, eGameSetting_ClassicCrafting) != 0
                        && gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0;
    SetClassicCrafting(m_classicCrafting);

    if (Minecraft::GetInstance()->mGameModes[sceneData->player] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[sceneData->player]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_InventoryMenu, this);
        }
    }

    AbstractContainerMenu* menu = sceneData->localPlayer->mInventoryMenu;
    sceneData->localPlayer->awardStat(GenericStats::openInventory(), GenericStats::param_openInventory());
    Initialize(false, sceneData->player, menu, false, InventoryMenu::INV_SLOT_START,
               IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar,
               m_classicCrafting ? IUIScene_AbstractContainerMenu::ESceneSection_InventoryMax
                                 : IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting,
               sceneData->creative);
    m_armorList.addSlots(InventoryMenu::ARMOR_SLOT_START,
                         InventoryMenu::ARMOR_SLOT_END - InventoryMenu::ARMOR_SLOT_START);
    m_shieldList.addSlots(InventoryMenu::USE_ROW_SLOT_END, 1);
    if (m_classicCrafting)
    {
        m_craftingList.addSlots(InventoryMenu::CRAFT_SLOT_START,
                                InventoryMenu::CRAFT_SLOT_END - InventoryMenu::CRAFT_SLOT_START);
        m_resultList.addSlots(InventoryMenu::RESULT_SLOT, 1);
        m_craftingLabel.setLabel(UIString(IDS_CONTROLS_CRAFTING), false, false);
    }
    delete sceneData;

    for (int i = 0; i < static_cast<int>(MobEffect::GetEffectCount()) + 1; ++i)
        m_effectDurations[i] = 0;
    updateEffectsDisplay();
    addTimer(10, 1000);
}

UIScene_InventoryMenu::~UIScene_InventoryMenu()
{
    delete[] m_effectDurations;
}

int UIScene_InventoryMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        columns = m_classicCrafting ? 2 : 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        columns = 1;
        break;
    default:
        break;
    }
    return columns;
}

int UIScene_InventoryMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        rows = 4;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        rows = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        rows = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        rows = m_classicCrafting ? 2 : 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_InventoryMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                                 IUIScene_AbstractContainerMenu::ETapState state,
                                                                 int* x, int* y)
{
    typedef IUIScene_AbstractContainerMenu::ESceneSection ESceneSection;
    const ESceneSection hotbar = ESceneSection_InventoryHotbar;
    const ESceneSection inventory = ESceneSection_InventoryInventory;
    const ESceneSection armor = ESceneSection_InventoryArmor;
    const ESceneSection shield = ESceneSection_InventoryShield;
    const ESceneSection crafting = ESceneSection_InventoryCrafting;
    const ESceneSection result = ESceneSection_InventoryResult;

    ESceneSection target = section;
    int xOffset = 0;
    int yOffset = 0;
    bool choosePlayerColumn = false;

    switch (section)
    {
    case shield:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory ? hotbar : inventory;
            xOffset = m_classicCrafting ? -5 : -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = m_classicCrafting ? crafting : hotbar;
            xOffset = m_classicCrafting ? -5 : -8;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left
                 || state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            target = armor;
            yOffset = 3;
        }
        break;
    case armor:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = m_smallInventory ? hotbar : inventory;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = hotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = m_classicCrafting && *y <= 2 ? result : shield;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
        {
            if (m_classicCrafting && *y < 3)
            {
                target = crafting;
                xOffset = 1;
                yOffset = 1;
            }
            else
            {
                target = shield;
            }
        }
        break;
    case inventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = hotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            choosePlayerColumn = true;
        break;
    case hotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            choosePlayerColumn = true;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (!m_smallInventory)
                target = inventory;
            else
                choosePlayerColumn = true;
        }
        break;
    case crafting:
        if (m_classicCrafting)
        {
            if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            {
                target = m_smallInventory ? hotbar : inventory;
                xOffset = -5;
            }
            else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            {
                target = hotbar;
                xOffset = -5;
            }
            else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            {
                target = armor;
                yOffset = -1;
            }
            else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            {
                target = result;
            }
        }
        break;
    case result:
        if (m_classicCrafting)
        {
            if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            {
                target = m_smallInventory ? hotbar : inventory;
                xOffset = -8;
            }
            else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            {
                target = hotbar;
                xOffset = -8;
            }
            else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            {
                *x = getSectionColumns(crafting);
                target = crafting;
            }
            else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            {
                target = armor;
            }
        }
        break;
    }

    if (choosePlayerColumn)
    {
        if (m_classicCrafting)
        {
            if (*x < 3)
                target = armor;
            else if (*x > 4 && *x < 7)
            {
                target = crafting;
                xOffset = 5;
            }
            else if (*x > 4)
                target = result;
            else
                target = shield;
        }
        else
        {
            target = *x < 5 ? armor : shield;
        }
    }

    updateSlotPosition(section, target, state, x, y, xOffset, yOffset);
    return static_cast<unsigned int>(target);
}

void UIScene_InventoryMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                 _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        position->x = m_armorList.getXPos();
        position->y = m_armorList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        position->x = m_inventoryList.getXPos();
        position->y = m_inventoryList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        position->x = m_hotbarList.getXPos();
        position->y = m_hotbarList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        if (m_classicCrafting)
        {
            position->x = m_craftingList.getXPos();
            position->y = m_craftingList.getYPos();
            break;
        }
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        if (m_classicCrafting)
        {
            position->x = m_resultList.getXPos();
            position->y = m_resultList.getYPos();
            break;
        }
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        position->x = m_shieldList.getXPos();
        position->y = m_shieldList.getYPos();
        break;
    default:
        break;
    }
}

void UIScene_InventoryMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int itemIndex,
                                              _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D sectionSize;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        sectionSize.x = m_armorList.getWidth();
        sectionSize.y = m_armorList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        sectionSize.x = m_inventoryList.getWidth();
        sectionSize.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        sectionSize.x = m_hotbarList.getWidth();
        sectionSize.y = m_hotbarList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        if (m_classicCrafting)
        {
            sectionSize.x = m_craftingList.getWidth();
            sectionSize.y = m_craftingList.getHeight();
            break;
        }
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        if (m_classicCrafting)
        {
            sectionSize.x = m_resultList.getWidth();
            sectionSize.y = m_resultList.getHeight();
            break;
        }
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        sectionSize.x = m_shieldList.getWidth();
        sectionSize.y = m_shieldList.getHeight();
        break;
    default:
        break;
    }
    int rows = getSectionRows(section);
    int columns = getSectionColumns(section);
    size->x = sectionSize.x / columns;
    size->y = sectionSize.y / rows;
    int column = itemIndex % columns;
    int row = itemIndex / columns;
    position->x = column * size->x;
    position->y = row * size->y;
}

void UIScene_InventoryMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_InventoryMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        return InventoryMenu::INV_SLOT_START + 27;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        return InventoryMenu::INV_SLOT_START;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        return InventoryMenu::ARMOR_SLOT_START;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        return InventoryMenu::USE_ROW_SLOT_END;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        return m_classicCrafting ? InventoryMenu::CRAFT_SLOT_START : InventoryMenu::INV_SLOT_START;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        return m_classicCrafting ? InventoryMenu::RESULT_SLOT : InventoryMenu::ARMOR_SLOT_START;
    default:
        return 0;
    }
}

void UIScene_InventoryMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                   int direction, bool wrap)
{
    int index = slot + direction * getSectionColumns(section);
    UIControl_SlotList* list = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        list = &m_armorList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        list = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        if (m_classicCrafting)
        {
            list = &m_craftingList;
            break;
        }
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        if (m_classicCrafting)
        {
            list = &m_resultList;
            break;
        }
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        list = &m_shieldList;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(index);
}

void UIScene_InventoryMenu::SetClassicCrafting(bool classicCrafting)
{
    static_cast<FJ_InventoryMenu*>(m_movie->getRootNode()->getFuiNode())->SetClassicCrafting(classicCrafting);
    if (!classicCrafting)
    {
        m_armorList.UpdateControl();
        m_playerControl.UpdateControl();
        m_shieldList.UpdateControl();
    }
}

void UIScene_InventoryMenu::updateEffectsDisplay()
{
    boost::shared_ptr<Player> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    if (player == NULL)
        return;

    std::vector<MobEffectInstance*>* activeEffects = player->getActiveEffects();
    int valueCount = 0;
    int* effectValues = new int[activeEffects->size() * 2];
    std::vector<MobEffectInstance*>::iterator it;
    for (it = activeEffects->begin(); it != activeEffects->end(); ++it)
    {
        MobEffectInstance* effect = *it;
        if (effect->getDuration() >= m_effectDurations[MobEffect::getId(effect->getEffect())])
        {
            std::wstring effectString = L10N::GetString(effect->getDescriptionId());
            if (effect->getAmplifier() > 0)
            {
                std::wstring amplifierString = L"";
                switch (effect->getAmplifier())
                {
                case 1:
                    amplifierString = L" ";
                    amplifierString += L10N::GetString(IDS_POTION_POTENCY_1);
                    break;
                case 2:
                    amplifierString = L" ";
                    amplifierString += L10N::GetString(IDS_POTION_POTENCY_2);
                    break;
                case 3:
                    amplifierString = L" ";
                    amplifierString += L10N::GetString(IDS_POTION_POTENCY_3);
                    break;
                case 4:
                    amplifierString = L" ";
                    amplifierString += L10N::GetString(IDS_POTION_POTENCY_4);
                    break;
                case 5:
                    amplifierString = L" ";
                    amplifierString += L10N::GetString(IDS_POTION_POTENCY_5);
                    break;
                default:
                    amplifierString = L10N::GetString(IDS_POTION_POTENCY_0);
                    break;
                }
                effectString += amplifierString;
            }
            int icon = 0;
            MobEffect* type = effect->getEffect();
            if (type->hasIcon())
                icon = type->getIcon();
            int seconds = effect->getDuration() / 20;
            static_cast<FJ_InventoryMenu*>(m_movie->getRootNode()->getFuiNode())
                ->AddEffect(icon, effectString, seconds);
        }
        if (effect->getEffect()->hasIcon())
        {
            effectValues[valueCount] = effect->getEffect()->getIcon();
            effectValues[valueCount + 1] = effect->getDuration() / 20;
            valueCount += 2;
        }
        m_effectDurations[MobEffect::getId(effect->getEffect())] = effect->getDuration();
    }
    static_cast<FJ_InventoryMenu*>(m_movie->getRootNode()->getFuiNode())->UpdateEffects(valueCount, effectValues);
    delete[] effectValues;
    delete activeEffects;
}

unsigned int UIScene_InventoryMenu::getSubSceneType() const
{
    return m_classicCrafting;
}

unsigned int UIScene_InventoryMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* rootNode = m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_armorList.setupControl(this, panel, std::string("armorList"));
        m_controls.push_back(&m_armorList);
    }
    {
        m_craftingList.setupControl(this, panel, std::string("craftingList"));
        m_controls.push_back(&m_craftingList);
    }
    {
        m_resultList.setupControl(this, panel, std::string("Result"));
        m_controls.push_back(&m_resultList);
    }
    {
        m_playerControl.setupControl(this, panel, std::string("iggy_player"));
        m_controls.push_back(&m_playerControl);
    }
    {
        m_craftingLabel.setupControl(this, panel, std::string("craftingLabel"));
        m_controls.push_back(&m_craftingLabel);
    }
    {
        m_shieldList.setupControl(this, panel, std::string("shieldList"));
        m_controls.push_back(&m_shieldList);
    }
    return 1;
}

std::wstring UIScene_InventoryMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"InventoryMenuSplit");
    return std::wstring(L"InventoryMenu");
}

void UIScene_InventoryMenu::handleReload()
{
    SetClassicCrafting(m_classicCrafting);
    Initialize(true, m_playerId, m_menu, false, InventoryMenu::INV_SLOT_START,
               IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar,
               m_classicCrafting ? IUIScene_AbstractContainerMenu::ESceneSection_InventoryMax
                                 : IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting,
               m_navigateBack);
    m_armorList.addSlots(InventoryMenu::ARMOR_SLOT_START,
                         InventoryMenu::ARMOR_SLOT_END - InventoryMenu::ARMOR_SLOT_START);
    m_shieldList.addSlots(InventoryMenu::USE_ROW_SLOT_END, 1);
    if (m_classicCrafting)
    {
        m_craftingList.addSlots(InventoryMenu::CRAFT_SLOT_START,
                                InventoryMenu::CRAFT_SLOT_END - InventoryMenu::CRAFT_SLOT_START);
        m_resultList.addSlots(InventoryMenu::RESULT_SLOT, 1);
    }
    for (int i = 0; i < static_cast<int>(MobEffect::GetEffectCount()) + 1; ++i)
        m_effectDurations[i] = 0;
}

UIControl* UIScene_InventoryMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = NULL;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryArmor:
        control = &m_armorList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryHotbar:
        control = &m_hotbarList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryCrafting:
        if (m_classicCrafting)
        {
            control = &m_craftingList;
            break;
        }
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryResult:
        if (m_classicCrafting)
        {
            control = &m_resultList;
            break;
        }
    case IUIScene_AbstractContainerMenu::ESceneSection_InventoryShield:
        control = &m_shieldList;
        break;
    default:
        break;
    }
    return control;
}

void UIScene_InventoryMenu::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == NULL || minecraft->mGameModes[m_playerId] == NULL)
        return;

    if (std::strcmp(name, "player") == 0)
    {
        m_playerControl.render(rect);
        return;
    }

    UIScene_AbstractContainerMenu::customDraw(name, rect);

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int slotId = -1;
    std::sscanf(name, "slot_%d", &slotId);
    if (slotId >= InventoryMenu::ARMOR_SLOT_START && slotId < InventoryMenu::ARMOR_SLOT_END)
    {
        item = m_menu->getSlot(slotId)->getItem();
        bool empty = item->isEmpty();
        static_cast<FJ_InventoryMenu*>(m_movie->getRootNode()->getFuiNode())
            ->ShowArmorSlot(slotId - InventoryMenu::ARMOR_SLOT_START, empty);
    }
}

void UIScene_InventoryMenu::handleTimerComplete(int timerId)
{
    if (timerId == 10)
        updateEffectsDisplay();
}

int UIScene_InventoryMenu::getSceneType()
{
    return EUIScene_InventoryMenu;
}
