#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/UIScene_DebugFillArea.hpp"
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
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace
{
const long long MAX_BLOCKS = 32768;

std::wstring Trim(const std::wstring& text)
{
    std::wstring::size_type first = text.find_first_not_of(L" \t");
    if (first == std::wstring::npos)
        return std::wstring();
    return text.substr(first, text.find_last_not_of(L" \t") - first + 1);
}

bool ParseInt(const std::wstring& text, int* value)
{
    if (text.empty())
        return false;
    wchar_t* end = NULL;
    long parsed = std::wcstol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != 0)
        return false;
    *value = static_cast<int>(parsed);
    return true;
}
}

int UIScene_DebugFillArea::s_points[2][3];
bool UIScene_DebugFillArea::s_pointSet[2];
std::wstring UIScene_DebugFillArea::s_texts[4] = {L"stone", L"0", L"", L"0"};

UIScene_DebugFillArea::UIScene_DebugFillArea(int type, void*, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();
    m_keyboardControl = -1;
    m_keyboardActive = false;

    m_labelFillTile.init(UIString(L"Fill Tile (number or name)"));
    m_labelFillData.init(UIString(L"Fill Data"));
    m_labelEdgeTile.init(UIString(L"Edge Tile (blank: same as fill)"));
    m_labelEdgeData.init(UIString(L"Edge Data"));
    m_textFillTile.init(UIString(s_texts[0].c_str()), eControl_FillTile);
    m_textFillData.init(UIString(s_texts[1].c_str()), eControl_FillData);
    m_textEdgeTile.init(UIString(s_texts[2].c_str()), eControl_EdgeTile);
    m_textEdgeData.init(UIString(s_texts[3].c_str()), eControl_EdgeData);
    m_buttonCreate.init(UIString(CGameNetworkManager::sInstance.IsHost() ? L"Create" : L"Host only"), eControl_Create);
    m_buttonUndo.init(UIString(L"Undo"), eControl_Undo);
    pointLabel(0);
    pointLabel(1);
    m_labelSetArea.init(UIString(areaText().c_str()));
}

void UIScene_DebugFillArea::pointLabel(int index)
{
    wchar_t text[64];
    if (s_pointSet[index])
        swprintf(text, 64, L"%d: %d %d %d", index + 1, s_points[index][0], s_points[index][1], s_points[index][2]);
    else
        swprintf(text, 64, L"Point %d", index + 1);
    (index == 0 ? m_buttonPoint1 : m_buttonPoint2).init(UIString(text), index == 0 ? eControl_Point1 : eControl_Point2);
}

UIScene_DebugFillArea::~UIScene_DebugFillArea()
{
}

unsigned int UIScene_DebugFillArea::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* node = m_movie->getRootNode();
    struct Mapping
    {
        UIControl* control;
        const char* name;
    };
    const Mapping mappings[] = {
        {&m_labelSetArea, "SetArea"},
        {&m_buttonPoint1, "Point1"},
        {&m_buttonPoint2, "Point2"},
        {&m_labelFillTile, "LabelFillTile"},
        {&m_labelFillData, "LabelFillData"},
        {&m_labelEdgeTile, "LabelEdgeTile"},
        {&m_labelEdgeData, "LabelEdgeData"},
        {&m_textFillTile, "FillTile"},
        {&m_textFillData, "FillData"},
        {&m_textEdgeTile, "EdgeTile"},
        {&m_textEdgeData, "EdgeData"},
        {&m_buttonCreate, "Create"},
        {&m_buttonUndo, "Undo"},
    };
    for (unsigned int i = 0; i < sizeof(mappings) / sizeof(mappings[0]); ++i)
    {
        mappings[i].control->setupControl(this, node, std::string(mappings[i].name));
        m_controls.push_back(mappings[i].control);
    }
    return 1;
}

void UIScene_DebugFillArea::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, (int)IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

std::wstring UIScene_DebugFillArea::getMoviePath()
{
    return std::wstring(L"DebugFillArea");
}

void UIScene_DebugFillArea::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
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

UIControl_TextInput* UIScene_DebugFillArea::inputFor(int control)
{
    switch (control)
    {
    case eControl_FillTile:
        return &m_textFillTile;
    case eControl_FillData:
        return &m_textFillData;
    case eControl_EdgeTile:
        return &m_textEdgeTile;
    case eControl_EdgeData:
        return &m_textEdgeData;
    }
    return NULL;
}

void UIScene_DebugFillArea::showStatus(const std::wstring& text)
{
    m_labelSetArea.setLabel(UIString(text.c_str()), false, false);
}

std::wstring UIScene_DebugFillArea::areaText()
{
    if (!s_pointSet[0] || !s_pointSet[1])
        return L"Set Area";
    int size[3];
    for (int axis = 0; axis < 3; ++axis)
        size[axis] = std::abs(s_points[0][axis] - s_points[1][axis]) + 1;
    wchar_t text[96];
    swprintf(text, 96, L"Set Area: %d x %d x %d (%lld blocks)", size[0], size[1], size[2],
             static_cast<long long>(size[0]) * size[1] * size[2]);
    return text;
}

void UIScene_DebugFillArea::setPoint(int index)
{
    Minecraft* mc = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> player = mc->mLocalPlayers[m_playerId];
    if (player == NULL)
        return;
    if (mc->mHitResult != NULL && mc->mHitResult->mType == 1)
    {
        const BlockPos& pos = mc->mHitResult->getBlockPos();
        s_points[index][0] = pos.x;
        s_points[index][1] = pos.y;
        s_points[index][2] = pos.z;
    }
    else
    {
        s_points[index][0] = static_cast<int>(std::floor(player->mX));
        s_points[index][1] = static_cast<int>(std::floor(player->mY));
        s_points[index][2] = static_cast<int>(std::floor(player->mZ));
    }
    s_pointSet[index] = true;
    wchar_t text[64];
    swprintf(text, 64, L"%d: %d %d %d", index + 1, s_points[index][0], s_points[index][1], s_points[index][2]);
    (index == 0 ? m_buttonPoint1 : m_buttonPoint2).setLabel(UIString(text), false, false);
    showStatus(areaText());
}

const BlockState* UIScene_DebugFillArea::readBlock(UIControl_TextInput& tile, UIControl_TextInput& data,
                                                   bool allowEmpty, std::wstring* error)
{
    std::wstring name = Trim(tile.getLabel());
    if (name.empty())
    {
        if (!allowEmpty)
            *error = L"Fill Tile is empty";
        return NULL;
    }
    Block* block = NULL;
    int id = 0;
    if (ParseInt(name, &id))
        block = Block::existsWithId(id) ? Block::byId(id) : NULL;
    else
    {
        block = Block::byString(name);
        if (block == Blocks::air && name != L"air" && name != L"minecraft:air")
            block = NULL;
    }
    if (block == NULL)
    {
        *error = L"No block called " + name;
        return NULL;
    }
    int value = 0;
    std::wstring dataText = Trim(data.getLabel());
    if (!dataText.empty() && (!ParseInt(dataText, &value) || value < 0 || value > 15))
    {
        *error = L"Data must be 0 to 15";
        return NULL;
    }
    return block->getBlockState(value);
}

void UIScene_DebugFillArea::handlePress(int controlId, int)
{
    switch (controlId)
    {
    case eControl_Point1:
    case eControl_Point2:
        setPoint(controlId == eControl_Point1 ? 0 : 1);
        break;
    case eControl_Create:
    {
        if (!CGameNetworkManager::sInstance.IsHost())
            break;
        if (!s_pointSet[0] || !s_pointSet[1])
        {
            showStatus(L"Set both points first");
            break;
        }
        std::wstring error;
        const BlockState* fill = readBlock(m_textFillTile, m_textFillData, false, &error);
        const BlockState* edge = error.empty() ? readBlock(m_textEdgeTile, m_textEdgeData, true, &error) : NULL;
        if (!error.empty())
        {
            showStatus(error);
            break;
        }
        long long count = 1;
        for (int axis = 0; axis < 3; ++axis)
            count *= std::abs(s_points[0][axis] - s_points[1][axis]) + 1;
        if (count > MAX_BLOCKS)
        {
            wchar_t text[64];
            swprintf(text, 64, L"Too big: %lld blocks (at most %lld)", count, MAX_BLOCKS);
            showStatus(text);
            break;
        }
        DebugFillAreaRequest* request = new DebugFillAreaRequest();
        request->x0 = s_points[0][0];
        request->y0 = s_points[0][1];
        request->z0 = s_points[0][2];
        request->x1 = s_points[1][0];
        request->y1 = s_points[1][1];
        request->z1 = s_points[1][2];
        request->fill = fill;
        request->edge = edge;
        gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_FillArea, request);
        wchar_t text[64];
        swprintf(text, 64, L"Filled %lld blocks", count);
        showStatus(text);
        break;
    }
    case eControl_Undo:
        if (CGameNetworkManager::sInstance.IsHost())
        {
            gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_UndoFillArea, NULL);
            showStatus(L"Undone");
        }
        break;
    case eControl_FillTile:
    case eControl_FillData:
    case eControl_EdgeTile:
    case eControl_EdgeData:
        m_keyboardControl = controlId;
        m_keyboardActive = true;
        if (g_pInput->RequestKeyboard(L"Block", inputFor(controlId)->getLabel(), m_playerId, 40,
                                      KeyboardCompleteCallback, this, static_cast<C_4JInput::EKeyboardMode>(0))
            != 0)
            m_keyboardActive = false;
        break;
    }
}

int UIScene_DebugFillArea::KeyboardCompleteCallback(void* data, bool accepted)
{
    UIScene_DebugFillArea* scene = static_cast<UIScene_DebugFillArea*>(data);
    if (scene == NULL)
        return 0;
    scene->m_keyboardActive = false;
    UIControl_TextInput* input = scene->inputFor(scene->m_keyboardControl);
    if (!accepted || input == NULL)
        return 0;
    wchar_t text[64];
    std::memset(text, 0, sizeof(text));
    g_pInput->GetText(text, 63);
    input->setLabel(UIString(text), false, false);
    scene->s_texts[scene->m_keyboardControl - eControl_FillTile] = text;
    return 0;
}

void UIScene_DebugFillArea::handleDestroy()
{
    g_pInput->DestroyKeyboard();
}

int UIScene_DebugFillArea::getSceneType()
{
    return EUIScene_DebugFillArea;
}
