#ifndef GAME_UI_IUISCENE_DLCSELECTMENU_HPP
#define GAME_UI_IUISCENE_DLCSELECTMENU_HPP

#include <stdint.h>

#include "platform/wiiu/C4JStorage.hpp"

class DLCPack;
class UIScene_SkinSelectMenu;
class UIScene_MapSelectMenu;

enum eDLCContentType
{
    eDLCContentType_MapPack = 0,
    eDLCContentType_SkinPack = 1,
    eDLCContentType_TexturePack = 2,
    eDLCContentType_MashupPack = 3,
    eDLCContentType_BundlePack = 4,
    eDLCContentType_All = 11
};

class IUIScene_DlcSelectMenu
{
public:
    friend class UIScene_SkinSelectMenu;

    IUIScene_DlcSelectMenu(eDLCContentType contentType);

    void tick();
    static int MustSignInReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    void showNotOnlineDialog(int userIndex);
    static int UnlockPackReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    void tryPurchaseDlc(int userIndex, bool& handled, eDLCContentType contentType);

private:
    friend class UIScene_MapSelectMenu;

    bool m_purchasePending;
    DLCPack* m_pack;
    eDLCContentType m_contentType;
};

#endif
