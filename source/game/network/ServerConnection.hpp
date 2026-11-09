#ifndef GAME_NETWORK_SERVERCONNECTION_HPP
#define GAME_NETWORK_SERVERCONNECTION_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/core/CriticalSection.hpp"

class MinecraftServer;
class Socket;
class ServerConnectionListener;
class ServerGamePacketListenerImpl;
class ServerSettingsChangedPacket;

class ServerConnection
{
public:
    explicit ServerConnection(MinecraftServer* server);
    ~ServerConnection();
    int addPendingTextureRequest(const std::wstring& request);
    void addServerGamePacketListenerImpl(boost::shared_ptr<ServerGamePacketListenerImpl> listener);
    void handleTextureReceived(const std::wstring& request);
    void handleTextureAndGeometryReceived(const std::wstring& request);
    void handleConnection(boost::shared_ptr<ServerConnectionListener> listener);
    void NewIncomingSocket(Socket* socket);
    void stop();
    void tick();
    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> >& getPlayers();
    void handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet);

private:
    int mUnused;
    int mConnectionCount;
    OSFastMutex mCriticalSection;
    std::vector<boost::shared_ptr<ServerConnectionListener> > mListeners;
    std::vector<boost::shared_ptr<ServerGamePacketListenerImpl> > mPlayers;
    std::vector<std::wstring> mPendingTextureRequests;
    MinecraftServer* mServer;
};

#endif
