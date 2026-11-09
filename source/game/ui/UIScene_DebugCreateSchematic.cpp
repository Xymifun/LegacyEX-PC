#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/UIScene_DebugCreateSchematic.hpp"
#include "game/core/StringIds.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/pc/PcDebugSchematic.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

int UIScene_DebugCreateSchematic::s_points[2][3];
bool UIScene_DebugCreateSchematic::s_pointSet[2];
int UIScene_DebugCreateSchematic::s_rotation = 0;
int UIScene_DebugCreateSchematic::s_mirror = 0;
std::wstring UIScene_DebugCreateSchematic::s_name = L"schematic";

namespace
{
const wchar_t* const ROTATION_NAMES[4] = {L"0", L"90", L"180", L"270"};
const wchar_t* const MIRROR_NAMES[4] = {L"None", L"X", L"Z", L"X+Z"};
}

UIScene_DebugCreateSchematic::UIScene_DebugCreateSchematic(int type, void*, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();
    m_keyboardActive = false;
    PcDebugSchematicStatus(&m_statusRevision);
    const bool host = CGameNetworkManager::sInstance.IsHost();

    m_labelSetArea.init(UIString(areaText().c_str()));
    pointLabel(0, false);
    pointLabel(1, false);
    m_buttonCopy.init(UIString(host ? L"Copy" : L"Host only"), eControl_Copy);
    m_labelRotation.init(UIString(rotationText().c_str()));
    for (int i = 0; i < 4; ++i)
        m_buttonRot[i].init(UIString(ROTATION_NAMES[i]), eControl_Rot0 + i);
    m_labelMirror.init(UIString(mirrorText().c_str()));
    m_buttonMirror[0].init(UIString(MIRROR_NAMES[0]), eControl_MirNone);
    m_buttonMirror[1].init(UIString(MIRROR_NAMES[1]), eControl_MirX);
    m_buttonMirror[2].init(UIString(MIRROR_NAMES[3]), eControl_MirY);
    m_buttonMirror[3].init(UIString(MIRROR_NAMES[2]), eControl_MirZ);
    m_labelPlace.init(UIString(L"Place"));
    m_buttonPaste.init(UIString(L"Paste"), eControl_Paste);
    m_buttonUndo.init(UIString(L"Undo"), eControl_Undo);
    m_labelFile.init(UIString(L"File (schematics folder)"));
    m_textName.init(UIString(s_name.c_str()), eControl_Name);
    m_buttonSave.init(UIString(L"Save"), eControl_Save);
    m_buttonLoad.init(UIString(L"Load"), eControl_Load);
}

UIScene_DebugCreateSchematic::~UIScene_DebugCreateSchematic()
{
}

unsigned int UIScene_DebugCreateSchematic::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* node = m_movie->getRootNode();
    struct Mapping
    {
        UIControl* control;
        const char* name;
    };
    const Mapping mappings[] = {
        {&m_labelSetArea, "SetAreaTitle"},
        {&m_buttonPoint1, "Point1"},
        {&m_buttonPoint2, "Point2"},
        {&m_buttonCopy, "Copy"},
        {&m_labelRotation, "RotTitle"},
        {&m_buttonRot[0], "Rot0"},
        {&m_buttonRot[1], "Rot90"},
        {&m_buttonRot[2], "Rot180"},
        {&m_buttonRot[3], "Rot270"},
        {&m_labelMirror, "MirTitle"},
        {&m_buttonMirror[0], "MirNone"},
        {&m_buttonMirror[1], "MirX"},
        {&m_buttonMirror[2], "MirY"},
        {&m_buttonMirror[3], "MirZ"},
        {&m_labelPlace, "PlaceTitle"},
        {&m_buttonPaste, "Paste"},
        {&m_buttonUndo, "Undo"},
        {&m_labelFile, "FileOpTitle"},
        {&m_textName, "Name"},
        {&m_buttonSave, "Save"},
        {&m_buttonLoad, "ButLoad"},
    };
    for (unsigned int i = 0; i < sizeof(mappings) / sizeof(mappings[0]); ++i)
    {
        mappings[i].control->setupControl(this, node, std::string(mappings[i].name));
        m_controls.push_back(mappings[i].control);
    }
    return 1;
}

void UIScene_DebugCreateSchematic::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_DebugCreateSchematic::tick()
{
    UIScene::tick();
    unsigned int revision = 0;
    std::wstring status = PcDebugSchematicStatus(&revision);
    if (revision != m_statusRevision)
    {
        m_statusRevision = revision;
        m_labelSetArea.setLabel(UIString(status.c_str()), false, false);
    }
}

std::wstring UIScene_DebugCreateSchematic::getMoviePath()
{
    return std::wstring(L"DebugCreateSchematic");
}

void UIScene_DebugCreateSchematic::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
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
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    }
}

void UIScene_DebugCreateSchematic::targetBlock(int* x, int* y, int* z, bool* lookedAt)
{
    Minecraft* mc = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> player = mc->mLocalPlayers[m_playerId];
    *lookedAt = mc->mHitResult != NULL && mc->mHitResult->mType == 1;
    if (*lookedAt)
    {
        const BlockPos& pos = mc->mHitResult->getBlockPos();
        *x = pos.x;
        *y = pos.y;
        *z = pos.z;
    }
    else if (player != NULL)
    {
        *x = static_cast<int>(std::floor(player->mX));
        *y = static_cast<int>(std::floor(player->mY));
        *z = static_cast<int>(std::floor(player->mZ));
    }
}

void UIScene_DebugCreateSchematic::pointLabel(int index, bool live)
{
    wchar_t text[64];
    if (s_pointSet[index])
        swprintf(text, 64, L"%d: %d %d %d", index + 1, s_points[index][0], s_points[index][1], s_points[index][2]);
    else
        swprintf(text, 64, L"Point %d", index + 1);
    UIControl_Button& button = index == 0 ? m_buttonPoint1 : m_buttonPoint2;
    if (live)
        button.setLabel(UIString(text), false, false);
    else
        button.init(UIString(text), index == 0 ? eControl_Point1 : eControl_Point2);
}

std::wstring UIScene_DebugCreateSchematic::areaText()
{
    if (!s_pointSet[0] || !s_pointSet[1])
        return L"Set Area";
    wchar_t text[64];
    swprintf(text, 64, L"Set Area: %d x %d x %d", std::abs(s_points[0][0] - s_points[1][0]) + 1,
             std::abs(s_points[0][1] - s_points[1][1]) + 1, std::abs(s_points[0][2] - s_points[1][2]) + 1);
    return text;
}

std::wstring UIScene_DebugCreateSchematic::rotationText()
{
    return std::wstring(L"Rotate: ") + ROTATION_NAMES[s_rotation];
}

std::wstring UIScene_DebugCreateSchematic::mirrorText()
{
    return std::wstring(L"Mirror: ") + MIRROR_NAMES[s_mirror];
}

void UIScene_DebugCreateSchematic::handlePress(int controlId, int)
{
    const bool host = CGameNetworkManager::sInstance.IsHost();
    const int pad = gProfile->GetPrimaryPad();
    switch (controlId)
    {
    case eControl_Point1:
    case eControl_Point2:
    {
        const int index = controlId == eControl_Point1 ? 0 : 1;
        bool lookedAt = false;
        targetBlock(&s_points[index][0], &s_points[index][1], &s_points[index][2], &lookedAt);
        s_pointSet[index] = true;
        pointLabel(index, true);
        m_labelSetArea.setLabel(UIString(areaText().c_str()), false, false);
        break;
    }
    case eControl_Copy:
        if (!host)
            break;
        if (!s_pointSet[0] || !s_pointSet[1])
        {
            m_labelSetArea.setLabel(UIString(L"Set both points first"), false, false);
            break;
        }
        {
            PcSchematicCopy* copy = new PcSchematicCopy();
            copy->x0 = s_points[0][0];
            copy->y0 = s_points[0][1];
            copy->z0 = s_points[0][2];
            copy->x1 = s_points[1][0];
            copy->y1 = s_points[1][1];
            copy->z1 = s_points[1][2];
            gApp.SetXuiServerAction(pad, eXuiServerAction_SchematicCopy, copy);
        }
        break;
    case eControl_Rot0:
    case eControl_Rot90:
    case eControl_Rot180:
    case eControl_Rot270:
        s_rotation = controlId - eControl_Rot0;
        m_labelRotation.setLabel(UIString(rotationText().c_str()), false, false);
        break;
    case eControl_MirNone:
        s_mirror = 0;
        m_labelMirror.setLabel(UIString(mirrorText().c_str()), false, false);
        break;
    case eControl_MirX:
        s_mirror = 1;
        m_labelMirror.setLabel(UIString(mirrorText().c_str()), false, false);
        break;
    case eControl_MirZ:
        s_mirror = 2;
        m_labelMirror.setLabel(UIString(mirrorText().c_str()), false, false);
        break;
    case eControl_MirY:
        s_mirror = 3;
        m_labelMirror.setLabel(UIString(mirrorText().c_str()), false, false);
        break;
    case eControl_Paste:
        if (host)
        {
            PcSchematicPaste* paste = new PcSchematicPaste();
            bool lookedAt = false;
            targetBlock(&paste->x, &paste->y, &paste->z, &lookedAt);
            if (lookedAt)
                ++paste->y;
            paste->rotation = s_rotation;
            paste->mirror = s_mirror;
            gApp.SetXuiServerAction(pad, eXuiServerAction_SchematicPaste, paste);
        }
        break;
    case eControl_Undo:
        if (host)
            gApp.SetXuiServerAction(pad, eXuiServerAction_SchematicUndo, NULL);
        break;
    case eControl_Save:
        if (host)
            gApp.SetXuiServerAction(pad, eXuiServerAction_SchematicSave, new std::wstring(s_name));
        break;
    case eControl_Load:
        if (host)
            gApp.SetXuiServerAction(pad, eXuiServerAction_SchematicLoad, new std::wstring(s_name));
        break;
    case eControl_Name:
        m_keyboardActive = true;
        if (g_pInput->RequestKeyboard(L"Schematic name", m_textName.getLabel(), m_playerId, 40, KeyboardCompleteCallback,
                                      this, static_cast<C_4JInput::EKeyboardMode>(0))
            != 0)
            m_keyboardActive = false;
        break;
    }
}

int UIScene_DebugCreateSchematic::KeyboardCompleteCallback(void* data, bool accepted)
{
    UIScene_DebugCreateSchematic* scene = static_cast<UIScene_DebugCreateSchematic*>(data);
    if (scene == NULL)
        return 0;
    scene->m_keyboardActive = false;
    if (!accepted)
        return 0;
    wchar_t text[64];
    std::memset(text, 0, sizeof(text));
    g_pInput->GetText(text, 63);
    s_name = text;
    scene->m_textName.setLabel(UIString(text), false, false);
    return 0;
}

void UIScene_DebugCreateSchematic::handleDestroy()
{
    g_pInput->DestroyKeyboard();
}

int UIScene_DebugCreateSchematic::getSceneType()
{
    return EUIScene_DebugCreateSchematic;
}
