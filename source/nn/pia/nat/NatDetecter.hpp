#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTER_HPP

#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/nat/NatCheckReply.hpp"
#include "nn/pia/nat/NatDetectionJob.hpp"
#include "nn/pia/nat/NatTraverser.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class CallContext;
}
namespace nat
{
class NatTraverser;
class NatDetecter;

struct NatCheckProbe
{
    NatCheckProbe() : destination(), message()
    {
        destination.m_address = 0;
        destination.m_port = 0;
        message.type = 0;
        message.portWord = 0;
        message.address = 0;
        message.payload = 0;
    }

    NatCheckProbe(common::InetAddress address, int type) : destination(address), message() { message.type = type; }

    common::InetAddress destination;
    NatCheckReply::Body message;
};

class NatDetecter
{
public:
    enum
    {
        cProbeQueueSize = 20,
        cReplyTypeCount = 3
    };

    NatDetecter();
    virtual ~NatDetecter();
    static unsigned short GetCurrentPort();
    static void SetCurrentPort(unsigned short port);
    static void SetReservedPort(unsigned int enabled, unsigned short port);
    static void SetRandomPortRange(unsigned short first, unsigned short last);
    static unsigned int GetRandomPortNumber();
    virtual Result Startup(NatTraverser* traverser, common::CallContext* context, const common::InetAddress& address);
    Result CloseSocket();
    virtual void Cleanup();
    virtual void Retry();
    void TraceNatCheckReplyArray(unsigned long long value);
    unsigned int BindRandomPort();
    Result OpenSocket();
    Result RetryBind();
    unsigned int SendNextQueuedProbe();
    unsigned int SendAllQueuedProbe();
    unsigned int ReceiveNatCheckReply();
    void PredictPort(common::InetAddress* address);
    void InitializeNatCheckReply();
    void AddNatCheckProbe(const NatCheckProbe& probe, unsigned short count);
    common::InetAddress* GetPrimaryServerPrimaryPortAddress();
    common::InetAddress* GetPrimaryServerSecondaryPortAddress();
    unsigned short GetPerceivedPort();
    NatCheckReply* GetNatCheckReply(unsigned int index);
    void SendDummyProbe();
    virtual void StartSendingNatCheckProbe() = 0;
    virtual bool IsReceivedAllMessage() = 0;
    virtual unsigned int HandleResult() = 0;
    virtual unsigned int CheckRetry();
    virtual unsigned int GetDetectionTimeout() const = 0;
    virtual void StartDetectionJob();
    virtual void CancelDetectionJob();

protected:
    friend class NatDetectionJob;

    typedef common::FixedPooledList<NatCheckProbe, cProbeQueueSize> ProbeQueue;

    static unsigned short s_currentPort;
    static unsigned short s_randomPortStart;
    static unsigned short s_randomPortRange;
    static unsigned short s_reservedPort;
    common::Socket m_socket;
    unsigned int m_retryCount;
    common::InetAddress m_primaryAddress;
    common::InetAddress m_senderAddress;
    NatCheckReply::Body m_sendMessage;
    NatCheckReply::Body m_receiveMessage;
    ProbeQueue m_probes;
    NatCheckReply m_replies[cReplyTypeCount];
    unsigned int m_firstReplyMsec;
    unsigned char m_padding328[8];
    NatDetectionJob m_detectionJob;
    NatTraverser* m_traverser;
};

}
}
}

#endif
