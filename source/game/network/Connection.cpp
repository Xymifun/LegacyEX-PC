#include "game/network/Connection.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"

#include <climits>
#include <cstdio>

#include "game/core/BufferedOutputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/Socket.hpp"
#include "game/core/System.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundKeepAlivePacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/network/ServerboundKeepAlivePacket.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

char* wstringtofilename(const std::wstring& path, char (&filename)[256]);
extern unsigned char s_altMainStopRequested;

static const int SOCKET_TIMEOUT_MS = 30000;
static const int IPTOS_LOWDELAY = 16;
static const int IPTOS_THROUGHPUT = 8;
static const int STREAM_BUFFER_SIZE = 5120;
static const int SLOW_WRITE_DELAY = 50;
static const int MAX_ESTIMATED_REMAINING = 1048576;
static const int NO_INPUT_TIMEOUT_TICKS = 1200;
static const int KEEP_ALIVE_INTERVAL_MS = 1000;
static const int MAX_PACKETS_PER_TICK = 1000;
static const int WRITE_BURST_MS = 200;
static const int THREAD_WAIT_MS = 100;
static const int CONNECTION_THREAD_PROCESSOR = 2;
static const int CONNECTION_THREAD_PRIORITY = 1;
static const unsigned int WAIT_SIGNALLED = 0;
static const unsigned int WAIT_TIMED_OUT = 258;
static const int PACKET_ID_COUNT = 256;
static const int PLAYER_ACTION_PACKET_ID = 14;
static const int MAP_ITEM_DATA_PACKET_ID = 211;

static int sReadThreadCount;
static int sWriteThreadCount;
static int sReadSizes[PACKET_ID_COUNT];
static int sWriteSizes[PACKET_ID_COUNT];
static char sConnectionName[256];

Connection::Connection(Socket* socket, const std::wstring& name, PacketListener* listener)
    : m_incoming(),
      m_outgoing(),
      m_outgoingSlow()
{
    _init();
    m_socket = socket;
    m_address = socket->getRemoteSocketAddress();
    m_listener = listener;
    socket->setSoTimeout(SOCKET_TIMEOUT_MS);
    socket->setTrafficClass(IPTOS_THROUGHPUT | IPTOS_LOWDELAY);
    m_input = new DataInputStream(socket->getInputStream(listener->isServerPacketListener()));
    m_socketOutput = static_cast<Socket::SocketOutputStream*>(
        socket->getOutputStream(listener->isServerPacketListener()));
    m_bufferedOutput = new DataOutputStream(new BufferedOutputStream(m_socketOutput, STREAM_BUFFER_SIZE));
    m_byteArrayStream = new ByteArrayOutputStream(STREAM_BUFFER_SIZE);
    m_byteArrayOutput = new DataOutputStream(m_byteArrayStream);
    m_readEvent = new C4JEventImpl(C4JEvent::Mode0);
    m_writeEvent = new C4JEventImpl(C4JEvent::Mode0);

    char readThreadName[256];
    char writeThreadName[256];
    char* connectionName = wstringtofilename(name, sConnectionName);
    std::sprintf(readThreadName, "%s read\n", connectionName);
    std::sprintf(writeThreadName, "%s write\n", connectionName);
    m_readThread = new C4JThreadImpl(&Connection::runRead, this, readThreadName, 0);
    m_writeThread = new C4JThreadImpl(&Connection::runWrite, this, writeThreadName, 0);
    m_readThread->SetProcessor(CONNECTION_THREAD_PROCESSOR);
    m_writeThread->SetProcessor(CONNECTION_THREAD_PROCESSOR);
    m_readThread->SetPriority(CONNECTION_THREAD_PRIORITY);
    m_writeThread->SetPriority(CONNECTION_THREAD_PRIORITY);
    m_readThread->Run();
    m_writeThread->Run();
    m_lastSendTime = 0;
}

Connection::~Connection()
{
    m_running = false;
    DeleteCriticalSection(&m_runningLock);
    if (m_input != NULL)
        m_input->close();
    m_readThread->WaitForCompletion(-1);
    m_writeThread->WaitForCompletion(-1);
    DeleteCriticalSection(&m_writeLock);
    DeleteCriticalSection(&m_threadCountLock);
    DeleteCriticalSection(&m_incomingLock);
    delete m_readEvent;
    delete m_writeEvent;
    delete m_byteArrayOutput;
    m_byteArrayOutput = NULL;
    delete m_byteArrayStream;
    m_byteArrayStream = NULL;
    if (m_bufferedOutput != NULL)
    {
        m_bufferedOutput->deleteChildStream();
        delete m_bufferedOutput;
        m_bufferedOutput = NULL;
    }
    delete m_input;
    m_input = NULL;
    delete m_readThread;
    m_readThread = NULL;
    delete m_writeThread;
    m_writeThread = NULL;
}

void Connection::_init()
{
    InitializeCriticalSection(&m_writeLock);
    InitializeCriticalSection(&m_threadCountLock);
    InitializeCriticalSection(&m_incomingLock);
    m_running = true;
    InitializeCriticalSection(&m_runningLock);
    m_estimatedRemaining = 0;
    m_slowWriteDelay = SLOW_WRITE_DELAY;
#ifdef MINECRAFT_PC
    m_pcQueueLogged = 0;
#endif
    m_noInputTicks = 0;
    m_disconnectReason = DisconnectPacket::eDisconnectReason_None;
    m_disconnected = false;
    m_quitting = false;
    m_fakeLag = 0;
    m_closeThread = NULL;
    m_sendAndQuitThread = NULL;
    m_packetsRead = 0;
    m_packetsWritten = 0;
    m_readError = false;
}

bool Connection::getAndSetRunning(bool running)
{
    EnterCriticalSection(&m_runningLock);
    bool wasRunning = m_running;
    m_running = running;
    LeaveCriticalSection(&m_runningLock);
    return wasRunning;
}

bool Connection::readTick()
{
    bool didSomething = false;
    if (m_input == NULL || m_readError)
        return false;

#ifdef MINECRAFT_PC
    boost::shared_ptr<Packet> packet =
        Packet::readPacket(m_input, m_listener->isServerPacketListener(), m_packetsRead, m_readError,
                           dynamic_cast<ServerGamePacketListenerImpl*>(m_listener) != NULL);
#else
    boost::shared_ptr<Packet> packet = Packet::readPacket(m_input, m_listener->isServerPacketListener(), m_packetsRead,
                                                          m_readError);
#endif
    if (packet != NULL)
    {
        sReadSizes[packet->getPacketId()] += packet->getEstimatedSize() + 1;
        EnterCriticalSection(&m_incomingLock);
        if (!m_quitting)
            m_incoming.push_back(packet);
        LeaveCriticalSection(&m_incomingLock);
        didSomething = true;
    }
    else if (m_readError)
    {
        return false;
    }
    return didSomething;
}

int Connection::runRead(void* argument)
{
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ConnectionRead);
    Connection* connection = static_cast<Connection*>(argument);
    if (connection != NULL)
    {
        Compression::CreateNewThreadStorage();
        OSFastMutex* threadCountLock = &connection->m_threadCountLock;
        EnterCriticalSection(threadCountLock);
        ++sReadThreadCount;
        LeaveCriticalSection(threadCountLock);
        MemSect(19);
        while (connection->m_running && !connection->m_quitting
               && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionRead))
        {
            while (connection->readTick())
            {
            }
            connection->m_readEvent->WaitForSignal(THREAD_WAIT_MS);
        }
        MemSect(0);
        Compression::ReleaseThreadStorage();
        ShutdownManager::HasFinished(ShutdownManager::eThreadId_ConnectionRead);
    }
    return 0;
}

bool Connection::writeTick()
{
    bool didSomething = false;

    if (m_bufferedOutput == NULL || m_byteArrayOutput == NULL)
        return didSomething;

    if (!m_outgoing.empty()
        && (m_fakeLag == 0 || System::processTimeInMilliSecs() - m_outgoing.front()->creationTime >= m_fakeLag))
    {
        boost::shared_ptr<Packet> packet;
        EnterCriticalSection(&m_writeLock);
        packet = m_outgoing.front();
        m_outgoing.pop_front();
        m_estimatedRemaining -= packet->getEstimatedSize() + 1;
        LeaveCriticalSection(&m_writeLock);

        Packet::writePacket(packet, m_bufferedOutput, m_listener->isServerPacketListener(), m_packetsWritten);
        sWriteSizes[packet->getPacketId()] += packet->getEstimatedSize() + 1;
        didSomething = true;
    }

    if (m_slowWriteDelay-- <= 0 && !m_outgoingSlow.empty()
        && (m_fakeLag == 0 || System::processTimeInMilliSecs() - m_outgoingSlow.front()->creationTime >= m_fakeLag))
    {
        boost::shared_ptr<Packet> packet;
        EnterCriticalSection(&m_writeLock);
        packet = m_outgoingSlow.front();
        m_outgoingSlow.pop_front();
        m_estimatedRemaining -= packet->getEstimatedSize() + 1;
        LeaveCriticalSection(&m_writeLock);

        if (packet->shouldDelay)
        {
            Packet::writePacket(packet, m_byteArrayOutput, m_listener->isServerPacketListener(), m_packetsWritten);
            if (m_bufferedOutput != NULL)
                m_bufferedOutput->flush();
            m_socketOutput->writeWithFlags(m_byteArrayStream->mBuffer, 0, m_byteArrayStream->mCount, 1);
            m_byteArrayStream->mCount = 0;
        }
        else
        {
            Packet::writePacket(packet, m_bufferedOutput, m_listener->isServerPacketListener(), m_packetsWritten);
        }
        sWriteSizes[packet->getPacketId()] += packet->getEstimatedSize() + 1;
        m_slowWriteDelay = 0;
        didSomething = true;
    }

    return didSomething;
}

int Connection::runWrite(void* argument)
{
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_ConnectionWrite);
    Connection* connection = static_cast<Connection*>(argument);
    if (connection != NULL)
    {
        Compression::CreateNewThreadStorage();
        OSFastMutex* threadCountLock = &connection->m_threadCountLock;
        EnterCriticalSection(threadCountLock);
        ++sWriteThreadCount;
        LeaveCriticalSection(threadCountLock);

        unsigned int waitResult = WAIT_TIMED_OUT;
        while ((connection->m_running || waitResult == WAIT_SIGNALLED)
               && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionWrite))
        {
            long long start = System::processTimeInMilliSecs();
            bool keepWriting;
            do
            {
                bool wrote = connection->writeTick();
                long long elapsed = System::processTimeInMilliSecs() - start;
                bool inBurst = elapsed < WRITE_BURST_MS;
                bool underLimit = connection->m_bufferedOutput->mWritten < INT_MAX;
                keepWriting = wrote && inBurst && underLimit;
            } while (keepWriting);
            waitResult = connection->m_writeEvent->WaitForSignal(THREAD_WAIT_MS);
            if (connection->m_bufferedOutput != NULL)
                connection->m_bufferedOutput->flush();
        }

        EnterCriticalSection(threadCountLock);
        --sWriteThreadCount;
        LeaveCriticalSection(threadCountLock);
        Compression::ReleaseThreadStorage();
    }
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_ConnectionWrite);
    return 0;
}

void Connection::setListener(PacketListener* listener)
{
    m_listener = listener;
}

void Connection::send(boost::shared_ptr<Packet> packet)
{
    if (!m_quitting)
    {
        m_lastSendTime = System::processTimeInMilliSecs();
        MemSect(15);
        EnterCriticalSection(&m_writeLock);
        m_estimatedRemaining += packet->getEstimatedSize() + 1;
        if (packet->shouldDelay)
        {
            packet->shouldDelay = false;
            m_outgoingSlow.push_back(packet);
        }
        else if (packet->getPacketId() != MAP_ITEM_DATA_PACKET_ID || !packet->tryReplaceDuplicatePacket(&m_outgoing))
        {
            m_outgoing.push_back(packet);
        }
        LeaveCriticalSection(&m_writeLock);
        MemSect(0);
    }
}

void Connection::queueSend(boost::shared_ptr<Packet> packet)
{
    if (m_quitting)
        return;

    EnterCriticalSection(&m_writeLock);
    m_estimatedRemaining += packet->getEstimatedSize() + 1;
    m_outgoingSlow.push_back(packet);
    LeaveCriticalSection(&m_writeLock);
}

void Connection::flush()
{
    m_readEvent->Set();
    m_writeEvent->Set();
}

void Connection::close(DisconnectPacket::eDisconnectReason reason)
{
    if (!getAndSetRunning(false))
        return;

    m_disconnected = true;
    m_disconnectReason = reason;
    m_disconnectData = NULL;
    if (m_input != NULL)
        m_input->close();
    if (m_readThread != NULL)
        m_readThread->WaitForCompletion(-1);
    if (m_writeThread != NULL)
        m_writeThread->WaitForCompletion(-1);
    delete m_input;
    m_input = NULL;
    if (m_bufferedOutput != NULL)
    {
        m_bufferedOutput->close();
        m_bufferedOutput->deleteChildStream();
        delete m_bufferedOutput;
        m_bufferedOutput = NULL;
    }
    if (m_byteArrayOutput != NULL)
    {
        m_byteArrayOutput->close();
        delete m_byteArrayOutput;
        m_byteArrayOutput = NULL;
    }
    if (m_socket != NULL)
    {
        m_socket->close(m_listener->isServerPacketListener());
        m_socket = NULL;
    }
}

void Connection::tick()
{
    if (m_readError)
        close(DisconnectPacket::eDisconnectReason_NetworkError);
    if (m_estimatedRemaining > MAX_ESTIMATED_REMAINING)
        close(DisconnectPacket::eDisconnectReason_SendQueueOverflow);
#ifdef MINECRAFT_PC
    {
        const long long now = System::processTimeInMilliSecs();
        long long oldest = -1;
        int waiting = 0;
        EnterCriticalSection(&m_writeLock);
        waiting = static_cast<int>(m_outgoing.size() + m_outgoingSlow.size());
        if (!m_outgoing.empty())
            oldest = now - m_outgoing.front()->creationTime;
        LeaveCriticalSection(&m_writeLock);
        if (oldest > 1000 && now - m_pcQueueLogged > 5000)
        {
            m_pcQueueLogged = now;
            printf("[pc] net: %d packets (%d KB) waiting to be sent, the oldest for %lld ms\n", waiting,
                   m_estimatedRemaining / 1024, oldest);
            fflush(stdout);
        }
    }
#endif

    EnterCriticalSection(&m_incomingLock);
    bool incomingEmpty = m_incoming.empty();
    LeaveCriticalSection(&m_incomingLock);
    if (incomingEmpty)
    {
        if (m_noInputTicks++ == NO_INPUT_TIMEOUT_TICKS)
            close(DisconnectPacket::eDisconnectReason_TimedOut);
    }
    else
    {
        m_noInputTicks = 0;
    }
    if (s_altMainStopRequested)
        m_noInputTicks = 0;

    int now = static_cast<int>(System::processTimeInMilliSecs());
    if (now - static_cast<int>(m_lastSendTime) > KEEP_ALIVE_INTERVAL_MS)
    {
        if (m_listener->isServerPacketListener())
            send(boost::shared_ptr<Packet>(new ClientboundKeepAlivePacket()));
        else
            send(boost::shared_ptr<Packet>(new ServerboundKeepAlivePacket()));
    }

    int remaining = MAX_PACKETS_PER_TICK;
    EnterCriticalSection(&m_incomingLock);
    std::deque<boost::shared_ptr<Packet> > packets;
    while (!CGameNetworkManager::sInstance.IsLeavingGame() && CGameNetworkManager::sInstance.IsInSession()
           && !m_incoming.empty() && remaining-- >= 0)
    {
        boost::shared_ptr<Packet> packet = m_incoming.front();
        if (!m_disconnected)
        {
            if (packet->getPacketId() == PLAYER_ACTION_PACKET_ID)
            {
                ServerboundPlayerActionPacket::Action action =
                    boost::static_pointer_cast<ServerboundPlayerActionPacket>(packet)->getAction();
                if (action == ServerboundPlayerActionPacket::DROP_ALL_ITEMS
                    || action == ServerboundPlayerActionPacket::DROP_ITEM)
                {
                    packets.push_front(packet);
                    m_incoming.pop_front();
                }
                else
                {
                    packets.push_back(packet);
                    m_incoming.pop_front();
                }
            }
            else
            {
                packets.push_back(packet);
                m_incoming.pop_front();
            }
        }
        else
        {
            m_incoming.pop_front();
        }
    }
    LeaveCriticalSection(&m_incomingLock);

    for (int i = 0; i < packets.size(); ++i)
    {
        PIXBeginNamedEvent(0.0f, "Handling packet %d\n", packets[i]->getPacketId());
        packets[i]->handle(m_listener);
        PIXEndNamedEvent();
    }

    flush();

    if (m_socket != NULL && (m_socket->mClosed[0] || m_socket->mClosed[1]))
        close(DisconnectPacket::eDisconnectReason_ConnectionClosed);

    if (m_disconnected)
    {
        EnterCriticalSection(&m_incomingLock);
        bool stillEmpty = m_incoming.empty();
        LeaveCriticalSection(&m_incomingLock);
        if (stillEmpty)
        {
            m_listener->onDisconnect(m_disconnectReason, m_disconnectData);
            m_disconnected = false;
        }
    }
}

void Connection::sendAndQuit()
{
    if (m_quitting)
        return;

    flush();
    m_quitting = true;
    close(DisconnectPacket::eDisconnectReason_ConnectionClosed);
}

int Connection::countDelayedPackets()
{
    return (int)m_outgoingSlow.size();
}
