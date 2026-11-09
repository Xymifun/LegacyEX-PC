#include "game/core/NS_JobConnectEndPoint_CPP.hpp"

namespace NS_JobConnectEndPoint_CPP
{

void ConnectCallback(nn::nex::EndPoint* pEndPoint, nn::nex::qResult resultConnect,
                     const nn::nex::UserContext* pUserContext)
{
    nn::nex::CriticalSection& csScheduler = nn::nex::Scheduler::GetInstance()->GetCriticalSection();

    csScheduler.Enter();

    nn::nex::JobConnectEndPoint* pJob = static_cast<nn::nex::JobConnectEndPoint*>(pUserContext->GetPointer());
    if (pJob->GetState() == nn::nex::Job::Suspended)
    {
        pJob->ReportConnectResult(resultConnect);
    }

    csScheduler.Leave();
}

void DisconnectCallback(nn::nex::EndPoint* pEndPoint, nn::nex::qResult resultDisconnect,
                        const nn::nex::UserContext* pUserContext)
{
    nn::nex::CriticalSection& csScheduler = nn::nex::Scheduler::GetInstance()->GetCriticalSection();

    csScheduler.Enter();

    nn::nex::JobConnectEndPoint* pJob = static_cast<nn::nex::JobConnectEndPoint*>(pUserContext->GetPointer());
    pJob->SetToReady();
    pJob->ReleaseRef();

    csScheduler.Leave();
}

}
