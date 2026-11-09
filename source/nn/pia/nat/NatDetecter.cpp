#include "nn/pia/nat/NatDetecter.hpp"

#include <new>

#include "nn/pia/common/CallContext.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

namespace
{
enum
{
    cResultInvalidState = 0x10c08,
    cReplyTypeFirst = 0x65,
    cProbeReceiveSize = 0x10,
    cRandomPortAttempts = 100,
    cBindRetryCount = 4,
    cTraceBindFailure = 0x800,
    cDummyProbePort = 0x8236,
    cDummyProbeCount = 5,
    cPortDeltaMax = 1,
    cDetectionPollMsec = 2
};

pead::Random s_natRandom;
}

unsigned short NatDetecter::s_currentPort = 0;
unsigned short NatDetecter::s_randomPortStart = 0;
unsigned short NatDetecter::s_randomPortRange = 0;
unsigned short NatDetecter::s_reservedPort = 0;

NatDetecter::NatDetecter()
    : m_socket(),
      m_primaryAddress(),
      m_senderAddress(),
      m_sendMessage(),
      m_receiveMessage(),
      m_probes(),
      m_detectionJob()
{
}

NatDetecter::~NatDetecter()
{
}

unsigned short NatDetecter::GetCurrentPort()
{
    return s_currentPort;
}

void NatDetecter::SetCurrentPort(unsigned short port)
{
    s_currentPort = port;
}

void NatDetecter::SetReservedPort(unsigned int enabled, unsigned short port)
{
    if (enabled != 0)
        return;
    s_reservedPort = port;
}

void NatDetecter::SetRandomPortRange(unsigned short first, unsigned short last)
{
    s_randomPortStart = first;
    s_randomPortRange = last;
}

unsigned int NatDetecter::GetRandomPortNumber()
{
    unsigned int port = 0;
    for (int attempt = cRandomPortAttempts; attempt != 0; --attempt)
    {
        const unsigned short end = s_randomPortRange;
        const unsigned short start = s_randomPortStart;
        const unsigned int range = end - start;
        const unsigned int random = s_natRandom.getU32();
        const unsigned int candidate = static_cast<unsigned short>(
            ((static_cast<unsigned long long>(random) * range) >> 32) + start);
        if (candidate != s_reservedPort && candidate != 0)
        {
            port = candidate;
            break;
        }
    }
    return port;
}

Result NatDetecter::Startup(NatTraverser* traverser, common::CallContext* context, const common::InetAddress& address)
{
    if (traverser == 0)
        return ResultInvalidArgument();

    m_retryCount = 0;
    m_primaryAddress = address;
    if (traverser->m_servers.size == 0)
        return cResultInvalidState;

    m_traverser = traverser;
    m_probes.Clear();
    return 0;
}

Result NatDetecter::CloseSocket()
{
    if (m_socket.m_openCount < 1)
        return cResultInvalidState;

    const int result = m_socket.Close();
    if (result != 0)
        return result;
    return 0;
}

void NatDetecter::Cleanup()
{
    m_probes.Clear();
    CloseSocket();
    m_traverser = 0;
}

void NatDetecter::Retry()
{
    ++m_retryCount;
    m_probes.Clear();
}

void NatDetecter::TraceNatCheckReplyArray(unsigned long long)
{
}

unsigned int NatDetecter::BindRandomPort()
{
    for (int attempt = 0; attempt <= cBindRetryCount; ++attempt)
    {
        m_primaryAddress.m_port = static_cast<unsigned short>(GetRandomPortNumber());
        const int result = m_socket.Bind(m_primaryAddress, false);
        if (result != 0)
        {
            m_primaryAddress.Trace(cTraceBindFailure);
        }
        else
        {
            return 1;
        }
    }
    return 0;
}

Result NatDetecter::OpenSocket()
{
    Result result;
    result = m_socket.Open();
    if (result != 0)
        return result;
    if ((m_primaryAddress.m_port == 0 || m_socket.Bind(m_primaryAddress, false) != 0) && BindRandomPort() == 0)
    {
        m_primaryAddress.Trace(cTraceBindFailure);
        m_socket.Close();
        return ResultInvalidState();
    }
    return 0;
}

Result NatDetecter::RetryBind()
{
    int result = m_socket.Close();
    if (result != 0)
        return result;
    result = m_socket.Open();
    if (result != 0)
        return result;
    if (BindRandomPort() == 0)
        return ResultInvalidState();
    return 0;
}

unsigned int NatDetecter::SendNextQueuedProbe()
{
    NatCheckProbe* probe = m_probes.Front();
    Result result;
    int sentSize = 0;
    m_sendMessage.type = probe->message.type;
    m_sendMessage.portWord = probe->message.portWord;
    m_sendMessage.address = probe->message.address;
    m_sendMessage.payload = probe->message.payload;
    result = m_socket.SendTo(&m_sendMessage, cProbeReceiveSize, probe->destination, &sentSize);
    if (result != 0)
        return 0;

    m_probes.Destroy(probe);
    return 1;
}

unsigned int NatDetecter::SendAllQueuedProbe()
{
    while (m_probes.GetNodeNum() != 0)
    {
        if (SendNextQueuedProbe() == 0)
            return 0;
    }
    return 1;
}

unsigned int NatDetecter::ReceiveNatCheckReply()
{
    unsigned char ancillary = 0;
    int receivedSize = 0;
    const int result = m_socket.RecvFrom(reinterpret_cast<unsigned char*>(&m_receiveMessage), cProbeReceiveSize,
                                         &m_senderAddress, &ancillary, &receivedSize);
    if (result != 0 || receivedSize != cProbeReceiveSize)
        return 0;

    const unsigned int index = m_receiveMessage.type - cReplyTypeFirst;
    if (index < cReplyTypeCount)
    {
        m_replies[index].body = m_receiveMessage;
        ++m_replies[index].count;
    }
    return 1;
}

void NatDetecter::PredictPort(common::InetAddress* address)
{
    common::InetAddress predicted(m_replies[0].body.address, static_cast<unsigned short>(m_replies[0].body.portWord));
    unsigned short delta = static_cast<unsigned short>(m_traverser->m_natProperty.m_portDelta);
    if (delta > cPortDeltaMax)
        delta = cPortDeltaMax;
    const unsigned short port = static_cast<unsigned short>(predicted.m_port + delta);
    predicted.m_port = port > s_randomPortRange ? s_randomPortStart : port;
    if (address != 0)
        *address = predicted;
}

void NatDetecter::InitializeNatCheckReply()
{
    for (unsigned int index = 0; index < cReplyTypeCount; ++index)
    {
        m_replies[index].count = 0;
        m_replies[index].body.type = 0;
        m_replies[index].body.portWord = 0;
        m_replies[index].body.address = 0;
        m_replies[index].body.payload = 0;
    }
}

void NatDetecter::AddNatCheckProbe(const NatCheckProbe& probe, unsigned short count)
{
    for (int i = 0; i != count; ++i)
    {
        NatCheckProbe* entry = m_probes.Alloc();
        if (entry != 0)
            *entry = probe;
    }
}

common::InetAddress* NatDetecter::GetPrimaryServerPrimaryPortAddress()
{
    common::SimpleContainer<common::InetAddress, 4>::iterator it = m_traverser->m_servers.begin();
    if (it == m_traverser->m_servers.end())
        return 0;
    return it;
}

common::InetAddress* NatDetecter::GetPrimaryServerSecondaryPortAddress()
{
    common::SimpleContainer<common::InetAddress, 4>::iterator it = m_traverser->m_servers.begin();
    common::SimpleContainer<common::InetAddress, 4>::iterator end = m_traverser->m_servers.end();
    if (it == end)
        return 0;
    ++it;
    if (it == end)
        return 0;
    return it;
}

unsigned short NatDetecter::GetPerceivedPort()
{
    return static_cast<unsigned short>(m_replies[0].body.portWord);
}

NatCheckReply* NatDetecter::GetNatCheckReply(unsigned int index)
{
    return &m_replies[index];
}

void NatDetecter::SendDummyProbe()
{
    common::InetAddress* primary = GetPrimaryServerPrimaryPortAddress();
    common::InetAddress destination(primary->m_address, cDummyProbePort);
    AddNatCheckProbe(NatCheckProbe(destination, 0), cDummyProbeCount);
}

unsigned int NatDetecter::CheckRetry()
{
    return 0;
}

void NatDetecter::StartDetectionJob()
{
    m_detectionJob.Ready(true);
}

void NatDetecter::CancelDetectionJob()
{
    m_detectionJob.m_cancelRequested = true;
    m_detectionJob.WaitForCompletion(cDetectionPollMsec);
}

}
}
}
