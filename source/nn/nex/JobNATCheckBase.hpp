#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBNATCHECKBASE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBNATCHECKBASE_HPP

#include <stdint.h>
#include <list>
#include <utility>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NATCheckMessage.hpp"
#include "nn/nex/Socket.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class JobNATCheckBase : public StepSequenceJob
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

    struct QueuedMessage
    {
        QueuedMessage(const InetAddress& rAddress, unsigned int uiType) : first(rAddress)
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

    JobNATCheckBase(CallContext* pCallContext, const InetAddress& rAddress);
    virtual ~JobNATCheckBase();

    virtual void StepInitializeImpl() = 0;
    virtual bool Finished() = 0;
    virtual qResult HandleResult() = 0;

    void StepInitialize();
    void StepDNSResolve();
    void StepExecute();
    void StepFinish();

    bool BindRandomPort();
    bool InitializeSocket();
    void ExecuteNATCheck(unsigned int uiTimeout);
    bool CanRetry();
    bool SendNextQueuedMessage();

protected:
    void QueueMessage(InetAddress address, unsigned int uiType) { m_queue.push_back(QueuedMessage(address, uiType)); }

    InetAddress m_localAddress;
    Socket m_socket;
    MessageQueue m_queue;
    NATCheckMessage m_recvMessage;
    InetAddress m_receivedFrom;
    unsigned int m_uiCallId;
    RecvNATCheckMessage m_aReceived[RECEIVED_MESSAGE_COUNT];
    uint64_t m_deadline;
    unsigned int m_uiAttempt;
    CallContext m_callContext;
};

}
}

#endif
