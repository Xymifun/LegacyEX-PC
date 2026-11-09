#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_WIIU_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_WIIU_HPP

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "game/network/INQRNetworkManagerListener.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/inet/NexSessionSearchCriteria.hpp"
#include "nn/pia/lan/LanSessionInfo.hpp"
#include "nn/pia/session/Session.hpp"
#include "platform/pc/PcNetworkSession.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"

#include <nn/fp/fp_cpp.h>
#include <nn/result.h>

class CMatchMakeSearchParam
{
public:
    enum EComparison
    {
        eComparison_Equal,
        eComparison_NotEqual,
        eComparison_LessThan,
        eComparison_LessThanOrEqual,
        eComparison_GreaterThan,
        eComparison_GreaterThanOrEqual
    };

    unsigned int id;
    unsigned int value;
    unsigned int comparison;
};

class NQRNetworkManager_WiiU : public PcNetworkSession
{
public:
    NQRNetworkManager_WiiU(INQRNetworkManagerListener* listener);

    void Pia_ShutdownForLan();

    static bool IsHandlingBootInvite();
    static void ClearIsHandlingBootInvite();
    static void SetAppSuspended(bool suspended);
    static void ShowErrorViewer();
    static int ShowConnectOnlineNintendoProgress(int userIndex, int (*callback)(void*, bool, int, int),
                                                 void* callbackData);

    static bool s_applicationConnected;
    static bool s_piaShutdownPending;
};

extern bool g_bGameBootInvitePending;
extern bool g_bHandlingGameBootInvite;
extern unsigned int g_uiGameBootInvitePrincipalId;

#endif
