#ifndef GAME_NETWORK_CONNECTION_HPP
#define GAME_NETWORK_CONNECTION_HPP
#include <deque>
#include <stdint.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/core/CriticalSection.hpp"
#include "game/core/Socket.hpp"
#include "game/network/DisconnectPacket.hpp"
class ByteArrayOutputStream;
class DataInputStream;
class DataOutputStream;
class OutputStream;
class Packet;
class PacketListener;
class ServerGamePacketListenerImpl;
class C4JEventImpl;
class C4JThreadImpl;
class Connection
{
public:
    Connection(Socket*, const std::wstring&, PacketListener*);
    ~Connection();
    bool getAndSetRunning(bool running);
    void _init();
    bool readTick();
    static int runRead(void*);
    bool writeTick();
    static int runWrite(void*);
    void setListener(PacketListener*);
    void send(boost::shared_ptr<Packet>);
    void queueSend(boost::shared_ptr<Packet>);
    void flush();
    void close(DisconnectPacket::eDisconnectReason);
    void tick();
    void sendAndQuit();
    int countDelayedPackets();
    void* getRemoteAddress();
    Socket* getSocket() { return m_socket; }
    static int runClose(void*);
    static int runSendAndQuit(void*);

private:
    friend class ServerGamePacketListenerImpl;
    friend class ClientPacketListener;
    friend class PlayerList;
    friend class ServerConnectionListener;

public:
    Socket* m_socket;

private:
    unsigned int m_address;
    DataInputStream* m_input;
    DataOutputStream* m_bufferedOutput;
    DataOutputStream* m_byteArrayOutput;
    ByteArrayOutputStream* m_byteArrayStream;
    Socket::SocketOutputStream* m_socketOutput;
    bool m_running;
    OSFastMutex m_runningLock;
    std::deque<boost::shared_ptr<Packet> > m_incoming;
    OSFastMutex m_incomingLock;
    std::deque<boost::shared_ptr<Packet> > m_outgoing;
    std::deque<boost::shared_ptr<Packet> > m_outgoingSlow;
    PacketListener* m_listener;
    bool m_quitting;
    bool m_readError;
    C4JThreadImpl* m_readThread;
    C4JThreadImpl* m_writeThread;
    C4JEventImpl* m_readEvent;
    C4JEventImpl* m_writeEvent;
    C4JThreadImpl* m_closeThread;
    C4JThreadImpl* m_sendAndQuitThread;
    bool m_disconnected;
    DisconnectPacket::eDisconnectReason m_disconnectReason;
    void* m_disconnectData;
    int m_noInputTicks;
    int m_estimatedRemaining;
    unsigned int m_packetsRead;
    unsigned int m_packetsWritten;
    int m_fakeLag;
#ifdef MINECRAFT_PC
    long long m_pcQueueLogged;
#endif
    OSFastMutex m_threadCountLock;
    OSFastMutex m_writeLock;
    int m_slowWriteDelay;
    long long m_lastSendTime;
};
#endif
