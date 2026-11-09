#include "game/ui/UIScene_HowToPlay.hpp"
#include "game/core/StringIds.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <new>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Font.hpp"
#include "game/ui/UIString.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/core/DyeColor.hpp"
#include "game/item/Items.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/FishingRodItem.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/PotionItem.hpp"
#include "game/item/ShearsItem.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HowToPlay.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/GrassBlock.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/StringUtils.hpp"

extern ConsoleUIController gConsoleUIController;


template <class T>
const UIScene_HowToPlay::sCustomItems SLOT(T item, int count, int auxValue)
{
    return UIScene_HowToPlay::sCustomItems(item, count, auxValue);
}

template <class T, class P>
const UIScene_HowToPlay::sCustomItems SLOT_POTION(T item, P potion, int count, int auxValue)
{
    return UIScene_HowToPlay::sCustomItems(item, potion, count, auxValue);
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Item* item, int count, int auxValue)
{
    m_item = item;
    m_potion = NULL;
    m_block = NULL;
    m_count = count;
    m_auxValue = auxValue;
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Block* block, int count, int auxValue)
    : m_block(block),
      m_item(NULL),
      m_potion(NULL),
      m_count(count),
      m_auxValue(auxValue)
{
}

UIScene_HowToPlay::sCustomItems::sCustomItems(int blockId, int count, int auxValue)
    : m_block(Block::byId(blockId)),
      m_item(NULL),
      m_potion(NULL),
      m_count(count),
      m_auxValue(auxValue)
{
}

UIScene_HowToPlay::sCustomItems::sCustomItems(Item* item, Potion* potion, int count, int auxValue)
{
    m_item = item;
    m_potion = potion;
    m_count = count;
    m_block = NULL;
    m_auxValue = auxValue;
}

not_null_ptr<ItemInstance> UIScene_HowToPlay::sCustomItems::GetItemInstance() const
{
    if (m_potion != NULL)
        return PotionUtils::setPotion(not_null_ptr<ItemInstance>(new ItemInstance(m_item, m_count, m_auxValue)),
                                      m_potion);
    if (m_item != NULL)
        return not_null_ptr<ItemInstance>(new ItemInstance(m_item, m_count, m_auxValue));
    return not_null_ptr<ItemInstance>(new ItemInstance(m_block, m_count, m_auxValue));
}

UIScene_HowToPlay::UIScene_HowToPlay(int playerId, void* data, UILayer* layer) : UIScene(playerId, layer)
{
    initialiseMovie();

    std::wstring repeated(L10N::GetString(IDS_INVENTORY));

    m_labels[5].init(UIString(L10N::GetString(IDS_FUEL)));
    m_labels[6].init(UIString(repeated));
    m_labels[7].init(UIString(L10N::GetString(IDS_INGREDIENT)));
    m_labels[8].init(UIString(L10N::GetString(IDS_FURNACE)));
    m_labels[3].init(UIString(repeated));
    m_labels[11].init(UIString(L10N::GetString(IDS_GROUPNAME_BUILDING_BLOCKS)));
    m_labels[4].init(UIString(L10N::GetString(IDS_CHEST_LARGE)));
    m_labels[1].init(UIString(repeated));
    m_labels[2].init(UIString(L10N::GetString(IDS_CHEST)));
    m_labels[0].init(UIString(repeated));
    m_labels[10].init(UIString(repeated));
    m_labels[9].init(UIString(L10N::GetString(IDS_DISPENSER)));
    m_labels[12].init(UIString(L10N::GetString(IDS_ENCHANT)));
    m_labels[13].init(UIString(repeated));
    m_labels[14].init(UIString(L10N::GetString(IDS_BREWING_STAND)));
    m_labels[15].init(UIString(repeated));
    m_labels[16].init(UIString(repeated.c_str()));

    std::wstring text(L10N::GetString(IDS_REPAIR_COST));
    text.replace(text.find(L"%d"), 2, std::wstring(L"8"));
    m_labels[17].init(UIString(text.c_str()));

    m_labels[18].init(UIString(L10N::GetString(IDS_REPAIR_AND_NAME)));
    m_labels[19].init(UIString(repeated.c_str()));
    m_labels[21].init(UIString(L10N::GetString(IDS_ITEM_EMERALD)));
    m_labels[22].init(UIString(L10N::GetString(IDS_REQUIRED_ITEMS_FOR_TRADE)));
    m_labels[24].init(UIString(L10N::GetString(IDS_CONTAINER_BEACON_PRIMARY_POWER)));
    m_labels[25].init(UIString(L10N::GetString(IDS_CONTAINER_BEACON_SECONDARY_POWER)));
    m_labels[26].init(UIString(L10N::GetString(IDS_TILE_HOPPER)));
    m_labels[27].init(UIString(repeated.c_str()));
    m_labels[28].init(UIString(L10N::GetString(IDS_TILE_DROPPER)));
    m_labels[29].init(UIString(repeated.c_str()));

    text = L10N::GetString(IDS_VILLAGER_OFFERS_ITEM);
    text = replaceAll(text, L"{*VILLAGER_TYPE*}", L10N::GetString(IDS_VILLAGER_CLERIC));
    text.replace(text.find(L"%s"), 2, L10N::GetString(IDS_TILE_LIGHT_GEM));
    m_labels[23].init(UIString(text.c_str()));

    m_pageCount = 0;
    StartPage(static_cast<EHowToPlayPage>((static_cast<unsigned int>(reinterpret_cast<uintptr_t>(data)) >> 0x10) & 0xfff));
}

namespace
{
struct HowToPlayTitle
{
    unsigned int stringId;
    int unknown0;
    int unknown1;
};

static const HowToPlayTitle kHowToPlayTitle[EHowToPlayPage_Count] = {
    {IDS_HOW_TO_PLAY_WHATSNEW, 0, 0},   {IDS_HOW_TO_PLAY_BASICS, 0, 0},       {IDS_HOW_TO_PLAY_MULTIPLAYER, 0, 0},
    {IDS_HOW_TO_PLAY_HUD, 0, 0},        {IDS_HOW_TO_PLAY_MG01, 0, 0},         {IDS_HOW_TO_PLAY_MG02, 0, 0},
    {IDS_HOW_TO_PLAY_MG03, 0, 0},       {IDS_HOW_TO_PLAY_CREATIVE, 0, 0},     {IDS_HOW_TO_PLAY_INVENTORY, 0, 0},
    {IDS_HOW_TO_PLAY_CHEST, 0, 0},      {IDS_HOW_TO_PLAY_LARGECHEST, 0, 0},   {IDS_HOW_TO_PLAY_ENDERCHEST, 0, 0},
    {IDS_HOW_TO_PLAY_CRAFTING, 0, 0},   {IDS_HOW_TO_PLAY_CRAFT_TABLE, 0, 0},  {IDS_HOW_TO_PLAY_FURNACE, 0, 0},
    {IDS_HOW_TO_PLAY_DISPENSER, 0, 0},  {IDS_HOW_TO_PLAY_BREWING, 0, 0},      {IDS_HOW_TO_PLAY_ENCHANTMENT, 0, 0},
    {IDS_HOW_TO_PLAY_ANVIL, 0, 0},      {IDS_HOW_TO_PLAY_FARMANIMALS, 0, 0},  {IDS_HOW_TO_PLAY_BREEDANIMALS, 0, 0},
    {IDS_HOW_TO_PLAY_TRADING, 0, 0},    {IDS_HOW_TO_PLAY_HORSES, 0, 0},       {IDS_HOW_TO_PLAY_BEACONS, 0, 0},
    {IDS_HOW_TO_PLAY_FIREWORKS, 0, 0},  {IDS_HOW_TO_PLAY_BANNERS, 0, 0},      {IDS_HOW_TO_PLAY_HOPPERS, 0, 0},
    {IDS_HOW_TO_PLAY_DROPPERS, 0, 0},   {IDS_HOW_TO_PLAY_NETHERPORTAL, 0, 0}, {IDS_HOW_TO_PLAY_THEEND, 0, 0},
    {IDS_HOW_TO_PLAY_ENDCITIES, 0, 0},  {IDS_HOW_TO_PLAY_ELYTRA, 0, 0},       {IDS_HOW_TO_PLAY_OCEANS, 0, 0},
    {IDS_HOW_TO_PLAY_OCEAN_MOBS, 0, 0}, {IDS_HOW_TO_PLAY_CONDUIT, 0, 0},      {IDS_HOW_TO_PLAY_HOSTOPTIONS, 0, 0}};

static const int kHowToPlayContentPage[EHowToPlayPage_Count] = {
    0x00, 0x01, 0x02, 0x03, 0x01, 0x02, 0x01, 0x04, 0x05, 0x06, 0x07, 0x17, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x15, 0x0e, 0x0f, 0x16, 0x18, 0x19, 0x01, 0x02, 0x1b, 0x1c, 0x10, 0x11, 0x01, 0x02, 0x01, 0x02, 0x01, 0x14};
}

void UIScene_HowToPlay::StartPage(EHowToPlayPage page)
{
    m_page = static_cast<int>(page);

    const wchar_t* title = L10N::GetString(static_cast<int>(kHowToPlayTitle[m_page].stringId));
    std::wstring text(title);
    text = gApp.FormatHTMLString(m_playerId, text, -1);

    text = replaceAll(text, L"{*KICK_PLAYER_DESCRIPTION*}", L10N::GetString(IDS_KICK_PLAYER_DESCRIPTION));
    text = replaceAll(text, L"{*BACK_BUTTON*}", L10N::GetString(IDS_BACK_BUTTON));
    text = replaceAll(text, L"{*DISABLES_ACHIEVEMENTS*}", L10N::GetString(IDS_HOST_OPTION_DISABLES_ACHIEVEMENTS));
    text = replaceAll(text, L"{*PLATFORM_NAME*}", L10N::GetString(IDS_PLATFORM_NAME));

    if (!gConsoleUIController.UsingBitmapFont())
    {
        text = replaceAll(text, L"\x00A9", L"(C)");
        text = replaceAll(text, L"\x00AE", L"(R)");
        text = replaceAll(text, L"\x2013", L"-");
    }

    stripWhitespaceForHtml(text, true);

    wchar_t colorTag[64];
    std::swprintf(colorTag, 0x40, L"<font color=#%08x>", gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
    std::wstring coloured(colorTag);
    coloured += text;

    std::vector<std::wstring> pages;
    std::wstring::size_type start = 0;
    std::wstring::size_type pos = coloured.find(L"\r\n");
    while (pos != std::wstring::npos)
    {
        pages.push_back(coloured.substr(start, pos - start) + L" ");
        start = pos + 2;
        pos = coloured.find(L"\r\n", start);
    }
    pages.push_back(coloured.substr(start, coloured.length() - start));

    static_cast<FJ_HowToPlay*>(m_movie->getRootNode()->getFuiNode())
        ->LoadHowToPlayPage(kHowToPlayContentPage[page], pages);

    updateTooltips();
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_HowToPlay::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_dynamicHtmlText.setupControl(this, currentNode, std::string("DynamicHtmlText"));
        m_controls.push_back(&m_dynamicHtmlText);
    }
    {
        m_container.setupControl(this, currentNode, std::string("HowToPlayContainer_13"));
        m_controls.push_back(&m_container);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_container.getFuiRenderNode();
        {
            m_enchantmentBook.setupControl(this, currentNode, std::string("iggy_EnchantmentBook"));
            m_controls.push_back(&m_enchantmentBook);
        }
        {
            m_enchantmentBooks[0].setupControl(this, currentNode, std::string("iggy_slot_Button_1"));
            m_controls.push_back(&m_enchantmentBooks[0]);
        }
        {
            m_enchantmentBooks[1].setupControl(this, currentNode, std::string("iggy_slot_Button_2"));
            m_controls.push_back(&m_enchantmentBooks[1]);
        }
        {
            m_enchantmentBooks[2].setupControl(this, currentNode, std::string("iggy_slot_Button_3"));
            m_controls.push_back(&m_enchantmentBooks[2]);
        }
        currentNode = parentNode;
    }
    {
        m_labels[8].setupControl(this, currentNode, std::string("Label1_10"));
        m_controls.push_back(&m_labels[8]);
    }
    {
        m_labels[7].setupControl(this, currentNode, std::string("Label2_10"));
        m_controls.push_back(&m_labels[7]);
    }
    {
        m_labels[5].setupControl(this, currentNode, std::string("Label3_10"));
        m_controls.push_back(&m_labels[5]);
    }
    {
        m_labels[6].setupControl(this, currentNode, std::string("Label4_10"));
        m_controls.push_back(&m_labels[6]);
    }
    {
        m_labels[4].setupControl(this, currentNode, std::string("Label1_7"));
        m_controls.push_back(&m_labels[4]);
    }
    {
        m_labels[3].setupControl(this, currentNode, std::string("Label2_7"));
        m_controls.push_back(&m_labels[3]);
    }
    {
        m_labels[11].setupControl(this, currentNode, std::string("Label1_4"));
        m_controls.push_back(&m_labels[11]);
    }
    {
        m_labels[2].setupControl(this, currentNode, std::string("Label1_6"));
        m_controls.push_back(&m_labels[2]);
    }
    {
        m_labels[1].setupControl(this, currentNode, std::string("Label2_6"));
        m_controls.push_back(&m_labels[1]);
    }
    {
        m_labels[0].setupControl(this, currentNode, std::string("Label1_5"));
        m_controls.push_back(&m_labels[0]);
    }
    {
        m_labels[9].setupControl(this, currentNode, std::string("Label1_11"));
        m_controls.push_back(&m_labels[9]);
    }
    {
        m_labels[10].setupControl(this, currentNode, std::string("Label2_11"));
        m_controls.push_back(&m_labels[10]);
    }
    {
        m_labels[12].setupControl(this, currentNode, std::string("Label1_13"));
        m_controls.push_back(&m_labels[12]);
    }
    {
        m_labels[13].setupControl(this, currentNode, std::string("Label2_13"));
        m_controls.push_back(&m_labels[13]);
    }
    {
        m_labels[14].setupControl(this, currentNode, std::string("Label1_12"));
        m_controls.push_back(&m_labels[14]);
    }
    {
        m_labels[15].setupControl(this, currentNode, std::string("Label2_12"));
        m_controls.push_back(&m_labels[15]);
    }
    {
        m_labels[23].setupControl(this, currentNode, std::string("Label1_22"));
        m_controls.push_back(&m_labels[23]);
    }
    {
        m_labels[22].setupControl(this, currentNode, std::string("Label2_22"));
        m_controls.push_back(&m_labels[22]);
    }
    {
        m_labels[19].setupControl(this, currentNode, std::string("Label3_22"));
        m_controls.push_back(&m_labels[19]);
    }
    {
        m_labels[21].setupControl(this, currentNode, std::string("Label4_22"));
        m_controls.push_back(&m_labels[21]);
    }
    {
        m_labels[18].setupControl(this, currentNode, std::string("Label1_21"));
        m_controls.push_back(&m_labels[18]);
    }
    {
        m_labels[17].setupControl(this, currentNode, std::string("Label2_21"));
        m_controls.push_back(&m_labels[17]);
    }
    {
        m_labels[16].setupControl(this, currentNode, std::string("Label3_21"));
        m_controls.push_back(&m_labels[16]);
    }
    {
        m_labels[24].setupControl(this, currentNode, std::string("Label1_25"));
        m_controls.push_back(&m_labels[24]);
    }
    {
        m_labels[25].setupControl(this, currentNode, std::string("Label2_25"));
        m_controls.push_back(&m_labels[25]);
    }
    {
        m_labels[26].setupControl(this, currentNode, std::string("Label1_27"));
        m_controls.push_back(&m_labels[26]);
    }
    {
        m_labels[27].setupControl(this, currentNode, std::string("Label2_27"));
        m_controls.push_back(&m_labels[27]);
    }
    {
        m_labels[28].setupControl(this, currentNode, std::string("Label1_28"));
        m_controls.push_back(&m_labels[28]);
    }
    {
        m_labels[29].setupControl(this, currentNode, std::string("Label2_28"));
        m_controls.push_back(&m_labels[29]);
    }
    return 1;
}

void UIScene_HowToPlay::updateTooltips()
{
    int first = -1;
    if (m_page == EHowToPlayPage_WhatsNew)
        gConsoleUIController.SetTooltips(m_playerId, IDS_HOW_TO_PLAY_NEXT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    else
    {
        if (m_page + 1 != EHowToPlayPage_Count)
            first = IDS_HOW_TO_PLAY_NEXT;
        gConsoleUIController.SetTooltips(m_playerId, first, IDS_TOOLTIPS_BACK, IDS_HOW_TO_PLAY_PREV, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    }
}

std::wstring UIScene_HowToPlay::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return L"HowToPlaySplit";
    else
        return L"HowToPlay";
}

void UIScene_HowToPlay::handleReload()
{
    static_cast<FJ_HowToPlay*>(m_movie->getRootNode()->getFuiNode())->InitScene();
    StartPage(static_cast<EHowToPlayPage>(m_page));
}

void UIScene_HowToPlay::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuA:
        if (pressed)
        {
            if (m_page + 1 != EHowToPlayPage_Count)
            {
                StartPage(static_cast<EHowToPlayPage>(m_page + 1));
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            }
            handled = true;
        }
        break;
    case EControllerActions_MenuX:
        if (pressed)
        {
            if (m_page - 1 >= m_pageCount)
            {
                StartPage(static_cast<EHowToPlayPage>(m_page - 1));
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            }
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_HowToPlay::customDrawEnchantmentFont(fuiRect* rect, int variant)
{
    unsigned int colour = gApp.GetHTMLColour(eMinecraftColour_Color_EnchantText) | 0xff000000;
    unsigned int focusColour = gApp.GetHTMLColour(eMinecraftColour_Color_EnchantTextFocus) | 0xff000000;
    unsigned int disabledColour = gApp.GetHTMLColour(eMinecraftColour_Color_EnchantTextDisabled) | 0xff000000;
    float rectWidth = rect->xMax - rect->xMin;
    float rectHeight = rect->yMax - rect->yMin;
    float width = m_enchantmentBooks[0].getWidth();
    float height = m_enchantmentBooks[0].getHeight();
    GlStateManager::scalef(rectWidth / width, rectHeight / height, 1.0F);
    float scale;
    if (m_movieResolution == 0)
    {
        scale = 3.0F;
        GlStateManager::scalef(scale, scale, scale);
    }
    else
    {
        scale = 2.0F;
        GlStateManager::scalef(scale, scale, scale);
    }

    std::wstring names[3] = {L"the elder scrolls", L"klaatu berata niktu", L"elemental spirit animal"};
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::enableAlphaTest();
    GlStateManager::alphaFunc(4, 0.1F);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (variant == 2)
        colour = disabledColour;
    else if (variant == 1)
        colour = focusColour;
    minecraft->mAlternateFont->drawWordWrap(names[variant], 0, 0, static_cast<int>(width / scale), colour,
                                            static_cast<int>(height / scale));
    GlStateManager::disableAlphaTest();
    GlStateManager::disableRescaleNormal();
}

int UIScene_HowToPlay::getSceneType()
{
    return EUIScene_HowToPlay;
}

unsigned int UIScene_HowToPlay::getSubSceneType() const
{
    return static_cast<unsigned int>(m_page);
}

const UIScene_HowToPlay::sCustomItems* SlotListHotBar()
{
    static const UIScene_HowToPlay::sCustomItems slots[10] = {
        SLOT(Items::ironSword, 1, 0), SLOT(Items::bow, 1, 0),
        SLOT(Items::arrow, 64, 0),    SLOT(Items::flintAndSteel, 1, 30),
        SLOT(Items::clock, 1, 0),     SLOT(static_cast<ShearsItem*>(Items::shears), 1, 0),
        SLOT(Items::ironAxe, 1, 50),  SLOT(static_cast<FishingRodItem*>(Items::fishingRod), 1, 0),
        SLOT(Items::compass, 1, 0),   SLOT(static_cast<MapItem*>(Items::filledMap), 1, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListInventory()
{
    static const UIScene_HowToPlay::sCustomItems slots[27] = {SLOT(Blocks::stone, 32, 0),
                                                              SLOT(Blocks::planks, 17, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(Blocks::fence_gate, 1, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(Items::arrow, 64, 0),
                                                              SLOT(Items::stick, 10, 0),
                                                              SLOT(Blocks::bookshelf, 3, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(Items::darkOakDoor, 1, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(Items::emerald, 1, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(Blocks::cobblestone_wall, 7, 0),
                                                              SLOT(Items::sign, 1, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(Blocks::stone_brick_stairs, 24, 0),
                                                              SLOT(Blocks::stone_stairs, 16, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(Blocks::log, 7, 0),
                                                              SLOT(Blocks::acacia_stairs, 15, 0),
                                                              SLOT(0, 0, 0),
                                                              SLOT(Items::diamondPickaxe, 1, 0),
                                                              SLOT(Items::paper, 21, 0),
                                                              SLOT(Items::gunpowder, 15, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListChestSmall()
{
    static const UIScene_HowToPlay::sCustomItems slots[27] = {
        SLOT(Items::experienceBottle, 64, 0),
        SLOT(static_cast<HalfSlabBlock*>(Blocks::wooden_slab), 5, 0),
        SLOT(0, 0, 0),
        SLOT(Items::glassBottle, 15, 0),
        SLOT(Blocks::enchanting_table, 1, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::wheat, 7, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::anvil, 1, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::cobblestone, 12, 0),
        SLOT(0, 0, 0),
        SLOT(Items::goldenApple, 1, 0),
        SLOT(0, 0, 0),
        SLOT(Items::stick, 12, 0),
        SLOT(0, 0, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListChestLarge()
{
    static const UIScene_HowToPlay::sCustomItems slots[54] = {
        SLOT(Blocks::glass_pane, 12, 0),
        SLOT(Items::ironDoor, 1, 0),
        SLOT(Items::pumpkinSeeds, 3, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::ironHoe, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::glass, 2, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::stoneAxe, 1, 175),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Blocks::cobblestone, 54, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::bone, 2, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(static_cast<ArmorItem*>(Items::ironHelmet), 1, 0),
        SLOT(static_cast<ArmorItem*>(Items::ironChestplate), 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::string, 3, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::woodenShovel, 1, 50),
        SLOT(Items::ironIngot, 0, 0),
        SLOT(Blocks::dirt, 6, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::leather, 2, 0),
        SLOT(0, 0, 0),
        SLOT(0, 0, 0),
        SLOT(Items::birchDoor, 6, 0),
        SLOT(0, 0, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListAnvil()
{
    static const UIScene_HowToPlay::sCustomItems slots[3] = {
        SLOT(Items::ironSword, 1, 150), SLOT(Items::ironSword, 1, 0), SLOT(Items::ironSword, 1, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListBeacon()
{
    static const UIScene_HowToPlay::sCustomItems slots[5] = {SLOT(Items::emerald, 1, 0), SLOT(Items::diamond, 1, 0),
                                                             SLOT(Items::goldIngot, 1, 0), SLOT(Items::ironIngot, 1, 0),
                                                             SLOT(Items::ironIngot, 1, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListBrewing()
{
    static const UIScene_HowToPlay::sCustomItems slots[5] = {
        SLOT(Items::fermentedSpiderEye, 1, 0),
        SLOT_POTION(static_cast<PotionItem*>(Items::potion), Potions::WATER, 1, 0),
        SLOT_POTION(static_cast<PotionItem*>(Items::potion), Potions::WATER, 1, 0),
        SLOT_POTION(static_cast<PotionItem*>(Items::potion), Potions::WATER, 1, 0), SLOT(Items::blazePowder, 3, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListDispenser()
{
    static const UIScene_HowToPlay::sCustomItems slots[9] = {SLOT(Items::arrow, 64, 0),
                                                             SLOT(Items::arrow, 64, 0),
                                                             SLOT(Items::arrow, 64, 0),
                                                             SLOT(Items::arrow, 64, 0),
                                                             SLOT(0, 0, 0),
                                                             SLOT(0, 0, 0),
                                                             SLOT(0, 0, 0),
                                                             SLOT(0, 0, 0),
                                                             SLOT(0, 0, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListEntchantment()
{
    static const UIScene_HowToPlay::sCustomItems slots[2] = {SLOT(static_cast<BowItem*>(Items::bow), 1, 0),
                                                             SLOT(Items::dye, 7, 4)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListFurnace()
{
    static const UIScene_HowToPlay::sCustomItems slots[3] = {SLOT(Blocks::iron_ore, 61, 0), SLOT(Items::coal, 63, 0),
                                                             SLOT(Items::ironIngot, 3, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListHopper()
{
    static const UIScene_HowToPlay::sCustomItems slots[5] = {SLOT(Blocks::torch, 3, 0), SLOT(Blocks::gold_block, 1, 0),
                                                             SLOT(Items::redstone, 32, 0), SLOT(Items::arrow, 32, 0),
                                                             SLOT(Items::glowstoneDust, 12, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListArmor()
{
    static const UIScene_HowToPlay::sCustomItems slots[4] = {
        SLOT(static_cast<ArmorItem*>(Items::ironHelmet), 1, 0),
        SLOT(static_cast<ArmorItem*>(Items::leatherChestplate), 1, 0), SLOT(0, 0, 0), SLOT(0, 0, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListTrading()
{
    static const UIScene_HowToPlay::sCustomItems slots[9] = {SLOT(Items::emerald, 1, 0),
                                                             SLOT(Items::emerald, 1, 0),
                                                             SLOT(Items::redstone, 2, 0),
                                                             SLOT(Items::dye, 1, DyeColor::BLUE->getItemData()),
                                                             SLOT(Items::enderEye, 1, 0),
                                                             SLOT(Blocks::glowstone, 3, 0),
                                                             SLOT(0, 0, 0),
                                                             SLOT(Items::emerald, 1, 0),
                                                             SLOT(0, 0, 0)};
    return slots;
}

const UIScene_HowToPlay::sCustomItems* SlotListCreative()
{
    static const UIScene_HowToPlay::sCustomItems slots[50] = {
        SLOT(Blocks::stone, 1, 0),
        SLOT(static_cast<GrassBlock*>(Blocks::grass), 1, 0),
        SLOT(Blocks::grass_path, 1, 0),
        SLOT(Blocks::dirt, 1, 0),
        SLOT(Blocks::cobblestone, 1, 0),
        SLOT(static_cast<SandBlock*>(Blocks::sand), 1, 0),
        SLOT(Blocks::sandstone, 1, SandStoneBlock::SANDSTONE->getData()),
        SLOT(Blocks::sandstone, 1, SandStoneBlock::SMOOTH->getData()),
        SLOT(Blocks::sandstone, 1, SandStoneBlock::CHISELED->getData()),
        SLOT(static_cast<SandBlock*>(Blocks::sand), 1, SandBlock::Variant::RED_SAND->getData()),
        SLOT(Blocks::red_sandstone, 1, SandStoneBlock::SANDSTONE->getData()),
        SLOT(Blocks::red_sandstone, 1, SandStoneBlock::SMOOTH->getData()),
        SLOT(Blocks::red_sandstone, 1, SandStoneBlock::CHISELED->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::GRANITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::SMOOTH_GRANITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::ANDESITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::SMOOTH_ANDESITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::DIORITE->getData()),
        SLOT(Blocks::stone, 1, StoneBlock::Variant::SMOOTH_DIORITE->getData()),
        SLOT(Blocks::coal_block, 1, 0),
        SLOT(Blocks::gold_block, 1, 0),
        SLOT(Blocks::iron_block, 1, 0),
        SLOT(Blocks::lapis_block, 1, 0),
        SLOT(Blocks::diamond_block, 1, 0),
        SLOT(Blocks::emerald_block, 1, 0),
        SLOT(Blocks::quartz_block, 1, 0),
        SLOT(Blocks::coal_ore, 1, 0),
        SLOT(Blocks::lapis_ore, 1, 0),
        SLOT(Blocks::diamond_ore, 1, 0),
        SLOT(Blocks::redstone_ore, 1, 0),
        SLOT(Blocks::iron_ore, 1, 0),
        SLOT(Blocks::gold_ore, 1, 0),
        SLOT(Blocks::emerald_ore, 1, 0),
        SLOT(Blocks::quartz_ore, 1, 0),
        SLOT(Blocks::nether_wart_block, 1, 0),
        SLOT(Blocks::bedrock, 1, 0),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::OAK->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::SPRUCE->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::BIRCH->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::JUNGLE->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::ACACIA->getData()),
        SLOT(Blocks::planks, 1, PlanksBlock::Variant::DARK_OAK->getData()),
        SLOT(Blocks::log, 1, PlanksBlock::Variant::OAK->getData()),
        SLOT(Blocks::log, 1, PlanksBlock::Variant::SPRUCE->getData()),
        SLOT(Blocks::log, 1, PlanksBlock::Variant::BIRCH->getData()),
        SLOT(Blocks::log, 1, PlanksBlock::Variant::JUNGLE->getData()),
        SLOT(Blocks::log2, 1, PlanksBlock::Variant::ACACIA->getData() - 4),
        SLOT(Blocks::log2, 1, PlanksBlock::Variant::DARK_OAK->getData() - 4),
        SLOT(Blocks::gravel, 1, 0),
        SLOT(Blocks::brick_block, 1, 0)};
    return slots;
}

void UIScene_HowToPlay::customDraw(const char* name, fuiRect* rect)
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int slot = -1;
    std::sscanf(name, "slot_%d", &slot);
    if (slot >= 0)
    {
        if (slot < 10 && SlotListHotBar()[slot].m_count > 0)
            item = SlotListHotBar()[slot].GetItemInstance();

        if (m_page != EHowToPlayPage_Creative && m_page != EHowToPlayPage_HUD && slot >= 9 && slot < 36
            && SlotListInventory()[slot - 9].m_count > 0)
            item = SlotListInventory()[slot - 9].GetItemInstance();

        const sCustomItems* slots = NULL;
        int index = slot - 36;
        switch (m_page)
        {
        case EHowToPlayPage_Creative:
            index = slot - 9;
            slots = SlotListCreative();
            break;
        case EHowToPlayPage_Inventory:
            slots = SlotListArmor();
            break;
        case EHowToPlayPage_Chest:
            slots = SlotListChestSmall();
            break;
        case EHowToPlayPage_LargeChest:
            slots = SlotListChestLarge();
            break;
        case EHowToPlayPage_Furnace:
            slots = SlotListFurnace();
            break;
        case EHowToPlayPage_Dispenser:
            slots = SlotListDispenser();
            break;
        case EHowToPlayPage_Brewing:
            slots = SlotListBrewing();
            break;
        case EHowToPlayPage_Enchanting:
            slots = SlotListEntchantment();
            break;
        case EHowToPlayPage_Anvil:
            slots = SlotListAnvil();
            break;
        case EHowToPlayPage_Trading:
            slots = SlotListTrading();
            break;
        case EHowToPlayPage_Beacons:
            slots = SlotListBeacon();
            break;
        case EHowToPlayPage_Hoppers:
            slots = SlotListHopper();
            break;
        case EHowToPlayPage_Droppers:
            slots = SlotListDispenser();
            break;
        default:
            break;
        }
        if (slots != NULL && index >= 0 && slots[index].m_count > 0)
            item = slots[index].GetItemInstance();
    }

    slot = -1;
    if (m_page == EHowToPlayPage_Enchanting)
    {
        if (std::strcmp(name, "EnchantmentBook") == 0)
            m_enchantmentBook.render(rect);
        std::sscanf(name, "slot_Button_%d", &slot);
        if (slot >= 0)
            customDrawEnchantmentFont(rect, slot - 1);
    }

    if (!item->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 1.0f, item->getItem()->isFoil(item), true, false);
}
