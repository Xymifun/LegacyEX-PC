#include "game/network/ServerConnection.hpp"
#include "game/network/Connection.hpp"
#include "game/core/Socket.hpp"
#include "game/network/ServerConnectionListener.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include <algorithm>
#include "game/core/StringConversion.hpp"

ServerConnection::ServerConnection(MinecraftServer* server)
{
    mConnectionCount = 0;
    InitializeCriticalSection(&mCriticalSection);
    mServer = server;
}

ServerConnection::~ServerConnection()
{
    DeleteCriticalSection(&mCriticalSection);
}

int ServerConnection::addPendingTextureRequest(const std::wstring& request)
{
    std::vector<std::wstring>::iterator it = std::find(mPendingTextureRequests.begin(), mPendingTextureRequests.end(),
                                                       request);
    if (it == mPendingTextureRequests.end())
    {
        mPendingTextureRequests.push_back(request);
    }
    return 1;
}

void ServerConnection::addServerGamePacketListenerImpl(boost::shared_ptr<ServerGamePacketListenerImpl> listener)
{
    mPlayers.push_back(listener);
}

void ServerConnection::handleTextureReceived(const std::wstring& request)
{
    std::vector<std::wstring>::iterator requestIterator = std::find(mPendingTextureRequests.begin(),
                                                                    mPendingTextureRequests.end(), request);
    if (requestIterator != mPendingTextureRequests.end())
    {
        mPendingTextureRequests.erase(requestIterator);
    }
    for (unsigned int playerIndex = 0; playerIndex < mPlayers.size(); ++playerIndex)
    {
        boost::shared_ptr<ServerGamePacketListenerImpl> player = mPlayers[playerIndex];
        if (!player->mDisconnected)
        {
            player->handleTextureReceived(request);
        }
    }
}

void ServerConnection::handleTextureAndGeometryReceived(const std::wstring& request)
{
    std::vector<std::wstring>::iterator requestIterator = std::find(mPendingTextureRequests.begin(),
                                                                    mPendingTextureRequests.end(), request);
    if (requestIterator != mPendingTextureRequests.end())
    {
        mPendingTextureRequests.erase(requestIterator);
    }
    for (unsigned int playerIndex = 0; playerIndex < mPlayers.size(); ++playerIndex)
    {
        boost::shared_ptr<ServerGamePacketListenerImpl> player = mPlayers[playerIndex];
        if (!player->mDisconnected)
        {
            player->handleTextureAndGeometryReceived(request);
        }
    }
}

void ServerConnection::handleConnection(boost::shared_ptr<ServerConnectionListener> listener)
{
    EnterCriticalSection(&mCriticalSection);
    mListeners.push_back(listener);
    LeaveCriticalSection(&mCriticalSection);
}

void ServerConnection::NewIncomingSocket(Socket* socket)
{
    boost::shared_ptr<ServerConnectionListener> listener(
        new ServerConnectionListener(mServer, socket, L"Connection #" + _toString<int>(mConnectionCount++)));
    handleConnection(listener);
}

void ServerConnection::stop()
{
    EnterCriticalSection(&mCriticalSection);
    for (unsigned int listenerIndex = 0; listenerIndex < mListeners.size(); ++listenerIndex)
    {
        boost::shared_ptr<ServerConnectionListener> listener = mListeners[listenerIndex];
        listener->m_connection->close(DisconnectPacket::eDisconnectReason_ConnectionClosed);
    }
    LeaveCriticalSection(&mCriticalSection);

    for (unsigned int playerIndex = 0; playerIndex < mPlayers.size(); ++playerIndex)
    {
        boost::shared_ptr<ServerGamePacketListenerImpl> player = mPlayers[playerIndex];
        player->mConnection->close(DisconnectPacket::eDisconnectReason_ConnectionClosed);
    }
}

void ServerConnection::tick()
{
    {
        EnterCriticalSection(&mCriticalSection);
        std::vector<boost::shared_ptr<ServerConnectionListener> > listeners(mListeners);
        LeaveCriticalSection(&mCriticalSection);

        for (unsigned int listenerIndex = 0; listenerIndex < listeners.size(); ++listenerIndex)
        {
            boost::shared_ptr<ServerConnectionListener> listener = listeners[listenerIndex];
            listener->tick();
            if (listener->m_connection)
            {
                listener->m_connection->flush();
            }
        }
    }

    EnterCriticalSection(&mCriticalSection);
    for (unsigned int listenerIndex = 0; listenerIndex < mListeners.size(); ++listenerIndex)
    {
        if (mListeners[listenerIndex]->m_disconnected)
        {
            mListeners.erase(mListeners.begin() + listenerIndex);
            --listenerIndex;
        }
    }
    LeaveCriticalSection(&mCriticalSection);

    for (unsigned int playerIndex = 0; playerIndex < mPlayers.size(); ++playerIndex)
    {
        boost::shared_ptr<ServerGamePacketListenerImpl> listener = mPlayers[playerIndex];
        boost::shared_ptr<ServerPlayer> player = listener->mPlayer;
        if (player && player->mConnection)
        {
            player->updateFrameTick();
#ifdef MINECRAFT_PC
            player->DoChunkSendingTicks();
#else
            player->doChunkSendingTick(false, false);
#endif
        }
        listener->tick();
        if (listener->mDisconnected)
        {
            mPlayers.erase(mPlayers.begin() + playerIndex);
            --playerIndex;
        }
        listener->mConnection->flush();
    }
}

std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >& ServerConnection::getPlayers()
{
    return mPlayers;
}
