#include "game/network/CPlatformNetworkManager.hpp"
#include "platform/wiiu/CPlatformNetworkManagerNintendo.hpp"

CPlatformNetworkManager* CPlatformNetworkManager::Create()
{
    return new CPlatformNetworkManagerNintendo;
}

void CPlatformNetworkManager::CancelJoinGame()
{
}

void CPlatformNetworkManager::FakeLocalPlayerJoined()
{
}
