#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/UIScene_DebugSetCamera.hpp"
#include "game/core/StringIds.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace
{
std::wstring FormatValue(double value)
{
    wchar_t text[64];
    if (value > -0.005 && value < 0.005)
        value = 0.0;
    swprintf(text, sizeof(text) / sizeof(text[0]), L"%.2f", value);
    return text;
}
}

UIScene_DebugSetCamera::UIScene_DebugSetCamera(int type, void*, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();
    m_keyboardControl = -1;
    m_keyboardActive = false;
    std::memset(&m_position, 0, sizeof(m_position));
    m_position.player = type;

    Minecraft* mc = Minecraft::GetInstance();
    if (mc != NULL && mc->mLocalPlayers[type] != NULL)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = mc->mLocalPlayers[type];
        m_position.m_camX = player->mX;
        m_position.m_camY = player->mY;
        m_position.m_camZ = player->mZ;
        m_position.m_yRot = player->mYRot;
        m_position.m_elev = player->mXRot;
    }

    m_textInputX.init(UIString(FormatValue(m_position.m_camX).c_str()), eControl_CamX);
    m_textInputY.init(UIString(FormatValue(m_position.m_camY).c_str()), eControl_CamY);
    m_textInputZ.init(UIString(FormatValue(m_position.m_camZ).c_str()), eControl_CamZ);
    m_textInputYRot.init(UIString(FormatValue(m_position.m_yRot).c_str()), eControl_YRot);
    m_textInputElevation.init(UIString(FormatValue(m_position.m_elev).c_str()), eControl_Elevation);
    m_checkboxLockPlayer.init(UIString(L"Lock Player"), eControl_LockPlayer, gApp.GetFreezePlayers());
    m_buttonTeleport.init(UIString(CGameNetworkManager::sInstance.IsHost() ? L"Teleport" : L"Teleport (host only)"),
                          eControl_Teleport);
    m_labelTitle.init(UIString(L"Set Camera Position"));
    m_labelCamX.init(UIString(L"CamX"));
    m_labelCamY.init(UIString(L"CamY"));
    m_labelCamZ.init(UIString(L"CamZ"));
    m_labelYRotElev.init(UIString(L"Y-Rot & Elevation (Degs)"));
}

UIScene_DebugSetCamera::~UIScene_DebugSetCamera()
{
}

unsigned int UIScene_DebugSetCamera::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* node = m_movie->getRootNode();
    struct Mapping
    {
        UIControl* control;
        const char* name;
    };
    const Mapping mappings[] = {
        {&m_textInputX, "CamX"},
        {&m_textInputY, "CamY"},
        {&m_textInputZ, "CamZ"},
        {&m_textInputYRot, "YRot"},
        {&m_textInputElevation, "Elevation"},
        {&m_checkboxLockPlayer, "LockPlayer"},
        {&m_buttonTeleport, "Teleport"},
        {&m_labelTitle, "LabelTitle"},
        {&m_labelCamX, "LabelCamX"},
        {&m_labelCamY, "LabelCamY"},
        {&m_labelCamZ, "LabelCamZ"},
        {&m_labelYRotElev, "LabelYRotElev"},
    };
    for (unsigned int i = 0; i < sizeof(mappings) / sizeof(mappings[0]); ++i)
    {
        mappings[i].control->setupControl(this, node, std::string(mappings[i].name));
        m_controls.push_back(mappings[i].control);
    }
    return 1;
}

void UIScene_DebugSetCamera::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_DebugSetCamera::getMoviePath()
{
    return std::wstring(L"DebugSetCamera");
}

void UIScene_DebugSetCamera::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                         bool& handled)
{
    if (m_keyboardActive)
        return;
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            navigateBack();
            gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
            handled = true;
        }
        break;
    case EControllerActions_MenuOk:
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    }
}

double* UIScene_DebugSetCamera::valueFor(int control)
{
    switch (control)
    {
    case eControl_CamX:
        return &m_position.m_camX;
    case eControl_CamY:
        return &m_position.m_camY;
    case eControl_CamZ:
        return &m_position.m_camZ;
    case eControl_YRot:
        return &m_position.m_yRot;
    case eControl_Elevation:
        return &m_position.m_elev;
    }
    return NULL;
}

UIControl_TextInput* UIScene_DebugSetCamera::inputFor(int control)
{
    switch (control)
    {
    case eControl_CamX:
        return &m_textInputX;
    case eControl_CamY:
        return &m_textInputY;
    case eControl_CamZ:
        return &m_textInputZ;
    case eControl_YRot:
        return &m_textInputYRot;
    case eControl_Elevation:
        return &m_textInputElevation;
    }
    return NULL;
}

void UIScene_DebugSetCamera::handlePress(int controlId, int)
{
    switch (controlId)
    {
    case eControl_Teleport:
        if (CGameNetworkManager::sInstance.IsHost())
            gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_SetCameraLocation,
                                    new DebugSetCameraPosition(m_position));
        break;
    case eControl_CamX:
    case eControl_CamY:
    case eControl_CamZ:
    case eControl_YRot:
    case eControl_Elevation:
        m_keyboardControl = controlId;
        m_keyboardActive = true;
        if (g_pInput->RequestKeyboard(L"Enter a number", inputFor(controlId)->getLabel(), m_playerId, 25,
                                      KeyboardCompleteCallback, this, static_cast<C_4JInput::EKeyboardMode>(0))
            != 0)
            m_keyboardActive = false;
        break;
    }
}

int UIScene_DebugSetCamera::KeyboardCompleteCallback(void* data, bool accepted)
{
    UIScene_DebugSetCamera* scene = static_cast<UIScene_DebugSetCamera*>(data);
    if (scene == NULL)
        return 0;
    scene->m_keyboardActive = false;
    if (!accepted)
        return 0;
    wchar_t text[64];
    std::memset(text, 0, sizeof(text));
    g_pInput->GetText(text, 63);
    double* value = scene->valueFor(scene->m_keyboardControl);
    UIControl_TextInput* input = scene->inputFor(scene->m_keyboardControl);
    if (value == NULL || input == NULL || text[0] == 0)
        return 0;
    char buf[64];
    for (int i = 0; i < 64; ++i)
        buf[i] = text[i] < 0x80 ? static_cast<char>(text[i]) : '?';
    buf[63] = 0;
    char* end = NULL;
    const double num = std::strtod(buf, &end);
    if (end == buf)
        return 0;
    *value = num;
    input->setLabel(UIString(FormatValue(num).c_str()), false, false);
    return 0;
}

void UIScene_DebugSetCamera::handleCheckboxToggled(int id, bool checked)
{
    if (id == eControl_LockPlayer)
        gApp.SetFreezePlayers(checked);
}

void UIScene_DebugSetCamera::handleDestroy()
{
    g_pInput->DestroyKeyboard();
}

int UIScene_DebugSetCamera::getSceneType()
{
    return EUIScene_DebugSetCamera;
}
