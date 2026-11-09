#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/UIScene_HowToPlayMenu.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIScene_HowToPlay.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/core/GameSettings.hpp"

#include <stdint.h>

extern ConsoleUIController gConsoleUIController;

unsigned int UIScene_HowToPlayMenu::m_buttonLabels[33] = {
    IDS_HOW_TO_PLAY_MENU_WHATSNEW,   IDS_HOW_TO_PLAY_MENU_BASICS,       IDS_HOW_TO_PLAY_MENU_MULTIPLAYER,
    IDS_HOW_TO_PLAY_MENU_HUD,        IDS_HOW_TO_PLAY_MENU_MG01,         IDS_HOW_TO_PLAY_MENU_MG02,
    IDS_HOW_TO_PLAY_MENU_MG03,       IDS_HOW_TO_PLAY_MENU_CREATIVE,     IDS_HOW_TO_PLAY_MENU_INVENTORY,
    IDS_HOW_TO_PLAY_MENU_CHESTS,     IDS_HOW_TO_PLAY_MENU_CRAFTING,     IDS_HOW_TO_PLAY_MENU_FURNACE,
    IDS_HOW_TO_PLAY_MENU_DISPENSER,  IDS_HOW_TO_PLAY_MENU_BREWING,      IDS_HOW_TO_PLAY_MENU_ENCHANTMENT,
    IDS_HOW_TO_PLAY_MENU_ANVIL,      IDS_HOW_TO_PLAY_MENU_FARMANIMALS,  IDS_HOW_TO_PLAY_MENU_BREEDANIMALS,
    IDS_HOW_TO_PLAY_MENU_TRADING,    IDS_HOW_TO_PLAY_MENU_HORSES,       IDS_HOW_TO_PLAY_MENU_BEACONS,
    IDS_HOW_TO_PLAY_MENU_FIREWORKS,  IDS_HOW_TO_PLAY_MENU_BANNERS,      IDS_HOW_TO_PLAY_MENU_HOPPERS,
    IDS_HOW_TO_PLAY_MENU_DROPPERS,   IDS_HOW_TO_PLAY_MENU_NETHERPORTAL, IDS_HOW_TO_PLAY_MENU_THEEND,
    IDS_HOW_TO_PLAY_MENU_ENDCITIES,  IDS_HOW_TO_PLAY_MENU_ELYTRA,       IDS_HOW_TO_PLAY_MENU_OCEANS,
    IDS_HOW_TO_PLAY_MENU_OCEAN_MOBS, IDS_HOW_TO_PLAY_MENU_CONDUIT,      IDS_HOW_TO_PLAY_MENU_HOSTOPTIONS};

static const EHowToPlayPage s_howToPlayMenuPages[33] = {
    EHowToPlayPage_WhatsNew,      EHowToPlayPage_Basics,         EHowToPlayPage_Multiplayer,
    EHowToPlayPage_HUD,           EHowToPlayPage_BattleMiniGame, EHowToPlayPage_TumbleMiniGame,
    EHowToPlayPage_GlideMiniGame, EHowToPlayPage_Creative,       EHowToPlayPage_Inventory,
    EHowToPlayPage_Chest,         EHowToPlayPage_Crafting,       EHowToPlayPage_Furnace,
    EHowToPlayPage_Dispenser,     EHowToPlayPage_Brewing,        EHowToPlayPage_Enchanting,
    EHowToPlayPage_Anvil,         EHowToPlayPage_FarmingAnimals, EHowToPlayPage_BreedingAnimals,
    EHowToPlayPage_Trading,       EHowToPlayPage_Horses,         EHowToPlayPage_Beacons,
    EHowToPlayPage_Fireworks,     EHowToPlayPage_Banners,        EHowToPlayPage_Hoppers,
    EHowToPlayPage_Droppers,      EHowToPlayPage_NetherPortal,   EHowToPlayPage_TheEnd,
    EHowToPlayPage_EndCities,     EHowToPlayPage_Elytra,         EHowToPlayPage_Oceans,
    EHowToPlayPage_OceanMobs,     EHowToPlayPage_Conduit,        EHowToPlayPage_HostOptions};

UIScene_HowToPlayMenu::UIScene_HowToPlayMenu(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_buttonList.init(0);
    int itemId = 0;
    for (int i = 0; i < 33; ++i)
        m_buttonList.addItem(L10N::GetString(m_buttonLabels[i]), itemId++);
    doHorizontalResizeCheck();
    m_selectedItem = 0;
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_HowToPlayMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_HowToPlayMenu::updateComponents()
{
    bool showSecond;
    bool noLevel = Minecraft::GetInstance()->mLevel == NULL;
    if (noLevel)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        showSecond = true;
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        showSecond = false;
        if (gApp.GetLocalPlayerCount() == 1)
            showSecond = true;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showSecond);
}

void UIScene_HowToPlayMenu::handleReload()
{
    int itemId = 0;
    for (int i = 0; i < 33; ++i)
        m_buttonList.addItem(L10N::GetString(m_buttonLabels[i]), itemId++);
    m_buttonList.setCurrentSelection(m_selectedItem, false, true);
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
}

std::wstring UIScene_HowToPlayMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"HowToPlayMenuSplit") : std::wstring(L"HowToPlayMenu");
}

void UIScene_HowToPlayMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
            navigateBack();
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_HowToPlayMenu::handlePress(int controlId, int childId)
{
    if (controlId == 0)
    {
        m_selectedItem = childId;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        const uint32_t sceneData = (s_howToPlayMenuPages[childId] << 16) | 0x80000000u
                                   | static_cast<int16_t>(m_playerId);
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_HowToPlay,
                                             reinterpret_cast<void*>(static_cast<uintptr_t>(sceneData)), EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
    }
}

unsigned int UIScene_HowToPlayMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_buttonList.setupControl(this, currentNode, std::string("HowToList"));
        m_controls.push_back(&m_buttonList);
    }
    return 1;
}

int UIScene_HowToPlayMenu::getSceneType()
{
    return EUIScene_HowToPlayMenu;
}
