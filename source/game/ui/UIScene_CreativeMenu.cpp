#include "game/ui/UIScene_CreativeMenu.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModContent.hpp"
#endif
#include "game/core/VirtualPadKeys.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"

#ifdef MINECRAFT_PC
#include "platform/pc/PcDebug.hpp"
#endif
#include "game/core/BannerPattern.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/FireworksItem.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potions.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/core/Slot.hpp"
#include "game/util/Math.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/FJ_CreativeMenu.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/MenuBuilder.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/FishFoodItem.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Direction.hpp"
#include "game/core/L10N.hpp"
#include "game/world/block/LeverBlock.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/SpongeBlock.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/RedSandStoneBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/QuartzBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/NewLogBlock.hpp"
#include "game/world/block/PrismarineBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/MonsterEggBlock.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/NewStoneSlabBlock.hpp"
#include "game/world/block/WoodSlabBlock.hpp"
#include "game/world/block/PrismarineSlabBlock.hpp"
#include "game/world/block/SaplingBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/NewLeafBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"

extern ConsoleUIController gConsoleUIController;

namespace
{
struct CreativeMenuSceneData
{
    boost::shared_ptr<LocalPlayer> localPlayer;
    bool creative;
    int playerIndex;
    bool splitscreen;
};
}

UIScene_CreativeMenu::TabSpec** UIScene_CreativeMenu::s_tabSpecs = NULL;
std::vector<const UIScene_CreativeMenu::ItemBuilder*>
    UIScene_CreativeMenu::s_groupItems[ECreative_Inventory_Groups_Count];

UIScene_CreativeMenu::TabSpec::TabSpec(const wchar_t* title, int titleStringId, int groupCount,
                                       ECreative_Inventory_Groups* groups, int specialGroupCount,
                                       ECreative_Inventory_Groups* specialGroups, int extraGroupCount,
                                       ECreative_Inventory_Groups* extraGroups)
    : m_title(title),
      m_titleStringId(titleStringId),
      m_groupCount(groupCount),
      m_groups(NULL),
      m_specialGroupCount(specialGroupCount),
      m_specialGroups(NULL),
      m_extraGroupCount(extraGroupCount),
      m_extraGroups(NULL),
      m_pageCount(0),
      m_itemsPerPage(0),
      m_itemCount(0),
      m_extraItemCount(0)
{
    if (groupCount > 0)
    {
        m_groups = new ECreative_Inventory_Groups[groupCount];
        for (int i = 0; i < groupCount; ++i)
        {
            m_groups[i] = groups[i];
            m_itemCount += s_groupItems[groups[i]].size();
        }
    }

    if (extraGroupCount > 0)
    {
        m_extraGroups = new ECreative_Inventory_Groups[extraGroupCount];
        for (int i = 0; i < extraGroupCount; ++i)
        {
            m_extraGroups[i] = extraGroups[i];
            m_extraItemCount += s_groupItems[extraGroups[i]].size();
        }
    }

    unsigned int specialItemCount = 0;
    if (specialGroupCount > 0 && specialGroups != NULL)
    {
        m_specialGroups = new ECreative_Inventory_Groups[specialGroupCount];
        for (int i = 0; i < specialGroupCount; ++i)
        {
            m_specialGroups[i] = specialGroups[i];
            specialItemCount += s_groupItems[specialGroups[i]].size();
        }
    }

    m_itemsPerPage = ITEM_SLOT_COUNT - specialItemCount;
    m_pageCount = static_cast<unsigned int>(
        std::ceil(static_cast<float>(m_itemCount) / static_cast<float>(m_itemsPerPage)));
}

UIScene_CreativeMenu::TabSpec::~TabSpec()
{
    delete[] m_groups;
    delete[] m_specialGroups;
    delete[] m_extraGroups;
}

unsigned int UIScene_CreativeMenu::TabSpec::getPageCount()
{
    return m_pageCount;
}

void UIScene_CreativeMenu::TabSpec::populateMenu(AbstractContainerMenu* menu, int specialGroup, unsigned int page)
{
    int slot = 0;

    if (m_specialGroupCount > 0 && m_specialGroups != NULL)
    {
        const std::vector<const ItemBuilder*>& items = s_groupItems[m_specialGroups[specialGroup]];
        for (std::vector<const ItemBuilder*>::const_iterator it = items.end();
             it != items.begin() && slot < ITEM_SLOT_COUNT; --it)
        {
            ++slot;
            menu->getSlot(slot)->set((*(it - 1))->build());
        }
    }

    const unsigned int firstItem = page * m_itemsPerPage;
    unsigned int itemsBefore = 0;
    unsigned int itemIndex = 0;
    int group = 0;
    bool found = false;
    for (; group < m_groupCount; ++group)
    {
        const unsigned int groupSize = s_groupItems[m_groups[group]].size();
        if (firstItem <= itemsBefore + groupSize)
        {
            itemIndex = firstItem - itemsBefore;
            found = true;
            break;
        }
        itemsBefore += groupSize;
    }

    if (found)
    {
        for (; group < m_groupCount; ++group, itemIndex = 0)
        {
            const std::vector<const ItemBuilder*>& items = s_groupItems[m_groups[group]];
            if (slot >= ITEM_SLOT_COUNT)
                return;
            do
            {
                menu->getSlot(slot)->set(items[itemIndex]->build());
                ++slot;
                ++itemIndex;
                if (items.empty() || itemIndex >= items.size())
                    break;
                if (slot >= ITEM_SLOT_COUNT)
                    return;
            } while (true);
        }
    }

    for (; slot < ITEM_SLOT_COUNT; ++slot)
        menu->getSlot(slot)->remove(1);
}

UIScene_CreativeMenu::ItemPickerMenu::ItemPickerMenu(boost::shared_ptr<SimpleContainer> container,
                                                     boost::shared_ptr<Inventory> inventory)
{
    m_inventory = inventory;
    m_container = container;
    for (int i = 0; i < 50; ++i)
    {
        Slot* slot = new Slot(m_container, i, -1, -1);
        addSlot(slot);
    }
    for (int i = 0; i < 9; ++i)
    {
        Slot* slot = new Slot(m_inventory, i, -1, -1);
        addSlot(slot);
    }
    m_containerId = -2;
}

bool UIScene_CreativeMenu::ItemPickerMenu::stillValid(boost::shared_ptr<Player>)
{
    return true;
}

bool UIScene_CreativeMenu::ItemPickerMenu::isOverrideResultClick(int slot, int button)
{
    return slot >= 0 && slot < 9 && button == 0;
}

bool UIScene_CreativeMenu::ItemPickerMenu::doQuickMoveLoop()
{
    return false;
}

UIScene_CreativeMenu::UIScene_CreativeMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer)
{
#ifdef MINECRAFT_PC
    m_pcScrollDrag = false;
#endif
    initialiseMovie();

    CreativeMenuSceneData* sceneData = static_cast<CreativeMenuSceneData*>(data);
    boost::shared_ptr<SimpleContainer> container(new SimpleContainer(0, std::wstring(L""), false, 50));
    m_containerMenu = new ItemPickerMenu(container, sceneData->localPlayer->mInventory);
    Initialize(false, sceneData->playerIndex, m_containerMenu, true, -1,
               IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_CreativeMax, sceneData->creative);
    m_inventoryLabel.setLabel(UIString(L""), false, false);

    m_restoreSelectorPending = false;
    m_savedSlotX = 0;
    m_savedSlotY = 0;
    m_emptySlotX = 0;
    m_emptySlotY = 0;
    m_unreferencedFlag = true;
    m_quickCraftDisabled = true;
    for (int i = 0; i < 8; ++i)
    {
        m_tabPages[i] = 0;
        m_tabCurrentPages[i] = 0;
    }

    for (unsigned int i = 0; i < 50; ++i)
        m_containerList.addSlot(i);
    for (unsigned int i = 50; i < 59; ++i)
        m_hotbarList.addSlot(i);

    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[type];
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_CreativeInventoryMenu, this);
        }
    }

    delete sceneData;
    m_currentTab = 8;
    switchTab(ECreativeInventoryTabs_Building);
    for (int i = 0; i < 9; ++i)
        m_touchPanels[i].init(i);
}

UIScene_CreativeMenu::~UIScene_CreativeMenu()
{
}

bool UIScene_CreativeMenu::IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector:
        return true;
    default:
        return false;
    }
}

unsigned int UIScene_CreativeMenu::CanHaveFocus(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector:
        return 1;
    default:
        return 0;
    }
}

int UIScene_CreativeMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector:
        columns = 10;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar:
        columns = 9;
        break;
    }
    return columns;
}

int UIScene_CreativeMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector:
        rows = 5;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar:
        rows = 1;
        break;
    }
    return rows;
}

unsigned int UIScene_CreativeMenu::GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                                IUIScene_AbstractContainerMenu::ETapState state, int* x,
                                                                int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection next = section;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down
            || state == IUIScene_AbstractContainerMenu::ETapState_Up)
            next = IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down
            || state == IUIScene_AbstractContainerMenu::ETapState_Up)
            next = IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector;
        break;
    }
    updateSlotPosition(section, next, state, x, y, 0, 0);
    return next;
}

void UIScene_CreativeMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector:
        position->x = m_containerList.getXPos();
        position->y = m_containerList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar:
        position->x = m_hotbarList.getXPos();
        position->y = m_hotbarList.getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab0:
        position->x = m_touchPanels[0].getXPos();
        position->y = m_touchPanels[0].getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab1:
        position->x = m_touchPanels[1].getXPos();
        position->y = m_touchPanels[1].getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab2:
        position->x = m_touchPanels[2].getXPos();
        position->y = m_touchPanels[2].getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab3:
        position->x = m_touchPanels[3].getXPos();
        position->y = m_touchPanels[3].getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab4:
        position->x = m_touchPanels[4].getXPos();
        position->y = m_touchPanels[4].getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab5:
        position->x = m_touchPanels[5].getXPos();
        position->y = m_touchPanels[5].getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab6:
        position->x = m_touchPanels[6].getXPos();
        position->y = m_touchPanels[6].getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab7:
        position->x = m_touchPanels[7].getXPos();
        position->y = m_touchPanels[7].getYPos();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSlider:
        position->x = m_touchPanels[8].getXPos();
        position->y = m_touchPanels[8].getYPos();
        break;
    }
}

void UIScene_CreativeMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                             _UIVec2D* position, _UIVec2D* size)
{
    float width;
    float height;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar:
        width = m_hotbarList.getWidth();
        height = m_hotbarList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector:
        width = m_containerList.getWidth();
        height = m_containerList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab0:
        width = m_touchPanels[0].getWidth();
        height = m_touchPanels[0].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab1:
        width = m_touchPanels[1].getWidth();
        height = m_touchPanels[1].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab2:
        width = m_touchPanels[2].getWidth();
        height = m_touchPanels[2].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab3:
        width = m_touchPanels[3].getWidth();
        height = m_touchPanels[3].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab4:
        width = m_touchPanels[4].getWidth();
        height = m_touchPanels[4].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab5:
        width = m_touchPanels[5].getWidth();
        height = m_touchPanels[5].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab6:
        width = m_touchPanels[6].getWidth();
        height = m_touchPanels[6].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab7:
        width = m_touchPanels[7].getWidth();
        height = m_touchPanels[7].getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSlider:
        width = m_touchPanels[8].getWidth();
        height = m_touchPanels[8].getHeight();
        break;
    default:
        break;
    }

    if (!IsSectionSlotList(section))
    {
        GetPositionOfSection(section, position);
        size->x = width;
        size->y = height;
        return;
    }

    const int rows = getSectionRows(section);
    const int columns = getSectionColumns(section);
    size->x = width / static_cast<float>(columns);
    size->y = height / static_cast<float>(rows);
    position->x = static_cast<float>(slot % columns) * size->x;
    position->y = static_cast<float>(slot / columns) * size->y;
}

unsigned int UIScene_CreativeMenu::handleValidKeyPress(int key, int value, int)
{
    if (value == 1)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        for (int index = 50; index < 59; ++index)
        {
            not_null_ptr<ItemInstance> item = m_menu->getSlot(index)->getItem();
            if (!item->isEmpty())
            {
                m_menu->getSlot(index)->set(ItemInstance::EMPTY);
                minecraft->mGameModes[key]->handleCreativeModeItemAdd(ItemInstance::EMPTY,
                                                                      (index - static_cast<int>(m_menu->m_slots.size()))
                                                                          + Inventory::INVENTORY_SIZE + 9);
            }
        }
        return 1;
    }
    return 0;
}

void UIScene_CreativeMenu::handleOutsideClicked(int playerId, int, int button)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Inventory* inventory = minecraft->mLocalPlayers[playerId]->mInventory.get();
    not_null_ptr<ItemInstance> carried = inventory->getCarried();
    if (carried->isEmpty())
        return;

    if (button == 0)
    {
        minecraft->mGameModes[playerId]->handleCreativeModeItemDrop(inventory->getCarried());
        inventory->setCarried(ItemInstance::EMPTY);
    }
    if (button == 1)
    {
        not_null_ptr<ItemInstance> one = inventory->getCarried()->split(1);
        minecraft->mGameModes[playerId]->handleCreativeModeItemDrop(one);
    }
}

void UIScene_CreativeMenu::updateTabHighlightAndText(ECreativeInventoryTabs tab)
{
    fuiRenderNode* root = m_movie->getRootNode();
    static_cast<FJ_CreativeMenu*>(root->getFuiNode()->asFJ_Document())->SetActiveTab(static_cast<int>(tab));
    m_inventoryLabel.setLabel(UIString(L10N::GetString(s_tabSpecs[tab]->m_titleStringId)), false, false);
}

void UIScene_CreativeMenu::updateScrollCurrentPage(int page, int pageCount)
{
    fuiRenderNode* root = m_movie->getRootNode();
    static_cast<FJ_CreativeMenu*>(root->getFuiNode())->SetScrollBar(pageCount, page - 1);
}

void UIScene_CreativeMenu::switchTab(ECreativeInventoryTabs tab)
{
    if (static_cast<int>(tab) != m_currentTab)
        updateTabHighlightAndText(tab);

    TabSpec* spec = s_tabSpecs[tab];
    m_currentTab = static_cast<int>(tab);
    updateScrollCurrentPage(m_tabCurrentPages[m_currentTab] + 1, spec->getPageCount());
    spec->populateMenu(m_containerMenu, m_tabPages[m_currentTab], m_tabCurrentPages[m_currentTab]);
}

void UIScene_CreativeMenu::ScrollBar(_UIVec2D position)
{
    _UIVec2D origin = {0.0f, 0.0f};
    _UIVec2D size = {0.0f, 0.0f};
    GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection_CreativeSlider, 0, &origin, &size);

    float fraction = (position.y - origin.y) / size.y;
    if (fraction > 1.0f)
        fraction = 1.0f;
    else if (fraction < 0.0f)
        fraction = 0.0f;

    const unsigned int pageCount = s_tabSpecs[m_currentTab]->getPageCount();
    const int page = Math::round(fraction * static_cast<float>(pageCount - 1));
    m_tabCurrentPages[m_currentTab] = page;
    switchTab(static_cast<ECreativeInventoryTabs>(m_currentTab));
}

#ifdef MINECRAFT_PC
void UIScene_CreativeMenu::PcWheelAndDrag(int wheelSteps, bool leftHeld)
{
    for (; wheelSteps > 0; --wheelSteps)
        handleAdditionalKeyPress(14);
    for (; wheelSteps < 0; ++wheelSteps)
        handleAdditionalKeyPress(15);
    if (!leftHeld)
    {
        m_pcScrollDrag = false;
        return;
    }
    if (!m_pcScrollDrag || s_tabSpecs[m_currentTab]->getPageCount() < 2)
        return;
    _UIVec2D origin = {0.0f, 0.0f};
    _UIVec2D size = {0.0f, 0.0f};
    GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection_CreativeSlider, 0, &origin, &size);
    float fraction = (m_pointerPos.y - origin.y) / size.y;
    fraction = fraction < 0.0f ? 0.0f : (fraction > 1.0f ? 1.0f : fraction);
    const int page = Math::round(fraction * static_cast<float>(s_tabSpecs[m_currentTab]->getPageCount() - 1));
    if (page != m_tabCurrentPages[m_currentTab])
        ScrollBar(m_pointerPos);
}
#endif

void UIScene_CreativeMenu::handleOtherClicked(int button, IUIScene_AbstractContainerMenu::ESceneSection section, int,
                                              bool)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab0:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab1:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab2:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab3:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab4:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab5:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab6:
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab7:
        if (section - IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab0 != m_currentTab)
        {
            switchTab(static_cast<ECreativeInventoryTabs>(
                section - IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab0));
            gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSlider:
        ScrollBar(m_pointerPos);
#ifdef MINECRAFT_PC
        m_pcScrollDrag = true;
#endif
        break;
    default:
        break;
    }
}

void UIScene_CreativeMenu::handleAdditionalKeyPress(int key)
{
    int direction = 1;
    switch (key)
    {
    case 11:
        direction = -1;
    case 10:
    {
        int newTab = m_currentTab + direction;
        if (newTab >= 0)
        {
            if (newTab >= ECreativeInventoryTabs_COUNT)
                newTab = 0;
            switchTab(static_cast<ECreativeInventoryTabs>(newTab));
            gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
            return;
        }
        switchTab(ECreativeInventoryTabs_Combat);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
        return;
    }
    case 8:
    {
        const int tab = m_currentTab;
        const int nextGroup = m_tabPages[tab] + 1;
        if (nextGroup < s_tabSpecs[tab]->m_specialGroupCount)
        {
            m_tabPages[tab] = nextGroup;
            switchTab(static_cast<ECreativeInventoryTabs>(tab));
        }
        else
        {
            m_tabPages[tab] = 0;
            switchTab(static_cast<ECreativeInventoryTabs>(tab));
        }
        break;
    }
    case 15:
    {
        const int tab = m_currentTab;
        const unsigned int nextPage = static_cast<unsigned int>(m_tabCurrentPages[tab] + 1);
        m_tabCurrentPages[tab] = static_cast<int>(nextPage);
        const unsigned int pageCount = s_tabSpecs[tab]->getPageCount();
        if (nextPage < pageCount)
        {
            switchTab(static_cast<ECreativeInventoryTabs>(tab));
            break;
        }
        m_tabCurrentPages[tab] = static_cast<int>(pageCount - 1);
        return;
    }
    case 14:
    {
        const int tab = m_currentTab;
        const int previousPage = m_tabCurrentPages[tab] - 1;
        if (previousPage >= 0)
        {
            m_tabCurrentPages[tab] = previousPage;
            switchTab(static_cast<ECreativeInventoryTabs>(tab));
            break;
        }
        m_tabCurrentPages[tab] = 0;
        return;
    }
    default:
        break;
    }
}

bool UIScene_CreativeMenu::getEmptyInventorySlot(not_null_ptr<ItemInstance> item, int& slot)
{
    bool found = false;
    int menuSlot = 50;
    int relativeSlot = 0;
    for (int remaining = 9; remaining > 0; --remaining, ++menuSlot, ++relativeSlot)
    {
        not_null_ptr<ItemInstance> slotItem = m_menu->getSlot(menuSlot)->getItem();
        if (!slotItem->isEmpty() && slotItem->sameItemWithTags(item, 0)
            && slotItem->getCount() + item->getCount() <= item->getMaxStackSize())
        {
            slot = relativeSlot;
            found = true;
            break;
        }
    }

    if (!found)
    {
        int groupsRemaining = 3;
        menuSlot = 50;
        int emptySlot = menuSlot;
        while (true)
        {
            emptySlot = menuSlot;
            if (m_menu->getSlot(emptySlot)->getItem()->isEmpty())
            {
                found = true;
                break;
            }
            emptySlot = menuSlot + 1;
            if (m_menu->getSlot(emptySlot)->getItem()->isEmpty())
            {
                found = true;
                break;
            }
            emptySlot = menuSlot + 2;
            if (m_menu->getSlot(emptySlot)->getItem()->isEmpty())
            {
                found = true;
                break;
            }
            groupsRemaining--;
            menuSlot += 3;
            if (groupsRemaining == 0)
                break;
        }
        if (found)
            slot = emptySlot - 50;
    }
    return found;
}

void UIScene_CreativeMenu::handleSlotListClicked(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                                 int button)
{
    int effectiveSlot = getCurrentIndex(section);
    Minecraft* minecraft = Minecraft::GetInstance();
    int found = 0;

#ifdef MINECRAFT_PC
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector)
    {
        boost::shared_ptr<Inventory> inventory = minecraft->mLocalPlayers[getPad()]->mInventory;
        if (!inventory->getCarried()->isEmpty())
        {
            inventory->setCarried(ItemInstance::EMPTY);
            return;
        }
    }
#endif

    if (section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector && slot == 0)
    {
        boost::shared_ptr<Inventory> inventory = minecraft->mLocalPlayers[getPad()]->mInventory;
        not_null_ptr<ItemInstance> carried = inventory->getCarried();
        not_null_ptr<ItemInstance> selected = m_menu->getSlot(effectiveSlot)->getItem();
        if (!selected->isEmpty())
        {
            inventory->setCarried(selected->copy());
            carried = inventory->getCarried();
            if (button == 1)
                carried->setCount(carried->getMaxStackSize());

            m_currentSection = IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar;
            m_savedSlotX = m_currentSlotX;
            m_tapState = IUIScene_AbstractContainerMenu::ETapState_NoMove;
            m_savedSlotY = m_currentSlotY;
            found = getEmptyInventorySlot(carried, m_emptySlotX);
            m_currentSlotX = m_emptySlotX;
            m_restoreSelectorPending = true;
            m_currentSlotY = m_emptySlotY;
        }
    }

    if (found == 0 && section != IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar)
        return;

    if (found != 0)
    {
        setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar, m_currentSlotX,
                               m_currentSlotY, section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar);
        effectiveSlot = getCurrentIndex(IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar);
        slot = 0;
        button = 0;
    }

    m_menu->clicked(effectiveSlot, slot, static_cast<ClickType::eClickType>(button != 0),
                    minecraft->mLocalPlayers[getPad()]);

    not_null_ptr<ItemInstance> addedItem = m_menu->getSlot(effectiveSlot)->getItem();
    const int itemsSize = static_cast<int>(m_menu->m_slots.size());
    minecraft->mGameModes[getPad()]->handleCreativeModeItemAdd(addedItem, (effectiveSlot - itemsSize)
                                                                              + Inventory::INVENTORY_SIZE + 9);

    if (m_restoreSelectorPending)
    {
        m_emptySlotX = m_currentSlotX;
        m_tapState = IUIScene_AbstractContainerMenu::ETapState_NoMove;
        m_emptySlotY = m_currentSlotY;
        m_currentSlotX = m_savedSlotX;
        m_currentSection = IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector;
        m_currentSlotY = m_savedSlotY;
        boost::shared_ptr<Inventory> inventory = minecraft->mLocalPlayers[getPad()]->mInventory;
        inventory->setCarried(ItemInstance::EMPTY);
        m_restoreSelectorPending = false;
    }
}

int UIScene_CreativeMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector:
        offset = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar:
        offset = 50;
        break;
    }
    return offset;
}

void UIScene_CreativeMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slotX,
                                                  int slotY, bool pointer)
{
    int columns = getSectionColumns(section);
    UIControl_SlotList* list = NULL;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar)
        list = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector)
        list = &m_containerList;
    if (pointer)
        list->setHighlightSlot(slotY * columns + slotX);
}

unsigned int UIScene_CreativeMenu::overrideTooltips(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                    not_null_ptr<ItemInstance> slotItem, bool, bool slotIsItem, bool,
                                                    int, EToolTipItem& toolTip0, EToolTipItem& toolTip2,
                                                    EToolTipItem& toolTip3, EToolTipItem&, EToolTipItem&, EToolTipItem&,
                                                    EToolTipItem&)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector)
    {
        if (slotIsItem)
        {
            toolTip0 = eToolTipTake;
            toolTip3 = slotItem->isStackable() ? eToolTipTakeAll : eToolTipNone;
        }
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar)
    {
        toolTip3 = eToolTipNone;
    }
    toolTip2 = eToolTipClearQuickSelect;
    return 1;
}

void MenuBuilder::ITEM_BLOCKSTATE(const BlockState* state)
{
    ADD(not_null_ptr<ItemInstance>(
        new ItemInstance(state->getBlock(), 1, state->getBlock()->convertBlockStateToLegacyData(state))));
}

void MenuBuilder::FIREWORK(unsigned char type, int colour, int flight, bool flicker, bool trail, int fadeColour)
{
    CompoundTag* explosion = new CompoundTag();

    std::vector<int> colours;
    colours.push_back(DyeItem::DYE_COLORS[colour]);
    if (flicker)
        explosion->putBoolean(FireworksItem::TAG_E_FLICKER, true);
    if (trail)
        explosion->putBoolean(FireworksItem::TAG_E_TRAIL, true);
    explosion->putIntArray(FireworksItem::TAG_E_COLORS, arrayWithLength<int>(colours));
    explosion->putByte(FireworksItem::TAG_E_TYPE, type);

    if (fadeColour != -1)
    {
        std::vector<int> fadeColours;
        fadeColours.push_back(DyeItem::DYE_COLORS[fadeColour]);
        explosion->putIntArray(FireworksItem::TAG_E_FADECOLORS, arrayWithLength<int>(fadeColours));
    }

    CompoundTag* tag = new CompoundTag();
    CompoundTag* fireworks = new CompoundTag();
    ListTag* explosions = new ListTag();
    explosions->add(explosion);
    fireworks->put(FireworksItem::TAG_EXPLOSIONS, explosions);
    fireworks->putByte(FireworksItem::TAG_FLIGHT, static_cast<unsigned char>(flight));
    tag->put(FireworksItem::TAG_FIREWORKS, fireworks);

    ItemInstance rocket(Items::fireworks);
    rocket.setTag(tag);
    mItems->push_back(new ItemInstanceClonerBuilder(rocket));
    rocket = *ItemInstance::EMPTY;
}

void UIScene_CreativeMenu::staticCtor()
{
    MenuBuilder builder;
    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_BuildingBlocks];
    builder.ITEM_BLOCKSTATE(Blocks::stone->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::grass->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::grass_path->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::dirt->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::cobblestone->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::sand->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::sandstone->defaultBlockState());
    builder.ITEM_BLOCKSTATE(
        Blocks::sandstone->defaultBlockState()->setValue(SandStoneBlock::TYPE, SandStoneBlock::SMOOTH));
    builder.ITEM_BLOCKSTATE(
        Blocks::sandstone->defaultBlockState()->setValue(SandStoneBlock::TYPE, SandStoneBlock::CHISELED));
    builder.ITEM_BLOCKSTATE(Blocks::sand->defaultBlockState()->setValue(SandBlock::TYPE, SandBlock::Variant::RED_SAND));
    builder.ITEM_BLOCKSTATE(Blocks::red_sandstone->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::red_sandstone->defaultBlockState()->setValue(RedSandStoneBlock::TYPE,
                                                                                 RedSandStoneBlock::Variant::SMOOTH));
    builder.ITEM_BLOCKSTATE(Blocks::red_sandstone->defaultBlockState()->setValue(RedSandStoneBlock::TYPE,
                                                                                 RedSandStoneBlock::Variant::CHISELED));
    builder.ITEM_BLOCKSTATE(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::GRANITE));
    builder.ITEM_BLOCKSTATE(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::SMOOTH_GRANITE));
    builder.ITEM_BLOCKSTATE(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::ANDESITE));
    builder.ITEM_BLOCKSTATE(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::SMOOTH_ANDESITE));
    builder.ITEM_BLOCKSTATE(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::DIORITE));
    builder.ITEM_BLOCKSTATE(
        Blocks::stone->defaultBlockState()->setValue(StoneBlock::VARIANT, StoneBlock::Variant::SMOOTH_DIORITE));
    builder.ITEM_BLOCKSTATE(Blocks::coal_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::dried_kelp_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::gold_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::iron_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::lapis_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::diamond_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::emerald_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(
        Blocks::quartz_block->defaultBlockState()->setValue(QuartzBlock::VARIANT, QuartzBlock::Variant::DEFAULT));
    builder.ITEM_BLOCKSTATE(Blocks::coal_ore->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::lapis_ore->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::diamond_ore->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::redstone_ore->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::iron_ore->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::gold_ore->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::emerald_ore->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::quartz_ore->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::nether_wart_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::bedrock->defaultBlockState());
    builder.ITEM_BLOCKSTATE(
        Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::OAK));
    builder.ITEM_BLOCKSTATE(
        Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::SPRUCE));
    builder.ITEM_BLOCKSTATE(
        Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::BIRCH));
    builder.ITEM_BLOCKSTATE(
        Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::JUNGLE));
    builder.ITEM_BLOCKSTATE(
        Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::ACACIA));
    builder.ITEM_BLOCKSTATE(
        Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::DARK_OAK));
    builder.ITEM_BLOCKSTATE(
        Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::OAK));
    builder.ITEM_BLOCKSTATE(
        Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::SPRUCE));
    builder.ITEM_BLOCKSTATE(
        Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::BIRCH));
    builder.ITEM_BLOCKSTATE(
        Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::JUNGLE));
    builder.ITEM_BLOCKSTATE(
        Blocks::log2->defaultBlockState()->setValue(NewLogBlock::VARIANT, PlanksBlock::Variant::ACACIA));
    builder.ITEM_BLOCKSTATE(
        Blocks::log2->defaultBlockState()->setValue(NewLogBlock::VARIANT, PlanksBlock::Variant::DARK_OAK));
    builder.ITEM_BLOCKSTATE(Blocks::stripped_log_oak->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::stripped_log_spruce->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::stripped_log_birch->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::stripped_log_jungle->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::stripped_log_acacia->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::stripped_log_dark_oak->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::gravel->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::brick_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::mossy_cobblestone->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::obsidian->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::clay->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::ice->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::packed_ice->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::blue_ice->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::snow->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::netherrack->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::soul_sand->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::glowstone->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::magma->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::sea_lantern->defaultBlockState());
    builder.ITEM_BLOCKSTATE(
        Blocks::prismarine->defaultBlockState()->setValue(PrismarineBlock::VARIANT, PrismarineBlock::Variant::ROUGH));
    builder.ITEM_BLOCKSTATE(
        Blocks::prismarine->defaultBlockState()->setValue(PrismarineBlock::VARIANT, PrismarineBlock::Variant::BRICKS));
    builder.ITEM_BLOCKSTATE(
        Blocks::prismarine->defaultBlockState()->setValue(PrismarineBlock::VARIANT, PrismarineBlock::Variant::DARK));
    builder.ITEM_BLOCKSTATE(Blocks::slime->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::fence->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::spruce_fence->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::birch_fence->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::jungle_fence->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::acacia_fence->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::dark_oak_fence->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::nether_brick_fence->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::iron_bars->defaultBlockState());
    builder.ITEM_BLOCKSTATE(
        Blocks::stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT, StoneBrickBlock::Variant::DEFAULT));
    builder.ITEM_BLOCKSTATE(
        Blocks::stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT, StoneBrickBlock::Variant::MOSSY));
    builder.ITEM_BLOCKSTATE(
        Blocks::stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT, StoneBrickBlock::Variant::CRACKED));
    builder.ITEM_BLOCKSTATE(Blocks::stonebrick->defaultBlockState()->setValue(StoneBrickBlock::VARIANT,
                                                                              StoneBrickBlock::Variant::CHISELED));
    builder.ITEM_BLOCKSTATE(
        Blocks::monster_egg->defaultBlockState()->setValue(MonsterEggBlock::VARIANT, MonsterEggBlock::Variant::STONE));
    builder.ITEM_BLOCKSTATE(Blocks::monster_egg->defaultBlockState()->setValue(MonsterEggBlock::VARIANT,
                                                                               MonsterEggBlock::Variant::COBBLESTONE));
    builder.ITEM_BLOCKSTATE(Blocks::monster_egg->defaultBlockState()->setValue(MonsterEggBlock::VARIANT,
                                                                               MonsterEggBlock::Variant::STONEBRICK));
    builder.ITEM_BLOCKSTATE(Blocks::monster_egg->defaultBlockState()->setValue(
        MonsterEggBlock::VARIANT, MonsterEggBlock::Variant::MOSSY_STONEBRICK));
    builder.ITEM_BLOCKSTATE(Blocks::monster_egg->defaultBlockState()->setValue(
        MonsterEggBlock::VARIANT, MonsterEggBlock::Variant::CRACKED_STONEBRICK));
    builder.ITEM_BLOCKSTATE(Blocks::monster_egg->defaultBlockState()->setValue(
        MonsterEggBlock::VARIANT, MonsterEggBlock::Variant::CHISELED_STONEBRICK));
    builder.ITEM_BLOCKSTATE(Blocks::mycelium->defaultBlockState());
    builder.ITEM_AUX(Blocks::dirt, DirtBlock::VARIANT, DirtBlock::Variant::COARSE_DIRT);
    builder.ITEM_AUX(Blocks::dirt, DirtBlock::VARIANT, DirtBlock::Variant::PODZOL);
    builder.ITEM_BLOCKSTATE(Blocks::nether_brick->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::red_nether_brick->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::end_stone->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::end_bricks->defaultBlockState());
    builder.ITEM_AUX(Blocks::quartz_block, QuartzBlock::VARIANT, QuartzBlock::Variant::CHISELED);
    builder.ITEM_AUX(Blocks::quartz_block, QuartzBlock::VARIANT, QuartzBlock::Variant::LINES_Y);
    builder.ITEM_BLOCKSTATE(Blocks::purpur_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::purpur_pillar->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::trapdoor->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::spruce_trapdoor->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::birch_trapdoor->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::jungle_trapdoor->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::acacia_trapdoor->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::dark_oak_trapdoor->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::iron_trapdoor->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::fence_gate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::spruce_fence_gate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::birch_fence_gate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::jungle_fence_gate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::acacia_fence_gate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::dark_oak_fence_gate->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::spruceDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::birchDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::jungleDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::acaciaDoor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::darkOakDoor, 1, 0)));
    builder.ITEM_AUX(Blocks::stone_slab, StoneSlabBlock::VARIANT, StoneSlabBlock::Variant::STONE);
    builder.ITEM_AUX(Blocks::stone_slab, StoneSlabBlock::VARIANT, StoneSlabBlock::Variant::SAND);
    builder.ITEM_AUX(Blocks::stone_slab2, NewStoneSlabBlock::VARIANT, NewStoneSlabBlock::Variant::RED_SANDSTONE);
    builder.ITEM_AUX(Blocks::wooden_slab, WoodSlabBlock::VARIANT, PlanksBlock::Variant::OAK);
    builder.ITEM_AUX(Blocks::wooden_slab, WoodSlabBlock::VARIANT, PlanksBlock::Variant::SPRUCE);
    builder.ITEM_AUX(Blocks::wooden_slab, WoodSlabBlock::VARIANT, PlanksBlock::Variant::BIRCH);
    builder.ITEM_AUX(Blocks::wooden_slab, WoodSlabBlock::VARIANT, PlanksBlock::Variant::JUNGLE);
    builder.ITEM_AUX(Blocks::wooden_slab, WoodSlabBlock::VARIANT, PlanksBlock::Variant::ACACIA);
    builder.ITEM_AUX(Blocks::wooden_slab, WoodSlabBlock::VARIANT, PlanksBlock::Variant::DARK_OAK);
    builder.ITEM_AUX(Blocks::stone_slab, StoneSlabBlock::VARIANT, StoneSlabBlock::Variant::COBBLESTONE);
    builder.ITEM_AUX(Blocks::stone_slab, StoneSlabBlock::VARIANT, StoneSlabBlock::Variant::BRICK);
    builder.ITEM_AUX(Blocks::stone_slab, StoneSlabBlock::VARIANT, StoneSlabBlock::Variant::SMOOTHBRICK);
    builder.ITEM_AUX(Blocks::stone_slab, StoneSlabBlock::VARIANT, StoneSlabBlock::Variant::NETHERBRICK);
    builder.ITEM_AUX(Blocks::stone_slab, StoneSlabBlock::VARIANT, StoneSlabBlock::Variant::QUARTZ);
    builder.ITEM_BLOCKSTATE(Blocks::purpur_slab->defaultBlockState());
    builder.ITEM_AUX(Blocks::prismarine_slab, PrismarineSlabBlock::VARIANT, PrismarineSlabBlock::Variant::ROUGH);
    builder.ITEM_AUX(Blocks::prismarine_slab, PrismarineSlabBlock::VARIANT, PrismarineSlabBlock::Variant::BRICKS);
    builder.ITEM_AUX(Blocks::prismarine_slab, PrismarineSlabBlock::VARIANT, PrismarineSlabBlock::Variant::DARK);
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::oak_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::spruce_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::birch_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::jungle_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::acacia_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::dark_oak_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::brick_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone_brick_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::nether_brick_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::sandstone_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::red_sandstone_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::quartz_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::purpur_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::prismarine_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::prismarine_bricks_stairs, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::dark_prismarine_stairs, 1, 2)));
    builder.ITEM_BLOCKSTATE(Blocks::hardened_clay->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_hardened_clay, 1, 7)));

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_Decoration];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 5)));
    builder.ITEM_BLOCKSTATE(Blocks::sponge->defaultBlockState());
    builder.ITEM_AUX(Blocks::sponge, SpongeBlock::WET, true);
    builder.ITEM_BLOCKSTATE(Blocks::melon_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::pumpkin->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::carved_pumpkin->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::lit_pumpkin->defaultBlockState());
    builder.ITEM_BLOCKSTATE(
        Blocks::sapling->defaultBlockState()->setValue(SaplingBlock::TYPE, PlanksBlock::Variant::OAK));
    builder.ITEM_BLOCKSTATE(
        Blocks::sapling->defaultBlockState()->setValue(SaplingBlock::TYPE, PlanksBlock::Variant::SPRUCE));
    builder.ITEM_AUX(Blocks::sapling, SaplingBlock::TYPE, PlanksBlock::Variant::BIRCH);
    builder.ITEM_AUX(Blocks::sapling, SaplingBlock::TYPE, PlanksBlock::Variant::JUNGLE);
    builder.ITEM_AUX(Blocks::sapling, SaplingBlock::TYPE, PlanksBlock::Variant::ACACIA);
    builder.ITEM_AUX(Blocks::sapling, SaplingBlock::TYPE, PlanksBlock::Variant::DARK_OAK);
    builder.ITEM_BLOCKSTATE(
        Blocks::leaves->defaultBlockState()->setValue(OldLeafBlock::VARIANT, PlanksBlock::Variant::OAK));
    builder.ITEM_AUX(Blocks::leaves, OldLeafBlock::VARIANT, PlanksBlock::Variant::SPRUCE);
    builder.ITEM_AUX(Blocks::leaves, OldLeafBlock::VARIANT, PlanksBlock::Variant::BIRCH);
    builder.ITEM_AUX(Blocks::leaves, OldLeafBlock::VARIANT, PlanksBlock::Variant::JUNGLE);
    builder.ITEM_BLOCKSTATE(
        Blocks::leaves2->defaultBlockState()->setValue(NewLeafBlock::VARIANT, PlanksBlock::Variant::ACACIA));
    builder.ITEM_AUX(Blocks::leaves2, NewLeafBlock::VARIANT, PlanksBlock::Variant::DARK_OAK);
    builder.ITEM_BLOCKSTATE(Blocks::vine->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::waterlily->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::torch, 1, 5)));
    builder.ITEM_AUX(Blocks::tallgrass, TallGrassBlock::VARIANT, TallGrassBlock::Variant::DEAD_BUSH);
    builder.ITEM_AUX(Blocks::tallgrass, TallGrassBlock::VARIANT, TallGrassBlock::Variant::GRASS);
    builder.ITEM_BLOCKSTATE(
        Blocks::tallgrass->defaultBlockState()->setValue(TallGrassBlock::VARIANT, TallGrassBlock::Variant::FERN));
    builder.ITEM_BLOCKSTATE(Blocks::deadbush->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::flowerPot, 1, 0)));
    builder.ITEM_AUX(Blocks::yellow_flower, Blocks::yellow_flower->getFlowerProperty(), FlowerBlock::Flower::DANDELION);
    builder.ITEM_AUX(Blocks::red_flower, Blocks::red_flower->getFlowerProperty(), FlowerBlock::Flower::POPPY);
    builder.ITEM_BLOCKSTATE(Blocks::red_flower->defaultBlockState()->setValue(Blocks::red_flower->getFlowerProperty(),
                                                                              FlowerBlock::Flower::BLUE_ORCHID));
    builder.ITEM_AUX(Blocks::red_flower, Blocks::red_flower->getFlowerProperty(), FlowerBlock::Flower::ALLIUM);
    builder.ITEM_AUX(Blocks::red_flower, Blocks::red_flower->getFlowerProperty(), FlowerBlock::Flower::HOUSTONIA);
    builder.ITEM_AUX(Blocks::red_flower, Blocks::red_flower->getFlowerProperty(), FlowerBlock::Flower::RED_TULIP);
    builder.ITEM_BLOCKSTATE(Blocks::red_flower->defaultBlockState()->setValue(Blocks::red_flower->getFlowerProperty(),
                                                                              FlowerBlock::Flower::ORANGE_TULIP));
    builder.ITEM_AUX(Blocks::red_flower, Blocks::red_flower->getFlowerProperty(), FlowerBlock::Flower::WHITE_TULIP);
    builder.ITEM_AUX(Blocks::red_flower, Blocks::red_flower->getFlowerProperty(), FlowerBlock::Flower::PINK_TULIP);
    builder.ITEM_AUX(Blocks::red_flower, Blocks::red_flower->getFlowerProperty(), FlowerBlock::Flower::OXEYE_DAISY);
    builder.ITEM_BLOCKSTATE(Blocks::double_plant->defaultBlockState()->setValue(DoublePlantBlock::VARIANT,
                                                                                DoublePlantBlock::Variant::SUNFLOWER));
    builder.ITEM_AUX(Blocks::double_plant, DoublePlantBlock::VARIANT, DoublePlantBlock::Variant::SYRINGA);
    builder.ITEM_AUX(Blocks::double_plant, DoublePlantBlock::VARIANT, DoublePlantBlock::Variant::GRASS);
    builder.ITEM_AUX(Blocks::double_plant, DoublePlantBlock::VARIANT, DoublePlantBlock::Variant::FERN);
    builder.ITEM_BLOCKSTATE(Blocks::double_plant->defaultBlockState()->setValue(DoublePlantBlock::VARIANT,
                                                                                DoublePlantBlock::Variant::ROSE));
    builder.ITEM_AUX(Blocks::double_plant, DoublePlantBlock::VARIANT, DoublePlantBlock::Variant::PAEONIA);
    builder.ITEM_BLOCKSTATE(Blocks::brown_mushroom->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::red_mushroom->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::cactus->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::snow_layer->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::web->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::glass_pane->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::glass->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::painting, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::itemFrame, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::sign, 1, 0)));
    builder.ITEM_BLOCKSTATE(Blocks::bookshelf->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::hay_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::bone_block->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::chorus_flower->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::chorus_plant->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::end_rod->defaultBlockState());
    builder.ITEM_BLOCKSTATE(
        Blocks::cobblestone_wall->defaultBlockState()->setValue(WallBlock::VARIANT, WallBlock::Variant::COBBLESTONE));
    builder.ITEM_BLOCKSTATE(
        Blocks::cobblestone_wall->defaultBlockState()->setValue(WallBlock::VARIANT, WallBlock::Variant::MOSSY));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete, 1, 7)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::concrete_powder, 1, 7)));
    builder.ITEM_BLOCKSTATE(Blocks::black_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::blue_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::light_blue_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::cyan_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::green_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::lime_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::pink_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::magenta_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::purple_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::red_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::brown_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::orange_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::yellow_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::white_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::silver_glazed_terracotta->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::gray_glazed_terracotta->defaultBlockState());
    builder.BANNER(DyeColor::RED, NULL);
    builder.BANNER(DyeColor::ORANGE, NULL);
    builder.BANNER(DyeColor::YELLOW, NULL);
    builder.BANNER(DyeColor::LIME, NULL);
    builder.BANNER(DyeColor::LIGHT_BLUE, NULL);
    builder.BANNER(DyeColor::CYAN, NULL);
    builder.BANNER(DyeColor::BLUE, NULL);
    builder.BANNER(DyeColor::PURPLE, NULL);
    builder.BANNER(DyeColor::MAGENTA, NULL);
    builder.BANNER(DyeColor::PINK, NULL);
    builder.BANNER(DyeColor::WHITE, NULL);
    builder.BANNER(DyeColor::GRAY, NULL);
    builder.BANNER(DyeColor::SILVER, NULL);
    builder.BANNER(DyeColor::BLACK, NULL);
    builder.BANNER(DyeColor::GREEN, NULL);
    builder.BANNER(DyeColor::BROWN, NULL);
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, 7)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carpet, 1, 7)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass, 1, 7)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stained_glass_pane, 1, 7)));
    builder.ITEM_AUX(Blocks::red_mushroom_block, HugeMushroomBlock::VARIANT, HugeMushroomBlock::Variant::ALL_OUTSIDE);
    builder.ITEM_BLOCKSTATE(Blocks::brown_mushroom_block->defaultBlockState()->setValue(
        HugeMushroomBlock::VARIANT, HugeMushroomBlock::Variant::ALL_OUTSIDE));
    builder.ITEM_AUX(Blocks::brown_mushroom_block, HugeMushroomBlock::VARIANT, HugeMushroomBlock::Variant::ALL_INSIDE);
    builder.ITEM_AUX(Blocks::brown_mushroom_block, HugeMushroomBlock::VARIANT, HugeMushroomBlock::Variant::STEM);
    builder.ITEM_BLOCKSTATE(Blocks::sea_pickle->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::kelp, 1, 0)));
    builder.ITEM_BLOCKSTATE(Blocks::sea_grass->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan_dead, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan_dead, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan_dead, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan_dead, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_fan_dead, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral_block, 1, 12)));

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_Redstone];
    builder.ITEM_BLOCKSTATE(Blocks::dispenser->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::noteblock->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::piston, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::sticky_piston, 1, 5)));
    builder.ITEM_BLOCKSTATE(Blocks::tnt->defaultBlockState());
    builder.ADD(
        not_null_ptr<ItemInstance>(new ItemInstance(Blocks::lever, 1, LeverBlock::getLeverFacing(Direction::NORTH))));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone_button, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wooden_button, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::spruce_button, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::birch_button, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::jungle_button, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::acacia_button, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::dark_oak_button, 1, 4)));
    builder.ITEM_BLOCKSTATE(Blocks::stone_pressure_plate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::wooden_pressure_plate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::spruce_pressure_plate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::birch_pressure_plate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::jungle_pressure_plate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::acacia_pressure_plate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::dark_oak_pressure_plate->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::redstone, 1, 0)));
    builder.ITEM_BLOCKSTATE(Blocks::redstone_block->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::redstone_torch, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::repeater, 1, 0)));
    builder.ITEM_BLOCKSTATE(Blocks::redstone_lamp->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::tripwire_hook->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::daylight_detector->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::dropper->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::hopper->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::comparator, 1, 0)));
    builder.ITEM_BLOCKSTATE(Blocks::trapped_chest->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::heavy_weighted_pressure_plate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::light_weighted_pressure_plate->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::observer->defaultBlockState());

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_Transport];
    builder.ITEM_BLOCKSTATE(Blocks::rail->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::golden_rail->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::detector_rail->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::activator_rail->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::ladder->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::minecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chestMinecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::furnaceMinecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::hopperMinecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::tntMinecart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::elytra, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::saddle, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::boat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::spruceBoat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::birchBoat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::jungleBoat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::acaciaBoat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::darkOakBoat, 1, 0)));

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_Miscellaneous];
    builder.ITEM_BLOCKSTATE(Blocks::chest->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::ender_chest->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::crafting_table->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::furnace->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::brewingStand, 1, 0)));
    builder.ITEM_BLOCKSTATE(Blocks::enchanting_table->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::beacon->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::conduit->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::black_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::blue_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::light_blue_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::cyan_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::green_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::lime_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::pink_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::magenta_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::purple_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::red_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::brown_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::orange_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::yellow_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::white_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::silver_shulker_box->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::gray_shulker_box->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 8)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bed, 1, 7)));
    builder.ITEM_BLOCKSTATE(Blocks::end_portal_frame->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::endCrystal, 1, 0)));
    builder.ITEM_BLOCKSTATE(Blocks::jukebox->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::anvil->defaultBlockState());
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::lavaBucket, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::waterBucket, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::milkBucket, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::fishBucket, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::salmonBucket, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::pufferBucket, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::tropicalBucket, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cauldron, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::snowball, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::paper, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::book, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::writableBook, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::enderPearl, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::enderEye, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::nameTag, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::netherStar, 1, 0)));
#ifdef MINECRAFT_PC
    if (PcDebugMode())
        builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::debugFourjItem, 1, 0)));
#endif
    builder.ITEM_BLOCKSTATE(Blocks::mob_spawner->defaultBlockState());
    builder.ITEM_BLOCKSTATE(Blocks::turtle_egg->defaultBlockState());
    builder.DEBUG_SPAWN_EGG(L"creeper", L"");
    builder.DEBUG_SPAWN_EGG(L"skeleton", L"");
    builder.DEBUG_SPAWN_EGG(L"wither_skeleton", L"");
    builder.DEBUG_SPAWN_EGG(L"stray", L"");
    builder.DEBUG_SPAWN_EGG(L"spider", L"");
    builder.DEBUG_SPAWN_EGG(L"cave_spider", L"");
    builder.DEBUG_SPAWN_EGG(L"zombie", L"");
    builder.DEBUG_SPAWN_EGG(L"zombie_villager", L"");
    builder.DEBUG_SPAWN_EGG(L"husk", L"");
    builder.DEBUG_SPAWN_EGG(L"drowned", L"");
    builder.DEBUG_SPAWN_EGG(L"slime", L"");
    builder.DEBUG_SPAWN_EGG(L"witch", L"");
    builder.DEBUG_SPAWN_EGG(L"silverfish", L"");
    builder.DEBUG_SPAWN_EGG(L"ghast", L"");
    builder.DEBUG_SPAWN_EGG(L"zombie_pigman", L"");
    builder.DEBUG_SPAWN_EGG(L"blaze", L"");
    builder.DEBUG_SPAWN_EGG(L"magma_cube", L"");
    builder.DEBUG_SPAWN_EGG(L"enderman", L"");
    builder.DEBUG_SPAWN_EGG(L"endermite", L"");
    builder.DEBUG_SPAWN_EGG(L"shulker", L"");
    builder.DEBUG_SPAWN_EGG(L"guardian", L"");
    builder.DEBUG_SPAWN_EGG(L"elder_guardian", L"");
    builder.DEBUG_SPAWN_EGG(L"evocation_illager", L"");
    builder.DEBUG_SPAWN_EGG(L"vex", L"");
    builder.DEBUG_SPAWN_EGG(L"vindication_illager", L"");
    builder.DEBUG_SPAWN_EGG(L"villager", L"");
    builder.DEBUG_SPAWN_EGG(L"bat", L"");
    builder.DEBUG_SPAWN_EGG(L"pig", L"");
    builder.DEBUG_SPAWN_EGG(L"sheep", L"");
    builder.DEBUG_SPAWN_EGG(L"cow", L"");
    builder.DEBUG_SPAWN_EGG(L"mooshroom", L"");
    builder.DEBUG_SPAWN_EGG(L"chicken", L"");
    builder.DEBUG_SPAWN_EGG(L"rabbit", L"");
    builder.DEBUG_SPAWN_EGG(L"squid", L"");
    builder.DEBUG_SPAWN_EGG(L"fish", L"");
    builder.DEBUG_SPAWN_EGG(L"salmon", L"");
    builder.DEBUG_SPAWN_EGG(L"pufferfish", L"");
    builder.DEBUG_SPAWN_EGG(L"tropicalfish", L"");
    builder.DEBUG_SPAWN_EGG(L"parrot", L"");
    builder.DEBUG_SPAWN_EGG(L"wolf", L"");
    builder.DEBUG_SPAWN_EGG(L"ocelot", L"");
    builder.DEBUG_SPAWN_EGG(L"polar_bear", L"");
    builder.DEBUG_SPAWN_EGG(L"horse", L"");
    builder.DEBUG_SPAWN_EGG(L"donkey", L"");
    builder.DEBUG_SPAWN_EGG(L"mule", L"");
    builder.DEBUG_SPAWN_EGG(L"skeleton_horse", L"");
    builder.DEBUG_SPAWN_EGG(L"zombie_horse", L"");
    builder.DEBUG_SPAWN_EGG(L"llama", L"");
    builder.DEBUG_SPAWN_EGG(L"turtle", L"");
    builder.DEBUG_SPAWN_EGG(L"dolphin", L"");
    builder.DEBUG_SPAWN_EGG(L"phantom", L"");
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::record13, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordCat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordBlocks, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordChirp, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordFar, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordMall, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordMellohi, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordStal, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordStrad, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordWard, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::record11, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::recordWait, 1, 0)));
    builder.FIREWORK(0, DyeColor::LIGHT_BLUE->getItemData(), 1, 1, 0, -1);
    builder.FIREWORK(3, DyeColor::GREEN->getItemData(), 2, 0, 0, -1);
    builder.FIREWORK(4, DyeColor::RED->getItemData(), 2, 0, 0, DyeColor::ORANGE->getItemData());
    builder.FIREWORK(4, DyeColor::MAGENTA->getItemData(), 3, 1, 0, DyeColor::BLUE->getItemData());
    builder.FIREWORK(2, DyeColor::YELLOW->getItemData(), 2, 0, 1, DyeColor::ORANGE->getItemData());

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_Food];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::apple, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenApple, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenApple, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::melon, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::mushroomStew, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::rabbitStew, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::beetrootSoup, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bread, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cake, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cookie, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cookedFish, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::fish, 1, 0)));
    builder.ITEM_AUX(Items::cookedFish, FishFoodItem::Type::SALMON);
    builder.ITEM_AUX(Items::fish, FishFoodItem::Type::SALMON);
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::fish, 1, FishFoodItem::Type::CLOWNFISH->getData())));
    builder.ITEM_AUX(Items::fish, FishFoodItem::Type::PUFFERFISH);
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cookedPorkchop, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::porkchop, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cookedBeef, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::beef, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cookedChicken, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chicken, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cookedMutton, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::mutton, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::cookedRabbit, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::rabbit, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::rottenFlesh, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::spiderEye, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::potato, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bakedPotato, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::poisonousPotato, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::carrot, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenCarrot, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::pumpkinPie, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::beetroot, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chorusFruit, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::driedKelp, 1, 0)));

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_ToolsAndCombat];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::compass, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::leatherHelmet, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::leatherChestplate, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::leatherLeggings, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::leatherBoots, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenSword, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenShovel, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenPickaxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenAxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::woodenHoe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::map, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chainmailHelmet, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chainmailChestplate, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chainmailLeggings, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chainmailBoots, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::stoneSword, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::stoneShovel, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::stonePickaxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::stoneAxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::stoneHoe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bow, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironHelmet, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironChestplate, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironLeggings, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironBoots, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironSword, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironShovel, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironPickaxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironAxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironHoe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::arrow, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenHelmet, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenChestplate, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenLeggings, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenBoots, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenSword, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenShovel, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenPickaxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenAxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenHoe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::flintAndSteel, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondHelmet, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondChestplate, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondLeggings, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondBoots, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondSword, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondShovel, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondPickaxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondAxe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondHoe, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::fireCharge, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::clock, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::shears, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::fishingRod, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::carrotOnAStick, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::lead, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::totemOfUndying, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamondHorseArmor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldenHorseArmor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironHorseArmor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::leatherHorseArmor, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::armorStand, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::turtleHelmet, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::trident, 1, 0)));
    builder.POTION(Items::tippedArrow, Potions::NIGHT_VISION);
    builder.POTION(Items::tippedArrow, Potions::INVISIBILITY);
    builder.POTION(Items::tippedArrow, Potions::LEAPING);
    builder.POTION(Items::tippedArrow, Potions::FIRE_RESISTANCE);
    builder.POTION(Items::tippedArrow, Potions::SWIFTNESS);
    builder.POTION(Items::tippedArrow, Potions::SLOWNESS);
    builder.POTION(Items::tippedArrow, Potions::WATER_BREATHING);
    builder.POTION(Items::tippedArrow, Potions::HEALING);
    builder.POTION(Items::tippedArrow, Potions::HARMING);
    builder.POTION(Items::tippedArrow, Potions::POISON);
    builder.POTION(Items::tippedArrow, Potions::REGENERATION);
    builder.POTION(Items::tippedArrow, Potions::STRENGTH);
    builder.POTION(Items::tippedArrow, Potions::WEAKNESS);
    builder.POTION(Items::tippedArrow, Potions::LUCK);
    builder.POTION(Items::tippedArrow, Potions::TURTLE_MASTER);
    builder.POTION(Items::tippedArrow, Potions::SLOW_FALLING);
    builder.POTION(Items::tippedArrow, Potions::LONG_NIGHT_VISION);
    builder.POTION(Items::tippedArrow, Potions::LONG_INVISIBILITY);
    builder.POTION(Items::tippedArrow, Potions::LONG_LEAPING);
    builder.POTION(Items::tippedArrow, Potions::LONG_FIRE_RESISTANCE);
    builder.POTION(Items::tippedArrow, Potions::LONG_SWIFTNESS);
    builder.POTION(Items::tippedArrow, Potions::LONG_SLOWNESS);
    builder.POTION(Items::tippedArrow, Potions::LONG_WATER_BREATHING);
    builder.POTION(Items::tippedArrow, Potions::LONG_POISON);
    builder.POTION(Items::tippedArrow, Potions::LONG_REGENERATION);
    builder.POTION(Items::tippedArrow, Potions::LONG_STRENGTH);
    builder.POTION(Items::tippedArrow, Potions::LONG_WEAKNESS);
    builder.POTION(Items::tippedArrow, Potions::WITHER);
    builder.POTION(Items::tippedArrow, Potions::LONG_TURTLE_MASTER);
    builder.POTION(Items::tippedArrow, Potions::LONG_SLOW_FALLING);
    builder.POTION(Items::tippedArrow, Potions::STRONG_LEAPING);
    builder.POTION(Items::tippedArrow, Potions::STRONG_SWIFTNESS);
    builder.POTION(Items::tippedArrow, Potions::STRONG_HEALING);
    builder.POTION(Items::tippedArrow, Potions::STRONG_HARMING);
    builder.POTION(Items::tippedArrow, Potions::STRONG_POISON);
    builder.POTION(Items::tippedArrow, Potions::STRONG_REGENERATION);
    builder.POTION(Items::tippedArrow, Potions::STRONG_STRENGTH);
    builder.POTION(Items::tippedArrow, Potions::STRONG_TURTLE_MASTER);

    {
        std::vector<int> enchantmentIds;
        const boost::unordered_map<Enchantment*, ResourceLocation, boost::hash<Enchantment*>,
                                   std::equal_to<Enchantment*> >& enchantments =
            Enchantment::enchantmentRegistry->mInverseRegistry;
        for (boost::unordered_map<Enchantment*, ResourceLocation, boost::hash<Enchantment*>,
                                  std::equal_to<Enchantment*> >::const_iterator it = enchantments.begin();
             it != enchantments.end(); ++it)
            enchantmentIds.push_back(Enchantment::enchantmentRegistry->getId(it->first));
        std::sort(enchantmentIds.begin(), enchantmentIds.end());

        for (std::vector<int>::const_iterator it = enchantmentIds.begin(); it != enchantmentIds.end(); ++it)
        {
            Enchantment* enchantment = Enchantment::byId(*it);
            if (enchantment != NULL && enchantment->mCategory != NULL)
                builder.ADD(EnchantedBookItem::createForEnchantment(
                    new EnchantmentInstance(enchantment, enchantment->getMaxLevel())));
        }
    }

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_Materials];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::coal, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::coal, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::diamond, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::emerald, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironIngot, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldIngot, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::quartz, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::brick, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::netherbrick, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::stick, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bowl, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::bone, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::string, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::feather, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::flint, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::leather, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::rabbitHide, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::gunpowder, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::clayBall, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::glowstoneDust, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::prismarineCrystals, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::prismarineShard, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::nautilus, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::nautilusCore, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::wheatSeeds, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::melonSeeds, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::pumpkinSeeds, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::beetrootSeeds, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::wheat, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::reeds, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::egg, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::sugar, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::slimeBall, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::blazeRod, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::goldNugget, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ironNugget, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::shulkerShell, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::netherWart, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::chorusFruitPopped, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::turtleShellPiece, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 4)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 12)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 6)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 2)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 10)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 9)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 13)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 5)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 1)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 3)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 14)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 11)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 15)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 7)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 8)));

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_BrewingIngredients];
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::experienceBottle, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::dragonBreath, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::ghastTear, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::fermentedSpiderEye, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::blazePowder, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::magmaCream, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::speckledMelon, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::rabbitFoot, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::phantomMembrane, 1, 0)));
    builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(Items::glassBottle, 1, 0)));

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_Potions];
    builder.POTION(Items::potion, Potions::WATER);
    builder.POTION(Items::potion, Potions::AWKWARD);
    builder.POTION(Items::potion, Potions::NIGHT_VISION);
    builder.POTION(Items::potion, Potions::INVISIBILITY);
    builder.POTION(Items::potion, Potions::LEAPING);
    builder.POTION(Items::potion, Potions::FIRE_RESISTANCE);
    builder.POTION(Items::potion, Potions::SWIFTNESS);
    builder.POTION(Items::potion, Potions::SLOWNESS);
    builder.POTION(Items::potion, Potions::WATER_BREATHING);
    builder.POTION(Items::potion, Potions::HEALING);
    builder.POTION(Items::potion, Potions::HARMING);
    builder.POTION(Items::potion, Potions::POISON);
    builder.POTION(Items::potion, Potions::REGENERATION);
    builder.POTION(Items::potion, Potions::STRENGTH);
    builder.POTION(Items::potion, Potions::WEAKNESS);
    builder.POTION(Items::potion, Potions::LUCK);
    builder.POTION(Items::potion, Potions::TURTLE_MASTER);
    builder.POTION(Items::potion, Potions::SLOW_FALLING);
    builder.POTION(Items::splashPotion, Potions::WATER);
    builder.POTION(Items::splashPotion, Potions::NIGHT_VISION);
    builder.POTION(Items::splashPotion, Potions::INVISIBILITY);
    builder.POTION(Items::splashPotion, Potions::LEAPING);
    builder.POTION(Items::splashPotion, Potions::FIRE_RESISTANCE);
    builder.POTION(Items::splashPotion, Potions::SWIFTNESS);
    builder.POTION(Items::splashPotion, Potions::SLOWNESS);
    builder.POTION(Items::splashPotion, Potions::WATER_BREATHING);
    builder.POTION(Items::splashPotion, Potions::HEALING);
    builder.POTION(Items::splashPotion, Potions::HARMING);
    builder.POTION(Items::splashPotion, Potions::POISON);
    builder.POTION(Items::splashPotion, Potions::REGENERATION);
    builder.POTION(Items::splashPotion, Potions::STRENGTH);
    builder.POTION(Items::splashPotion, Potions::WEAKNESS);
    builder.POTION(Items::splashPotion, Potions::LUCK);
    builder.POTION(Items::splashPotion, Potions::TURTLE_MASTER);
    builder.POTION(Items::splashPotion, Potions::SLOW_FALLING);
    builder.POTION(Items::lingeringPotion, Potions::WATER);
    builder.POTION(Items::lingeringPotion, Potions::NIGHT_VISION);
    builder.POTION(Items::lingeringPotion, Potions::INVISIBILITY);
    builder.POTION(Items::lingeringPotion, Potions::LEAPING);
    builder.POTION(Items::lingeringPotion, Potions::FIRE_RESISTANCE);
    builder.POTION(Items::lingeringPotion, Potions::SWIFTNESS);
    builder.POTION(Items::lingeringPotion, Potions::SLOWNESS);
    builder.POTION(Items::lingeringPotion, Potions::WATER_BREATHING);
    builder.POTION(Items::lingeringPotion, Potions::HEALING);
    builder.POTION(Items::lingeringPotion, Potions::HARMING);
    builder.POTION(Items::lingeringPotion, Potions::POISON);
    builder.POTION(Items::lingeringPotion, Potions::REGENERATION);
    builder.POTION(Items::lingeringPotion, Potions::STRENGTH);
    builder.POTION(Items::lingeringPotion, Potions::WEAKNESS);
    builder.POTION(Items::lingeringPotion, Potions::LUCK);
    builder.POTION(Items::lingeringPotion, Potions::TURTLE_MASTER);
    builder.POTION(Items::lingeringPotion, Potions::SLOW_FALLING);

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_LongPotions];
    builder.POTION(Items::potion, Potions::LONG_NIGHT_VISION);
    builder.POTION(Items::potion, Potions::LONG_INVISIBILITY);
    builder.POTION(Items::potion, Potions::LONG_LEAPING);
    builder.POTION(Items::potion, Potions::LONG_FIRE_RESISTANCE);
    builder.POTION(Items::potion, Potions::LONG_SWIFTNESS);
    builder.POTION(Items::potion, Potions::LONG_SLOWNESS);
    builder.POTION(Items::potion, Potions::LONG_WATER_BREATHING);
    builder.POTION(Items::potion, Potions::LONG_POISON);
    builder.POTION(Items::potion, Potions::LONG_REGENERATION);
    builder.POTION(Items::potion, Potions::LONG_STRENGTH);
    builder.POTION(Items::potion, Potions::LONG_WEAKNESS);
    builder.POTION(Items::potion, Potions::WITHER);
    builder.POTION(Items::potion, Potions::LONG_TURTLE_MASTER);
    builder.POTION(Items::potion, Potions::LONG_SLOW_FALLING);
    builder.POTION(Items::splashPotion, Potions::LONG_NIGHT_VISION);
    builder.POTION(Items::splashPotion, Potions::LONG_INVISIBILITY);
    builder.POTION(Items::splashPotion, Potions::LONG_LEAPING);
    builder.POTION(Items::splashPotion, Potions::LONG_FIRE_RESISTANCE);
    builder.POTION(Items::splashPotion, Potions::LONG_SWIFTNESS);
    builder.POTION(Items::splashPotion, Potions::LONG_SLOWNESS);
    builder.POTION(Items::splashPotion, Potions::LONG_WATER_BREATHING);
    builder.POTION(Items::splashPotion, Potions::LONG_POISON);
    builder.POTION(Items::splashPotion, Potions::LONG_REGENERATION);
    builder.POTION(Items::splashPotion, Potions::LONG_STRENGTH);
    builder.POTION(Items::splashPotion, Potions::LONG_WEAKNESS);
    builder.POTION(Items::splashPotion, Potions::WITHER);
    builder.POTION(Items::splashPotion, Potions::LONG_TURTLE_MASTER);
    builder.POTION(Items::splashPotion, Potions::LONG_SLOW_FALLING);
    builder.POTION(Items::lingeringPotion, Potions::LONG_NIGHT_VISION);
    builder.POTION(Items::lingeringPotion, Potions::LONG_INVISIBILITY);
    builder.POTION(Items::lingeringPotion, Potions::LONG_LEAPING);
    builder.POTION(Items::lingeringPotion, Potions::LONG_FIRE_RESISTANCE);
    builder.POTION(Items::lingeringPotion, Potions::LONG_SWIFTNESS);
    builder.POTION(Items::lingeringPotion, Potions::LONG_SLOWNESS);
    builder.POTION(Items::lingeringPotion, Potions::LONG_WATER_BREATHING);
    builder.POTION(Items::lingeringPotion, Potions::LONG_POISON);
    builder.POTION(Items::lingeringPotion, Potions::LONG_REGENERATION);
    builder.POTION(Items::lingeringPotion, Potions::LONG_STRENGTH);
    builder.POTION(Items::lingeringPotion, Potions::LONG_WEAKNESS);
    builder.POTION(Items::lingeringPotion, Potions::WITHER);
    builder.POTION(Items::lingeringPotion, Potions::LONG_TURTLE_MASTER);
    builder.POTION(Items::lingeringPotion, Potions::LONG_SLOW_FALLING);

    builder.mItems = &s_groupItems[ECreative_Inventory_Groups_StrongPotions];
    builder.POTION(Items::potion, Potions::STRONG_LEAPING);
    builder.POTION(Items::potion, Potions::STRONG_SWIFTNESS);
    builder.POTION(Items::potion, Potions::STRONG_HEALING);
    builder.POTION(Items::potion, Potions::STRONG_HARMING);
    builder.POTION(Items::potion, Potions::STRONG_POISON);
    builder.POTION(Items::potion, Potions::STRONG_REGENERATION);
    builder.POTION(Items::potion, Potions::STRONG_STRENGTH);
    builder.POTION(Items::potion, Potions::STRONG_TURTLE_MASTER);
    builder.POTION(Items::splashPotion, Potions::STRONG_LEAPING);
    builder.POTION(Items::splashPotion, Potions::STRONG_SWIFTNESS);
    builder.POTION(Items::splashPotion, Potions::STRONG_HEALING);
    builder.POTION(Items::splashPotion, Potions::STRONG_HARMING);
    builder.POTION(Items::splashPotion, Potions::STRONG_POISON);
    builder.POTION(Items::splashPotion, Potions::STRONG_REGENERATION);
    builder.POTION(Items::splashPotion, Potions::STRONG_STRENGTH);
    builder.POTION(Items::splashPotion, Potions::STRONG_TURTLE_MASTER);
    builder.POTION(Items::lingeringPotion, Potions::STRONG_LEAPING);
    builder.POTION(Items::lingeringPotion, Potions::STRONG_SWIFTNESS);
    builder.POTION(Items::lingeringPotion, Potions::STRONG_HEALING);
    builder.POTION(Items::lingeringPotion, Potions::STRONG_HARMING);
    builder.POTION(Items::lingeringPotion, Potions::STRONG_POISON);
    builder.POTION(Items::lingeringPotion, Potions::STRONG_REGENERATION);
    builder.POTION(Items::lingeringPotion, Potions::STRONG_STRENGTH);
    builder.POTION(Items::lingeringPotion, Potions::STRONG_TURTLE_MASTER);

#ifdef MINECRAFT_PC
    {
        std::vector<PcModCreativeEntry> entries;
        PcModContentCreativeItems(&entries);
        for (size_t i = 0; i < entries.size(); ++i)
        {
            static const ECreative_Inventory_Groups groups[] = {
                ECreative_Inventory_Groups_None,          ECreative_Inventory_Groups_BuildingBlocks,
                ECreative_Inventory_Groups_Decoration,    ECreative_Inventory_Groups_Redstone,
                ECreative_Inventory_Groups_Transport,     ECreative_Inventory_Groups_Materials,
                ECreative_Inventory_Groups_Food,          ECreative_Inventory_Groups_ToolsAndCombat,
                ECreative_Inventory_Groups_BrewingIngredients, ECreative_Inventory_Groups_Miscellaneous};
            const ECreative_Inventory_Groups group = groups[entries[i].tab];
            if (group == ECreative_Inventory_Groups_None)
                continue;
            builder.mItems = &s_groupItems[group];
            builder.ADD(not_null_ptr<ItemInstance>(new ItemInstance(entries[i].item, 1, 0)));
        }
    }
#endif
    ECreative_Inventory_Groups structuresGroups[] = {ECreative_Inventory_Groups_BuildingBlocks};
    ECreative_Inventory_Groups decorationGroups[] = {ECreative_Inventory_Groups_Decoration};
    ECreative_Inventory_Groups redstoneAndTransportGroups[] = {ECreative_Inventory_Groups_Transport,
                                                               ECreative_Inventory_Groups_Redstone};
    ECreative_Inventory_Groups materialsGroups[] = {ECreative_Inventory_Groups_Materials};
    ECreative_Inventory_Groups foodGroups[] = {ECreative_Inventory_Groups_Food};
    ECreative_Inventory_Groups toolsGroups[] = {ECreative_Inventory_Groups_ToolsAndCombat};
    ECreative_Inventory_Groups brewingGroups[] = {
        ECreative_Inventory_Groups_BrewingIngredients, ECreative_Inventory_Groups_Potions,
        ECreative_Inventory_Groups_LongPotions, ECreative_Inventory_Groups_StrongPotions};
    ECreative_Inventory_Groups miscGroups[] = {ECreative_Inventory_Groups_Miscellaneous};

    s_tabSpecs = new TabSpec*[8];
    s_tabSpecs[0] = new TabSpec(L"Structures", 0x0FBC6E08, 1, structuresGroups, 0, NULL, 0, NULL);
    s_tabSpecs[1] = new TabSpec(L"Decoration", static_cast<int>(0xAC698282), 1, decorationGroups, 0, NULL, 0, NULL);
    s_tabSpecs[2] = new TabSpec(L"RedstoneAndTransport", 0x060380CB, 2, redstoneAndTransportGroups, 0, NULL, 0, NULL);
    s_tabSpecs[3] = new TabSpec(L"Materials", 0x448C117B, 1, materialsGroups, 0, NULL, 0, NULL);
    s_tabSpecs[4] = new TabSpec(L"Food", static_cast<int>(0xEF31F329), 1, foodGroups, 0, NULL, 0, NULL);
    s_tabSpecs[5] = new TabSpec(L"Tools", static_cast<int>(0xAD2EBC52), 1, toolsGroups, 0, NULL, 0, NULL);
    s_tabSpecs[6] = new TabSpec(L"Brewing", 0x50D06098, 4, brewingGroups, 0, NULL, 0, NULL);
    s_tabSpecs[7] = new TabSpec(L"Misc", static_cast<int>(0xBC0FD661), 1, miscGroups, 0, NULL, 0, NULL);
}

unsigned int UIScene_CreativeMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_touchPanels[0].setupControl(this, currentNode, std::string("TouchPanel_0"));
            m_controls.push_back(&m_touchPanels[0]);
        }
        {
            m_touchPanels[1].setupControl(this, currentNode, std::string("TouchPanel_1"));
            m_controls.push_back(&m_touchPanels[1]);
        }
        {
            m_touchPanels[2].setupControl(this, currentNode, std::string("TouchPanel_2"));
            m_controls.push_back(&m_touchPanels[2]);
        }
        {
            m_touchPanels[3].setupControl(this, currentNode, std::string("TouchPanel_3"));
            m_controls.push_back(&m_touchPanels[3]);
        }
        {
            m_touchPanels[4].setupControl(this, currentNode, std::string("TouchPanel_4"));
            m_controls.push_back(&m_touchPanels[4]);
        }
        {
            m_touchPanels[5].setupControl(this, currentNode, std::string("TouchPanel_5"));
            m_controls.push_back(&m_touchPanels[5]);
        }
        {
            m_touchPanels[6].setupControl(this, currentNode, std::string("TouchPanel_6"));
            m_controls.push_back(&m_touchPanels[6]);
        }
        {
            m_touchPanels[7].setupControl(this, currentNode, std::string("TouchPanel_7"));
            m_controls.push_back(&m_touchPanels[7]);
        }
        {
            m_touchPanels[8].setupControl(this, currentNode, std::string("TouchPanel_Slider"));
            m_controls.push_back(&m_touchPanels[8]);
        }
        {
            m_containerList.setupControl(this, currentNode, std::string("containerList"));
            m_controls.push_back(&m_containerList);
        }
        currentNode = parentNode;
    }
    return 1;
}

std::wstring UIScene_CreativeMenu::getMoviePath()
{
    return std::wstring(gApp.GetLocalPlayerCount() < 2 ? L"CreativeMenu" : L"CreativeMenuSplit");
}

void UIScene_CreativeMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, false, -1, IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar,
               IUIScene_AbstractContainerMenu::ESceneSection_CreativeMax, m_navigateBack != 0);

    for (unsigned int i = 0; i < 50; ++i)
        m_containerList.addSlot(i);
    for (unsigned int i = 50; i < 59; ++i)
        m_hotbarList.addSlot(i);

    const int previousTab = m_currentTab;
    m_currentTab = 8;
    switchTab(static_cast<ECreativeInventoryTabs>(previousTab));
}

UIControl* UIScene_CreativeMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeHotbar)
        return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_CreativeSelector)
        return &m_containerList;
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab0
        && section <= IUIScene_AbstractContainerMenu::ESceneSection_CreativeSlider)
        return &m_touchPanels[section - IUIScene_AbstractContainerMenu::ESceneSection_CreativeTab0];
    return NULL;
}

void UIScene_CreativeMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                       bool& handled)
{
    if (repeat)
        return;

    gConsoleUIController.AnimateKeyPress(m_playerId, action, false, pressed, released);
    switch (action)
    {
    case VK_PAD_RSHOULDER:
    case VK_PAD_LSHOULDER:
        if (pressed)
        {
            switchTab(static_cast<ECreativeInventoryTabs>(
                (m_currentTab + (action == VK_PAD_RSHOULDER ? 1 : -1) + ECreativeInventoryTabs_COUNT)
                % ECreativeInventoryTabs_COUNT));
            gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
        }
        break;
    case VK_PAD_LTRIGGER:
        if (pressed)
        {
            int nextGroup = m_tabPages[m_currentTab] + 1;
            if (nextGroup >= s_tabSpecs[m_currentTab]->m_specialGroupCount)
                nextGroup = 0;
            m_tabPages[m_currentTab] = nextGroup;
            switchTab(static_cast<ECreativeInventoryTabs>(m_currentTab));
        }
        break;
    default:
        UIScene_AbstractContainerMenu::handleInput(playerId, action, false, pressed, released, handled);
        break;
    }
}

void UIScene_CreativeMenu::handleTouchInput(unsigned int, int, int touchY, int touchBox, bool, bool repeat,
                                            bool released)
{
    if (released && (unsigned int)touchBox < 8)
    {
        switchTab(static_cast<ECreativeInventoryTabs>(touchBox));
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    if (repeat && touchBox == 8)
    {
        if (s_tabSpecs[m_currentTab]->getPageCount() > 1)
        {
            float fraction = (static_cast<float>(touchY) - m_touchPanels[8].getYPos() - m_mainPanel.getYPos())
                             / m_touchPanels[8].getHeight();
            if (fraction > 1.0f)
                fraction = 1.0f;
            else if (fraction < 0.0f)
                fraction = 0.0f;
            m_tabCurrentPages[m_currentTab] = Math::round(
                fraction * static_cast<float>(s_tabSpecs[m_currentTab]->getPageCount() - 1));
            switchTab(static_cast<ECreativeInventoryTabs>(m_currentTab));
        }
    }
}

UIControl* UIScene_CreativeMenu::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_CreativeMenu::handleTouchBoxRebuild()
{
    addTimer(0, 100);
}

void UIScene_CreativeMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0)
    {
        GetMainPanel()->UpdateControl();
        gConsoleUIController.TouchBoxRebuild(this);
        killTimer(0);
    }
}

void UIScene_CreativeMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_CreativeMenu::getSceneType()
{
    return EUIScene_CreativeMenu;
}
