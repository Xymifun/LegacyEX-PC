#include "game/ui/UIScene_WritableBookMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/WrittenBookItem.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/ui/FJ_BookAndQuillMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/XLanguage.hpp"

static const std::wstring TAG_TITLE = L"title";
static const std::wstring TAG_AUTHOR = L"author";
static const std::wstring TAG_PAGES = L"pages";
static const std::wstring TAG_OWNER_UUID = L"ownerUUID";
static const std::wstring TAG_CREATED_BY_RESTRICTED_PLAYER = L"createdByRestrictedPlayer";
static const std::wstring TAG_CREATED_BY_HOST = L"createdByHost";

template <typename T> std::wstring _toString(T value);
extern ConsoleUIController gConsoleUIController;
extern C_4JProfile* gProfile;

struct UIScene_WritableBookMenuData
{
    not_null_ptr<ItemInstance> item;
    int player;
    bool canEdit;
};

UIScene_WritableBookMenu::UIScene_WritableBookMenu(int type, void* data, UILayer* layer) : UIScene(type, layer)
{
    UIScene_WritableBookMenuData* sceneData = static_cast<UIScene_WritableBookMenuData*>(data);
    initialiseMovie();
    m_playerId = sceneData->player;
    m_book = sceneData->item;
    m_canEdit = sceneData->canEdit;
    m_textChanged = false;
    m_titleText = L"";
    m_currentPage = 0;
    m_caretHidden = false;
    m_signing = false;
    m_closeRequested = false;
    m_pages = 0;
    m_keyboardOpen = false;
    m_pageCount = 0;
    m_caretOffset = 0;
    std::memset(m_reserved, 0, sizeof(m_reserved));
    m_contentRestricted = false;

    wchar_t buttonLabels[4][256];
    swprintf(buttonLabels[0], 256, L"%ls", L10N::GetString(IDS_TITLE_BOOK_SIGN));
    swprintf(buttonLabels[1], 256, L"%ls", L10N::GetString(IDS_BOOK_SIGN_AND_CLOSE));
    m_signButton.setAllPossibleLabels(0, 2, buttonLabels);
    swprintf(buttonLabels[2], 256, L"%ls", L10N::GetString(IDS_DONE));
    swprintf(buttonLabels[3], 256, L"%ls", L10N::GetString(IDS_CONFIRM_CANCEL));
    m_doneButton.setAllPossibleLabels(0, 2, buttonLabels);
    HandleFontSizeOverride();

    m_doneButton.init(UIString(L10N::GetString(IDS_DONE)), 1);
    if (!m_canEdit)
        SetBookIsSigned();
    else
        m_signButton.init(UIString(L10N::GetString(IDS_TITLE_BOOK_SIGN)), 0);
    m_pageLabel.init(UIString(L""));

    doHorizontalResizeCheck();
    Init();
    RetrieveBookText();
    SetCaretPosition(m_caretPosition);
    FormatAndDisplayText(EFormatTextType_Display);
    layer->addComponent(type, EUIScene_MenuBackground, 0);
    m_unusedControlByte = 0;
    m_bookTouch.init(2);
    m_leftTouch.init(3);
    m_rightTouch.init(4);
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_WritableBookMenu::~UIScene_WritableBookMenu()
{
    m_layer->removeComponent(EUIScene_MenuBackground);
}

std::wstring UIScene_WritableBookMenu::getMoviePath()
{
    int playerCount = gApp.GetLocalPlayerCount();
    if (playerCount > 1)
        return std::wstring(L"BookAndQuillMenuSplit");
    return std::wstring(L"BookAndQuillMenu");
}

unsigned int UIScene_WritableBookMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_pageLabel.setupControl(this, root, std::string("PageLabel"));
        m_controls.push_back(&m_pageLabel);
    }
    {
        m_signButton.setupControl(this, root, std::string("ButtonSign"));
        m_controls.push_back(&m_signButton);
    }
    {
        m_doneButton.setupControl(this, root, std::string("ButtonClose"));
        m_controls.push_back(&m_doneButton);
    }
    {
        m_bookTouch.setupControl(this, root, std::string("TouchControlBook"));
        m_controls.push_back(&m_bookTouch);
    }
    {
        m_leftTouch.setupControl(this, root, std::string("TouchControlScrollLeft"));
        m_controls.push_back(&m_leftTouch);
    }
    {
        m_rightTouch.setupControl(this, root, std::string("TouchControlScrollRight"));
        m_controls.push_back(&m_rightTouch);
    }
    return 1;
}

void UIScene_WritableBookMenu::HandleFontSizeOverride()
{
    if (XGetLanguage() == XC_LANGUAGE_ZH_HANS && !gRenderer->IsWidescreen())
    {
        FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
        menu->SetFontSizeOverride(11);
    }
}

void UIScene_WritableBookMenu::SetBookIsSigned()
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->SetBookIsSigned();
    m_signButton.removeUINodeRef();
    m_doneButton.removeUINodeRef();
}

void UIScene_WritableBookMenu::SetPageLabel(std::wstring& text)
{
    m_pageLabel.setLabel(UIString(text), false, false);
}

void UIScene_WritableBookMenu::ChangePage(int page, int pageCount)
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->ChangePage(page, pageCount);
}

void UIScene_WritableBookMenu::UpdatePageNumber()
{
    std::wstring label = L10N::GetString(0x5351d8b1);
    size_t pos = label.find(L"%d");
    label.replace(pos, 2, _toString(m_currentPage + 1));
    pos = label.find(L"%d");
    label.replace(pos, 2, _toString(m_pageCount));
    SetPageLabel(label);
    if (m_canEdit && m_signing)
    {
        ChangePage(0, 0);
    }
    else
    {
        int pages = m_canEdit ? MAX_PAGES - 1 : m_pageCount - 1;
        ChangePage(m_currentPage, pages);
    }
}

void UIScene_WritableBookMenu::Init()
{
    if (WrittenBookItem::HasBookTags(m_book))
    {
        CompoundTag* tag = m_book.get()->getTag();

        bool ownedByLocalPlayer = false;
        if (tag->getString(TAG_OWNER_UUID) != L"")
        {
            for (int i = 0; i < 4; ++i)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[i];
                if (localPlayer != NULL && localPlayer.get()->getUUID() == tag->getString(TAG_OWNER_UUID))
                {
                    ownedByLocalPlayer = true;
                    break;
                }
            }
        }

        bool chatCensored = gApp.IsConsoleChatCensored();
        bool createdByRestrictedPlayer = false;
        if (tag->contains(TAG_CREATED_BY_RESTRICTED_PLAYER) && tag->getByte(TAG_CREATED_BY_RESTRICTED_PLAYER) != 0)
            createdByRestrictedPlayer = true;
        bool createdByHost = true;
        if (tag->contains(TAG_CREATED_BY_HOST) && tag->getByte(TAG_CREATED_BY_HOST) == 0)
            createdByHost = false;

        if (!((!chatCensored && !createdByRestrictedPlayer) || ownedByLocalPlayer
              || (CGameNetworkManager::sInstance.IsHost() && createdByHost)))
            m_contentRestricted = true;

        if (!m_contentRestricted)
        {
            ListTag* pages = tag->getList(TAG_PAGES, Tag::TAG_STRING);
            if (pages != 0 && pages->size() > 0)
            {
                m_pages = pages->copy();
                int pageCount = m_pages->size();
                if (pageCount > 0)
                    m_pageCount = pageCount;
                else
                    m_pageCount = 1;
            }
        }
        else
        {
            std::wstring censoredText = L"";
            unsigned int language = XGetLanguage();
            if (language == XC_LANGUAGE_JAPANESE || language == XC_LANGUAGE_KOREAN || language == XC_LANGUAGE_TCHINESE
                || language == XC_LANGUAGE_ZH_HANS)
                censoredText = L"Censored";
            else
                censoredText = L10N::GetString(IDS_STRINGVERIFY_CENSORED);
            m_pages = new ListTag();
            m_pages->add(new StringTag(censoredText));
            m_pageCount = 1;
        }
    }

    if (m_pages == 0 && m_canEdit)
    {
        m_pages = new ListTag();
        m_pages->add(new StringTag(L""));
        m_pageCount = 1;
    }
    UpdatePageNumber();
}

void UIScene_WritableBookMenu::RetrieveBookText()
{
    m_pageText = getCurrentPageText();
    m_caretPosition = static_cast<int>(m_pageText.size());
}

void UIScene_WritableBookMenu::SetCaretPosition(int position)
{
    bool visible = !m_caretHidden;
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->SetCaretPosition(position + m_caretOffset, visible, false);
}

int UIScene_WritableBookMenu::FormatAndDisplayText(EFormatTextType type)
{
    std::wstring text;
    if (!m_signing)
    {
        text = m_pageText;
        FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
        return menu->SetPageText(text, type) ? 1 : 0;
    }

    std::wstring newline = L"\n";
    m_signText = L10N::GetString(IDS_BOOK_SIGN_TEXT);
    std::wstring::size_type pos = m_signText.find(L"\\r", 0);
    m_caretOffset = static_cast<int>(pos) + 1;
    SetCaretPosition(m_caretPosition);
    while (pos != std::wstring::npos)
    {
        m_signText.replace(pos, 2, newline);
        pos = m_signText.find(L"\\r", 0);
    }
    m_signText.replace(m_signText.find(L"%s", 0), 2, m_titleText);
    pos = m_signText.find(L"%s", 0);
    std::wstring author = gProfile->GetDisplayName(m_playerId);
    m_signText.replace(pos, 2, author.c_str(), wcslen(author.c_str()));
    text = m_signText;

    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    return menu->SetPageText(text, type) ? 1 : 0;
}

void UIScene_WritableBookMenu::tick()
{
    UIScene::tick();
    if (m_closeRequested)
        navigateBack();
}

void UIScene_WritableBookMenu::handlePreReload()
{
    wchar_t buttonLabels[4][256];
    swprintf(buttonLabels[0], 256, L"%ls", L10N::GetString(IDS_TITLE_BOOK_SIGN));
    swprintf(buttonLabels[1], 256, L"%ls", L10N::GetString(IDS_BOOK_SIGN_AND_CLOSE));
    m_signButton.setAllPossibleLabels(0, 2, buttonLabels);
    swprintf(buttonLabels[2], 256, L"%ls", L10N::GetString(IDS_DONE));
    swprintf(buttonLabels[3], 256, L"%ls", L10N::GetString(IDS_CONFIRM_CANCEL));
    m_doneButton.setAllPossibleLabels(0, 2, buttonLabels);
    HandleFontSizeOverride();
}

void UIScene_WritableBookMenu::SetFocusToBook(bool focus)
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->SetFocusToBook(focus);
}

void UIScene_WritableBookMenu::handleReload()
{
    UpdatePageNumber();
    if (!m_canEdit)
        SetBookIsSigned();
    doHorizontalResizeCheck();
    FormatAndDisplayText(EFormatTextType_Display);
    if (m_canEdit)
        SetCaretPosition(m_caretPosition);
    if (m_doneButton.hasFocus() || m_signButton.hasFocus())
        SetFocusToBook(false);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_WritableBookMenu::updateTooltips()
{
    int select = IDS_TOOLTIPS_SELECT;
    int previousPage = -1;
    int nextPage = -1;
    if (!m_signing)
    {
        if (m_currentPage > 0)
            previousPage = IDS_TOOLTIPS_PREVIOUS_PAGE;
        if (!m_canEdit)
        {
            if (m_currentPage < m_pageCount - 1)
                nextPage = IDS_TOOLTIPS_NEXT_PAGE;
            select = -1;
        }
        else
        {
            if (m_currentPage < m_pageCount - 1)
            {
                gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1,
                                                 previousPage, IDS_TOOLTIPS_NEXT_PAGE, -1, -1, -1, -1, -1, -1, -1, -1,
                                                 false, false);
                return;
            }
            if (m_currentPage < MAX_PAGES - 1)
            {
                gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1,
                                                 previousPage, IDS_TOOLTIPS_ADD_PAGE, -1, -1, -1, -1, -1, -1, -1, -1,
                                                 false, false);
                return;
            }
        }
    }
    gConsoleUIController.SetTooltips(m_playerId, select, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, previousPage, nextPage, -1,
                                     -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_WritableBookMenu::OnStopSigning()
{
    m_pageLabel.setVisible(true);
    m_signButton.setLabel(UIString(L10N::GetString(IDS_TITLE_BOOK_SIGN)), false, false);
    m_doneButton.setLabel(UIString(L10N::GetString(IDS_DONE)), false, false);
    doHorizontalResizeCheck();
    m_caretPosition = static_cast<int>(m_pageText.size());
    FormatAndDisplayText(EFormatTextType_Display);
    SetCaretPosition(m_caretPosition);
    UpdatePageNumber();
    updateTooltips();
    SetFocusToBook(true);
    gConsoleUIController.TouchBoxRebuild(this);
}

int UIScene_WritableBookMenu::ExitBookDialogReturned(void* context, int, C4JStorage::EMessageResult result)
{
    if (context != 0 && result == C4JStorage::EMessageResult_Decline)
        static_cast<UIScene_WritableBookMenu*>(context)->m_closeRequested = true;
    return 0;
}

void UIScene_WritableBookMenu::HandleCancel()
{
    if (m_signing)
    {
        m_signing = false;
        OnStopSigning();
        return;
    }
    if (m_textChanged)
    {
        int parameters[2] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK)};
        gConsoleUIController.RequestAlertMessage(0xa95e7f9a, IDS_TEXT_BOOK_LOSE_TEXT, parameters, 2, m_playerId,
                                                 ExitBookDialogReturned, this, NULL, true);
        return;
    }
    navigateBack();
    gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
}

bool UIScene_WritableBookMenu::GetBookHasFocus()
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    return menu->GetBookHasFocus();
}

int UIScene_WritableBookMenu::KeyboardCompleteCallback(void* context, bool success)
{
    unsigned int callbackId = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context));
    UIScene_WritableBookMenu* scene = static_cast<UIScene_WritableBookMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(callbackId));
    if (scene == 0)
        return 0;

    scene->m_keyboardOpen = false;
    if (!success)
        return 0;

    wchar_t text[MAX_PAGE_LENGTH + 4];
    std::memset(text, 0, sizeof(wchar_t) * (MAX_PAGE_LENGTH + 1));
    if (scene->m_signing)
    {
        g_pInput->GetText(text, MAX_TITLE_LENGTH);
        scene->m_titleText.assign(text, wcslen(text));
        scene->m_caretPosition = static_cast<int>(scene->m_titleText.size());
    }
    else
    {
        g_pInput->GetText(text, MAX_PAGE_LENGTH);
        scene->m_pageText.assign(text, wcslen(text));
        scene->setCurrentPageText(scene->m_pageText);
        scene->m_caretPosition = static_cast<int>(scene->m_pageText.size());
    }
    scene->addTimer(10, 50);
    return 0;
}

void UIScene_WritableBookMenu::StoreBookText()
{
    setCurrentPageText(m_pageText);
}

void UIScene_WritableBookMenu::OnStartSigning()
{
    m_pageLabel.setVisible(false);
    m_signButton.setLabel(UIString(L10N::GetString(IDS_BOOK_SIGN_AND_CLOSE)), false, false);
    m_doneButton.setLabel(UIString(L10N::GetString(IDS_CONFIRM_CANCEL)), false, false);
    doHorizontalResizeCheck();
    m_caretPosition = 0;
    FormatAndDisplayText(EFormatTextType_Display);
    SetCaretPosition(m_caretPosition);
    UpdatePageNumber();
    updateTooltips();
    SetFocusToBook(true);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_WritableBookMenu::SaveChanges(bool signing)
{
    if (!m_canEdit)
        return;

    if (!m_book.get()->hasTag())
        m_book.get()->setTag(new CompoundTag());
    boost::shared_ptr<MultiplayerLocalPlayer>& player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    m_book.get()->getTag()->putString(TAG_OWNER_UUID, player.get()->getUUID());
    m_book.get()->getTag()->putByte(TAG_CREATED_BY_RESTRICTED_PLAYER, gApp.IsConsoleChatCensored());
    m_book.get()->getTag()->putByte(TAG_CREATED_BY_HOST, CGameNetworkManager::sInstance.IsHost());

    if (m_pages == 0)
        return;

    int pageCount = m_pages->size();
    while (pageCount > 1)
    {
        StringTag* lastPage = static_cast<StringTag*>(m_pages->get(m_pages->size() - 1));
        if (!lastPage->mData.empty())
            break;
        m_pages->remove(m_pages->size() - 1);
        pageCount = m_pages->size();
    }

    if (!m_book.get()->hasTag())
        m_book.get()->addTagElement(TAG_PAGES, m_pages);
    else
        m_book.get()->getTag()->put(TAG_PAGES, m_pages);

    std::wstring channel = ServerboundCustomPayloadPacket::CUSTOM_BOOK_PACKET;
    if (signing)
    {
        channel = ServerboundCustomPayloadPacket::CUSTOM_BOOK_SIGN_PACKET;
        m_book.get()->addTagElement(
            TAG_AUTHOR, new StringTag(Minecraft::GetInstance()->mLocalPlayers[m_playerId].get()->getName()));
        m_book.get()->addTagElement(TAG_TITLE, new StringTag(m_titleText));
    }

    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeItem(m_book);
    {
        boost::shared_ptr<Packet> packet(new ServerboundCustomPayloadPacket(channel, bytes.toByteArray()));
        Minecraft::GetInstance()->getConnection(m_playerId)->send(packet);
    }
    m_textChanged = false;
}

void UIScene_WritableBookMenu::Sign()
{
    SaveChanges(true);
    navigateBack();
    gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
}

void UIScene_WritableBookMenu::OnSignButtonPress()
{
    if (!m_signing)
    {
        if (m_textChanged)
            StoreBookText();
        m_signing = true;
        OnStartSigning();
        return;
    }
    if (!trimString(m_titleText).empty())
        Sign();
}

void UIScene_WritableBookMenu::OnDoneButtonPress()
{
    if (!m_canEdit)
        return;
    if (m_signing)
    {
        m_signing = false;
        OnStopSigning();
        return;
    }
    if (m_textChanged)
    {
        StoreBookText();
        SaveChanges(false);
    }
    navigateBack();
    gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
}

void UIScene_WritableBookMenu::ScrollPageLeft()
{
    if (m_currentPage > 0 && !m_signing)
    {
        if (m_textChanged)
            StoreBookText();
        --m_currentPage;
        RetrieveBookText();
        UpdatePageNumber();
        updateTooltips();
        FormatAndDisplayText(EFormatTextType_Display);
        SetCaretPosition(m_caretPosition);
    }
}

void UIScene_WritableBookMenu::appendPageToBook()
{
    if (m_pages != 0 && m_pages->size() < MAX_PAGES)
    {
        m_pages->add(new StringTag(L""));
        m_textChanged = true;
        ++m_pageCount;
    }
}

void UIScene_WritableBookMenu::ScrollPageRight()
{
    if (m_signing)
        return;
    if (m_textChanged)
        StoreBookText();
    if (m_currentPage >= m_pageCount - 1)
    {
        if (m_pageCount >= MAX_PAGES || !m_canEdit)
            return;
        appendPageToBook();
    }
    ++m_currentPage;
    RetrieveBookText();
    FormatAndDisplayText(EFormatTextType_Display);
    SetCaretPosition(m_caretPosition);
    UpdatePageNumber();
    updateTooltips();
}

void UIScene_WritableBookMenu::handleInput(int player, int action, bool repeat, bool pressed, bool released,
                                           bool& handled)
{
    gConsoleUIController.AnimateKeyPress(player, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (!pressed)
            return;
        HandleCancel();
        break;
    case EControllerActions_MenuOk:
        if (!pressed)
            return;
        if (GetBookHasFocus())
        {
            if (m_canEdit)
            {
                m_keyboardOpen = true;
                unsigned int maxLength = m_signing ? MAX_TITLE_LENGTH : MAX_PAGE_LENGTH;
                if (m_unusedControlByte == 0)
                {
                    if (m_signing)
                        g_pInput->RequestKeyboard(L10N::GetString(IDS_TITLE_KEYBOARD_BOOK_TITLE), m_titleText.c_str(),
                                                  m_playerId, maxLength, KeyboardCompleteCallback,
                                                  reinterpret_cast<void*>(GetCallbackUniqueId()),
                                                  static_cast<C_4JInput::EKeyboardMode>(3));
                    else
                        g_pInput->RequestKeyboard(L10N::GetString(IDS_TITLE_KEYBOARD_BOOK_TEXT), m_pageText.c_str(),
                                                  m_playerId, maxLength, KeyboardCompleteCallback,
                                                  reinterpret_cast<void*>(GetCallbackUniqueId()),
                                                  static_cast<C_4JInput::EKeyboardMode>(3));
                }
            }
        }
        else
        {
            if (m_signButton.hasFocus())
                OnSignButtonPress();
            else if (m_doneButton.hasFocus())
                OnDoneButtonPress();
        }
        break;
    case EControllerActions_MenuLeft:
        if (GetBookHasFocus())
        {
            if (!pressed && !repeat)
                return;
            int position = m_caretPosition - 1;
            if (position < 0)
                position = 0;
            m_caretPosition = position;
            SetCaretPosition(position);
            break;
        }
        UIScene::sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuRight:
        if (GetBookHasFocus())
        {
            if (!pressed && !repeat)
                return;
            int position = m_caretPosition;
            if (!m_signing)
            {
                if (static_cast<unsigned int>(position) < m_pageText.size())
                {
                    ++position;
                    m_caretPosition = position;
                }
                if (position > MAX_PAGE_LENGTH)
                    m_caretPosition = MAX_PAGE_LENGTH;
            }
            else
            {
                if (static_cast<unsigned int>(position) < m_titleText.size())
                {
                    ++position;
                    m_caretPosition = position;
                }
                if (position > MAX_TITLE_LENGTH)
                    m_caretPosition = MAX_TITLE_LENGTH;
            }
            SetCaretPosition(m_caretPosition);
            break;
        }
        UIScene::sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuLeftScroll:
        if (pressed)
            ScrollPageLeft();
        break;
    case EControllerActions_MenuRightScroll:
        if (pressed)
            ScrollPageRight();
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
        UIScene::sendInputToMovie(action, repeat, pressed, released);
        break;
    default:
        return;
    }
    handled = true;
}

void UIScene_WritableBookMenu::handleTouchInput(unsigned int, int, int, int touchBox, bool, bool, bool released)
{
    if (!released)
        return;
    if (touchBox == 2)
    {
        if (m_canEdit)
        {
            if (GetBookHasFocus())
            {
                bool handled = false;
                handleInput(m_playerId, EControllerActions_MenuOk, false, true, false, handled);
                return;
            }
            SetFocusToBook(true);
            m_doneButton.setFocus(false);
            m_signButton.setFocus(false);
        }
        return;
    }
    if (touchBox == 3)
        ScrollPageLeft();
    else if (touchBox == 4)
        ScrollPageRight();
}

void UIScene_WritableBookMenu::SetFocusToElement(int focus)
{
    if (static_cast<unsigned int>(focus) <= 1)
        SetFocusToBook(false);
    UIScene::SetFocusToElement(focus);
}

void UIScene_WritableBookMenu::handleDestroy()
{
    g_pInput->DestroyKeyboard();
}

void UIScene_WritableBookMenu::GetPageText()
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    m_pageText = menu->GetPageText(true);
}

void UIScene_WritableBookMenu::handleTimerComplete(int timerId)
{
    if (timerId == 10)
    {
        killTimer(10);
        if (!FormatAndDisplayText(EFormatTextType_Keyboard))
        {
            GetPageText();
            setCurrentPageText(m_pageText);
            m_caretPosition = static_cast<int>(m_pageText.size());
            FormatAndDisplayText(EFormatTextType_Display);
        }
        SetCaretPosition(m_caretPosition);
    }
}

void UIScene_WritableBookMenu::setCurrentPageText(std::wstring& text)
{
    if (m_pages != 0 && m_currentPage >= 0 && m_currentPage < m_pages->size())
    {
        static_cast<StringTag*>(m_pages->get(m_currentPage))->mData = text;
        m_textChanged = true;
    }
}

std::wstring UIScene_WritableBookMenu::getCurrentPageText()
{
    if (m_pages != 0 && m_currentPage >= 0 && m_currentPage < m_pages->size())
        return m_pages->get(m_currentPage)->toString();
    return L"";
}

int UIScene_WritableBookMenu::getSceneType()
{
    return EUIScene_WritableBookMenu;
}
