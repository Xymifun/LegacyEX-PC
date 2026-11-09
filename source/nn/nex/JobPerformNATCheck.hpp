#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBPERFORMNATCHECK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBPERFORMNATCHECK_HPP

#include <stdint.h>
#include <list>
#include <utility>

#include "nn/nex/IOCompletionNotifier.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NATCheckMessage.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/Socket.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StepSequenceJob.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class NATRelayInterface;

class JobPerformNATCheck : public StepSequenceJob
{
public:
    class RecvNATCheckMessage
    {
    public:
        RecvNATCheckMessage() : m_message(), m_uiCount(0) {}

        ~RecvNATCheckMessage() {}

        NATCheckMessage m_message;
        unsigned int m_uiCount;
    };

    typedef std::list<InetAddress, MemAllocator<InetAddress> > ServerList;
    struct QueuedMessage
    {
        QueuedMessage(InetAddress address, unsigned int uiType) : first(address)
        {
            second.m_uiType = uiType;
            second.ToNetworkByteOrder();
        }

        InetAddress first;
        NATCheckMessage second;
    };
    typedef std::list<QueuedMessage, MemAllocator<QueuedMessage> > MessageQueue;

    enum
    {
        RECEIVED_MESSAGE_COUNT = 3
    };

    JobPerformNATCheck(CallContext* pCallContext, const InetAddress& rAddress);
    virtual ~JobPerformNATCheck();

    void DetectFullNATProperties();
    void DetectSimplifiedNATProperties();
    void DetectCurrentPortMapping(NATRelayInterface* pRelay, const StationURL& rTargetURL, const StationURL& rSourceURL,
                                  bool bRelayProbe);

    bool BindRandomPort();
    bool HandleSimplifiedResults();
    bool HandleFullResults();
    void MessageReceived(const NATCheckMessage& rMessage);
    void TimeoutReceived();
    bool SendNextQueuedMessage();

    void StepPreTest();
    void StepStart();
    void StepTest1();
    void StepTest2();
    void StepTest3();
    void StepTest4();
    void StepTest5();
    void StepSend();
    void StepRecv();
    void StepRun();
    void StepSendProbeRequest();
    void StepEnd();

private:
    void QueueMessage(InetAddress address, unsigned int uiType) { m_queue.push_back(QueuedMessage(address, uiType)); }

    IOCompletionNotifier m_ioNotifierFirst;
    IOCompletionNotifier m_ioNotifierSecond;
    uint32_t m_unknown158;
    uint32_t m_unknown15C;
    ServerList m_servers;
    InetAddress m_localAddress;
    InetAddress m_publicAddress;
    InetAddress m_receivedFrom;
    unsigned int m_mode;
    NATCheckMessage m_recvMessage;
    Socket m_socket;
    unsigned int m_uiCallId;
    NATRelayInterface* m_pRelay;
    StationURL m_targetURL;
    StationURL m_sourceURL;
    unsigned int m_uiTimeoutCount;
    unsigned int m_uiRoundTripTime;
    unsigned int m_uiTest;
    const wchar_t* m_pMapping;
    const wchar_t* m_pFiltering;
    uint64_t m_deadline;
    uint64_t m_sendTime;
    InetAddress m_destination;
    NATCheckMessage m_sendMessage;
    bool m_bPortPreserved;
    bool m_bFirstPass;
    unsigned short m_uiFirstPassPort;
    unsigned short m_uiFirstPassFinalPort;
    unsigned short m_uiSecondPassPort;
    unsigned short m_uiSecondPassFinalPort;
    unsigned int m_uiAttempt;
    bool m_bResponsesReceived;
    uint32_t m_unknown2C8;
    uint32_t m_unknown2CC;
    MessageQueue m_queue;
    NATProperties m_natProperties;
    RecvNATCheckMessage m_aReceived[RECEIVED_MESSAGE_COUNT];
    bool m_bRelayProbe;
};

}
}

#endif
