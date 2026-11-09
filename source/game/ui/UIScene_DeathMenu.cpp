#include "game/ui/UIScene_DeathMenu.hpp"
#include "game/core/StringIds.hpp"

#include <string>
#include <vector>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Gui.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/GameSettings.hpp"

UIScene_DeathMenu::UIScene_DeathMenu(int playerId, void*, UILayer* layer) : UIScene(playerId, layer)
{
    initialiseMovie();

    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
    {
        m_deathMenuActive = true;
        m_exitButton.setVisible(false);
        m_respawnButton.setVisible(false);
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
            addTimer(0, 1000);
        else
            addTimer(0, 2000);
    }
    else
    {
        m_deathMenuActive = false;
        m_exitButton.init(UIString(L10N::GetString(IDS_EXIT_GAME)), 1);
        m_respawnButton.init(UIString(L10N::GetString(IDS_RESPAWN)), 0);
        m_exitButton.setEnable(false, true);
        m_respawnButton.setEnable(false, true);
        addTimer(1, 1000);
    }

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        m_titleLabel.setLabel(UIString(L10N::GetString(IDS_MG03_YOU_CRASHED)), false, false);
    else
        m_titleLabel.setLabel(UIString(L10N::GetString(IDS_YOU_DIED)), false, false);
    Minecraft* minecraft = Minecraft::GetInstance();
    Gui* gui = minecraft->mGui;

    {
        std::wstring deathMessage = !Minecraft::InMiniGame(eMiniGameId_Tumble, true)
                                            && !Minecraft::InMiniGame(eMiniGameId_Glide, true)
                                        ? gui->mNowPlaying[playerId]
                                        : std::wstring(L"");
        m_deathMessageLabel.setLabel(UIString(deathMessage), false, false);
        m_unknownStateA = false;
        m_unknownStateB = false;
    }

    std::wstring extraMessage(L"");
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && Minecraft::GetInstance()->GetMiniGame()->GetLivesPerRound() > 0)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerId];
        if (!player->isSpectator())
            extraMessage = std::wstring(gui->mSingleMessages[playerId]);
    }
    m_extraMessageLabel.setLabel(UIString(extraMessage), false, false);

    Minecraft* currentMinecraft = Minecraft::GetInstance();
    if (minecraft != NULL && currentMinecraft->mGameModes[playerId] != NULL)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[playerId]->getTutorial();
        if (tutorial != NULL)
            tutorial->showTutorialPopup(false);
    }
}

UIScene_DeathMenu::~UIScene_DeathMenu()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    ClientMasterGameMode* gameMode = minecraft->GetClientMasterGameMode();
    if (gameMode != NULL && gameMode->IsRoundRestarting() && hasTimer(0))
        handleTimerComplete(0);

    if (minecraft != NULL && minecraft->mGameModes[m_playerId] != NULL)
    {
        MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[m_playerId];
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != NULL)
            tutorial->showTutorialPopup(true);
    }
}

unsigned int UIScene_DeathMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_respawnButton.setupControl(this, currentNode, std::string("Respawn"));
        m_controls.push_back(&m_respawnButton);
    }
    {
        m_exitButton.setupControl(this, currentNode, std::string("ExitGame"));
        m_controls.push_back(&m_exitButton);
    }
    {
        m_titleLabel.setupControl(this, currentNode, std::string("Title"));
        m_controls.push_back(&m_titleLabel);
    }
    {
        m_deathMessageLabel.setupControl(this, currentNode, std::string("DeathMessage"));
        m_controls.push_back(&m_deathMessageLabel);
    }
    {
        m_extraMessageLabel.setupControl(this, currentNode, std::string("ExtraMessage"));
        m_controls.push_back(&m_extraMessageLabel);
    }
    return 1;
}

void UIScene_DeathMenu::handleTimerComplete(int timerId)
{
    switch (timerId)
    {
    case 0:
        gApp.SetAction(m_playerId, eXuiAction_Respawn, NULL);
        killTimer(0);
    case 1:
        m_exitButton.setEnable(true, true);
        m_respawnButton.setEnable(true, true);
        break;
    }
}

void UIScene_DeathMenu::updateTooltips()
{
    int tooltip = IDS_TOOLTIPS_SELECT;
    int timerId;
    int timerLength;
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
        tooltip = -1;
    gConsoleUIController.SetTooltips(m_playerId, tooltip, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, false);
}

std::wstring UIScene_DeathMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"DeathMenuSplit");
    return std::wstring(L"DeathMenu");
}

void UIScene_DeathMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_deathMenuActive)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    case EControllerActions_MenuCancel:
        handled = true;
        break;
    }
}

void UIScene_DeathMenu::tick()
{
    UIScene::tick();
}

void UIScene_DeathMenu::handlePress(int controlId, int)
{
    switch (controlId)
    {
    case 0:
        m_deathMenuActive = true;
        gApp.SetAction(m_playerId, eXuiAction_Respawn, NULL);
        break;
    case 1:
        telemetryManagerInstance->RecordLevelExit(m_playerId, eSen_LevelExitStatus_Quit);
        unsigned int callbackId = GetCallbackUniqueId();
        UIScene_PauseMenu::OpenExitGameDialog(
            m_playerId, reinterpret_cast<void*>(callbackId), UIScene_PauseMenu::ExitGameDialogReturned,
            UIScene_PauseMenu::ExitGameSaveDialogReturned, UIScene_PauseMenu::EOpenExitGameDialog_ExitGame);
        break;
    }
}

int UIScene_DeathMenu::getSceneType()
{
    return EUIScene_DeathMenu;
}
