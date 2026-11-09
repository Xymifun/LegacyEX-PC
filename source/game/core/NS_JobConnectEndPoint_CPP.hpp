#ifndef GAME_CORE_NS_JOBCONNECTENDPOINT_CPP_HPP
#define GAME_CORE_NS_JOBCONNECTENDPOINT_CPP_HPP

#include <nex.h>

namespace NS_JobConnectEndPoint_CPP
{
void ConnectCallback(nn::nex::EndPoint* pEndPoint, nn::nex::qResult resultConnect,
                     const nn::nex::UserContext* pUserContext);
void DisconnectCallback(nn::nex::EndPoint* pEndPoint, nn::nex::qResult resultDisconnect,
                        const nn::nex::UserContext* pUserContext);
}

#endif
