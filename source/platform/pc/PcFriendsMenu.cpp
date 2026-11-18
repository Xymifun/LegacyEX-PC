#include "platform/pc/PcFriendsMenu.hpp"

#include <string.h>
#include <wchar.h>

#include <map>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringIds.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LoadCreateJoinMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIScene_MessageBox.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "platform/pc/PcLobbyClient.hpp"
#include "platform/pc/PcMultiplayerMenu.hpp"
#include "platform/pc/PcOnlineConnection.hpp"

extern ConsoleUIController gConsoleUIController;
extern CConsoleMinecraftApp gApp;

namespace
{
const int CONNECT_ID = 201;
const int DISCONNECT_ID = 202;
const int IP_ID = 203;
const int CANCEL_ID = 204;
const int FRIEND_LIST_ID = 205;
const int REQUEST_LIST_ID = 206;
const int TOUCH_ID = 4;

const char* TAB_NAMES[3] = {"LoadGame", "NewGame", "JoinGame"};
const char* TITLE_NAMES[3] = {"LoadGameTabTitle", "NewGameTabTitle", "JoinGameTabTitle"};
const char* TOUCH_NAMES[3] = {"TouchTabLoad", "TouchTabCreate", "TouchTabJoin"};
const char* TIMER_NAMES[3] = {"LoadGameTimer", "NewGameTimer", "JoinGamesTimer"};
float textTop = 0.0f;

std::string removeId;

std::map<unsigned int, std::vector<unsigned char> > heads;

BufferedImage* SkinImage(unsigned int skin)
{
    if ((skin >> 31) != 0)
    {
        wchar_t path[64];
        swprintf(path, 64, L"dlcskin%08d.png", skin & 0x7fffffff);
        unsigned char* data = NULL;
        unsigned int size = 0;
        gApp.GetMemFileDetails(path, &data, &size);
        if (data != NULL && size > 0)
        {
            BufferedImage* img = new BufferedImage(data, size);
            if (img->getData() != NULL)
                return img;
            delete img;
        }
    }
    static const int TEXTURES[17] = {0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x70, 0x71,
                                     0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0xaa, 0xa9};
    static const wchar_t* NAMES[17] = {L"mob/char1", L"mob/char2", L"mob/char3", L"mob/char4", L"mob/char5",
                                       L"mob/char6", L"mob/char7", L"mob/alex", L"mob/alex1", L"mob/alex2",
                                       L"mob/alex3", L"mob/alex4", L"mob/alex5", L"mob/alex6", L"mob/alex7",
                                       L"mob/DevAlex", L"mob/DevSteve"};
    int d = 0;
    if ((skin >> 31) == 0 && (skin & 0x7fffffe0) == 0)
        d = skin & 0x1f;
    Textures* t = Minecraft::GetInstance()->getTextureManager();
    if (d >= 2 && d <= 18)
        return t->readImage((_TEXTURE_NAME)TEXTURES[d - 2], NAMES[d - 2]);
    return t->readImage((_TEXTURE_NAME)0x25, L"mob/steve");
}

const std::vector<unsigned char>& Head(unsigned int skin)
{
    std::map<unsigned int, std::vector<unsigned char> >::iterator found = heads.find(skin);
    if (found != heads.end())
        return found->second;
    BufferedImage* img = SkinImage(skin);
    int w = (int)img->getWidth();
    int s = w / 64;
    if (img->getData() == NULL)
        s = 0;
    static unsigned char rgba[64 * 64 * 4];
    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            unsigned int c = 0xFF808080;
            if (s > 0)
            {
                int px = x / 8 * s;
                int py = y / 8 * s;
                c = (unsigned int)img->getData()[(8 * s + py) * w + 8 * s + px];
                unsigned int hat = (unsigned int)img->getData()[(8 * s + py) * w + 40 * s + px];
                if ((hat >> 24) > 127)
                    c = hat;
            }
            unsigned char* o = &rgba[(y * 64 + x) * 4];
            o[0] = (unsigned char)(c >> 16);
            o[1] = (unsigned char)(c >> 8);
            o[2] = (unsigned char)c;
            o[3] = 255;
        }
    }
    delete img;
    std::vector<unsigned char>& png = heads[skin];
    png.resize(64 * 64 * 4 * 2 + 1024);
    int size = 0;
    gRenderer->SaveTextureDataToMemory(&png[0], (int)png.size(), &size, 64, 64, (int*)rgba);
    png.resize(size > 0 ? size : 0);
    return png;
}

std::string requestId;

int RequestReturned(void*, int, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Accept)
        PcLobbyClientAnswerRequest(requestId, true);
    else if (result == C4JStorage::EMessageResult_Decline)
        PcLobbyClientAnswerRequest(requestId, false);
    return 0;
}

std::wstring LabelFor(const PcLobbyFriend& f)
{
    std::wstring label = f.name;
    if (label.empty())
        label = f.discord;
    else if (!f.discord.empty())
        label += L" (" + f.discord + L")";
    return label;
}

std::wstring Colored(const std::wstring& text, const wchar_t* color)
{
    std::wstring safe;
    for (size_t c = 0; c < text.size(); c++)
    {
        if (text[c] == L'<')
            safe += L"&lt;";
        else if (text[c] == L'&')
            safe += L"&amp;";
        else
            safe += text[c];
    }
    return std::wstring(L"<font color=\"") + color + L"\">" + safe + L"</font>";
}

unsigned char profileUuid[16];

int ProfileReturned(void*, int, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Accept)
        PcLobbyClientSendFriendRequest(profileUuid);
    return 0;
}

int RemoveReturned(void*, int, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Accept)
        PcLobbyClientRemoveFriend(removeId);
    return 0;
}

const char* BUTTON_NAMES[4] = {"PcConnectButton", "PcDisconnectButton", "PcIpButton", "PcCancelButton"};

void Make(UIScene* scene, fuiRenderNode* tab, const char* symbol, const char* name, UIControl* control)
{
    FJ_FuiNode* made = FJ_Factory::createObject(symbol, tab, true, NULL);
    if (made == NULL || made->m_pRenderNode == NULL || tab->getFuiNode() == NULL)
        return;
    made->m_pRenderNode->PcSetName(name);
    tab->getFuiNode()->addChild(made);
    control->setupControl(scene, tab, std::string(name));
}

const wchar_t* IpLabel()
{
    return L10N::GetString(PcMultiplayerShowingIp() ? ePcString_HideLocalIp : ePcString_ShowLocalIp);
}
}

void PcOpenProfile(int pad, const std::wstring& name, const unsigned char uuid[16])
{
    memcpy(profileUuid, uuid, 16);
    static wchar_t text[64];
    size_t n = 0;
    for (size_t i = 0; i < name.size() && n + 2 < 64; i++)
    {
        text[n++] = name[i];
        if (name[i] == L'%')
            text[n++] = L'%';
    }
    text[n] = 0;
    static int buttons[2];
    buttons[0] = ePcString_SendFriendRequest;
    buttons[1] = IDS_CONFIRM_CANCEL;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    gConsoleUIController.RequestAlertMessage(ePcString_Profile, ePcString_Profile, buttons, 2, pad, ProfileReturned,
                                             NULL, text, true);
}

UIScene_PcFriendsMenu::UIScene_PcFriendsMenu(int player, void* data, UILayer* layer)
    : UIScene(player, layer), m_tab(0), m_inGame(data != NULL), m_left(false), m_friendsRevision(0), m_requestsRevision(0), m_focused(-1)
{
    initialiseMovie();
    m_titles[0].init(UIString(L"Connect"));
    m_titles[1].init(UIString(L"Friends"));
    m_titles[2].init(UIString(L"Requests"));
    for (int i = 0; i < 3; i++)
    {
        m_touch[i].init(TOUCH_ID + i);
        m_timers[i].setVisible(false);
    }
    if (m_text.getFuiRenderNode() != NULL && m_text.getFuiRenderNode()->getFuiNode()->isFJ_Label())
    {
        FJ_Label* l = m_text.getFuiRenderNode()->getFuiNode()->asFJ_Label();
        char on[] = "true";
        l->setAction(6, on);
        l->setAction(8, on);
        l->setUseHtmlText(true);
    }
    m_buttons[0].init(UIString(L10N::GetString(ePcString_ConnectToHost)), CONNECT_ID);
    m_buttons[1].init(UIString(L10N::GetString(ePcString_Disconnect)), DISCONNECT_ID);
    m_buttons[2].init(UIString(IpLabel()), IP_ID);
    m_buttons[3].init(UIString(L10N::GetString(IDS_CONFIRM_CANCEL)), CANCEL_ID);
    m_empty[0].init(UIString(L"[Empty]"));
    m_empty[1].init(UIString(L"[Empty]"));
    m_friendList.init(FRIEND_LIST_ID);
    m_requestList.init(REQUEST_LIST_ID);
    Layout();
    FillFriends();
    FillRequests();
    ShowText();
    if (m_inGame)
    {
        FJ_LoadCreateJoinMenu* menu = (FJ_LoadCreateJoinMenu*)m_movie->getRootNode()->getFuiNode();
        menu->HideLoadTab();
        float w = menu->getChildByName("LoadGame")->getChildAt(0)->getWidth();
        m_titleX[1] = m_titles[1].getFuiRenderNode()->getX() + w * (0.25f - 0.5f);
        m_titleX[2] = m_titles[2].getFuiRenderNode()->getX() + w * (0.75f - 5.0f / 6.0f);
        SetTab(1);
    }
    else
        SetTab(0);
}

std::wstring UIScene_PcFriendsMenu::getMoviePath()
{
    return std::wstring(L"LoadCreateJoinMenu");
}

unsigned int UIScene_PcFriendsMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    for (int i = 0; i < 3; i++)
    {
        m_tabs[i].setupControl(this, root, std::string(TAB_NAMES[i]));
        m_controls.push_back(&m_tabs[i]);
        m_titles[i].setupControl(this, root, std::string(TITLE_NAMES[i]));
        m_controls.push_back(&m_titles[i]);
        fuiRenderNode* tab = m_tabs[i].getFuiRenderNode();
        m_touch[i].setupControl(this, tab, std::string(TOUCH_NAMES[i]));
        m_controls.push_back(&m_touch[i]);
        m_timers[i].setupControl(this, tab, std::string(TIMER_NAMES[i]));
        m_controls.push_back(&m_timers[i]);
    }

    fuiRenderNode* connect = m_tabs[0].getFuiRenderNode();
    Make(this, connect, "fourj.Labels.FJ_Label_SmallBlack", "PcConnectText", &m_text);
    m_controls.push_back(&m_text);
    for (int i = 0; i < 4; i++)
    {
        Make(this, connect, "fourj.Buttons.FJ_MenuButton_Normal", BUTTON_NAMES[i], &m_buttons[i]);
        m_controls.push_back(&m_buttons[i]);
    }

    m_friendList.setupControl(this, m_tabs[1].getFuiRenderNode(), std::string("NewGameList"));
    m_controls.push_back(&m_friendList);
    m_requestList.setupControl(this, m_tabs[2].getFuiRenderNode(), std::string("JoinGameGamesList"));
    m_controls.push_back(&m_requestList);
    Make(this, m_tabs[1].getFuiRenderNode(), "fourj.Labels.FJ_Label_Black", "PcFriendsEmpty", &m_empty[0]);
    m_controls.push_back(&m_empty[0]);
    Make(this, m_tabs[2].getFuiRenderNode(), "fourj.Labels.FJ_Label_Black", "PcRequestsEmpty",
         &m_empty[1]);
    m_controls.push_back(&m_empty[1]);
    return 1;
}

int UIScene_PcFriendsMenu::getSceneType()
{
    return EUIScene_PcFriendsMenu;
}

void UIScene_PcFriendsMenu::Layout()
{
    FJ_LoadCreateJoinMenu* menu = (FJ_LoadCreateJoinMenu*)m_movie->getRootNode()->getFuiNode();
    menu->ShowSaveSizeBar(false);
    FJ_FuiNode* recess = menu->getChildByName("NewGame.NewGameListRecessPanel");
    if (recess == NULL)
        return;
    float x = recess->getX();
    float y = recess->getY();
    float w = recess->getChildAt(0)->getWidth();
    float h = recess->getChildAt(0)->getHeight();
    const float PAD = 16.0f;
    const float GAP = 6.0f;

    fuiRenderNode* n = m_text.getFuiRenderNode();
    if (n != NULL)
    {
        n->setX(x + PAD);
        textTop = y + PAD;
        n->getFuiNode()->asFJ_Base()->SetNewObjectWidth((int)(w - PAD * 2));
    }

    float by = y + h - PAD;
    for (int i = 3; i >= 0; i--)
    {
        n = m_buttons[i].getFuiRenderNode();
        if (n == NULL)
            continue;
        by -= n->getHeight();
        n->setX(x + (float)(int)((w - n->getWidth()) / 2));
        n->setY(by);
        by -= GAP;
        FJ_Base* b = n->getFuiNode()->asFJ_Base();
        b->PcSetNavUpDown(i > 0 ? BUTTON_NAMES[i - 1] : "", i < 3 ? BUTTON_NAMES[i + 1] : "");
    }

    for (int i = 0; i < 2; i++)
    {
        n = m_empty[i].getFuiRenderNode();
        if (n == NULL)
            continue;
        fuiRenderNodeEditText* f = n->getFuiNode()->asFJ_Base()->GetTextField();
        if (f == NULL || f->getText().empty())
            continue;
        float left = f->getCharBoundaries(0).xMin;
        float right = f->getCharBoundaries((int)f->getText().size() - 1).xMax;
        n->setX((float)(int)(x + w / 2 - (f->getX() + (left + right) / 2)));
        n->setY((float)(int)(y + h / 2 - (f->getY() + f->getHeight() / 2)));
    }
}

void UIScene_PcFriendsMenu::ShowText()
{
    m_text.init(UIString(PcMultiplayerText()));
    fuiRenderNode* n = m_text.getFuiRenderNode();
    if (n == NULL)
        return;
    fuiRenderNodeEditText* f = n->getFuiNode()->asFJ_Base()->GetTextField();
    float fy = f != NULL ? f->getY() : 0.0f;
    n->setY(textTop - fy);
}

std::wstring UIScene_PcFriendsMenu::HeadFor(unsigned int skin)
{
    wchar_t head[32];
    swprintf(head, 32, L"PcFriendHead%08X", skin);
    const std::vector<unsigned char>& png = Head(skin);
    if (!png.empty())
        registerSubstitutionTexture(head, (unsigned char*)&png[0], (unsigned int)png.size(), false);
    return head;
}

void UIScene_PcFriendsMenu::FillFriends()
{
    std::vector<PcLobbyFriend> list = PcLobbyClientFriends(&m_friendsRevision);
    m_friendList.clearList();
    m_labels.clear();
    m_plain.clear();
    m_focused = -1;
    for (size_t i = 0; i < list.size(); i++)
    {
        std::wstring label = LabelFor(list[i]);
        m_plain.push_back(label);
        if (!list[i].online)
            label = Colored(label, L"#AAAAAA");
        m_labels.push_back(label);
        m_friendList.addItem(label, HeadFor(list[i].skin));
    }
    m_empty[0].setVisible(list.empty());
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_PcFriendsMenu::FillRequests()
{
    std::vector<PcLobbyFriend> list = PcLobbyClientRequests(&m_requestsRevision);
    m_requestList.clearList();
    for (size_t i = 0; i < list.size(); i++)
        m_requestList.addItem(Colored(LabelFor(list[i]), L"#FFFF55"), HeadFor(list[i].skin));
    m_empty[1].setVisible(list.empty());
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_PcFriendsMenu::SetTab(int t)
{
    m_tab = t;
    ((FJ_LoadCreateJoinMenu*)m_movie->getRootNode()->getFuiNode())->SetActiveTab(t);
    if (m_inGame)
    {
        m_movie->getRootNode()->gather(false);
        m_titles[1].getFuiRenderNode()->setX(m_titleX[1]);
        m_titles[2].getFuiRenderNode()->setX(m_titleX[2]);
    }
    gConsoleUIController.TouchBoxRebuild(this);
    updateTooltips();
}

void UIScene_PcFriendsMenu::tick()
{
    UIScene::tick();
    unsigned int revision = 0;
    PcLobbyClientFriends(&revision);
    if (revision != m_friendsRevision)
        FillFriends();
    PcLobbyClientRequests(&revision);
    if (revision != m_requestsRevision)
        FillRequests();
    if (m_tab == 0 && m_movie != NULL && m_buttons[0].getFuiRenderNode() != NULL)
    {
        FJ_FuiNode* focus = m_movie->getRootNode()->getFuiNode()->getStage()->getFocus();
        bool onButton = false;
        for (int i = 0; i < 4; i++)
            if (focus == m_buttons[i].getFuiRenderNode()->getFuiNode())
                onButton = true;
        if (!onButton)
            SetFocusToElement(CONNECT_ID);
    }
    if (!m_left && hasFocus(m_playerId) && !PcOnlineConnectionIsFriendsLobby())
        Leave();
}

void UIScene_PcFriendsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, m_tab == 0 ? IDS_TOOLTIPS_SELECT : -1, IDS_TOOLTIPS_BACK, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_PcFriendsMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, !m_inGame);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_PcFriendsMenu::handleGainFocus(bool navigatedBack)
{
    UIScene::handleGainFocus(navigatedBack);
    ShowText();
}

void UIScene_PcFriendsMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                        bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (!pressed)
        return;
    switch (action)
    {
    case EControllerActions_MenuCancel:
        Leave();
        handled = true;
        break;
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuLeftScroll:
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
        if (m_inGame)
            SetTab(m_tab == 1 ? 2 : 1);
        else
            SetTab((m_tab + 2) % 3);
        handled = true;
        break;
    case EControllerActions_MenuRight:
    case EControllerActions_MenuRightScroll:
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
        if (m_inGame)
            SetTab(m_tab == 1 ? 2 : 1);
        else
            SetTab((m_tab + 1) % 3);
        handled = true;
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    }
}

void UIScene_PcFriendsMenu::handlePress(int controlId, int childId)
{
    if (controlId == REQUEST_LIST_ID)
    {
        unsigned int revision = 0;
        std::vector<PcLobbyFriend> list = PcLobbyClientRequests(&revision);
        if (childId < 0 || childId >= (int)list.size())
            return;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        requestId = list[childId].id;
        std::wstring name = LabelFor(list[childId]);
        std::wstring safe;
        for (size_t i = 0; i < name.size(); i++)
        {
            safe += name[i];
            if (name[i] == L'%')
                safe += L'%';
        }
        static wchar_t text[256];
        swprintf(text, 256, L"You received a request from %ls", safe.c_str());
        static int buttons[3];
        buttons[0] = ePcString_AcceptRequest;
        buttons[1] = ePcString_DenyRequest;
        buttons[2] = IDS_CONFIRM_CANCEL;
        gConsoleUIController.RequestAlertMessage(ePcString_FriendRequest, ePcString_FriendRequest, buttons, 3,
                                                 m_playerId, RequestReturned, NULL, text, true);
        return;
    }
    if (controlId == FRIEND_LIST_ID)
    {
        unsigned int revision = 0;
        std::vector<PcLobbyFriend> list = PcLobbyClientFriends(&revision);
        if (childId < 0 || childId >= (int)list.size())
            return;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        removeId = list[childId].id;
        std::wstring name = list[childId].name.empty() ? list[childId].discord : list[childId].name;
        std::wstring safe;
        for (size_t i = 0; i < name.size(); i++)
        {
            safe += name[i];
            if (name[i] == L'%')
                safe += L'%';
        }
        static wchar_t text[256];
        swprintf(text, 256, L"Remove %ls as a friend?", safe.c_str());
        static int buttons[2];
        buttons[0] = ePcString_RemoveFriend;
        buttons[1] = IDS_CONFIRM_CANCEL;
        PcMessageBoxConfirmTwice();
        gConsoleUIController.RequestAlertMessage(ePcString_RemoveFriend, ePcString_RemoveFriend, buttons, 2,
                                                 m_playerId, RemoveReturned, NULL, text, true);
        return;
    }
    if (controlId < CONNECT_ID || controlId > CANCEL_ID)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (controlId == CONNECT_ID)
        PcMultiplayerAskAddress(m_playerId);
    else if (controlId == DISCONNECT_ID)
    {
        PcOnlineConnectionSetAddress(std::string());
        Leave();
    }
    else if (controlId == IP_ID)
    {
        PcMultiplayerToggleIp();
        m_buttons[2].setLabel(UIString(IpLabel()), true, false);
        ShowText();
    }
    else
        Leave();
}

void UIScene_PcFriendsMenu::handleFocusChange(int controlId, int childId)
{
    if (controlId != FRIEND_LIST_ID || childId == m_focused)
        return;
    if (m_focused >= 0 && m_focused < (int)m_labels.size())
        m_friendList.setButtonLabel(m_focused, m_labels[m_focused]);
    m_focused = childId;
    if (m_focused >= 0 && m_focused < (int)m_plain.size())
        m_friendList.setButtonLabel(m_focused, m_plain[m_focused]);
}

void UIScene_PcFriendsMenu::handleTouchInput(unsigned int, int, int, int box, bool pressed, bool, bool)
{
    int t = box - TOUCH_ID;
    if (!pressed || t < 0 || t > 2 || t == m_tab || (m_inGame && t == 0))
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    SetTab(t);
}

UIControl* UIScene_PcFriendsMenu::GetMainPanel()
{
    return &m_tabs[m_tab];
}

void UIScene_PcFriendsMenu::Leave()
{
    if (m_left)
        return;
    m_left = true;
    navigateBack();
}
