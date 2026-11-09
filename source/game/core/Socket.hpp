#ifndef GAME_CORE_SOCKET_HPP
#define GAME_CORE_SOCKET_HPP
#include <queue>
#include "game/core/InputStream.hpp"
#include "game/core/OutputStream.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/world/biome/arrayWithLength.hpp"
class C4JEventImpl;
class INetworkPlayer;
class ServerConnection;
class Socket
{
    friend class Connection;
    friend class ServerGamePacketListenerImpl;
    friend class ClientPacketListener;
    friend class PlayerList;
    friend class CPlatformNetworkManagerNintendo;
    friend void closeSocketForLockedProfile();

public:
    class SocketInputStreamLocal : public InputStream
    {
        friend class Socket;

    public:
        explicit SocketInputStreamLocal(int);
        virtual ~SocketInputStreamLocal();
        virtual int read();
        virtual int read(arrayWithLength<unsigned char>);
        virtual int read(arrayWithLength<unsigned char>, unsigned int, unsigned int);
        virtual void close();
        virtual long long skip(long long);
        virtual void flush();

    private:
        bool mOpen;
        int mIndex;
    };
    class SocketInputStreamNetwork : public InputStream
    {
    public:
        SocketInputStreamNetwork(Socket*, int);
        virtual ~SocketInputStreamNetwork();
        virtual int read();
        virtual int read(arrayWithLength<unsigned char>);
        virtual int read(arrayWithLength<unsigned char>, unsigned int, unsigned int);
        virtual void close();
        virtual long long skip(long long);
        virtual void flush();

    private:
        bool mOpen;
        int mIndex;
        Socket* mSocket;
    };
    class SocketOutputStream : public OutputStream
    {
    public:
        virtual ~SocketOutputStream();
        virtual void writeWithFlags(arrayWithLength<unsigned char>, unsigned int, unsigned int, int);
    };
    class SocketOutputStreamLocal : public SocketOutputStream
    {
        friend class Socket;

    public:
        explicit SocketOutputStreamLocal(int);
        virtual ~SocketOutputStreamLocal();
        virtual void write(unsigned int);
        virtual void write(arrayWithLength<unsigned char>);
        virtual void write(arrayWithLength<unsigned char>, unsigned int, unsigned int);
        virtual void close();
        virtual void flush();

    private:
        bool mOpen;
        int mIndex;
    };
    class SocketOutputStreamNetwork : public SocketOutputStream
    {
    public:
        SocketOutputStreamNetwork(Socket*, int);
        virtual ~SocketOutputStreamNetwork();
        virtual void write(unsigned int);
        virtual void write(arrayWithLength<unsigned char>);
        virtual void write(arrayWithLength<unsigned char>, unsigned int, unsigned int);
        virtual void writeWithFlags(arrayWithLength<unsigned char>, unsigned int, unsigned int, int);
        virtual void close();
        virtual void flush();

    private:
        bool mOpen;
        int mIndex;
        Socket* mSocket;
    };
    Socket(bool response = false);
    Socket(INetworkPlayer* player, bool response, bool hostLocal);
    INetworkPlayer* getPlayer();
    void setPlayer(INetworkPlayer*);
    static void Initialise(ServerConnection*);
    unsigned int getRemoteSocketAddress();
    void pushDataToQueue(const unsigned char*, unsigned int, bool);
    static void addIncomingSocket(Socket*);
    InputStream* getInputStream(bool);
    void setSoTimeout(int);
    void setTrafficClass(int);
    OutputStream* getOutputStream(bool);
    bool close(bool);
    bool isLocal() { return mHostLocal; }

private:
    bool mHostServerConnection;
    bool mHostLocal;
    int mIndex;
    std::queue<unsigned char> mQueues[2];
    OSFastMutex mMutexes[2];
    InputStream* mInputStreams[2];
    OutputStream* mOutputStreams[2];
    bool mClosed[2];
    unsigned char mPlayerId;
    C4JEventImpl* mCloseEvent;
    bool mOpen;
    static ServerConnection* sConnection;
};
#endif
