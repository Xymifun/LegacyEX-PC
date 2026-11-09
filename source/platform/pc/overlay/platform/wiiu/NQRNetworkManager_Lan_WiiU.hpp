#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_LAN_WIIU_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_LAN_WIIU_HPP

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

class NQRNetworkManager_Lan_WiiU : public PcNetworkSession
{
public:
    enum ESaveTransferState
    {
        eSaveTransfer_Idle,
        eSaveTransfer_CreatingSession,
        eSaveTransfer_CreateSessionFailed,
        eSaveTransfer_WaitingForReceiver,
        eSaveTransfer_Transferring,
        eSaveTransfer_SendComplete,
        eSaveTransfer_Searching,
        eSaveTransfer_NoSessionFound,
        eSaveTransfer_Unused,
        eSaveTransfer_SessionFound,
        eSaveTransfer_Joining,
        eSaveTransfer_JoinFailed,
        eSaveTransfer_Joined,
        eSaveTransfer_ReceiveComplete,
        eSaveTransfer_ConnectionLost
    };

    NQRNetworkManager_Lan_WiiU(INQRNetworkManagerListener* listener);

    void SignalInitialiseAfterOnline();
    void Pia_ShutdownForOnline();
    void CreateSessionForSaveTransfer(int dataSize, void* data, int metadataSize, void* metadata, long long seed,
                                      unsigned int hostOptions, unsigned int texturePack, wchar_t* name);

    ESaveTransferState m_saveTransferState;
    unsigned int m_saveTransferOffset;
    unsigned int m_saveTransferSize;

    static bool s_shutdownTickPending;
};

extern NQRNetworkManager_Lan_WiiU* gLanNetworkManager;

#endif
