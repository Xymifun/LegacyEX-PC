#ifndef GAME_CORE_IUISCENE_DLCUPSELL_HPP
#define GAME_CORE_IUISCENE_DLCUPSELL_HPP

#include "platform/wiiu/C4JStorage.hpp"

class IUIScene_DLCUpsell
{
public:
    virtual void SetIgnoreInput(bool ignore) = 0;

    static int NotOnlineDialogCallback(void* context, int userIndex, C4JStorage::EMessageResult result);
    void ShowNotOnlineDialog(int userIndex);
};

#endif
