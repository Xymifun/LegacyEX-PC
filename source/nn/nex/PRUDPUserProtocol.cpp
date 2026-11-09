#include "nn/nex/PRUDPUserProtocol.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/PacketBuffer.hpp"
#include "nn/nex/PacketBufferManager.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum ResultCode
{
    RESULT_PACKET_ALLOCATION_FAILED = static_cast<int>(0x8005001Fu)
};

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;

enum SendFlag
{
    SEND_FLAG_HISTORY_BUNDLE = 0x04
};

}

PRUDPUserProtocol::PRUDPUserProtocol()
    : m_instanceControl(ClassID),
      m_pUserReceiveCallback(0),
      m_pUserPrivateReceiveCallback(0)
{
    m_instanceControl.SetDelegatorInstance(this);
}

PRUDPUserProtocol::~PRUDPUserProtocol()
{
}

void PRUDPUserProtocol::AcquireInstance()
{
    PRUDPUserProtocol* pInstance = GetInstance();
    if (pInstance == 0)
        new PRUDPUserProtocol();
    else
        pInstance->AddRef();
}

void PRUDPUserProtocol::ReleaseInstance()
{
    GetInstance()->ReleaseRef();
}

void PRUDPUserProtocol::RegisterUserReceiveCallback(UserReceiveCallback pCallback)
{
    m_pUserReceiveCallback = pCallback;
}

void PRUDPUserProtocol::UnregisterUserReceiveCallback()
{
    m_pUserReceiveCallback = 0;
}

void PRUDPUserProtocol::RegisterUserPrivateReceiveCallback(UserPrivateReceiveCallback pCallback)
{
    m_pUserPrivateReceiveCallback = pCallback;
}

void PRUDPUserProtocol::UnregisterUserPrivateReceiveCallback()
{
    m_pUserPrivateReceiveCallback = 0;
}

void PRUDPUserProtocol::FaultDetection(EndPoint* pEndPoint, unsigned int uiError)
{
}

void PRUDPUserProtocol::PeerDisconnected(EndPoint* pEndPoint)
{
}

void PRUDPUserProtocol::Receive(EndPoint* pEndPoint, Buffer* pBuffer, unsigned char uiSubStreamID)
{
    Receive(pEndPoint, pBuffer, 0, uiSubStreamID);
}

void PRUDPUserProtocol::Receive(EndPoint* pEndPoint, Buffer* pBuffer, unsigned int uiFlags, unsigned char uiSubStreamID)
{
    if (m_pUserPrivateReceiveCallback == 0 && m_pUserReceiveCallback == 0)
        return;
    if (pEndPoint == 0)
        return;
    unsigned int uiConnectionID = pEndPoint->GetConnectionID();
    if (m_pUserPrivateReceiveCallback != 0)
    {
        m_pUserPrivateReceiveCallback(uiConnectionID, pBuffer, uiFlags, uiSubStreamID);
        return;
    }
    if (uiSubStreamID != 0)
        return;
    m_pUserReceiveCallback(uiConnectionID, pBuffer->GetData(), pBuffer->GetLength());
}

qResult PRUDPUserProtocol::Send(EndPoint* pEndPoint, const unsigned char* pData, unsigned int uiSize,
                                unsigned int uiFlags, unsigned char uiSubStreamID, unsigned int uiKey)
{
    Buffer* pBuffer;
    if (uiSize <= PacketBufferManager::GetInstance()->GetMaxUnitSize())
    {
        pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT) PacketBuffer(uiSize, Buffer::BUFFER_FLAG_POOLED);
        if (pBuffer == 0 || !pBuffer->IsValid())
        {
            delete pBuffer;
            return qResult(RESULT_PACKET_ALLOCATION_FAILED);
        }
    }
    else
    {
        pBuffer = new Buffer(uiSize, 0);
    }
    pBuffer->AppendData(pData, uiSize, APPEND_AT_END);
    CriticalSection& criticalSection = Scheduler::GetInstance()->GetCriticalSection();
    criticalSection.Enter();
    qResult result = pEndPoint->_Send(pBuffer, uiFlags | SEND_FLAG_HISTORY_BUNDLE, false, uiSubStreamID, uiKey);
    criticalSection.Leave();
    pBuffer->ReleaseRef();
    return result;
}

}
}
