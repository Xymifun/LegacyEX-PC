#include "game/ui/IUIScene_DlcSelectMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/cCommerce.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;

static const unsigned int kStringConnectToNintendoNetwork = 0x051d141b;
static const unsigned int kStringContinuePlayingOffline = 0x3f0766c7;
static const unsigned int kStringNotConnectedTitle = 0x8a21894b;
static const unsigned int kStringNotConnectedText = 0xb837024e;
static const unsigned int kStringError = 0x0e6bf122;
static const unsigned int kStringGuestsCannotPurchase = 0x8ba9896f;
static const unsigned int kStringUnlockMapPackTitle = 0x8983d4db;
static const unsigned int kStringUnlockMapPackText = 0xa9be05dc;
static const unsigned int kStringUnlockMashUpPackTitle = 0x1139ff03;
static const unsigned int kStringUnlockMashUpPackText = 0xa78136d4;

IUIScene_DlcSelectMenu::IUIScene_DlcSelectMenu(eDLCContentType contentType)
    : m_purchasePending(false),
      m_pack(NULL),
      m_contentType(contentType)
{
}

void IUIScene_DlcSelectMenu::tick()
{
}

int IUIScene_DlcSelectMenu::MustSignInReturned(void*, int userIndex, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Accept)
        CGameNetworkManager::sInstance.ShowConnectOnlineNintendoProgress(userIndex, NULL, NULL);
    return 0;
}

void IUIScene_DlcSelectMenu::showNotOnlineDialog(int userIndex)
{
    int messageParameters[2] = {kStringConnectToNintendoNetwork, kStringContinuePlayingOffline};
    gConsoleUIController.RequestErrorMessage(kStringNotConnectedTitle, kStringNotConnectedText, messageParameters, 2,
                                             userIndex, MustSignInReturned, this, NULL, true);
}

int IUIScene_DlcSelectMenu::UnlockPackReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    IUIScene_DlcSelectMenu* menu = static_cast<IUIScene_DlcSelectMenu*>(context);
    if (result == C4JStorage::EMessageResult_Accept && gProfile->IsSignedIn(userIndex))
    {
        if (gProfile->IsSignedInLive(userIndex))
        {
            cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
            if (!commerce->PurchaseDLCByPackID(menu->m_pack->GetPackId()))
            {
                int messageParameter = (int)IDS_CONFIRM_OK;
                gConsoleUIController.RequestErrorMessage(IDS_DLC_ALREADY_INSTALLED_TITLE,
                                                         IDS_DLC_ALREADY_INSTALLED_TEXT, &messageParameter, 1,
                                                         userIndex, NULL, NULL, NULL, true);
                menu->m_purchasePending = false;
                return 0;
            }
        }
        else
            menu->showNotOnlineDialog(userIndex);
    }
    menu->m_purchasePending = false;
    return 0;
}

void IUIScene_DlcSelectMenu::tryPurchaseDlc(int userIndex, bool& handled, eDLCContentType contentType)
{
    int messageParameter = (int)IDS_CONFIRM_OK;
    if (gProfile->IsGuest(userIndex))
    {
        gConsoleUIController.RequestAlertMessage(kStringError, kStringGuestsCannotPurchase, &messageParameter, 1,
                                                 userIndex, NULL, NULL, NULL, true);
        return;
    }
    if (!gProfile->IsSignedInLive(userIndex))
    {
        showNotOnlineDialog(userIndex);
        return;
    }

    cDLCInfo* dlcInfo = static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance());
    DLC_INFO* info = dlcInfo->GetDLCInfoForPackID(m_pack->GetPackId());
    if (info == NULL)
        return;

    m_contentType = (eDLCContentType)info->ContentType;
    cCommerce* commerce = static_cast<cCommerce*>((void*)gApp.GetCommerceInstance());
    if (commerce->CheckForEmptyStore(userIndex) != 0)
        return;

    m_purchasePending = true;
    handled = false;
    int parameters[2] = {(int)IDS_CONFIRM_OK, (int)IDS_CONFIRM_CANCEL};
    eDLCContentType selectedType = m_contentType;
    if (contentType != eDLCContentType_All)
        selectedType = contentType;
    unsigned int titleId;
    unsigned int messageId;
    switch (selectedType)
    {
    case eDLCContentType_SkinPack:
        titleId = IDS_UNLOCK_DLC_TITLE;
        messageId = IDS_UNLOCK_DLC_SKIN;
        break;
    case eDLCContentType_MashupPack:
        titleId = kStringUnlockMashUpPackTitle;
        messageId = kStringUnlockMashUpPackText;
        break;
    default:
        titleId = kStringUnlockMapPackTitle;
        messageId = kStringUnlockMapPackText;
        break;
    }
    gConsoleUIController.RequestAlertMessage(titleId, messageId, parameters, 2, userIndex, UnlockPackReturned, this,
                                             NULL, true);
}
