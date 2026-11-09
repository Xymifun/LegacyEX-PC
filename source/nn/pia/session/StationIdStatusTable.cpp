#include "nn/pia/session/StationIdStatusTable.hpp"

#include "nn/pia/common/Memory.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
const unsigned short s_maxStationNum = 0x20;
const unsigned int s_maxSessionIdNum = 0x20;

enum StationIndexValue
{
    cStationIndexReserved = 0xfe
};

enum StationStateValue
{
    cConnectedStationState = 5
};

struct DisconnectedStation
{
    unsigned long long stationId;
    StationIndex stationIndex;
};
}

StationIdStatusTable::StationIdStatusTable() : m_storage(0)
{
}

StationIdStatusTable::~StationIdStatusTable()
{
    Finalize();
}

void StationIdStatusTable::Finalize()
{
    if (m_storage != 0)
    {
        if (m_elements.IsInitialized())
            m_elements.Clear();
        common::DeleteArray(m_storage);
        m_storage = 0;
    }
}

Result StationIdStatusTable::Initialize(unsigned short stationNum)
{
    if (stationNum == 0 || stationNum > s_maxStationNum)
        return static_cast<Result>(0x10c07);

    if (m_storage == 0)
    {
        m_storage = common::NewArray<unsigned long long>((sizeof(Entry) * stationNum + sizeof(unsigned long long) - 1)
                                                         / sizeof(unsigned long long));
        m_elements.Initialize(m_storage, stationNum);
    }
    return 0;
}

void StationIdStatusTable::ClearAllElements()
{
    m_elements.Clear();
}

Result StationIdStatusTable::AddElement(unsigned long long stationId, unsigned int sessionId, PrepareStatus status,
                                        bool eventNotified)
{
    Element* element = m_elements.Alloc();
    if (element == 0)
        return static_cast<Result>(0xc10);

    element->stationId = stationId;
    element->sessionId = sessionId;
    element->prepare = status;
    element->connect = true;
    element->eventNotified = eventNotified;
    return 0;
}

Result StationIdStatusTable::RemoveElement(unsigned long long stationId)
{
    common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            element->stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
            element->prepare = cPrepareStatusNone;
            element->stationId = 0;
            element->migration = false;
            element->invitation = false;
            element->host = false;
            element->connect = false;
            element->sessionId = 0;
            element->eventNotified = false;
            element->answer = cAnswerStatusNone;
            m_elements.Free(ElementList::NodeOf(element));
            return 0;
        }
    }
    return static_cast<Result>(0x2c0a);
}

StationIdStatusTable::Element* StationIdStatusTable::FindElement(unsigned long long stationId)
{
    for (common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
            return element;
    }
    return 0;
}

void StationIdStatusTable::ClearAllPrepareStatus()
{
    const common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->value.prepare = cPrepareStatusNone;
}

void StationIdStatusTable::SetPrepareStatus(unsigned long long stationId, bool value)
{
    common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            element->prepare = value ? cPrepareStatusPrepared : cPrepareStatusNotPrepared;
            return;
        }
    }
}

StationIdStatusTable::PrepareCheck StationIdStatusTable::CheckAllPrepareStatus(unsigned long long* stationIds,
                                                                               unsigned short count) const
{
    bool filtered = false;
    if (stationIds != 0 && count != 0)
        filtered = true;
    PrepareCheck result = cPrepareCheckComplete;
    const common::ListNode* head = m_elements.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (filtered)
        {
            int i = 0;
            for (; i < count; ++i)
            {
                if (entry->value.stationId == stationIds[i])
                    break;
            }
            if (i == count)
                continue;
        }
        if (entry->value.prepare == cPrepareStatusNotPrepared)
            return cPrepareCheckNotPrepared;
        if (entry->value.prepare == cPrepareStatusNone)
            result = cPrepareCheckPending;
    }
    return result;
}

StationIdStatusTable::PrepareStatus StationIdStatusTable::CheckPrepareStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->value.stationId == stationId)
            return entry->value.prepare;
    }
    return cPrepareStatusNone;
}

void StationIdStatusTable::ClearAllConnectStatus()
{
    const common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->value.connect = false;
}

void StationIdStatusTable::SetConnect(unsigned long long stationId, bool value)
{
    const common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
            element->connect = value;
    }
}

bool StationIdStatusTable::CheckAllConnectStatus(unsigned long long* stationIds, unsigned short count) const
{
    const common::ListNode* head = m_elements.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Entry* entry = entryOf(node);
        bool found = false;
        for (unsigned short i = 0; i < count; ++i)
        {
            if (entry->value.stationId == stationIds[i])
            {
                found = true;
                break;
            }
        }
        if (found && !entry->value.connect)
            return false;
    }
    return true;
}

bool StationIdStatusTable::CheckConnectStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->value.stationId == stationId)
            return entry->value.connect;
    }
    return false;
}

bool StationIdStatusTable::SetAnswer(unsigned long long stationId, bool value)
{
    for (common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            element->answer = value ? cAnswerStatusTrue : cAnswerStatusFalse;
            return true;
        }
    }
    return false;
}

StationIdStatusTable::AnswerStatus StationIdStatusTable::CheckAnswerStatus(unsigned long long stationId)
{
    for (common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        Entry* entry = entryOf(node);
        if (entry->value.stationId == stationId)
            return entry->value.answer;
    }
    return cAnswerStatusNone;
}

void StationIdStatusTable::ClearAllAnswerStatus()
{
    const common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->value.answer = cAnswerStatusNone;
}

bool StationIdStatusTable::SetSendInvitationStatus(unsigned long long stationId, bool value)
{
    for (common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            element->invitation = value;
            return true;
        }
    }
    return false;
}

bool StationIdStatusTable::CheckSendInvitationStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->value.stationId == stationId)
            return entry->value.invitation;
    }
    return false;
}

void StationIdStatusTable::ClearAllSendInvitationStatus()
{
    const common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->value.invitation = false;
}

bool StationIdStatusTable::SetMigrationCompleteStatus(unsigned long long stationId, bool value)
{
    for (common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            element->migration = value;
            return true;
        }
    }
    return false;
}

bool StationIdStatusTable::CheckMigrationCompleteStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->value.stationId == stationId)
            return entry->value.migration;
    }
    return false;
}

void StationIdStatusTable::ClearAllMigrationCompleteStatus()
{
    const common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
        entryOf(node)->value.migration = false;
}

void StationIdStatusTable::SetEventNotifiedStatus(unsigned long long stationId, bool value)
{
    const common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
            element->eventNotified = value;
    }
}

bool StationIdStatusTable::CheckEventNotifiedStatus(unsigned long long stationId) const
{
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        const Entry* entry = entryOf(node);
        if (entry->value.stationId == stationId)
            return entry->value.eventNotified;
    }
    return false;
}

void StationIdStatusTable::CallJoinEventNotYetNotified()
{
    Session* session = g_piaSession;
    const unsigned long long localStationId = session->m_localStationId;
    if (!CheckEventNotifiedStatus(localStationId))
    {
        session->validateStationId(localStationId);
        session->JoinNotice(localStationId);
        SetEventNotifiedStatus(localStationId, true);
    }

    const unsigned long long hostStationId = session->m_hostStationId;
    if (!CheckEventNotifiedStatus(hostStationId) && CheckConnectStatus(hostStationId))
    {
        session->validateStationId(hostStationId);
        session->JoinNotice(hostStationId);
        SetEventNotifiedStatus(hostStationId, true);
    }

    common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (!element->eventNotified)
        {
            session->validateStationId(element->stationId);
            session->JoinNotice(element->stationId);
            element->eventNotified = true;
        }
    }
}

void StationIdStatusTable::ClearDisconnectedStationId()
{
    Session* session = g_piaSession;
    unsigned long long disconnectedStationIds[s_maxStationNum];
    unsigned short disconnectedNum = 0;
    bool localDisconnected = false;
    const unsigned long long localStationId = session->m_localStationId;

    common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->value;
        const transport::Station* station = transport::StationManager::s_pInstance->GetStation(element->stationId);
        if (station != 0 && station->m_field20 == cConnectedStationState && element->connect)
            continue;

        if (element->stationId == localStationId)
            localDisconnected = true;
        else
            disconnectedStationIds[disconnectedNum++] = element->stationId;

        transport::StationIdTable::Entry* entry = transport::g_piaTransport->m_stationIdTable->FindByStationIdCore(
            element->stationId);
        if (entry != 0 && entry->stationIndex == cStationIndexInvalid)
            entry->stationIndex = element->stationIndex;
    }
    if (localDisconnected)
        disconnectedStationIds[disconnectedNum++] = localStationId;

    for (unsigned int i = 0; i < disconnectedNum; ++i)
        session->RemoveStationId(disconnectedStationIds[i], static_cast<StationIndex>(cStationIndexInvalid), 0, 0);
}

bool StationIdStatusTable::DisconnectionEventListenerForJointSession(void* event) const
{
    const DisconnectedStation* station = static_cast<const DisconnectedStation*>(event);
    if (station == 0 || station->stationIndex == static_cast<StationIndex>(cStationIndexReserved))
        return false;
    const Session* session = g_piaSession;
    if (session->m_fieldc0 == 0)
        return false;

    const Entry* entry = find(station->stationId);
    if (entry == 0)
        return false;

    const JointSessionJob* jointSessionJob = session->m_jointSessionJob;
    if (jointSessionJob == 0)
        return false;
    if ((jointSessionJob->m_processType == Session::cProcessTypeCreateJointSession
         || jointSessionJob->m_processType == Session::cProcessTypeJoinJointSession
         || jointSessionJob->m_processType == Session::cProcessTypeJoinRandomJointSession)
        && entry->value.prepare != cPrepareStatusPrepared)
        return false;
    return true;
}

bool StationIdStatusTable::GetSessionId(unsigned long long stationId, unsigned int* sessionId) const
{
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        const Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            *sessionId = element->sessionId;
            return true;
        }
    }
    return false;
}

void StationIdStatusTable::SetSessionHost(unsigned long long stationId, bool value)
{
    common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            element->host = value;
            return;
        }
    }
}

bool StationIdStatusTable::GetSessionHost(unsigned long long stationId, bool* value) const
{
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        const Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            *value = element->host;
            return true;
        }
    }
    return false;
}

unsigned short StationIdStatusTable::GetStationIdNum(unsigned int sessionId) const
{
    unsigned short num = 0;
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        if (entryOf(node)->value.sessionId == sessionId)
            ++num;
    }
    return num;
}

unsigned char StationIdStatusTable::GetSessionIdRank(unsigned int sessionId) const
{
    unsigned int sessionIds[s_maxSessionIdNum];
    unsigned char num = 0;
    const common::ListNode* head = m_elements.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Element* element = &entryOf(node)->value;
        bool found = false;
        for (int i = 0; i < num; ++i)
        {
            if (sessionIds[i] == entryOf(node)->value.sessionId)
            {
                found = true;
                break;
            }
        }
        if (!found)
            sessionIds[num++] = element->sessionId;
    }

    unsigned char rank = 1;
    for (unsigned char i = 0; i < num; ++i)
    {
        if (sessionIds[i] < sessionId)
            ++rank;
    }
    return rank;
}

unsigned char StationIdStatusTable::GetSessionIdArray(unsigned int* sessionIds) const
{
    unsigned char num = 0;
    const common::ListNode* head = m_elements.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Element* element = &entryOf(node)->value;
        bool found = false;
        for (unsigned char i = 0; i < num; ++i)
        {
            if (sessionIds[i] == entryOf(node)->value.sessionId)
            {
                found = true;
                break;
            }
        }
        if (!found)
            sessionIds[num++] = element->sessionId;
    }
    return num;
}

void StationIdStatusTable::SetStationIndex(unsigned long long stationId, StationIndex index)
{
    common::ListNode* head = m_elements.Sentinel();
    for (common::ListNode* node = head->next; node != head; node = node->next)
    {
        Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            element->stationIndex = index;
            return;
        }
    }
}

bool StationIdStatusTable::GetStationIndex(unsigned long long stationId, StationIndex* index) const
{
    for (const common::ListNode* node = m_elements.Sentinel()->next; node != m_elements.Sentinel(); node = node->next)
    {
        const Element* element = &entryOf(node)->value;
        if (element->stationId == stationId)
        {
            *index = element->stationIndex;
            return true;
        }
    }
    return false;
}

void StationIdStatusTable::Trace(unsigned long long) const
{
}

unsigned int StationIdStatusTable::GetValidStationIdNum() const
{
    unsigned int num = 0;
    const common::ListNode* head = m_elements.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
        ++num;
    return num;
}

void StationIdStatusTable::SetSessionId(unsigned long long stationId, unsigned int sessionId)
{
    Entry* entry = findMutable(stationId);
    if (entry != 0)
        entry->value.sessionId = sessionId;
}

bool StationIdStatusTable::CheckSessionHost(unsigned int sessionId) const
{
    const common::ListNode* head = m_elements.Sentinel();
    for (const common::ListNode* node = head->next; node != head; node = node->next)
    {
        const Element& element = entryOf(node)->value;
        if (element.sessionId == sessionId)
            return element.host;
    }
    return false;
}

}
}
}
