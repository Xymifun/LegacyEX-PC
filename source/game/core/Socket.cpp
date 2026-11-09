#include "game/core/Socket.hpp"

#include <algorithm>

#include "game/core/MemSect.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerConnection.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

ServerConnection* Socket::sConnection = NULL;

OSFastMutex gLocalMutexes[2];
std::queue<unsigned char> gLocalQueues[2];
Socket::SocketOutputStreamLocal* gLocalOutputStreams[2];
Socket::SocketInputStreamLocal* gLocalInputStreams[2];
bool gLocalStreamsInitialised = false;

Socket::SocketInputStreamLocal::SocketInputStreamLocal(int index) : mOpen(true), mIndex(index)
{
}

Socket::SocketInputStreamLocal::~SocketInputStreamLocal()
{
}

int Socket::SocketInputStreamLocal::read()
{
    while (mOpen && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionRead))
    {
        if (TryEnterCriticalSection(&gLocalMutexes[mIndex]))
        {
            if (!gLocalQueues[mIndex].empty())
            {
                unsigned char value = gLocalQueues[mIndex].front();
                gLocalQueues[mIndex].pop();
                LeaveCriticalSection(&gLocalMutexes[mIndex]);
                return value;
            }
            LeaveCriticalSection(&gLocalMutexes[mIndex]);
        }
        Sleep(1);
    }
    return -1;
}

int Socket::SocketInputStreamLocal::read(arrayWithLength<unsigned char> buffer)
{
    return read(buffer, 0, buffer.length);
}

int Socket::SocketInputStreamLocal::read(arrayWithLength<unsigned char> buffer, unsigned int offset,
                                         unsigned int length)
{
    while (mOpen)
    {
        if (TryEnterCriticalSection(&gLocalMutexes[mIndex]))
        {
            if (gLocalQueues[mIndex].size() >= length)
            {
                for (unsigned int i = 0; i < length; ++i)
                {
                    buffer.data[i + offset] = gLocalQueues[mIndex].front();
                    gLocalQueues[mIndex].pop();
                }
                LeaveCriticalSection(&gLocalMutexes[mIndex]);
                return length;
            }
            LeaveCriticalSection(&gLocalMutexes[mIndex]);
        }
        Sleep(1);
    }
    return -1;
}

void Socket::SocketInputStreamLocal::close()
{
    mOpen = false;
    EnterCriticalSection(&gLocalMutexes[mIndex]);
    LeaveCriticalSection(&gLocalMutexes[mIndex]);
}

long long Socket::SocketInputStreamLocal::skip(long long count)
{
    return count;
}

void Socket::SocketInputStreamLocal::flush()
{
}

Socket::SocketInputStreamNetwork::SocketInputStreamNetwork(Socket* socket, int index)
    : mOpen(true),
      mIndex(index),
      mSocket(socket)
{
}

Socket::SocketInputStreamNetwork::~SocketInputStreamNetwork()
{
}

int Socket::SocketInputStreamNetwork::read()
{
    while (mOpen && ShutdownManager::ShouldRun(ShutdownManager::eThreadId_ConnectionRead))
    {
        if (TryEnterCriticalSection(&mSocket->mMutexes[mIndex]))
        {
            if (!mSocket->mQueues[mIndex].empty())
            {
                unsigned char value = mSocket->mQueues[mIndex].front();
                mSocket->mQueues[mIndex].pop();
                LeaveCriticalSection(&mSocket->mMutexes[mIndex]);
                return value;
            }
            LeaveCriticalSection(&mSocket->mMutexes[mIndex]);
        }
        Sleep(1);
    }
    return -1;
}

int Socket::SocketInputStreamNetwork::read(arrayWithLength<unsigned char> buffer)
{
    return read(buffer, 0, buffer.length);
}

int Socket::SocketInputStreamNetwork::read(arrayWithLength<unsigned char> buffer, unsigned int offset,
                                           unsigned int length)
{
    while (mOpen)
    {
        if (TryEnterCriticalSection(&mSocket->mMutexes[mIndex]))
        {
            if (mSocket->mQueues[mIndex].size() >= length)
            {
                for (unsigned int i = 0; i < length; ++i)
                {
                    buffer.data[i + offset] = mSocket->mQueues[mIndex].front();
                    mSocket->mQueues[mIndex].pop();
                }
                LeaveCriticalSection(&mSocket->mMutexes[mIndex]);
                return length;
            }
            LeaveCriticalSection(&mSocket->mMutexes[mIndex]);
        }
        Sleep(1);
    }
    return -1;
}

void Socket::SocketInputStreamNetwork::close()
{
    mOpen = false;
}

long long Socket::SocketInputStreamNetwork::skip(long long count)
{
    return count;
}

void Socket::SocketInputStreamNetwork::flush()
{
}

Socket::SocketOutputStream::~SocketOutputStream()
{
}

void Socket::SocketOutputStream::writeWithFlags(arrayWithLength<unsigned char> buffer, unsigned int offset,
                                                unsigned int length, int)
{
    write(buffer, offset, length);
}

Socket::SocketOutputStreamLocal::SocketOutputStreamLocal(int index) : mOpen(true), mIndex(index)
{
}

Socket::SocketOutputStreamLocal::~SocketOutputStreamLocal()
{
}

void Socket::SocketOutputStreamLocal::write(unsigned int value)
{
    if (!mOpen)
        return;

    EnterCriticalSection(&gLocalMutexes[mIndex]);
    gLocalQueues[mIndex].push(static_cast<unsigned char>(value));
    LeaveCriticalSection(&gLocalMutexes[mIndex]);
}

void Socket::SocketOutputStreamLocal::write(arrayWithLength<unsigned char> buffer)
{
    write(buffer, 0, buffer.length);
}

void Socket::SocketOutputStreamLocal::write(arrayWithLength<unsigned char> buffer, unsigned int offset,
                                            unsigned int length)
{
    if (!mOpen)
        return;

    MemSect(12);
    EnterCriticalSection(&gLocalMutexes[mIndex]);
    for (unsigned int i = 0; i < length; ++i)
        gLocalQueues[mIndex].push(buffer.data[offset + i]);
    LeaveCriticalSection(&gLocalMutexes[mIndex]);
    MemSect(0);
}

void Socket::SocketOutputStreamLocal::close()
{
    mOpen = false;
    EnterCriticalSection(&gLocalMutexes[mIndex]);
    LeaveCriticalSection(&gLocalMutexes[mIndex]);
}

void Socket::SocketOutputStreamLocal::flush()
{
}

Socket::SocketOutputStreamNetwork::SocketOutputStreamNetwork(Socket* socket, int index) : mIndex(index), mSocket(socket)
{
    mOpen = true;
}

Socket::SocketOutputStreamNetwork::~SocketOutputStreamNetwork()
{
}

void Socket::SocketOutputStreamNetwork::write(unsigned int value)
{
    if (!mOpen)
        return;

    unsigned char byte = static_cast<unsigned char>(value);
    arrayWithLength<unsigned char> buffer;
    buffer.data = &byte;
    buffer.length = 1;
    write(buffer, 0, 1);
}

void Socket::SocketOutputStreamNetwork::write(arrayWithLength<unsigned char> buffer)
{
    write(buffer, 0, buffer.length);
}

void Socket::SocketOutputStreamNetwork::write(arrayWithLength<unsigned char> buffer, unsigned int offset,
                                              unsigned int length)
{
    writeWithFlags(buffer, offset, length, 0);
}

void Socket::SocketOutputStreamNetwork::writeWithFlags(arrayWithLength<unsigned char> buffer, unsigned int offset,
                                                       unsigned int length, int flags)
{
    if (!mOpen || length == 0)
        return;

    if (mSocket->mHostLocal)
    {
        int queue = mIndex == 0 ? 1 : 0;
        EnterCriticalSection(&mSocket->mMutexes[queue]);
        for (unsigned int i = 0; i < length; ++i)
            mSocket->mQueues[queue].push(buffer.data[offset + i]);
        LeaveCriticalSection(&mSocket->mMutexes[queue]);
        return;
    }

    unsigned char* data = buffer.data;
    INetworkPlayer* host = CGameNetworkManager::sInstance.GetHostPlayer();
    INetworkPlayer* player;
    if (host == NULL || (player = mSocket->getPlayer()) == NULL)
        return;

    if (mIndex == 1)
        host->SendData(player, data + offset, length, false, (flags & 1) != 0);
    else
        player->SendData(host, data + offset, length, false, (flags & 1) != 0);
}

void Socket::SocketOutputStreamNetwork::close()
{
    mOpen = false;
}

void Socket::SocketOutputStreamNetwork::flush()
{
}

Socket::Socket(bool response)
{
    mHostServerConnection = true;
    mHostLocal = true;
    if (response)
    {
        mClosed[1] = false;
        mClosed[0] = false;
        mIndex = 1;
    }
    else
    {
        mIndex = 0;
        sConnection->NewIncomingSocket(new Socket(true));
        mClosed[0] = false;
        mClosed[1] = false;
    }
    mCloseEvent = NULL;
    mOpen = true;

    INetworkPlayer* host = CGameNetworkManager::sInstance.GetHostPlayer();
    if (host != NULL)
        mPlayerId = host->GetSmallId();
    else
        mPlayerId = 0;
}

Socket::Socket(INetworkPlayer* player, bool response, bool hostLocal)
{
    mHostServerConnection = false;
    mHostLocal = hostLocal;
    InitializeCriticalSection(&mMutexes[0]);
    mInputStreams[0] = NULL;
    mOutputStreams[0] = NULL;
    mClosed[0] = false;
    InitializeCriticalSection(&mMutexes[1]);
    mOutputStreams[1] = NULL;
    mInputStreams[1] = NULL;
    mClosed[1] = false;

    if (!response || hostLocal)
    {
        mInputStreams[0] = new SocketInputStreamNetwork(this, 0);
        mOutputStreams[0] = new SocketOutputStreamNetwork(this, 0);
        mIndex = 0;
    }
    if (response || hostLocal)
    {
        mInputStreams[1] = new SocketInputStreamNetwork(this, 1);
        mOutputStreams[1] = new SocketOutputStreamNetwork(this, 1);
        mIndex = 1;
    }

    mCloseEvent = new C4JEventImpl(C4JEvent::Mode0);
    mPlayerId = player->GetSmallId();
    mOpen = true;
}

INetworkPlayer* Socket::getPlayer()
{
    return CGameNetworkManager::sInstance.GetPlayerBySmallId(mPlayerId);
}

void Socket::setPlayer(INetworkPlayer* player)
{
    mPlayerId = player != NULL ? player->GetSmallId() : 0;
}

void Socket::Initialise(ServerConnection* connection)
{
    sConnection = connection;

    if (gLocalStreamsInitialised)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (TryEnterCriticalSection(&gLocalMutexes[i]))
            {
                std::queue<unsigned char> empty;
                std::swap(gLocalQueues[i], empty);
                LeaveCriticalSection(&gLocalMutexes[i]);
            }
            gLocalOutputStreams[i]->mOpen = true;
            gLocalInputStreams[i]->mOpen = true;
        }
        return;
    }

    gLocalStreamsInitialised = true;
    InitializeCriticalSection(&gLocalMutexes[0]);
    gLocalOutputStreams[0] = new SocketOutputStreamLocal(0);
    gLocalInputStreams[0] = new SocketInputStreamLocal(0);
    InitializeCriticalSection(&gLocalMutexes[1]);
    gLocalOutputStreams[1] = new SocketOutputStreamLocal(1);
    gLocalInputStreams[1] = new SocketInputStreamLocal(1);
}

unsigned int Socket::getRemoteSocketAddress()
{
    return 0;
}

void Socket::pushDataToQueue(const unsigned char* data, unsigned int length, bool fromHost)
{
    unsigned int queue = 0;
    if (!fromHost)
        queue = 1;
    if (queue != mIndex && !mHostLocal)
        return;

    EnterCriticalSection(&mMutexes[queue]);
    for (unsigned int i = 0; i < length; ++i)
        mQueues[queue].push(*data++);
    LeaveCriticalSection(&mMutexes[queue]);
}

void Socket::addIncomingSocket(Socket* socket)
{
    if (sConnection != NULL)
        sConnection->NewIncomingSocket(socket);
}

InputStream* Socket::getInputStream(bool host)
{
    if (!mHostServerConnection)
    {
        if (mHostLocal)
        {
            if (host)
                return mInputStreams[1];
            return mInputStreams[0];
        }
        return mInputStreams[mIndex];
    }
    return gLocalInputStreams[mIndex];
}

void Socket::setSoTimeout(int)
{
}

void Socket::setTrafficClass(int)
{
}

OutputStream* Socket::getOutputStream(bool host)
{
    if (!mHostServerConnection)
    {
        if (mHostLocal)
        {
            if (host)
                return mOutputStreams[1];
            return mOutputStreams[0];
        }
        return mOutputStreams[mIndex];
    }
    return gLocalOutputStreams[1 - mIndex];
}

bool Socket::close(bool host)
{
    bool result = false;
    if (mHostLocal)
    {
        bool otherClosed;
        if (host)
        {
            otherClosed = mClosed[0];
            mClosed[1] = true;
        }
        else
        {
            otherClosed = mClosed[1];
            mClosed[0] = true;
        }
        if (!otherClosed)
            return result;

        result = true;
    }
    else
    {
        result = true;
        mClosed[mIndex] = result;
    }

    if (mCloseEvent != NULL)
        mCloseEvent->Set();
    mOpen = false;
    return result;
}
