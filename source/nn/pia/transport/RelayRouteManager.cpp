#include "nn/pia/transport/RelayRouteManager.hpp"

#include <coreinit/memory.h>
#include <cstring>
#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

namespace
{

enum StationRankField
{
    cStationRankStation,
    cStationRankBrokenRouteCount,
    cStationRankFieldNum
};

enum RttCandidateField
{
    cRttCandidateStation,
    cRttCandidateRtt,
    cRttCandidateFieldNum
};

unsigned short g_rttMax;
unsigned short g_relayNumMax;

}

RelayRouteManager::RelayRouteManager()
    : m_rttTable(0),
      m_routeTable(0),
      m_relayNumTable(0),
      m_directStationBitmaps(0),
      m_relayEmulationTable(0),
      m_failureReasonTable(0),
      m_rttMax(500),
      m_relayNumMax(0x400),
      m_unknown1c(0),
      m_unknown20(0),
      m_unknown24(cStationIndexInvalid),
      m_routeBitCount(5),
      m_invalidRoute(0xff),
      m_joiningStationBitmap(0),
      m_unknown30(0),
      m_originalRouteTable(0)
{
}

RelayRouteManager::~RelayRouteManager()
{
}

Result RelayRouteManager::Initialize(unsigned int stationNum, bool relayEmulationEnabled)
{
    if (stationNum == 0 || stationNum > cStationNumMax)
        return cResultInvalidArgument;

    const unsigned int tableSize = stationNum * stationNum;
    m_rttTable = common::NewArray<unsigned short>(tableSize);
    if (m_rttTable == 0)
        return cResultOutOfMemory;

    OSBlockSet(m_rttTable, 0, tableSize * 2);
    m_routeTable = common::NewArray<unsigned char>(tableSize);
    if (m_routeTable == 0)
        return cResultOutOfMemory;

    unsigned int routeIndex = 0;
    for (unsigned int source = 0; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < stationNum; ++destination)
            m_routeTable[routeIndex++] = static_cast<unsigned char>(source);
    }

    m_relayNumTable = common::NewArray<unsigned char>(stationNum);
    if (m_relayNumTable == 0)
        return cResultOutOfMemory;

    OSBlockSet(m_relayNumTable, 0, stationNum);
    m_directStationBitmaps = common::NewArray<unsigned int>(stationNum);
    if (m_directStationBitmaps == 0)
        return cResultOutOfMemory;

    OSBlockSet(m_directStationBitmaps, 0, stationNum << 2);
    m_relayEmulationEnabled = relayEmulationEnabled;
    if (relayEmulationEnabled)
    {
        m_relayEmulationTable = common::NewArray<unsigned char>(tableSize);
        if (m_relayEmulationTable == 0)
            return cResultOutOfMemory;

        OSBlockSet(m_relayEmulationTable, 0, tableSize);
    }

    m_failureReasonTable = common::NewArray<unsigned char>(stationNum);
    if (m_failureReasonTable == 0)
        return cResultOutOfMemory;

    OSBlockSet(m_failureReasonTable, 0, stationNum);
    m_originalRouteTable = common::NewArray<unsigned char>(tableSize);
    if (m_originalRouteTable == 0)
        return cResultOutOfMemory;

    OSBlockMove(m_originalRouteTable, m_routeTable, tableSize, FALSE);
    return 0;
}

void RelayRouteManager::Finalize()
{
    if (m_rttTable != 0)
    {
        common::DeleteArray<unsigned short>(m_rttTable);
        m_rttTable = 0;
    }
    if (m_routeTable != 0)
    {
        common::DeleteArray<unsigned char>(m_routeTable);
        m_routeTable = 0;
    }
    if (m_relayNumTable != 0)
    {
        common::DeleteArray<unsigned char>(m_relayNumTable);
        m_relayNumTable = 0;
    }
    if (m_directStationBitmaps != 0)
    {
        common::DeleteArray<unsigned int>(m_directStationBitmaps);
        m_directStationBitmaps = 0;
    }
    if (m_relayEmulationEnabled && m_relayEmulationTable != 0)
    {
        common::DeleteArray<unsigned char>(m_relayEmulationTable);
        m_relayEmulationTable = 0;
    }
    if (m_failureReasonTable != 0)
    {
        common::DeleteArray<unsigned char>(m_failureReasonTable);
        m_failureReasonTable = 0;
    }
    if (m_originalRouteTable != 0)
    {
        common::DeleteArray<unsigned char>(m_originalRouteTable);
        m_originalRouteTable = 0;
    }
}

void RelayRouteManager::CalcStationType(unsigned int stationBitmap, unsigned int* stayStationBitmap,
                                        unsigned int* leaveStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    *stayStationBitmap = 0;
    *leaveStationBitmap = 0;
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        const unsigned int stationFlag = 1 << station;
        const bool isJoining = (m_joiningStationBitmap & stationFlag) != 0;
        const bool isInBitmap = (stationBitmap & stationFlag) != 0;
        bool hasRtt = false;
        for (unsigned int other = 0; other < stationNum; ++other)
        {
            if (m_rttTable[station * stationNum + other] != 0)
            {
                hasRtt = true;
                break;
            }
        }

        if (!hasRtt && isInBitmap)
        {
            if (m_relayEmulationEnabled)
            {
                for (unsigned int other = 0; other < stationNum; ++other)
                {
                    if (m_relayEmulationTable[station * stationNum + other] == 1)
                    {
                        *leaveStationBitmap |= stationFlag;
                        break;
                    }
                }
            }
        }
        else if (hasRtt)
        {
            if (isJoining)
            {
                if (!isInBitmap)
                    *stayStationBitmap |= stationFlag;
                else
                    *leaveStationBitmap |= stationFlag;
            }
        }
        else
        {
            if (isJoining)
            {
                if (!m_relayEmulationEnabled || station != m_unknown24)
                    *leaveStationBitmap |= stationFlag;
                else
                    *stayStationBitmap |= stationFlag;
            }
        }
    }
}

void RelayRouteManager::ProcLeaveStation(unsigned int leaveStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        if ((leaveStationBitmap & (1 << station)) != 0)
        {
            for (unsigned int other = 0; other < stationNum; ++other)
            {
                m_routeTable[station * stationNum + other] = static_cast<unsigned char>(station);
                m_routeTable[other * stationNum + station] = static_cast<unsigned char>(other);
            }
        }
    }

    for (unsigned int source = 0; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < stationNum; ++destination)
        {
            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (source != relay && destination != relay && (leaveStationBitmap & (1 << relay)) != 0)
                m_routeTable[source * stationNum + destination] = m_invalidRoute;
        }
    }
}

void RelayRouteManager::CalcRelayNumTable()
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    OSBlockSet(m_relayNumTable, 0, stationNum);
    for (unsigned int source = 0; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < stationNum; ++destination)
        {
            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (relay != source && relay != destination && relay < stationNum)
                ++m_relayNumTable[relay];
        }
    }
}

inline bool RelayRouteManager::IsRelayEnabled(unsigned int source, unsigned int relay, unsigned int destination,
                                              RelayFailureReason* failureReason)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    const unsigned short sourceRtt = m_rttTable[source * stationNum + relay];
    unsigned short relayRtt;
    if (sourceRtt == 0 || (relayRtt = m_rttTable[relay * stationNum + destination]) == 0)
    {
        if (failureReason != 0)
            *failureReason = cRelayFailureReasonNoRtt;
        return false;
    }

    if (static_cast<unsigned int>(sourceRtt + relayRtt) > m_rttMax)
    {
        if (failureReason != 0)
            *failureReason = cRelayFailureReasonRttTooLarge;
        return false;
    }

    if (m_relayNumTable[relay] + 2 > m_relayNumMax)
    {
        if (failureReason != 0)
            *failureReason = cRelayFailureReasonRelayLimit;
        return false;
    }
    return true;
}

bool RelayRouteManager::SwitchRelay(unsigned int relayStation, unsigned int stationBitmap, unsigned short rttLimit,
                                    unsigned short relayNumLimit)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int source = 1; source < stationNum; ++source)
    {
        if (source == relayStation)
            continue;

        for (unsigned int destination = 0; destination < source; ++destination)
        {
            if (destination == relayStation || m_routeTable[source * stationNum + destination] != relayStation)
                continue;

            for (unsigned int relay = 0; relay < stationNum; ++relay)
            {
                if ((stationBitmap & (1 << relay)) != 0 && relay != source && relay != destination
                    && relay != relayStation && IsRelayEnabled(source, relay, destination, 0)
                    && static_cast<int>(m_rttTable[source * stationNum + relay]
                                        + m_rttTable[relay * stationNum + destination])
                           <= rttLimit
                    && static_cast<int>(m_relayNumTable[relay] + 2) <= relayNumLimit)
                {
                    m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + source] = static_cast<unsigned char>(relay);
                    m_relayNumTable[relay] += 2;
                    m_relayNumTable[relayStation] -= 2;
                    return true;
                }
            }
        }
    }
    return false;
}

bool RelayRouteManager::CalcRelayRouteWithSwitchingRelay(unsigned int source, unsigned int destination,
                                                         unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int candidateBitmap = 0;
    for (unsigned int relay = 0; relay < stationNum; ++relay)
    {
        const unsigned int relayFlag = 1 << relay;
        if ((stationBitmap & relayFlag) != 0)
        {
            const unsigned short sourceRtt = m_rttTable[source * stationNum + relay];
            if (sourceRtt != 0)
            {
                const unsigned short destinationRtt = m_rttTable[relay * stationNum + destination];
                if (destinationRtt != 0)
                {
                    if (sourceRtt + destinationRtt <= m_rttMax)
                    {
                        if (static_cast<unsigned short>(m_relayNumTable[relay]) <= m_relayNumMax)
                            candidateBitmap |= relayFlag;
                    }
                }
            }
        }
    }

    for (unsigned int relay = 0; relay < stationNum; ++relay)
    {
        if ((candidateBitmap & (1 << relay)) != 0 && SwitchRelay(relay, stationBitmap, m_rttMax, m_relayNumMax))
        {
            m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(relay);
            m_routeTable[destination * stationNum + source] = static_cast<unsigned char>(relay);
            m_relayNumTable[relay] += 2;
            return true;
        }
    }
    return false;
}

bool RelayRouteManager::DecideBrokenRouteRelayStation(unsigned int stationBitmap, unsigned int* refusedStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    RelayFailureReason reason;
    unsigned char relayCandidates[32][cStationRankFieldNum];
    unsigned char ranks[32][cStationRankFieldNum];
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        unsigned char brokenRouteCount = 0;
        for (unsigned int other = 0; other < stationNum; ++other)
        {
            if (m_routeTable[station * stationNum + other] == m_invalidRoute)
                ++brokenRouteCount;
            if (m_routeTable[other * stationNum + station] == m_invalidRoute)
                ++brokenRouteCount;
        }
        ranks[station][cStationRankStation] = static_cast<unsigned char>(station);
        ranks[station][cStationRankBrokenRouteCount] = brokenRouteCount;
    }

    for (unsigned int pass = stationNum - 1; pass > 0; --pass)
    {
        for (unsigned int index = 0; index < pass; ++index)
        {
            if (ranks[index][cStationRankBrokenRouteCount] > ranks[index + 1][cStationRankBrokenRouteCount])
            {
                for (unsigned int field = 0; field < cStationRankFieldNum; ++field)
                {
                    const unsigned char swapped = ranks[index + 1][field];
                    ranks[index + 1][field] = ranks[index][field];
                    ranks[index][field] = swapped;
                }
            }
        }
    }

    for (unsigned int index = 0; index < stationNum; ++index)
    {
        relayCandidates[index][cStationRankStation] = ranks[index][cStationRankStation];
        relayCandidates[index][cStationRankBrokenRouteCount] = ranks[index][cStationRankBrokenRouteCount];
    }

    unsigned int otherIndex;
    unsigned int destination;
    unsigned int rankIndex;
    unsigned int source;
    for (rankIndex = 1; rankIndex < stationNum; ++rankIndex)
    {
        source = ranks[rankIndex][cStationRankStation];
        for (otherIndex = 0; otherIndex < rankIndex; ++otherIndex)
        {
            destination = ranks[otherIndex][cStationRankStation];
            if ((stationBitmap & (1 << source)) == 0 || (stationBitmap & (1 << destination)) == 0
                || m_routeTable[source * stationNum + destination] != m_invalidRoute)
                continue;

            RelayFailureReason failureReason = cRelayFailureReasonNoRtt;
            bool relayFound = false;
            for (unsigned int candidateIndex = 0; candidateIndex < stationNum; ++candidateIndex)
            {
                const unsigned int relay = relayCandidates[candidateIndex][cStationRankStation];
                if (relay == source || relay == destination)
                    continue;

                const unsigned int relayFlag = 1 << relay;
                if ((stationBitmap & relayFlag) == 0 || (*refusedStationBitmap & relayFlag) != 0)
                    continue;

                if (IsRelayEnabled(source, relay, destination, &reason))
                {
                    m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + source] = static_cast<unsigned char>(relay);
                    for (unsigned int index = 0; index < stationNum; ++index)
                    {
                        if (relayCandidates[index][cStationRankStation] == source
                            || relayCandidates[index][cStationRankStation] == destination)
                            relayCandidates[index][cStationRankBrokenRouteCount] -= 2;
                    }

                    for (unsigned int pass = stationNum - 1; pass > 0; --pass)
                    {
                        for (unsigned int index = 0; index < pass; ++index)
                        {
                            if (relayCandidates[index][cStationRankBrokenRouteCount]
                                > relayCandidates[index + 1][cStationRankBrokenRouteCount])
                            {
                                for (unsigned int field = 0; field < cStationRankFieldNum; ++field)
                                {
                                    const unsigned char swapped = relayCandidates[index + 1][field];
                                    relayCandidates[index + 1][field] = relayCandidates[index][field];
                                    relayCandidates[index][field] = swapped;
                                }
                            }
                        }
                    }
                    m_relayNumTable[relay] += 2;
                    relayFound = true;
                    break;
                }
                if (reason != cRelayFailureReasonNoRtt)
                    failureReason = reason;
            }

            if (relayFound
                || CalcRelayRouteWithSwitchingRelay(source, destination, stationBitmap & ~*refusedStationBitmap))
                continue;

            unsigned int refusedStation = 0;
            if (source == m_unknown24)
            {
                refusedStation = destination;
            }
            else if (destination == m_unknown24)
            {
                refusedStation = source;
            }
            else
            {
                for (unsigned int index = 0; index < stationNum; ++index)
                {
                    if (relayCandidates[index][cStationRankStation] == source)
                    {
                        refusedStation = destination;
                        break;
                    }
                    if (relayCandidates[index][cStationRankStation] == destination)
                    {
                        refusedStation = source;
                        break;
                    }
                }
            }

            *refusedStationBitmap |= 1 << refusedStation;
            m_failureReasonTable[refusedStation] = static_cast<unsigned char>(failureReason);
            for (unsigned int other = 0; other < stationNum; ++other)
            {
                for (unsigned int direction = 0; direction < 2; ++direction)
                {
                    unsigned int first;
                    unsigned int second;
                    if (direction == 0)
                    {
                        first = refusedStation;
                        second = other;
                    }
                    else
                    {
                        first = other;
                        second = refusedStation;
                    }

                    const unsigned int relay = m_routeTable[first * stationNum + second];
                    if (relay != first && relay != second && relay < stationNum)
                        --m_relayNumTable[relay];
                    m_routeTable[first * stationNum + second] = static_cast<unsigned char>(first);
                }
            }

            for (unsigned int row = 0; row < stationNum; ++row)
            {
                for (unsigned int column = 0; column < stationNum; ++column)
                {
                    const unsigned int relay = m_routeTable[row * stationNum + column];
                    if (relay != row && relay != column && relay == refusedStation)
                        m_routeTable[row * stationNum + column] = m_invalidRoute;
                }
            }
            return false;
        }
    }
    return true;
}

void RelayRouteManager::ProcStayStation(unsigned int stayStationBitmap, unsigned int* refusedStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int source = 0; source < stationNum; ++source)
    {
        if ((stayStationBitmap & (1 << source)) == 0)
            continue;

        for (unsigned int destination = 0; destination < stationNum; ++destination)
        {
            if (source == destination || (stayStationBitmap & (1 << destination)) == 0
                || m_rttTable[source * stationNum + destination] != 0)
                continue;

            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (relay == source)
                continue;

            if (relay != destination)
            {
                if (relay >= stationNum)
                    continue;
                if (m_rttTable[source * stationNum + relay] != 0 && m_rttTable[relay * stationNum + destination] != 0)
                    continue;
            }
            m_routeTable[source * stationNum + destination] = m_invalidRoute;
        }
    }

    CalcRelayNumTable();
    while (!DecideBrokenRouteRelayStation(stayStationBitmap, refusedStationBitmap))
    {
    }
}

void RelayRouteManager::Trace(unsigned long long) const
{
}

bool RelayRouteManager::ProcNewStationOne(unsigned int station, unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int destination = 0; destination < stationNum; ++destination)
    {
        if ((stationBitmap & (1 << destination)) == 0)
            continue;

        if (m_rttTable[station * stationNum + destination] != 0)
        {
            m_routeTable[station * stationNum + destination] = static_cast<unsigned char>(destination);
            m_routeTable[destination * stationNum + station] = static_cast<unsigned char>(station);
            continue;
        }

        unsigned int failureReason = cRelayFailureReasonNoRtt;
        bool relayFound = false;
        for (unsigned int relay = 0; relay < stationNum; ++relay)
        {
            if ((stationBitmap & (1 << relay)) != 0 && relay != station && relay != destination)
            {
                RelayFailureReason reason;
                if (IsRelayEnabled(station, relay, destination, &reason))
                {
                    m_routeTable[station * stationNum + destination] = static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + station] = static_cast<unsigned char>(relay);
                    m_relayNumTable[relay] += 2;
                    relayFound = true;
                    break;
                }
                if (reason != cRelayFailureReasonNoRtt)
                    failureReason = reason;
            }
        }

        if (relayFound || CalcRelayRouteWithSwitchingRelay(station, destination, stationBitmap))
            continue;

        m_failureReasonTable[station] = static_cast<unsigned char>(failureReason);
        for (unsigned int other = 0; other < destination; ++other)
        {
            const unsigned int relay = m_routeTable[station * stationNum + other];
            if (relay != station && relay != other)
                m_relayNumTable[relay] -= 2;
            m_routeTable[station * stationNum + other] = static_cast<unsigned char>(station);
            m_routeTable[other * stationNum + station] = static_cast<unsigned char>(other);
        }
        return false;
    }
    return true;
}

void RelayRouteManager::ProcNewStation(unsigned int newStationBitmap, unsigned int* refusedStationBitmap,
                                       unsigned int* acceptedStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if ((newStationBitmap & (1 << m_unknown24)) != 0)
    {
        ProcNewStationOne(m_unknown24, *acceptedStationBitmap);
        const unsigned int stationFlag = 1 << m_unknown24;
        newStationBitmap -= stationFlag;
        *acceptedStationBitmap |= stationFlag;
    }

    bool accepted;
    do
    {
        accepted = false;
        for (unsigned int station = 0; station < stationNum; ++station)
        {
            const unsigned int stationFlag = 1 << station;
            if ((newStationBitmap & stationFlag) != 0 && ProcNewStationOne(station, *acceptedStationBitmap))
            {
                accepted = true;
                newStationBitmap -= stationFlag;
                *acceptedStationBitmap |= stationFlag;
            }
        }
    } while (accepted);

    for (unsigned int station = 0; station < stationNum; ++station)
    {
        const unsigned int stationFlag = 1 << station;
        if ((newStationBitmap & stationFlag) != 0)
            *refusedStationBitmap |= stationFlag;
    }
}

bool RelayRouteManager::RelayRouteOptimizationRTTOne(unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int longestRtt = 0;
    unsigned int longestSource;
    unsigned int longestDestination;
    unsigned int longestRelay;
    for (unsigned int source = 0; source < stationNum; ++source)
    {
        if ((stationBitmap & (1 << source)) == 0)
            continue;

        for (unsigned int destination = source + 1; destination < stationNum; ++destination)
        {
            if ((stationBitmap & (1 << destination)) == 0)
                continue;

            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (relay == source || relay == destination)
                continue;

            const unsigned int rtt = m_rttTable[source * stationNum + relay]
                                     + m_rttTable[relay * stationNum + destination];
            if (rtt > longestRtt)
            {
                longestRtt = rtt;
                longestRelay = relay;
                longestSource = source;
                longestDestination = destination;
            }
        }
    }

    if (longestRtt == 0)
        return false;

    unsigned int candidateNum = 0;
    unsigned int candidates[32][cRttCandidateFieldNum];
    for (unsigned int index = 0; index < 32; ++index)
    {
        candidates[index][cRttCandidateStation] = 0;
        candidates[index][cRttCandidateRtt] = 0;
    }
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        if ((stationBitmap & (1 << station)) == 0)
            continue;

        const unsigned int sourceRtt = m_rttTable[longestSource * stationNum + station];
        if (sourceRtt == 0)
            continue;

        const unsigned int destinationRtt = m_rttTable[station * stationNum + longestDestination];
        if (destinationRtt == 0)
            continue;

        const unsigned int rtt = sourceRtt + destinationRtt;
        if (rtt < longestRtt && rtt <= m_rttMax)
        {
            candidates[candidateNum][cRttCandidateStation] = station;
            candidates[candidateNum][cRttCandidateRtt] = rtt;
            ++candidateNum;
        }
    }

    if (candidateNum == 0)
        return false;

    for (unsigned int pass = candidateNum - 1; pass > 0; --pass)
    {
        for (unsigned int index = 0; index < pass; ++index)
        {
            if (candidates[index][cRttCandidateRtt] > candidates[index + 1][cRttCandidateRtt])
            {
                for (unsigned int field = 0; field < cRttCandidateFieldNum; ++field)
                {
                    const unsigned char swapped = candidates[index + 1][field];
                    candidates[index + 1][field] = candidates[index][field];
                    candidates[index][field] = swapped;
                }
            }
        }
    }

    for (unsigned int index = 0; index < candidateNum; ++index)
    {
        const unsigned int relay = candidates[index][cRttCandidateStation];
        bool switched = false;
        if (m_relayNumTable[relay] + 2 <= m_relayNumMax)
        {
            switched = true;
        }
        else if (m_relayNumTable[relay] <= m_relayNumMax)
        {
            m_relayNumTable[longestRelay] -= 2;
            if (SwitchRelay(relay, stationBitmap, static_cast<unsigned short>(longestRtt - 1), m_relayNumMax))
                switched = true;
            m_relayNumTable[longestRelay] += 2;
        }

        if (switched)
        {
            m_routeTable[longestSource * stationNum + longestDestination] = static_cast<unsigned char>(relay);
            m_routeTable[longestDestination * stationNum + longestSource] = static_cast<unsigned char>(relay);
            m_relayNumTable[longestRelay] -= 2;
            m_relayNumTable[relay] += 2;
            return true;
        }
    }
    return false;
}

void RelayRouteManager::RelayRouteOptimizationRTT(unsigned int stationBitmap)
{
    while (RelayRouteOptimizationRTTOne(stationBitmap))
    {
    }
}

bool RelayRouteManager::RelayRouteOptimizationRelayNumOne(unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int longestRtt = 0;
    unsigned short busiestRelayNum;
    unsigned int busiestStation;
    for (unsigned int source = 0; source < stationNum; ++source)
    {
        if ((stationBitmap & (1 << source)) == 0)
            continue;

        for (unsigned int destination = source + 1; destination < stationNum; ++destination)
        {
            if ((stationBitmap & (1 << destination)) == 0)
                continue;

            const unsigned int relay = m_routeTable[source * stationNum + destination];
            if (relay != source && relay != destination)
            {
                const unsigned int rtt = m_rttTable[source * stationNum + relay]
                                         + m_rttTable[relay * stationNum + destination];
                if (rtt > longestRtt)
                    longestRtt = rtt;
            }
        }
    }

    busiestRelayNum = 0;
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        if (busiestRelayNum < m_relayNumTable[station])
        {
            busiestRelayNum = m_relayNumTable[station];
            busiestStation = station;
        }
    }

    if (busiestRelayNum != 0
        && SwitchRelay(busiestStation, stationBitmap, static_cast<unsigned short>(longestRtt), busiestRelayNum - 1))
        return true;
    return false;
}

void RelayRouteManager::RelayRouteOptimizationRelayNum(unsigned int stationBitmap)
{
    while (RelayRouteOptimizationRelayNumOne(stationBitmap))
    {
    }
}

void RelayRouteManager::RelayRouteOptimization(unsigned int stationBitmap)
{
    RelayRouteOptimizationRTT(stationBitmap);
    RelayRouteOptimizationRelayNum(stationBitmap);
}

void RelayRouteManager::ProcRefusedStation(unsigned int refusedStationBitmap, unsigned int acceptedStationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int refused = 0; refused < stationNum; ++refused)
    {
        if ((refusedStationBitmap & (1 << refused)) == 0)
            continue;

        for (unsigned int destination = 0; destination < stationNum; ++destination)
        {
            if (((acceptedStationBitmap | refusedStationBitmap) & (1 << destination)) == 0 || refused == destination)
                continue;

            if (m_rttTable[refused * stationNum + destination] != 0)
            {
                m_routeTable[refused * stationNum + destination] = static_cast<unsigned char>(destination);
                m_routeTable[destination * stationNum + refused] = static_cast<unsigned char>(refused);
                continue;
            }

            bool relayFound = false;
            for (unsigned int relay = 0; relay < stationNum; ++relay)
            {
                if ((acceptedStationBitmap & (1 << relay)) != 0 && IsRelayEnabled(refused, relay, destination, 0))
                {
                    m_routeTable[refused * stationNum + destination] = static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + refused] = static_cast<unsigned char>(relay);
                    m_relayNumTable[relay] += 2;
                    relayFound = true;
                    break;
                }
            }
            if (relayFound)
                continue;

            for (unsigned int relay = 0; relay < stationNum; ++relay)
            {
                if ((acceptedStationBitmap & (1 << relay)) != 0 && m_rttTable[refused * stationNum + relay] != 0
                    && m_rttTable[relay * stationNum + destination] != 0)
                {
                    m_routeTable[refused * stationNum + destination] = static_cast<unsigned char>(relay);
                    m_routeTable[destination * stationNum + refused] = static_cast<unsigned char>(relay);
                    m_relayNumTable[relay] += 2;
                    break;
                }
            }
        }
    }
}

void RelayRouteManager::UpdateJoiningStationBitmap()
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int joiningStationBitmap = 0;
    m_joiningStationBitmap = 0;
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        for (unsigned int other = 0; other < stationNum; ++other)
        {
            if (m_routeTable[station * stationNum + other] != station)
            {
                joiningStationBitmap |= 1 << station;
                m_joiningStationBitmap = joiningStationBitmap;
                break;
            }
        }
    }
}

Result RelayRouteManager::UpdateDirectConnectionData(const unsigned char* rttBuffer, unsigned int rttBufferSize,
                                                     unsigned int stationBitmap, unsigned int* refusedStationBitmap,
                                                     ::nn::pia::StationIndex stationIndex,
                                                     unsigned char* failureReasons)
{
    m_unknown24 = stationIndex;
    if (m_joiningStationBitmap == 0 && (stationBitmap & (1 << stationIndex)) == 0)
        m_joiningStationBitmap = 1 << stationIndex;

    const unsigned int stationNum = g_piaTransport->m_stationNum;
    const unsigned int tableSize = stationNum * stationNum;
    if (rttBufferSize != tableSize)
        return cResultInvalidArgument;
    if (rttBuffer == 0)
        return cResultInvalidArgument;
    if (refusedStationBitmap == 0)
        return cResultInvalidArgument;
    if (failureReasons == 0)
        return cResultInvalidArgument;

    for (unsigned int source = 0; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < stationNum; ++destination)
            m_rttTable[source * stationNum + destination] = static_cast<unsigned short>(
                rttBuffer[source * stationNum + destination] << 2);
    }

    for (unsigned int source = 0; source < stationNum; ++source)
    {
        unsigned int directStationBitmap = 0;
        for (unsigned int destination = 0; destination < stationNum; ++destination)
        {
            if (m_rttTable[source * stationNum + destination] != 0)
                directStationBitmap |= 1 << destination;
        }
        m_directStationBitmaps[source] = directStationBitmap;
    }

    if (m_relayEmulationEnabled)
    {
        for (unsigned int source = 0; source < stationNum; ++source)
        {
            for (unsigned int destination = 0; destination < stationNum; ++destination)
            {
                if (m_relayEmulationTable[source * stationNum + destination] == 1)
                    m_rttTable[source * stationNum + destination] = 0;
            }
        }
    }

    for (unsigned int source = 1; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < source; ++destination)
        {
            const unsigned int forwardRtt = m_rttTable[source * stationNum + destination];
            const unsigned int backwardRtt = m_rttTable[destination * stationNum + source];
            unsigned int averageRtt;
            if (forwardRtt == 0 || backwardRtt == 0)
                averageRtt = 0;
            else
                averageRtt = (forwardRtt + backwardRtt) >> 1;
            m_rttTable[source * stationNum + destination] = static_cast<unsigned short>(averageRtt);
            m_rttTable[destination * stationNum + source] = static_cast<unsigned short>(averageRtt);
        }
    }

    *refusedStationBitmap = 0;
    unsigned int leaveStationBitmap = 0;
    unsigned int stayStationBitmap = 0;
    CalcStationType(stationBitmap, &stayStationBitmap, &leaveStationBitmap);
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        const unsigned int stationFlag = 1 << station;
        if ((stayStationBitmap & stationFlag) != 0 && (m_unknown30 & stationFlag) != 0)
        {
            stayStationBitmap &= ~stationFlag;
            leaveStationBitmap |= stationFlag;
            *refusedStationBitmap |= stationFlag;
        }
    }

    ProcLeaveStation(leaveStationBitmap);
    ProcStayStation(stayStationBitmap, refusedStationBitmap);

    const unsigned int refusedByStay = *refusedStationBitmap;
    *refusedStationBitmap = 0;
    unsigned int acceptedStationBitmap = stayStationBitmap & ~refusedByStay;
    ProcNewStation(stationBitmap | refusedByStay, refusedStationBitmap, &acceptedStationBitmap);
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        unsigned char failureReason = 0;
        if ((*refusedStationBitmap & (1 << station)) != 0)
            failureReason = m_failureReasonTable[station];
        failureReasons[station] = failureReason;
    }

    RelayRouteOptimization(acceptedStationBitmap);
    ProcRefusedStation(*refusedStationBitmap, acceptedStationBitmap);
    UpdateJoiningStationBitmap();
    m_unknown30 = *refusedStationBitmap;
    OSBlockMove(m_originalRouteTable, m_routeTable, tableSize, FALSE);
    return 0;
}

Result RelayRouteManager::WriteRelayRouteDirectionsToBuffer(unsigned char* buffer, unsigned int size,
                                                            unsigned int* writtenSize) const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int routeBits = m_routeBitCount * stationNum * stationNum;
    unsigned int bitsUsed = 0;
    unsigned int offset = 0;
    if ((routeBits & 7) != 0)
        routeBits = (routeBits & ~7u) + 8;
    const unsigned int routeBytes = routeBits >> 3;
    if (size < routeBytes + stationNum * 4 + 4)
    {
        *writtenSize = 0;
        return cResultInvalidArgument;
    }
    if (buffer == 0)
        return cResultInvalidArgument;

    std::memset(buffer, 0, size);
    for (unsigned int source = 0; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < stationNum; ++destination)
        {
            const char freeBits = static_cast<char>(8 - bitsUsed - m_routeBitCount);
            unsigned int shift = freeBits;
            const unsigned char route = m_routeTable[source * stationNum + destination];
            if (freeBits < 0)
            {
                const unsigned int fitBits = m_routeBitCount + freeBits;
                bitsUsed = -shift;
                shift = static_cast<char>(freeBits + 8);
                buffer[offset] += (route >> bitsUsed) & ((1 << fitBits) - 1);
                ++offset;
            }
            bitsUsed = (8 - shift) & 0xff;
            buffer[offset] += route << shift;
            if (shift == 0)
            {
                ++offset;
                bitsUsed = 0;
            }
        }
    }

    offset = routeBytes;
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        unsigned int directStationBitmap = m_directStationBitmaps[station];
        unsigned int shift = 24;
        for (unsigned int byteIndex = 0; byteIndex < 4; ++byteIndex)
        {
            const unsigned int value = directStationBitmap >> shift;
            buffer[offset] = static_cast<unsigned char>(value);
            ++offset;
            directStationBitmap -= value << shift;
            shift -= 8;
        }
    }

    OSBlockMove(buffer + offset, &m_unknown30, 4, FALSE);
    *writtenSize = offset + 4;
    return 0;
}

Result RelayRouteManager::ReadRelayRouteDirectionsFromBuffer(const unsigned char* buffer, unsigned int size)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int routeBits = m_routeBitCount * stationNum * stationNum;
    unsigned int bitsUsed = 0;
    unsigned int offset = 0;
    if ((routeBits & 7) != 0)
        routeBits = (routeBits & ~7u) + 8;
    const unsigned int routeBytes = routeBits >> 3;
    if (size < routeBytes + stationNum * 4 + 4)
        return cResultInvalidArgument;
    if (buffer == 0)
        return cResultInvalidArgument;

    for (unsigned int source = 0; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < stationNum; ++destination)
        {
            const char freeBits = static_cast<char>(8 - bitsUsed - m_routeBitCount);
            unsigned int shift = freeBits;
            unsigned char highBits = 0;
            if (freeBits < 0)
            {
                const unsigned int fitBits = m_routeBitCount + freeBits;
                const unsigned int highShift = -shift;
                shift = static_cast<char>(freeBits + 8);
                highBits = static_cast<unsigned char>((buffer[offset] & ((1 << fitBits) - 1)) << highShift);
                ++offset;
                bitsUsed = 0;
            }
            const unsigned int lowBits = (8 - shift) - bitsUsed;
            bitsUsed = (8 - shift) & 0xff;
            m_routeTable[source * stationNum + destination] = highBits
                                                              + ((buffer[offset] >> shift) & ((1 << lowBits) - 1));
            if (shift == 0)
            {
                ++offset;
                bitsUsed = 0;
            }
        }
    }

    offset = routeBytes;
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        unsigned int directStationBitmap = 0;
        unsigned int shift = 24;
        for (unsigned int byteIndex = 0; byteIndex < 4; ++byteIndex)
        {
            directStationBitmap += buffer[offset] << shift;
            ++offset;
            shift -= 8;
        }
        m_directStationBitmaps[station] = directStationBitmap;
    }

    OSBlockMove(&m_unknown30, buffer + offset, 4, FALSE);
    UpdateJoiningStationBitmap();
    OSBlockMove(m_originalRouteTable, m_routeTable, stationNum * stationNum, FALSE);
    return 0;
}

unsigned int RelayRouteManager::GetRelayRouteDirectionsSize() const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    unsigned int routeBits = m_routeBitCount * stationNum * stationNum;
    if ((routeBits & 7) != 0)
        routeBits = (routeBits & ~7u) + 8;
    const unsigned int routeBytes = routeBits >> 3;
    return routeBytes + stationNum * 4 + 4;
}

Result RelayRouteManager::GetRelayRoute(::nn::pia::StationIndex source, ::nn::pia::StationIndex destination,
                                        ::nn::pia::StationIndex* relay) const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum || static_cast<unsigned int>(destination) >= stationNum)
        return cResultInvalidArgument;
    if (relay == 0)
        return cResultInvalidArgument;
    *relay = static_cast< ::nn::pia::StationIndex>(m_routeTable[source * stationNum + destination]);
    return 0;
}

Result RelayRouteManager::SetRelayRoute(::nn::pia::StationIndex source, ::nn::pia::StationIndex destination,
                                        ::nn::pia::StationIndex relay)
{
    if (static_cast<unsigned int>(source) >= g_piaTransport->m_stationNum
        || static_cast<unsigned int>(destination) >= g_piaTransport->m_stationNum)
        return cResultInvalidArgument;
    m_routeTable[source * g_piaTransport->m_stationNum + destination] = static_cast<unsigned char>(relay);
    return 0;
}

Result RelayRouteManager::GetDestStationList(::nn::pia::StationIndex source, ::nn::pia::StationIndex relay,
                                             unsigned int* destinationBitmap) const
{
    unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum || static_cast<unsigned int>(relay) >= stationNum)
        return cResultInvalidArgument;
    if (destinationBitmap == 0)
        return cResultInvalidArgument;

    unsigned int bitmap = 0;
    unsigned int stationFlag = 1;
    *destinationBitmap = 0;
    for (unsigned int destination = 0; destination < stationNum; ++destination)
    {
        if (m_routeTable[source * stationNum + destination] == relay)
        {
            bitmap |= stationFlag;
            *destinationBitmap = bitmap;
        }
        stationFlag <<= 1;
    }
    return 0;
}

Result RelayRouteManager::GetDirectStationList(::nn::pia::StationIndex source, unsigned int* directBitmap) const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum)
        return cResultInvalidArgument;
    if (directBitmap == 0)
        return cResultInvalidArgument;

    unsigned int bitmap = 0;
    unsigned int stationFlag = 1;
    *directBitmap = 0;
    for (unsigned int destination = 0; destination < stationNum; ++destination)
    {
        if (m_routeTable[source * stationNum + destination] == destination)
        {
            bitmap |= stationFlag;
            *directBitmap = bitmap;
        }
        stationFlag <<= 1;
    }
    return 0;
}

void RelayRouteManager::SetRttMax(unsigned short rttMax)
{
    m_rttMax = rttMax;
    g_rttMax = rttMax;
}

void RelayRouteManager::SetRelayNumMax(unsigned short relayNumMax)
{
    m_relayNumMax = relayNumMax & 0xfffe;
    g_relayNumMax = relayNumMax & 0xfffe;
}

void RelayRouteManager::UpdateRelayTable(unsigned int stationBitmap)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int station = 0; station < stationNum; ++station)
    {
        if ((m_joiningStationBitmap & (1 << station)) == 0 && (stationBitmap & (1 << station)) != 0)
        {
            for (unsigned int other = 0; other < stationNum; ++other)
            {
                if (((m_joiningStationBitmap | stationBitmap) & (1 << other)) != 0)
                {
                    m_routeTable[station * stationNum + other] = static_cast<unsigned char>(other);
                    m_routeTable[other * stationNum + station] = static_cast<unsigned char>(station);
                }
            }
            m_joiningStationBitmap |= 1 << station;
        }
    }

    for (unsigned int source = 0; source < stationNum; ++source)
    {
        for (unsigned int destination = source; destination < stationNum; ++destination)
        {
            if (((m_joiningStationBitmap & (1 << source)) == 0 || (m_joiningStationBitmap & (1 << destination)) == 0)
                && (m_routeTable[source * stationNum + destination] != source
                    || m_routeTable[destination * stationNum + source] != destination))
            {
                m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(source);
                m_routeTable[destination * stationNum + source] = static_cast<unsigned char>(destination);
            }
        }
    }
}

Result RelayRouteManager::SearchRefugeRelayRoute(::nn::pia::StationIndex station)
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    CalcRelayNumTable();
    if (m_relayNumTable[station] == 0)
        return 0;

    bool found = true;
    for (unsigned int source = 0; source < stationNum; ++source)
    {
        if (source == station)
            continue;

        for (unsigned int destination = source + 1; destination < stationNum; ++destination)
        {
            if (destination == station || m_routeTable[source * stationNum + destination] != station)
                continue;

            unsigned int bestRelay = stationNum;
            unsigned int bestRelayNum = stationNum * stationNum;
            for (unsigned int relay = 0; relay < stationNum; ++relay)
            {
                if (relay != source && relay != destination && relay != station
                    && m_routeTable[source * stationNum + relay] == relay
                    && m_routeTable[relay * stationNum + destination] == destination
                    && m_relayNumTable[relay] < bestRelayNum)
                {
                    bestRelayNum = m_relayNumTable[relay];
                    bestRelay = relay;
                }
            }

            if (bestRelay != stationNum)
            {
                m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(bestRelay);
                m_routeTable[destination * stationNum + source] = static_cast<unsigned char>(bestRelay);
                m_relayNumTable[station] -= 2;
                m_relayNumTable[bestRelay] += 2;
            }
            else
            {
                found = false;
            }
        }
    }

    if (found)
        return 0;
    return 0x2c0a;
}

void RelayRouteManager::InitRelayRouteManagerData()
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    for (unsigned int source = 0; source < stationNum; ++source)
    {
        for (unsigned int destination = 0; destination < stationNum; ++destination)
            m_routeTable[source * stationNum + destination] = static_cast<unsigned char>(source);
    }
    m_joiningStationBitmap = 0;
    m_unknown20 = 0;
    m_unknown1c = 0;
    m_unknown30 = 0;
}

Result RelayRouteManager::GetOriginalRelayRoute(::nn::pia::StationIndex source, ::nn::pia::StationIndex destination,
                                                ::nn::pia::StationIndex* relay) const
{
    const unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum || static_cast<unsigned int>(destination) >= stationNum)
        return cResultInvalidArgument;
    if (relay == 0)
        return cResultInvalidArgument;
    *relay = static_cast< ::nn::pia::StationIndex>(m_originalRouteTable[source * stationNum + destination]);
    return 0;
}

Result RelayRouteManager::GetOriginalDirectStationList(::nn::pia::StationIndex source, unsigned int* directBitmap) const
{
    unsigned int stationNum = g_piaTransport->m_stationNum;
    if (static_cast<unsigned int>(source) >= stationNum)
        return cResultInvalidArgument;
    if (directBitmap == 0)
        return cResultInvalidArgument;

    unsigned int bitmap = 0;
    unsigned int stationFlag = 1;
    *directBitmap = 0;
    unsigned int index = source * stationNum;
    unsigned int destination = 0;
    while (stationNum != 0)
    {
        if (m_originalRouteTable[index] == destination)
        {
            bitmap |= stationFlag;
            *directBitmap = bitmap;
        }
        stationFlag <<= 1;
        ++index;
        ++destination;
        --stationNum;
    }
    return 0;
}

}
}
}
