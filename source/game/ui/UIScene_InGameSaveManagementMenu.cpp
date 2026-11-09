#include "game/ui/UIScene_InGameSaveManagementMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringConversion.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_SaveListDetails.hpp"

extern CConsoleMinecraftApp gApp;
extern ConsoleUIController gConsoleUIController;
extern C_4JProfile* gProfile;
extern C4JStorage* gStorage;

UIScene_InGameSaveManagementMenu::UIScene_InGameSaveManagementMenu(int playerId, void*, UILayer* layer)
    : UIScene(playerId, layer)
{
    initialiseMovie();
    m_nextThumbnail = 0;
    m_saveListRefreshState = 0;
    m_ignoreInput = false;
    m_pendingAction = ePendingAction_None;
    m_saveList.init(0);
    m_saveListTitle.init(UIString(L10N::GetString(IDS_SAVE_INCOMPLETE_DELETE_SAVES)));
    m_saveTimer.setVisible(true);
    m_saveSizeBar.init(UIString(L""), 1, 0, SAVE_DATA_QUOTA);
    m_selectionChanged = false;
    m_thumbnailsComplete = false;
    m_thumbnailRequested = false;
    m_thumbnailReturned = false;
    m_closing = false;
    m_saveDetails = NULL;
    m_saveInfoReady = false;
    m_saveListDetails = NULL;
    m_saveDetailsCount = 0;
    gStorage->ClearSavesInfo();
    if (gApp.StartInstallDLCProcess(m_playerId) != 0 || gApp.DLCInstallPending() != 0)
    {
        m_ignoreInput = true;
    }
    else
    {
        Initialise();
        if (!m_ignoreInput)
            gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
    }
    layer->addComponent(playerId, EUIScene_MenuBackground, NULL);
}

UIScene_InGameSaveManagementMenu::~UIScene_InGameSaveManagementMenu()
{
    m_layer->removeComponent(EUIScene_MenuBackground);
    if (m_saveListDetails != NULL)
    {
        for (int i = 0; i < m_saveDetailsCount; ++i)
            delete m_saveListDetails[i].m_thumbnail;
        delete[] m_saveListDetails;
    }
    gApp.LeaveSaveNotificationSection();
    gStorage->ContinueIncompleteOperation();
}

void UIScene_InGameSaveManagementMenu::GetSaveInfo()
{
    m_saveInfoReady = false;
    m_saveList.clearList();
    m_saveListRefreshState = 0;
    m_saveTimer.setVisible(true);
    m_saveDetails = gStorage->ReturnSavesInfo();
    if (m_saveDetails == NULL)
        gStorage->GetSavesInfo(m_playerId, NULL, this, "save");
}

void UIScene_InGameSaveManagementMenu::Initialise()
{
    m_selectedSaveIndex = 0;
    if (gProfile->IsFullVersion())
    {
        if (!gStorage->GetSaveDisabled())
            gStorage->EnoughSpaceForAMinSaveGame();
        GetSaveInfo();
    }
    m_ignoreInput = false;
}

unsigned int UIScene_InGameSaveManagementMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_saveList.setupControl(this, currentNode, std::string("SavesList"));
        m_controls.push_back(&m_saveList);
    }
    {
        m_saveListTitle.setupControl(this, currentNode, std::string("SavesListTitle"));
        m_controls.push_back(&m_saveListTitle);
    }
    {
        m_saveTimer.setupControl(this, currentNode, std::string("SavesTimer"));
        m_controls.push_back(&m_saveTimer);
    }
    {
        m_saveSizeBar.setupControl(this, currentNode, std::string("SaveSizeBar"));
        m_controls.push_back(&m_saveSizeBar);
    }
    return 1;
}

void UIScene_InGameSaveManagementMenu::updateTooltips()
{
    int deleteTooltip = -1;
    if (m_saveInfoReady && m_saveDetailsCount > 0)
        deleteTooltip = IDS_TOOLTIPS_DELETESAVE;
    gConsoleUIController.SetTooltips(m_playerId, deleteTooltip, IDS_SAVE_INCOMPLETE_RETRY_SAVING, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, m_layer->IsFullscreenGroup());
}

void UIScene_InGameSaveManagementMenu::handleReload()
{
    m_ignoreInput = false;
    m_nextThumbnail = 0;
    m_thumbnailsComplete = false;
    m_thumbnailRequested = false;
    m_saveInfoReady = false;
    m_saveListRefreshState = 0;
}

void UIScene_InGameSaveManagementMenu::handleGainFocus(bool navigatedBack)
{
    updateTooltips();
    if (navigatedBack)
        m_ignoreInput = false;
}

void UIScene_InGameSaveManagementMenu::handleInput(int, int action, bool repeat, bool pressed, bool released,
                                                   bool& handled)
{
    if (m_ignoreInput || !m_saveInfoReady)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (!pressed)
            break;
        navigateBack();
        handled = true;
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    }
}

void UIScene_InGameSaveManagementMenu::handleFocusChange(int, int childId)
{
    m_selectedSaveIndex = childId;
    if (m_saveInfoReady)
        m_selectionChanged = true;
    updateTooltips();
}

void UIScene_InGameSaveManagementMenu::handleInitFocus(int, int)
{
}

bool UIScene_InGameSaveManagementMenu::hasFocus(int playerId)
{
    return m_hasFocus && (playerId == m_playerId || m_playerId == 0xff);
}

int UIScene_InGameSaveManagementMenu::LoadSaveDataThumbnailReturned(void* context, unsigned char* data,
                                                                    unsigned int size)
{
    UIScene_InGameSaveManagementMenu* menu = static_cast<UIScene_InGameSaveManagementMenu*>(context);
    if (data != NULL && size != 0)
    {
        menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnail = new unsigned char[size];
        std::memcpy(menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnail, data, size);
        menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnailSize = size;
    }
    else
    {
        menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnail = NULL;
        menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnailSize = 0;
    }
    menu->m_thumbnailReturned = true;
    return 0;
}

void UIScene_InGameSaveManagementMenu::tick()
{
    UIScene::tick();
    if (m_closing && !m_thumbnailRequested)
        navigateBack();

    if (hasFocus(m_playerId))
    {
        if (m_selectionChanged)
        {
            m_saveSizeBar.selectSave(m_selectedSaveIndex);
            m_selectionChanged = false;
        }

        if (!m_saveInfoReady)
        {
            m_saveDetails = gStorage->ReturnSavesInfo();
            if (m_saveDetails != NULL)
            {
                m_saveSizeBar.reset();
                m_saveInfoReady = true;
                if (m_saveListDetails != NULL)
                {
                    for (int i = 0; i < m_saveDetails->count; ++i)
                    {
                        if (m_saveListDetails[i].m_thumbnail != NULL)
                            delete m_saveListDetails[i].m_thumbnail;
                    }
                    delete[] m_saveListDetails;
                }
                m_saveListDetails = new _SaveListDetails[m_saveDetails->count];
                m_saveDetailsCount = m_saveDetails->count;
                for (int i = 0; i < m_saveDetails->count; ++i)
                {
                    m_saveSizeBar.addSave(m_saveDetails->saves[i].nandSize);
                    m_saveList.addItem(std::wstring(m_saveDetails->saves[i].title), L"");
                    m_saveListDetails[i].m_saveIndex = i;
                    std::memcpy(m_saveListDetails[i].m_title, m_saveDetails->saves[i].title,
                                sizeof(m_saveDetails->saves[i].title));
                    std::memcpy(m_saveListDetails[i].m_filename, m_saveDetails->saves[i].filename,
                                sizeof(m_saveDetails->saves[i].filename));
                }
                m_saveTimer.setVisible(false);
            }
        }

        if (m_saveInfoReady)
        {
            if (!m_closing && !m_thumbnailRequested && !m_thumbnailsComplete)
            {
                if (m_nextThumbnail < static_cast<int>(m_saveList.getItemCount()))
                {
                    m_thumbnailRequested = true;
                    SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
                    if (gStorage->LoadSaveDataThumbnail(&details->saves[m_nextThumbnail],
                                                        &LoadSaveDataThumbnailReturned, this)
                        != C4JStorage::ESaveGame_GetThumbnail)
                    {
                        m_thumbnailsComplete = true;
                        m_thumbnailRequested = false;
                    }
                }
            }
            else if (m_thumbnailReturned)
            {
                m_thumbnailReturned = false;
                if (!m_closing)
                {
                    std::wstring saveFileName = convUtf8ToWstring(m_saveListDetails[m_nextThumbnail].m_filename);
                    wchar_t thumbnailName[32];
                    wcsncpy(thumbnailName, saveFileName.c_str(), 32);
                    if (m_saveListDetails[m_nextThumbnail].m_thumbnail != NULL)
                    {
                        registerSubstitutionTexture(std::wstring(thumbnailName),
                                                    m_saveListDetails[m_nextThumbnail].m_thumbnail,
                                                    m_saveListDetails[m_nextThumbnail].m_thumbnailSize, false);
                    }
                    m_saveList.setTextureName(m_nextThumbnail, std::wstring(thumbnailName));
                    ++m_nextThumbnail;
                    if (m_nextThumbnail < static_cast<int>(m_saveList.getItemCount()))
                    {
                        SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
                        if (gStorage->LoadSaveDataThumbnail(&details->saves[m_nextThumbnail],
                                                            &LoadSaveDataThumbnailReturned, this)
                            != C4JStorage::ESaveGame_GetThumbnail)
                        {
                            m_thumbnailRequested = false;
                            m_thumbnailsComplete = true;
                        }
                    }
                    else
                    {
                        m_thumbnailRequested = false;
                        m_thumbnailsComplete = true;
                    }
                }
                else
                {
                    m_thumbnailRequested = false;
                }
            }
        }
    }

    if (m_pendingAction == ePendingAction_RebuildSaveList)
    {
        m_thumbnailRequested = false;
        m_saveListRefreshState = 0;
        m_ignoreInput = false;
        m_thumbnailsComplete = false;
        m_nextThumbnail = 0;
        m_saveInfoReady = false;
        m_saveList.clearList();
        m_pendingAction = ePendingAction_None;
    }
}

std::wstring UIScene_InGameSaveManagementMenu::getMoviePath()
{
    return L"SaveMenu";
}

int UIScene_InGameSaveManagementMenu::DeleteSaveDataReturned(void* context, bool success)
{
    UIScene_InGameSaveManagementMenu* menu = static_cast<UIScene_InGameSaveManagementMenu*>(context);
    if (success)
        menu->m_pendingAction = ePendingAction_RebuildSaveList;
    else
        menu->m_ignoreInput = false;
    menu->updateTooltips();
    return 0;
}

int UIScene_InGameSaveManagementMenu::DeleteSaveDialogReturned(void* context, int, C4JStorage::EMessageResult result)
{
    UIScene_InGameSaveManagementMenu* menu = static_cast<UIScene_InGameSaveManagementMenu*>(context);
    if (result == C4JStorage::EMessageResult_Decline
        && (!gApp.DebugSettingsOn() || !gApp.GetLoadSavesFromFolderEnabled()))
    {
        gStorage->DeleteSaveData(&menu->m_saveDetails->saves[menu->m_selectedSaveIndex], &DeleteSaveDataReturned, menu);
        menu->m_saveTimer.setVisible(true);
    }
    else
    {
        menu->m_ignoreInput = false;
    }
    return 0;
}

void UIScene_InGameSaveManagementMenu::handlePress(int controlId, int)
{
    if (controlId != 0)
        return;
    m_ignoreInput = true;
    int options[2];
    options[0] = IDS_CONFIRM_CANCEL;
    options[1] = IDS_CONFIRM_OK;
    gConsoleUIController.RequestErrorMessage(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, options, 2, m_playerId,
                                             &DeleteSaveDialogReturned, this, NULL, true);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

int UIScene_InGameSaveManagementMenu::getSceneType()
{
    return EUIScene_LoadCreateJoinMenu;
}

bool UIScene_InGameSaveManagementMenu::hidesLowerScenes()
{
    return true;
}
