#include "game/ui/UIScene_TrialExitUpsell.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/UnityClient6.hpp"
#include "platform/wiiu/CProfile.hpp"

UIScene_TrialExitUpsell::UIScene_TrialExitUpsell(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer)
{
    initialiseMovie();
}

void UIScene_TrialExitUpsell::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, IDS_EXIT_GAME, IDS_TOOLTIPS_BACK, IDS_UNLOCK_TITLE, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_TrialExitUpsell::getMoviePath()
{
    return L"TrialExitUpsell";
}

void UIScene_TrialExitUpsell::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool&)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        navigateBack();
        break;
    case EControllerActions_MenuOk:
        if (pressed)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            gApp.ExitGame();
        }
        break;
    case EControllerActions_MenuX:
        if (gProfile->IsSignedIn(playerId))
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            telemetryManagerInstance->RecordUpsellPresented(playerId, eSen_UpsellID_FullGame, 1);
            gProfile->DisplayFullVersionPurchase(false, playerId, 1);
        }
        break;
    }
}

void UIScene_TrialExitUpsell::handleAnimationEnd()
{
    gConsoleUIController.NavigateToScene(0, EUIScene_SaveMessage, 0, EUILayer_Scene, EUIGroup_PlayerDefault);
}

int UIScene_TrialExitUpsell::getSceneType()
{
    return EUIScene_TrialExitUpsell;
}

bool UIScene_TrialExitUpsell::hasFocus(int)
{
    return m_hasFocus;
}
