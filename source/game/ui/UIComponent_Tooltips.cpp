#include "game/ui/UIComponent_Tooltips.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcModViews.hpp"
#endif

#include <cstring>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_ToolTips.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

extern C_4JProfile* gProfile;

UIComponent_Tooltips::_TooltipValues::_TooltipValues()
{
    m_enabled = false;
    m_textId = -1;
    m_type = -1;
    m_subtype = -1;
    m_dynamic = false;
    m_button = EControllerActions_Jump;
}

UIComponent_Tooltips::UIComponent_Tooltips(int sceneType, void* data, UILayer* layer) : UIScene(sceneType, layer)
{
    for (int pad = 0; pad < 4; ++pad)
    {
        for (int action = 0; action < 23; ++action)
            m_overrideSFX[pad][action] = false;
    }
    initialiseMovie();
    m_controllerType = -1;
    for (unsigned int i = 0; i < 16; ++i)
        m_touchControls[i].init(i);
}

void UIComponent_Tooltips::overrideSFX(int pad, int action, bool enabled)
{
    m_overrideSFX[pad][action] = enabled;
}

unsigned int UIComponent_Tooltips::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_touchControls[0].setupControl(this, root, std::string("Touch_0"));
        m_controls.push_back(&m_touchControls[0]);
    }
    {
        m_touchControls[1].setupControl(this, root, std::string("Touch_1"));
        m_controls.push_back(&m_touchControls[1]);
    }
    {
        m_touchControls[2].setupControl(this, root, std::string("Touch_2"));
        m_controls.push_back(&m_touchControls[2]);
    }
    {
        m_touchControls[3].setupControl(this, root, std::string("Touch_3"));
        m_controls.push_back(&m_touchControls[3]);
    }
    {
        m_touchControls[4].setupControl(this, root, std::string("Touch_4"));
        m_controls.push_back(&m_touchControls[4]);
    }
    {
        m_touchControls[5].setupControl(this, root, std::string("Touch_5"));
        m_controls.push_back(&m_touchControls[5]);
    }
    {
        m_touchControls[6].setupControl(this, root, std::string("Touch_6"));
        m_controls.push_back(&m_touchControls[6]);
    }
    {
        m_touchControls[7].setupControl(this, root, std::string("Touch_7"));
        m_controls.push_back(&m_touchControls[7]);
    }
    {
        m_touchControls[8].setupControl(this, root, std::string("Touch_8"));
        m_controls.push_back(&m_touchControls[8]);
    }
    {
        m_touchControls[9].setupControl(this, root, std::string("Touch_9"));
        m_controls.push_back(&m_touchControls[9]);
    }
    {
        m_touchControls[10].setupControl(this, root, std::string("Touch_10"));
        m_controls.push_back(&m_touchControls[10]);
    }
    {
        m_touchControls[11].setupControl(this, root, std::string("Touch_11"));
        m_controls.push_back(&m_touchControls[11]);
    }
    {
        m_touchControls[12].setupControl(this, root, std::string("Touch_12"));
        m_controls.push_back(&m_touchControls[12]);
    }
    {
        m_touchControls[13].setupControl(this, root, std::string("Touch_13"));
        m_controls.push_back(&m_touchControls[13]);
    }
    {
        m_touchControls[14].setupControl(this, root, std::string("Touch_14"));
        m_controls.push_back(&m_touchControls[14]);
    }
    {
        m_touchControls[15].setupControl(this, root, std::string("Touch_15"));
        m_controls.push_back(&m_touchControls[15]);
    }
    return 1;
}

std::wstring UIComponent_Tooltips::getMoviePath()
{
    switch (m_layer->getViewport())
    {
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_BottomHalf:
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_TopLeftQuarter:
    case C4JRender::eViewportType_TopRightQuarter:
    case C4JRender::eViewportType_BottomLeftQuarter:
    case C4JRender::eViewportType_BottomRightQuarter:
        m_splitTooltips = true;
        return L"ToolTipsSplit";
    default:
        m_splitTooltips = false;
        return L"ToolTips";
    }
}

double UIComponent_Tooltips::getSafeZoneHalfWidth()
{
    float screenWidth = gConsoleUIController.getScreenWidth();
    float halfWidth;
    if (gRenderer->IsHiDef() == 0)
        halfWidth = m_stageWidth * 0.075f;
    else
        halfWidth = screenWidth * 0.05f;
    return halfWidth;
}

void UIComponent_Tooltips::updateSafeZone()
{
    double top = 0.0;
    double bottom = 0.0;
    double left = 0.0;
    double right = 0.0;

    switch (m_layer->getViewport())
    {
    case 3:
        left = getSafeZoneHalfWidth();
        bottom = getSafeZoneHalfHeight();
        break;
    case 4:
        right = getSafeZoneHalfWidth();
        bottom = getSafeZoneHalfHeight();
        break;
    case 1:
    case 5:
        top = getSafeZoneHalfHeight();
        left = getSafeZoneHalfWidth();
        break;
    case 6:
        top = getSafeZoneHalfHeight();
        right = getSafeZoneHalfWidth();
        break;
    case 2:
    case 7:
        bottom = getSafeZoneHalfHeight();
        left = getSafeZoneHalfWidth();
        break;
    case 8:
        bottom = getSafeZoneHalfHeight();
        right = getSafeZoneHalfWidth();
        break;
    default:
    case 0:
        top = getSafeZoneHalfHeight();
        bottom = getSafeZoneHalfHeight();
        left = getSafeZoneHalfWidth();
        right = getSafeZoneHalfWidth();
        break;
    }

    setSafeZone(top, bottom, left, right);
}

void UIComponent_Tooltips::_SetTooltip(unsigned int index, int type, int subtype, UIString text, bool enabled,
                                       bool force)
{
    _TooltipValues& value = m_tooltipValues[index];
    if (force || enabled || value.m_enabled)
    {
        int finalType = type;
        int finalSubtype = subtype;
        if (subtype >= 0)
        {
            finalType = subtype;
            finalSubtype = type;
        }
        value.m_enabled = enabled;
        value.m_text = text;
        const std::wstring& textString = text.getString();
        static_cast<FJ_ToolTips*>(m_movie->getRootNode()->getFuiNode())
            ->SetToolTip((int)index, finalType, finalSubtype, textString, enabled);
    }
}

void UIComponent_Tooltips::_Relayout()
{
    FJ_ToolTips* toolTips = static_cast<FJ_ToolTips*>(m_movie->getRootNode()->getFuiNode());
    toolTips->UpdateLayout();
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIComponent_Tooltips::tick()
{
    UIScene::tick();

    if (gProfile->GetPrimaryPad() >= 0)
    {
        unsigned int level = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_InterfaceOpacity);
        if (level < 80)
        {
            int playerId = m_playerId;
            if (gConsoleUIController.GetMenuDisplayed(playerId) != 0 && level < 15)
                level = 15;
            unsigned int timer = gApp.GetOpacityTimer(playerId);
            if (timer == 0)
            {
                setOpacity(0.01f * (float)level);
            }
            else if (timer < 10)
            {
                float fade = 80.0f - (10.0f - (float)timer) * ((80.0f - (float)level) / 10.0f);
                setOpacity(0.01f * fade);
            }
            else
            {
                setOpacity(0.01f * 80.0f);
            }
        }
        else
        {
            setOpacity(0.01f * (float)level);
        }
    }

    bool changed = false;
    int i;
    for (i = 0; i < 16; ++i)
    {
        if (gConsoleUIController.IsReloadingSkin())
            continue;
        _TooltipValues& value = m_tooltipValues[i];
        if (value.m_enabled && value.m_text.needsUpdating())
        {
            changed = true;
            _SetTooltip((unsigned int)i, value.m_type, value.m_subtype, value.m_text, value.m_enabled, true);
            value.m_text.setUpdated();
        }
    }
    if (changed)
        _Relayout();
}

void UIComponent_Tooltips::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (gProfile->GetLockedProfile() != -1)
    {
        int playerId = m_playerId;
        if (gConsoleUIController.GetMenuDisplayed(playerId) == 0)
        {
            if (GameSettings::GetGameSettings(playerId, eGameSetting_Tooltips) == 0)
                return;
            if (GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayHUD) == 0)
                return;
#ifdef MINECRAFT_PC
            if (PcModViewsHideHints())
                return;
#endif
        }
    }

    if (m_splitTooltips)
    {
        int x = 0;
        int y = 0;
        switch (viewport)
        {
        case C4JRender::eViewportType_BottomHalf:
        case C4JRender::eViewportType_BottomLeftQuarter:
            y = static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f);
            break;
        case C4JRender::eViewportType_RightHalf:
        case C4JRender::eViewportType_TopRightQuarter:
            x = static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f);
            break;
        case C4JRender::eViewportType_BottomRightQuarter:
            x = static_cast<int>(gConsoleUIController.getScreenWidth() * 0.5f);
            y = static_cast<int>(gConsoleUIController.getScreenHeight() * 0.5f);
            break;
        }
        gConsoleUIController.setupRenderPosition(x, y);

        int top = 0;
        switch (viewport)
        {
        case C4JRender::eViewportType_TopHalf:
        case C4JRender::eViewportType_BottomHalf:
            gFui->render(m_movie, 0.0f, m_stageHeight / 2, static_cast<int>(gConsoleUIController.getScreenWidth()),
                         height);
            return;
        case C4JRender::eViewportType_LeftHalf:
        case C4JRender::eViewportType_RightHalf:
            gFui->render(m_movie, 0.0f, 0, width, static_cast<int>(gConsoleUIController.getScreenHeight()));
            return;
        case C4JRender::eViewportType_FullScreen:
            break;
        case C4JRender::eViewportType_TopLeftQuarter:
        case C4JRender::eViewportType_TopRightQuarter:
        case C4JRender::eViewportType_BottomLeftQuarter:
        case C4JRender::eViewportType_BottomRightQuarter:
            top = m_stageHeight / 2;
            break;
        default:
            gFui->render(m_movie, 0.0f, 0, width, height);
            return;
        }
        gFui->render(m_movie, 0.0f, top, width, height);
        return;
    }

    UIScene::render(width, height, viewport);
}

void UIComponent_Tooltips::updateViewportTouchOffset()
{
    int x = 0;
    int y = 0;
    switch (m_layer->getViewport())
    {
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_TopLeftQuarter:
        y = -(int)(gConsoleUIController.getScreenHeight() * 0.5f);
        break;
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_BottomRightQuarter:
        x = (int)(gConsoleUIController.getScreenWidth() * 0.5f);
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        y = -(int)(gConsoleUIController.getScreenHeight() * 0.5f);
        x = (int)(gConsoleUIController.getScreenWidth() * 0.5f);
        break;
    }
    gConsoleUIController.updateViewportTouchOffset(x, y);
}

unsigned int UIComponent_Tooltips::_SetTooltip(unsigned int index, int value)
{
    _TooltipValues& slot = m_tooltipValues[index];
    unsigned int result = 0;
    if (value == slot.m_textId)
    {
        if (value == -1)
            return result;
        if (slot.m_enabled)
            return result;
    }

    int type = (int)index;
    if (g_pInput->IsCircleCrossSwapped())
    {
        if (index == 0)
            type = 1;
        else if (index == 1)
            type = 0;
    }

    slot.m_textId = value;
    slot.m_type = type;
    slot.m_subtype = -1;
    slot.m_dynamic = false;
    if (value == -1)
        _SetTooltip(index, type, -1, UIString(L""), false, false);
    else
        _SetTooltip(index, type, -1, UIString(value), true, false);
    result = 1;
    return result;
}

void UIComponent_Tooltips::SetTooltipText(unsigned int player, int index)
{
    if (_SetTooltip(player, index) != 0)
        _Relayout();
}

void UIComponent_Tooltips::SetEnableTooltips(bool enabled)
{
}

void UIComponent_Tooltips::ShowTooltip(unsigned int player, bool visible)
{
    if (visible != m_tooltipValues[player].m_enabled)
    {
        _SetTooltip(player, (int)player, -1, UIString(L""), visible, false);
        _Relayout();
    }
}

unsigned int UIComponent_Tooltips::GetToolTipButton(unsigned int button)
{
    switch (button)
    {
    case 1:
        return 0;
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    case 0x1000:
        return 11;
    case 0x2000:
        return 12;
    case 0x400:
        return 13;
    case 0x800:
        return 14;
    case 0x40:
        return 7;
    case 0x80:
        return 6;
    case 0x400000:
        return 5;
    case 0x800000:
        return 4;
    case 0x100:
        return 9;
    case 0x200:
        return 8;
    default:
        return 0xffffffff;
    }
}

unsigned int UIComponent_Tooltips::_SetTooltip(unsigned int index, int type, int subtype, int textId, bool& changed)
{
    unsigned int result = 0;
    if (type == -1 || textId == -1)
    {
        changed = false;
    }
    else
    {
        changed = true;
        _TooltipValues& value = m_tooltipValues[index];
        if (value.m_textId != textId || !value.m_enabled)
        {
            value.m_type = type;
            result = 1;
            value.m_subtype = subtype;
            value.m_dynamic = true;
            value.m_textId = textId;
            _SetTooltip(index, type, subtype, UIString(textId), true, false);
        }
    }
    return result;
}

unsigned int UIComponent_Tooltips::HandleToolTipButton(unsigned int player, EControllerActions action,
                                                       unsigned int& button, int type)
{
    unsigned int result = 0;
    bool changed = false;
    unsigned int primary = GetToolTipButton(
        static_cast<unsigned int>(GameSettings::GetInputButton((int)player, false, action)));
    unsigned int secondary = GetToolTipButton(
        static_cast<unsigned int>(GameSettings::GetInputButton((int)player, true, action)));

    unsigned int index = button;
    if (index < 16)
    {
        result = _SetTooltip(index, (int)primary, (int)secondary, type, changed);
        index = button;
        _TooltipValues& value = m_tooltipValues[index];
        if (value.m_dynamic && value.m_enabled)
            value.m_button = action;
    }
    if (changed)
        button = index + 1;
    return result;
}

void UIComponent_Tooltips::UpdateControllerType(int controllerType)
{
    unsigned int previousType = m_controllerType;

    if (g_pInput->IsPadConnected(4) == 0)
    {
        if (g_pInput->IsPadClassicController(controllerType) != 0)
        {
            m_controllerType = 2;
            if (previousType == 2)
                return;
        }
        else
        {
            m_controllerType = 0;
            if (previousType == 0)
                return;
        }
    }
    else if (g_pInput->IsPadDRCController(controllerType) != 0 || controllerType == 4)
    {
        m_controllerType = 1;
        if (previousType == 1)
            return;
    }
    else if (g_pInput->IsPadClassicController(controllerType) != 0)
    {
        m_controllerType = 2;
        if (previousType == 2)
            return;
    }
    else
    {
        m_controllerType = 0;
        if (previousType == 0)
            return;
    }

    static_cast<FJ_ToolTips*>(m_movie->getRootNode()->getFuiNode())->UpdateControllerType(m_controllerType);
}

void UIComponent_Tooltips::SetTooltips(int player, int* tooltips, bool visible)
{
    unsigned int button = 0;
    bool changed = false;
    changed = HandleToolTipButton(player, EControllerActions_Jump, button,
                                  tooltips[EControllerActions_Jump - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_Drop, button,
                                  tooltips[EControllerActions_Drop - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_Inventory, button,
                                  tooltips[EControllerActions_Inventory - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_Crafting, button,
                                  tooltips[EControllerActions_Crafting - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_Use, button,
                                  tooltips[EControllerActions_Use - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_Action, button,
                                  tooltips[EControllerActions_Action - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_Sneak, button,
                                  tooltips[EControllerActions_Sneak - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_Sprint, button,
                                  tooltips[EControllerActions_Sprint - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_PickBlock, button,
                                  tooltips[EControllerActions_PickBlock - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_CycleHeldItemLeft, button,
                                  tooltips[EControllerActions_CycleHeldItemLeft - EControllerActions_Jump])
              || changed;
#ifdef MINECRAFT_PC
    const bool ghostToggleLast = PcInputGlideKeys();
    if (!ghostToggleLast)
        changed = HandleToolTipButton(player, EControllerActions_CycleHeldItemRight, button,
                                      tooltips[EControllerActions_CycleHeldItemRight - EControllerActions_Jump])
                  || changed;
#else
    changed = HandleToolTipButton(player, EControllerActions_CycleHeldItemRight, button,
                                  tooltips[EControllerActions_CycleHeldItemRight - EControllerActions_Jump])
              || changed;
#endif
    changed = HandleToolTipButton(player, EControllerActions_ChangeCameraMode, button,
                                  tooltips[EControllerActions_ChangeCameraMode - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_FlyLeft, button,
                                  tooltips[EControllerActions_FlyLeft - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_FlyRight, button,
                                  tooltips[EControllerActions_FlyRight - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_FlyUp, button,
                                  tooltips[EControllerActions_FlyUp - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_FlyDown, button,
                                  tooltips[EControllerActions_FlyDown - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_SpectateCancel, button,
                                  tooltips[EControllerActions_SpectateCancel - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_SpectateNoise, button,
                                  tooltips[EControllerActions_SpectateNoise - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_SpectatePrevious, button,
                                  tooltips[EControllerActions_SpectatePrevious - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_SpectateNext, button,
                                  tooltips[EControllerActions_SpectateNext - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_ConfirmReady, button,
                                  tooltips[EControllerActions_ConfirmReady - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_Vote, button,
                                  tooltips[EControllerActions_Vote - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_LookBehind, button,
                                  tooltips[EControllerActions_LookBehind - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_RestartSection, button,
                                  tooltips[EControllerActions_RestartSection - EControllerActions_Jump])
              || changed;
    changed = HandleToolTipButton(player, EControllerActions_RestartRace, button,
                                  tooltips[EControllerActions_RestartRace - EControllerActions_Jump])
              || changed;
#ifdef MINECRAFT_PC
    if (ghostToggleLast)
        changed = HandleToolTipButton(player, EControllerActions_CycleHeldItemRight, button,
                                      tooltips[EControllerActions_CycleHeldItemRight - EControllerActions_Jump])
                  || changed;
#endif
    for (unsigned int index = button; index < 16; ++index)
    {
        if (m_tooltipValues[index].m_enabled)
        {
            changed = true;
            _SetTooltip(index, -1, -1, UIString(L""), false, false);
        }
    }
    if (changed)
        _Relayout();
    if (player >= 0)
        UpdateControllerType(player);
}

void UIComponent_Tooltips::SetTooltips(int player, int first, int second, int third, int fourth, int fifth, int sixth,
                                       int seventh, int eighth, int ninth, int tenth, int eleventh, int twelfth,
                                       int thirteenth, int fourteenth, int fifteenth, int sixteenth, bool visible)
{
    bool changed = false;
    changed = _SetTooltip(0, first) || changed;
    changed = _SetTooltip(1, second) || changed;
    changed = _SetTooltip(2, third) || changed;
    changed = _SetTooltip(3, fourth) || changed;
    changed = _SetTooltip(4, fifth) || changed;
    changed = _SetTooltip(5, sixth) || changed;
    changed = _SetTooltip(6, seventh) || changed;
    changed = _SetTooltip(7, eighth) || changed;
    changed = _SetTooltip(8, ninth) || changed;
    changed = _SetTooltip(9, tenth) || changed;
    changed = _SetTooltip(9, tenth) || changed;
    changed = _SetTooltip(10, eleventh) || changed;
    changed = _SetTooltip(11, twelfth) || changed;
    changed = _SetTooltip(12, thirteenth) || changed;
    changed = _SetTooltip(13, fourteenth) || changed;
    changed = _SetTooltip(14, fifteenth) || changed;
    changed = _SetTooltip(15, sixteenth) || changed;
    if (changed)
        _Relayout();
    if (player >= 0)
        UpdateControllerType(player);
}

void UIComponent_Tooltips::EnableTooltip(unsigned int player, bool enabled)
{
}

bool UIComponent_Tooltips::IsToolTipDynamic(unsigned int player, EControllerActions& action)
{
    if (player < 16)
    {
        action = m_tooltipValues[player].m_button;
        return m_tooltipValues[player].m_dynamic;
    }
    return false;
}

void UIComponent_Tooltips::handleReload()
{
    m_controllerType = -1;
    for (unsigned int i = 0; i < 16; ++i)
    {
        _SetTooltip(i, m_tooltipValues[i].m_type, m_tooltipValues[i].m_subtype, UIString(m_tooltipValues[i].m_textId),
                    m_tooltipValues[i].m_enabled, true);
    }
    _Relayout();
}

void UIComponent_Tooltips::handleInput(int player, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if ((unsigned int)action >= EControllerActions_MenuStickPress)
    {
        if ((unsigned int)action < EControllerActions_MenuOk)
            return;
        if ((unsigned int)action > EControllerActions_MenuCancel)
            return;
    }
    sendInputToMovie(action, repeat, pressed, released);
}

void UIComponent_Tooltips::handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                            bool repeat, bool released)
{
    if (gProfile->GetPrimaryPad() == -1)
        return;

    if (gConsoleUIController.GetMenuDisplayed(gProfile->GetPrimaryPad()) == 0)
    {
        if (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_Tooltips) == 0)
            return;
    }

    int player = (int)playerId;
    if (player == 4)
        player = 0;
    unsigned int settingsValue = GameSettings::GetGameSettings(player, eGameSetting_ControlScheme);

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        const GameType* gameType = Minecraft::GetInstance()->mLocalPlayers[player]->GetGameType();
        if (gameType != GameType::NOT_SET && repeat && touchBox == 3)
            g_pInput->MapTouchInput(player, 8);
    }

    if (!released)
        return;

    EControllerActions dynamicAction;
    if (gConsoleUIController.IsToolTipDynamic((unsigned int)player, (unsigned int)touchBox, dynamicAction))
    {
        unsigned int primary = (unsigned int)GameSettings::GetInputButton(player, false, dynamicAction);
        unsigned int secondary = (unsigned int)GameSettings::GetInputButton(player, true, dynamicAction);
        g_pInput->MapTouchInput(player, primary | secondary);
        return;
    }

    switch (touchBox)
    {
    case 0:
        if (settingsValue != 2 || gConsoleUIController.GetMenuDisplayed(player))
        {
            if (g_pInput->IsCircleCrossSwapped())
                g_pInput->MapTouchInput(player, 2);
            else
                g_pInput->MapTouchInput(player, 1);
            return;
        }
        break;
    case 1:
        if (g_pInput->IsCircleCrossSwapped())
            g_pInput->MapTouchInput(player, 1);
        else
            g_pInput->MapTouchInput(player, 2);
        return;
    case 2:
        g_pInput->MapTouchInput(player, 4);
        return;
    case 3:
        g_pInput->MapTouchInput(player, 8);
        return;
    case 4:
        if (gConsoleUIController.GetMenuDisplayed(player))
        {
            g_pInput->MapTouchInput(player, 0x800000);
            return;
        }
        break;
    case 5:
        if (gConsoleUIController.GetMenuDisplayed(player))
        {
            g_pInput->MapTouchInput(player, 0x400000);
            return;
        }
        break;
    case 6:
        if (settingsValue != 1 || gConsoleUIController.GetMenuDisplayed(player))
        {
            g_pInput->MapTouchInput(player, 0x80);
            return;
        }
        break;
    case 7:
        if ((settingsValue != 1 && settingsValue != 2) || gConsoleUIController.GetMenuDisplayed(player))
        {
            g_pInput->MapTouchInput(player, 0x40);
            return;
        }
        break;
    case 8:
        break;
    case 9:
        g_pInput->MapTouchInput(player, 0x100000);
        return;
    case 10:
        g_pInput->MapTouchInput(player, 0x20);
        return;
    }
}

int UIComponent_Tooltips::getSceneType()
{
    return EUIScene_Tooltips;
}

bool UIComponent_Tooltips::stealsFocus()
{
    return false;
}

bool UIComponent_Tooltips::hasFocus(int sceneType)
{
    return false;
}

bool UIComponent_Tooltips::hidesLowerScenes()
{
    return false;
}
