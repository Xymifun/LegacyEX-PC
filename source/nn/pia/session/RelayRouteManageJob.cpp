#include "nn/pia/session/RelayRouteManageJob.hpp"

#include <coreinit/memory.h>
#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/transport/NetworkRttManager.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
enum
{
    cStationIndexMax = 31,
    cStationStateConnected = 5,
    cReportHeaderSize = 0x10,
    cReportGenerationOffset = 4,
    cReportSerialOffset = 8,
    cTraceRelayRouteLevel = 0x1000000,
    cDummyMessageIntervalMsec = 300,
    cRttShift = 2,
    cRttValueMax = 0xff,
    cRttValueMinimum = 1
};

typedef common::Job::ExecuteResult (common::StepSequenceJob::*StepFunction)();

const char* const cStepNameWaitAllDirectConnectionReport = "RelayRouteManageJob::WaitAllDirectConnectionReport";
const char* const cStepNameSendRelayRouteDirections = "RelayRouteManageJob::SendRelayRouteDirections";
}

RelayRouteManageJob::RelayRouteManageJob()
    : common::StepSequenceJob(),
      m_generation(0),
      m_sequenceNumber(0),
      m_resendTime()
{
    m_stationNum = transport::g_piaTransport->m_stationNum;
    m_connectionReports = common::NewArray<unsigned char>(m_stationNum * m_stationNum);
    m_reportSerials = common::NewArray<unsigned int>(m_stationNum);
    OSBlockSet(m_connectionReports, 0, m_stationNum * m_stationNum);
    OSBlockSet(m_reportSerials, 0, m_stationNum << 2);
    m_stationBitmap = 0;
    m_refusedStationBitmap = 0;
    m_failureReasons = common::NewArray<unsigned char>(m_stationNum);
    OSBlockSet(m_failureReasons, 0, m_stationNum);
}

RelayRouteManageJob::~RelayRouteManageJob()
{
    common::DeleteArray<unsigned char>(m_connectionReports);
    common::DeleteArray<unsigned int>(m_reportSerials);
    common::DeleteArray<unsigned char>(m_failureReasons);
}

void RelayRouteManageJob::Trace(unsigned long long) const
{
}

bool RelayRouteManageJob::Startup(StationIndex stationIndex, unsigned int generation, unsigned int reportSerial)
{
    if (IsRunning() || stationIndex < 0 || stationIndex >= m_stationNum || generation == 0 || generation < m_generation
        || (generation == m_generation && m_reportSerials[stationIndex] >= reportSerial))
        return false;

    m_resendTime = common::g_piaJobScheduler->m_dispatchTime;
    Reset(true);
    m_currentStep = Step(static_cast<StepFunction>(&RelayRouteManageJob::WaitAllDirectConnectionReport),
                         cStepNameWaitAllDirectConnectionReport);
    return true;
}

void RelayRouteManageJob::UpdateConnectionReport(StationIndex stationIndex, const unsigned char* report,
                                                 unsigned int size)
{
    if (stationIndex < 0 || stationIndex >= m_stationNum || size <= cReportHeaderSize)
        return;

    const unsigned int generation = common::deserializeU32(report + cReportGenerationOffset);
    const unsigned int reportSerial = common::deserializeU32(report + cReportSerialOffset);
    if (generation == 0 || generation < m_generation)
        return;

    if (generation == m_generation)
    {
        if (m_reportSerials[stationIndex] >= reportSerial)
            return;
    }
    else
    {
        m_generation = generation;
        OSBlockSet(m_connectionReports, 0, m_stationNum * m_stationNum);
        OSBlockSet(m_reportSerials, 0, m_stationNum << 2);
        m_sequenceNumber = 1;
    }

    OSBlockMove(m_connectionReports + stationIndex * m_stationNum, report + cReportHeaderSize, size - cReportHeaderSize,
                FALSE);
    m_reportSerials[stationIndex] = reportSerial;
}

RelayRouteManageJob::ExecuteResult RelayRouteManageJob::WaitAllDirectConnectionReport()
{
    const StationIndex localStation = g_piaMesh->GetLocalStationIndex();
    if (localStation > cStationIndexMax)
        return MakeExecuteResult(cExecuteKindFinished);

    unsigned short index;
    const unsigned short localIndex = localStation;
    bool allReported = true;
    for (index = 0; index < m_stationNum; ++index)
    {
        if (index != localIndex)
        {
            if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
            {
                if (m_reportSerials[index] == 0)
                    allReported = false;
            }
            else if (m_reportSerials[index] != 0)
            {
                allReported = false;
            }
        }
    }
    if (allReported)
    {
        const common::Time now = common::g_piaJobScheduler->m_dispatchTime;
        const bool isResendTime = m_resendTime <= now;
        unsigned short reportIndex = localIndex * m_stationNum;
        for (index = 0; index < m_stationNum; ++index, ++reportIndex)
        {
            unsigned short rtt = 0;
            transport::Station* station = transport::StationManager::s_pInstance->GetStation(
                static_cast<StationIndex>(index));
            if (station != 0 && station->m_field20 == cStationStateConnected)
                rtt = transport::NetworkRttManager::sInstance->GetAverage(station->m_stationAddress);

            if (index != localIndex && g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
            {
                if (rtt == 0)
                {
                    if (!station->IsConnectionRouteRelay())
                    {
                        if (isResendTime)
                        {
                            g_piaMesh->GetMeshProtocol()->SendDummyMessage(static_cast<StationIndex>(index));
                            const common::TimeSpan interval = common::TimeSpan::FromMSec(cDummyMessageIntervalMsec);
                            m_resendTime = common::g_piaJobScheduler->m_dispatchTime + interval;
                        }
                        allReported = false;
                    }
                }
                else if (station->IsConnectionRouteRelay())
                {
                    rtt = 0;
                }
            }
            if (rtt != 0 && (rtt >> cRttShift) == 0)
                rtt = cRttValueMinimum;
            else
                rtt >>= cRttShift;
            m_connectionReports[reportIndex] = rtt < cRttValueMax ? rtt : cRttValueMax;
        }
    }
    if (!allReported)
        return MakeExecuteResult(cExecuteKindNextStep);

    m_currentStep = Step(static_cast<StepFunction>(&RelayRouteManageJob::SendRelayRouteDirections),
                         cStepNameSendRelayRouteDirections);
    return MakeExecuteResult(cExecuteKindContinue);
}

RelayRouteManageJob::ExecuteResult RelayRouteManageJob::SendRelayRouteDirections()
{
    const StationIndex localStation = g_piaMesh->GetLocalStationIndex();
    if (localStation > cStationIndexMax)
        return MakeExecuteResult(cExecuteKindFinished);

    Trace(cTraceRelayRouteLevel);
    transport::RelayRouteManager* manager = transport::g_piaTransport->m_relayRouteManager;
    manager->Trace(cTraceRelayRouteLevel);
    if (manager->UpdateDirectConnectionData(m_connectionReports, m_stationNum * m_stationNum, m_stationBitmap,
                                            &m_refusedStationBitmap, localStation, m_failureReasons)
        == 0)
    {
        manager->m_unknown20 = m_generation;
        manager->m_unknown1c = m_sequenceNumber;
        m_stationBitmap = 0;
        manager->Trace(cTraceRelayRouteLevel);
        if (!g_piaMesh->GetMeshProtocol()->SendRelayRouteDirections())
        {
            m_currentStep = Step(static_cast<StepFunction>(&RelayRouteManageJob::WaitAllDirectConnectionReport),
                                 cStepNameWaitAllDirectConnectionReport);
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        ++m_sequenceNumber;
    }
    return MakeExecuteResult(cExecuteKindFinished);
}

void RelayRouteManageJob::Cleanup()
{
    m_generation = 0;
    OSBlockSet(m_connectionReports, 0, m_stationNum * m_stationNum);
    OSBlockSet(m_reportSerials, 0, sizeof(unsigned int) * m_stationNum);
    m_stationBitmap = 0;
    m_refusedStationBitmap = 0;
}

void RelayRouteManageJob::PrepareForBecomingNewHost()
{
    transport::g_piaTransport->m_relayRouteManager->UpdateRelayTable(m_stationBitmap);
    m_stationBitmap = 0;
}

}
}
}
