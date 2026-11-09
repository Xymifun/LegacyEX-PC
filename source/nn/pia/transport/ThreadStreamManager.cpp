#include "nn/pia/transport/ThreadStreamManager.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/transport/TransportSupport.hpp"

#include <new>

namespace nn
{
namespace pia
{
namespace transport
{

ThreadStreamManager* g_piaThreadStreamManager = 0;

namespace
{
enum
{
    cThreadPriority = 10,
    cThreadSleepMilliSecond = 5,
    cInstanceAlignment = 8
};

unsigned char* s_threadStreamManagerStorage = 0;
}

ThreadStreamManager::ThreadStreamManager(NetworkFactory* networkFactory, unsigned int sendPacketNum,
                                         unsigned int receivePacketNum, unsigned int sendLatencyEntryNum,
                                         unsigned int receiveLatencyEntryNum, bool isPacketLossEnabled)
    : m_packetInput(0),
      m_packetOutput(0),
      m_receiveThreadStream(0),
      m_sendThreadStream(0)
{
    m_packetInput = networkFactory->CreateInputStream();
    m_receiveThreadStream = common::NewObj<ReceiveThreadStream>();
    m_receiveThreadStream->Initialize(m_packetInput, receivePacketNum, cThreadPriority, receiveLatencyEntryNum,
                                      isPacketLossEnabled);
    m_receiveThreadStream->m_sleepMilliSecond = cThreadSleepMilliSecond;

    m_packetOutput = networkFactory->CreateOutputStream();
    m_sendThreadStream = common::NewObj<SendThreadStream>();
    m_sendThreadStream->Initialize(m_packetOutput, sendPacketNum, cThreadPriority, sendLatencyEntryNum,
                                   isPacketLossEnabled);
    m_sendThreadStream->m_sleepMilliSecond = cThreadSleepMilliSecond;
}

Result ThreadStreamManager::CreateInstance(NetworkFactory* networkFactory, unsigned int sendPacketNum,
                                           unsigned int receivePacketNum, unsigned int sendLatencyEntryNum,
                                           unsigned int receiveLatencyEntryNum, bool isPacketLossEnabled)
{
    if (networkFactory == 0)
        return cResultInvalidArgument;
    if (IsInitialized() == 0)
        return ResultNotInitialized();
    if (g_piaThreadStreamManager != 0)
        return cResultAlreadyExists;

    s_threadStreamManagerStorage = common::NewArray<unsigned char>(sizeof(ThreadStreamManager), cInstanceAlignment);
    if (s_threadStreamManagerStorage == 0)
        return cResultOutOfMemory;

    g_piaThreadStreamManager = new (s_threadStreamManagerStorage)
        ThreadStreamManager(networkFactory, sendPacketNum, receivePacketNum, sendLatencyEntryNum,
                            receiveLatencyEntryNum, isPacketLossEnabled);
    return 0;
}

void ThreadStreamManager::DestroyInstance()
{
    if (g_piaThreadStreamManager != 0)
    {
        g_piaThreadStreamManager->~ThreadStreamManager();
        g_piaThreadStreamManager = 0;
    }
    if (s_threadStreamManagerStorage != 0)
    {
        common::DeleteArray(s_threadStreamManagerStorage);
        s_threadStreamManagerStorage = 0;
    }
}

Result ThreadStreamManager::Startup()
{
    if (m_receiveThreadStream == 0 || m_sendThreadStream == 0)
        return cResultInvalidState;

    m_receiveThreadStream->TransportThreadStream::Startup();
    m_sendThreadStream->TransportThreadStream::Startup();
    return 0;
}

void ThreadStreamManager::Cleanup()
{
    if (m_receiveThreadStream != 0)
        m_receiveThreadStream->TransportThreadStream::Cleanup();
    if (m_sendThreadStream != 0)
        m_sendThreadStream->TransportThreadStream::Cleanup();
}

void ThreadStreamManager::SetSendThreadPriority(int priority)
{
    m_sendThreadStream->m_thread->setPriority(priority);
}

int ThreadStreamManager::GetSendThreadPriority() const
{
    return m_sendThreadStream->m_thread->getPriority();
}

void ThreadStreamManager::SetReceiveThreadPriority(int priority)
{
    m_receiveThreadStream->m_thread->setPriority(priority);
}

int ThreadStreamManager::GetReceiveThreadPriority() const
{
    return m_receiveThreadStream->m_thread->getPriority();
}

void ThreadStreamManager::SetSendThreadSleepTimeSpan(unsigned int milliSecond)
{
    m_sendThreadStream->m_sleepMilliSecond = milliSecond;
}

unsigned int ThreadStreamManager::GetSendThreadSleepTimeSpan() const
{
    return m_sendThreadStream->m_sleepMilliSecond;
}

void ThreadStreamManager::SetReceiveThreadSleepTimeSpan(unsigned int milliSecond)
{
    m_receiveThreadStream->m_sleepMilliSecond = milliSecond;
}

unsigned int ThreadStreamManager::GetReceiveThreadSleepTimeSpan() const
{
    return m_receiveThreadStream->m_sleepMilliSecond;
}

void ThreadStreamManager::SetSendThreadLatencyEmulation(int minimumLatency, int maximumLatency)
{
    m_sendThreadStream->SetLatency(minimumLatency, maximumLatency);
}

void ThreadStreamManager::SetReceiveThreadLatencyEmulation(int minimumLatency, int maximumLatency)
{
    m_receiveThreadStream->SetLatency(minimumLatency, maximumLatency);
}

unsigned int ThreadStreamManager::GetSendThreadSendPacketNum() const
{
    return m_sendThreadStream->m_sendPacketNum;
}

void ThreadStreamManager::ClearSendThreadSendPacketNum()
{
    m_sendThreadStream->m_sendPacketNum = 0;
}

unsigned int ThreadStreamManager::GetReceiveThreadReceivePacketNum() const
{
    return m_receiveThreadStream->m_receivePacketNum;
}

void ThreadStreamManager::ClearReceiveThreadReceivePacketNum()
{
    m_receiveThreadStream->m_receivePacketNum = 0;
}

unsigned long long ThreadStreamManager::GetSendThreadSendDataSize() const
{
    return m_sendThreadStream->m_sendDataSize;
}

void ThreadStreamManager::ClearSendThreadSendDataSize()
{
    m_sendThreadStream->m_sendDataSize = 0;
}

unsigned long long ThreadStreamManager::GetReceiveThreadReceiveDataSize() const
{
    return m_receiveThreadStream->m_receiveDataSize;
}

void ThreadStreamManager::ClearReceiveThreadReceiveDataSize()
{
    m_receiveThreadStream->m_receiveDataSize = 0;
}

void ThreadStreamManager::SetSendThreadPacketLossRatio(int packetLossRatio)
{
    m_sendThreadStream->SetPacketLossRatio(packetLossRatio);
}

int ThreadStreamManager::GetSendThreadPacketLossRatio() const
{
    return m_sendThreadStream->m_packetLossRatio;
}

void ThreadStreamManager::SetReceiveThreadPacketLossRatio(int packetLossRatio)
{
    m_receiveThreadStream->SetPacketLossRatio(packetLossRatio);
}

int ThreadStreamManager::GetReceiveThreadPacketLossRatio() const
{
    return m_receiveThreadStream->m_packetLossRatio;
}

void ThreadStreamManager::SetMonitoringData()
{
    if (m_sendThreadStream != 0)
        m_sendThreadStream->SetMonitoringData();
    if (m_receiveThreadStream != 0)
        m_receiveThreadStream->SetMonitoringData();
}

void ThreadStreamManager::Trace(unsigned long long) const
{
}

ThreadStreamManager::~ThreadStreamManager()
{
    if (m_sendThreadStream != 0)
    {
        m_sendThreadStream->Finalize();
        common::DeleteObj(m_sendThreadStream);
        m_sendThreadStream = 0;
    }
    if (m_receiveThreadStream != 0)
    {
        m_receiveThreadStream->Finalize();
        common::DeleteObj(m_receiveThreadStream);
        m_receiveThreadStream = 0;
    }
    if (m_packetOutput != 0)
    {
        g_piaTransport->GetNetworkFactory()->DeleteOutputStream(m_packetOutput);
        m_packetOutput = 0;
    }
    if (m_packetInput != 0)
    {
        g_piaTransport->GetNetworkFactory()->DeleteInputStream(m_packetInput);
        m_packetInput = 0;
    }
}

}
}
}
