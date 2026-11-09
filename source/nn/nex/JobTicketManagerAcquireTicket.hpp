#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBTICKETMANAGERACQUIRETICKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBTICKETMANAGERACQUIRETICKET_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class Ticket;
class TicketManager;

class JobTicketManagerAcquireTicket : public StepSequenceJob
{
public:
    JobTicketManagerAcquireTicket(unsigned int uiCallId, TicketManager* pTicketManager,
                                  unsigned int uiSourcePrincipalID, unsigned int uiTargetPrincipalID,
                                  Ticket** ppTicket);
    virtual ~JobTicketManagerAcquireTicket();

    void CompleteJob(CallContext::State state);
    void PrepareCall();
    void ProcessResponse();

    static void AcquireTicketCompletionCallback(CallContext* pCallContext, const UserContext* pUserContext);

private:
    unsigned int m_uiCallId;
    TicketManager* m_pTicketManager;
    unsigned int m_uiSourcePrincipalID;
    unsigned int m_uiTargetPrincipalID;
    Ticket** m_ppTicket;
    ProtocolCallContext m_acquireContext;
    Buffer m_ticketData;
    qResult m_result;
};

}
}

#endif
