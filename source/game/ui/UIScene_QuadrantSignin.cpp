#include "game/ui/UIScene_QuadrantSignin.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_QuadrantSignin.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace
{
int s_avatarImageCount = 0;

struct AvatarRequest
{
    UIScene_QuadrantSignin* scene;
    int quadrant;

    AvatarRequest(UIScene_QuadrantSignin* owner, int index) : scene(owner), quadrant(index) {}
};
}

UIScene_QuadrantSignin::UIScene_QuadrantSignin(int playerId, void* data, UILayer* layer) : UIScene(playerId, layer)
{
    initialiseMovie();
    m_signInInfo = *static_cast<SignInInfo*>(data);
    m_signInPending = false;
    _initQuadrants();
    if (g_pInput->IsCircleCrossSwapped())
    {
        static_cast<FJ_QuadrantSignin*>(m_movie->getRootNode()->getFuiNode())->SetABSwap(true);
        gApp.SetDRCIconEnabled(false);
        layer->addComponent(playerId, EUIScene_MenuBackground, NULL);
    }
}

UIScene_QuadrantSignin::~UIScene_QuadrantSignin()
{
    m_layer->removeComponent(EUIScene_MenuBackground);
}

unsigned int UIScene_QuadrantSignin::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_controllerControls[0].setupControl(this, currentNode, std::string("Controller1"));
        m_controls.push_back(&m_controllerControls[0]);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_controllerControls[0].getFuiRenderNode();
        {
            m_pressLabels[0].setupControl(this, currentNode, std::string("PressLabel"));
            m_controls.push_back(&m_pressLabels[0]);
        }
        {
            m_gamerTagLabels[0].setupControl(this, currentNode, std::string("GamerTag"));
            m_controls.push_back(&m_gamerTagLabels[0]);
        }
        {
            m_accountTypeLabels[0].setupControl(this, currentNode, std::string("AccountType"));
            m_controls.push_back(&m_accountTypeLabels[0]);
        }
        {
            m_playerNumberLabels[0].setupControl(this, currentNode, std::string("PlayerNumber"));
            m_controls.push_back(&m_playerNumberLabels[0]);
        }
        {
            m_playerPictures[0].setupControl(this, currentNode, std::string("PlayerPic"));
            m_controls.push_back(&m_playerPictures[0]);
        }
        {
            m_connectControllerLabels[0].setupControl(this, currentNode, std::string("ConnectControllerLabel"));
            m_controls.push_back(&m_connectControllerLabels[0]);
        }
        currentNode = parentNode;
    }
    {
        m_controllerControls[1].setupControl(this, currentNode, std::string("Controller2"));
        m_controls.push_back(&m_controllerControls[1]);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_controllerControls[1].getFuiRenderNode();
        {
            m_pressLabels[1].setupControl(this, currentNode, std::string("PressLabel"));
            m_controls.push_back(&m_pressLabels[1]);
        }
        {
            m_gamerTagLabels[1].setupControl(this, currentNode, std::string("GamerTag"));
            m_controls.push_back(&m_gamerTagLabels[1]);
        }
        {
            m_accountTypeLabels[1].setupControl(this, currentNode, std::string("AccountType"));
            m_controls.push_back(&m_accountTypeLabels[1]);
        }
        {
            m_playerNumberLabels[1].setupControl(this, currentNode, std::string("PlayerNumber"));
            m_controls.push_back(&m_playerNumberLabels[1]);
        }
        {
            m_playerPictures[1].setupControl(this, currentNode, std::string("PlayerPic"));
            m_controls.push_back(&m_playerPictures[1]);
        }
        {
            m_connectControllerLabels[1].setupControl(this, currentNode, std::string("ConnectControllerLabel"));
            m_controls.push_back(&m_connectControllerLabels[1]);
        }
        currentNode = parentNode;
    }
    {
        m_controllerControls[2].setupControl(this, currentNode, std::string("Controller3"));
        m_controls.push_back(&m_controllerControls[2]);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_controllerControls[2].getFuiRenderNode();
        {
            m_pressLabels[2].setupControl(this, currentNode, std::string("PressLabel"));
            m_controls.push_back(&m_pressLabels[2]);
        }
        {
            m_gamerTagLabels[2].setupControl(this, currentNode, std::string("GamerTag"));
            m_controls.push_back(&m_gamerTagLabels[2]);
        }
        {
            m_accountTypeLabels[2].setupControl(this, currentNode, std::string("AccountType"));
            m_controls.push_back(&m_accountTypeLabels[2]);
        }
        {
            m_playerNumberLabels[2].setupControl(this, currentNode, std::string("PlayerNumber"));
            m_controls.push_back(&m_playerNumberLabels[2]);
        }
        {
            m_playerPictures[2].setupControl(this, currentNode, std::string("PlayerPic"));
            m_controls.push_back(&m_playerPictures[2]);
        }
        {
            m_connectControllerLabels[2].setupControl(this, currentNode, std::string("ConnectControllerLabel"));
            m_controls.push_back(&m_connectControllerLabels[2]);
        }
        currentNode = parentNode;
    }
    {
        m_controllerControls[3].setupControl(this, currentNode, std::string("Controller4"));
        m_controls.push_back(&m_controllerControls[3]);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_controllerControls[3].getFuiRenderNode();
        {
            m_pressLabels[3].setupControl(this, currentNode, std::string("PressLabel"));
            m_controls.push_back(&m_pressLabels[3]);
        }
        {
            m_gamerTagLabels[3].setupControl(this, currentNode, std::string("GamerTag"));
            m_controls.push_back(&m_gamerTagLabels[3]);
        }
        {
            m_accountTypeLabels[3].setupControl(this, currentNode, std::string("AccountType"));
            m_controls.push_back(&m_accountTypeLabels[3]);
        }
        {
            m_playerNumberLabels[3].setupControl(this, currentNode, std::string("PlayerNumber"));
            m_controls.push_back(&m_playerNumberLabels[3]);
        }
        {
            m_playerPictures[3].setupControl(this, currentNode, std::string("PlayerPic"));
            m_controls.push_back(&m_playerPictures[3]);
        }
        {
            m_connectControllerLabels[3].setupControl(this, currentNode, std::string("ConnectControllerLabel"));
            m_controls.push_back(&m_connectControllerLabels[3]);
        }
        currentNode = parentNode;
    }
    return 1;
}

void UIScene_QuadrantSignin::setControllerState(int controller, EControllerStatus status)
{
    if (m_controllerStates[controller] == status)
        return;
    m_controllerStates[controller] = status;
    if (g_pInput->IsPadDRCController(controller))
    {
        static_cast<FJ_QuadrantSignin*>(m_movie->getRootNode()->getFuiNode())
            ->SetControllerStatus(controller, status, 0);
        return;
    }
    int controllerType = g_pInput->IsPadClassicController(controller) ? 2 : 1;
    static_cast<FJ_QuadrantSignin*>(m_movie->getRootNode()->getFuiNode())
        ->SetControllerStatus(controller, status, controllerType);
}

void UIScene_QuadrantSignin::_initQuadrants()
{
    for (int i = 0; i < 4; ++i)
    {
        m_avatarPending[i] = false;
        m_pressLabels[i].init(UIString(IDS_MUST_SIGN_IN_TITLE));
        m_connectControllerLabels[i].init(UIString(L""));
        m_accountTypeLabels[i].init(UIString(L""));
        m_controllerStates[i] = eControllerStatus_Disconnected;
        if (gProfile->IsSignedIn(i))
        {
            setControllerState(i, eControllerStatus_SignedIn);
            m_gamerTagLabels[i].init(UIString(gProfile->GetMiiName(i)));
        }
        else if (g_pInput->IsPadConnected(i))
        {
            setControllerState(i, eControllerStatus_Connected);
            m_gamerTagLabels[i].init(UIString(L""));
        }
        else
        {
            setControllerState(i, eControllerStatus_Disconnected);
        }
    }
}

void UIScene_QuadrantSignin::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_CONTINUE, IDS_TOOLTIPS_CANCEL, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, false, false);
}

bool UIScene_QuadrantSignin::hasFocus(int)
{
    return m_hasFocus;
}

bool UIScene_QuadrantSignin::hidesLowerScenes()
{
    return false;
}

int UIScene_QuadrantSignin::AvatarReturned(void* context, unsigned char* data, unsigned int size)
{
    AvatarRequest* request = static_cast<AvatarRequest*>(context);
    if (data == NULL && size != 0)
    {
        wchar_t name[32];
        std::swprintf(name, 32, L"quadrantImage%05d", s_avatarImageCount++);
        request->scene->registerSubstitutionTexture(std::wstring(name), data, size, false);
        request->scene->m_playerPictures[request->quadrant].setTextureName(std::wstring(name), true);
    }
    delete request;
    return 0;
}

void UIScene_QuadrantSignin::updateState()
{
    for (int i = 0; i < 4; ++i)
    {
        if (gProfile->IsSignedIn(i) && g_pInput->IsPadConnected(i) && !gProfile->IsSigningIn(i))
        {
            setControllerState(i, eControllerStatus_SignedIn);
            m_gamerTagLabels[i].setLabel(UIString(gProfile->GetMiiName(i)), false, false);
            if (!m_avatarPending[i] && gProfile->GetProfileAvatar(i, AvatarReturned, new AvatarRequest(this, i)))
                m_avatarPending[i] = true;
        }
        else if (g_pInput->IsPadConnected(i))
        {
            setControllerState(i, eControllerStatus_Connected);
            m_gamerTagLabels[i].setLabel(UIString(L""), false, false);
            m_avatarPending[i] = false;
        }
        else
        {
            setControllerState(i, eControllerStatus_Disconnected);
            m_avatarPending[i] = false;
        }
    }
}

void UIScene_QuadrantSignin::tick()
{
    if (m_movie != NULL)
    {
        UIScene::tick();
        updateState();
    }
}

std::wstring UIScene_QuadrantSignin::getMoviePath()
{
    return L"QuadrantSignin";
}

int UIScene_QuadrantSignin::SignInReturned(void* context, bool, int, int)
{
    UIScene_QuadrantSignin* scene = static_cast<UIScene_QuadrantSignin*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (scene != NULL)
    {
        scene->m_signInPending = false;
        scene->updateState();
    }
    return 0;
}

void UIScene_QuadrantSignin::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                         bool& handled)
{
    if (!m_signInPending)
    {
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
        switch (action)
        {
        case EControllerActions_MenuCancel:
            if (!pressed)
                break;
            if (playerId != gProfile->GetPrimaryPad())
            {
                if (gProfile->IsSignedIn(playerId))
                {
                    gProfile->SignOutUser(playerId);
                    handled = true;
                    return;
                }
                break;
            }
            m_signInPending = true;
            m_signInInfo.callback(m_signInInfo.context, false, playerId);
            gProfile->CancelProfileAvatarRequest();
            if (m_signInInfo.returnToHome)
            {
                gConsoleUIController.NavigateToHomeMenu(false);
                handled = true;
                return;
            }
            gApp.SetDRCIconEnabled(true);
            navigateBack();
            handled = true;
            return;
        case EControllerActions_MenuOk:
            if (!pressed)
                break;
            m_signInPending = true;
            if (gProfile->IsSignedIn(playerId))
            {
                gProfile->CancelProfileAvatarRequest();
                navigateBack();
                m_signInInfo.callback(m_signInInfo.context, true, m_playerId);
                handled = true;
                return;
            }
            gProfile->RequestSignInUI(false, false, false, false, true, SignInReturned,
                                      reinterpret_cast<void*>(GetCallbackUniqueId()), playerId);
            handled = true;
            return;
        case EControllerActions_MenuUp:
        case EControllerActions_MenuDown:
            if (pressed)
                sendInputToMovie(action, repeat, pressed, released);
            break;
        }
    }
    handled = true;
}

void UIScene_QuadrantSignin::handleReload()
{
    _initQuadrants();
}

int UIScene_QuadrantSignin::getSceneType()
{
    return EUIScene_QuadrantSignin;
}
