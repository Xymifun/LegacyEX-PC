#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_STATION_ID_STATUS_TABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_STATION_ID_STATUS_TABLE_HPP

#include "nn/pia.h"
#include "nn/pia/common/PooledList.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class StationIdStatusTable
{
    friend class MeshEventListener;

public:
    enum PrepareStatus
    {
        cPrepareStatusNone = 0,
        cPrepareStatusNotPrepared = 1,
        cPrepareStatusPrepared = 2
    };

    enum AnswerStatus
    {
        cAnswerStatusNone = 0,
        cAnswerStatusTrue = 1,
        cAnswerStatusFalse = 2
    };

    enum PrepareCheck
    {
        cPrepareCheckComplete = 0,
        cPrepareCheckPending = 1,
        cPrepareCheckNotPrepared = 2
    };

private:
    enum StationIndexValue
    {
        cStationIndexInvalid = 0xfd
    };

    struct Element
    {
        PrepareStatus prepare;
        AnswerStatus answer;
        bool connect;
        bool eventNotified;
        unsigned long long stationId;
        unsigned int sessionId;
        bool host;
        StationIndex stationIndex;
        bool invitation;
        bool migration;
    };

    typedef common::PooledList<Element> ElementList;
    typedef ElementList::Node Entry;

public:
    StationIdStatusTable();
    virtual ~StationIdStatusTable();
    void Finalize();
    Result Initialize(unsigned short stationNum);
    void ClearAllElements();
    Result AddElement(unsigned long long stationId, unsigned int sessionId, PrepareStatus status, bool eventNotified);
    Result RemoveElement(unsigned long long stationId);
    Element* FindElement(unsigned long long stationId);
    void ClearAllPrepareStatus();
    void SetPrepareStatus(unsigned long long stationId, bool value);
    PrepareCheck CheckAllPrepareStatus(unsigned long long* stationIds, unsigned short count) const;
    PrepareStatus CheckPrepareStatus(unsigned long long stationId) const;
    void ClearAllConnectStatus();
    void SetConnect(unsigned long long stationId, bool value);
    bool CheckAllConnectStatus(unsigned long long* stationIds, unsigned short count) const;
    bool CheckConnectStatus(unsigned long long stationId) const;
    bool SetAnswer(unsigned long long stationId, bool value);
    AnswerStatus CheckAnswerStatus(unsigned long long stationId);
    void ClearAllAnswerStatus();
    bool SetSendInvitationStatus(unsigned long long stationId, bool value);
    bool CheckSendInvitationStatus(unsigned long long stationId) const;
    void ClearAllSendInvitationStatus();
    bool SetMigrationCompleteStatus(unsigned long long stationId, bool value);
    bool CheckMigrationCompleteStatus(unsigned long long stationId) const;
    void ClearAllMigrationCompleteStatus();
    void SetEventNotifiedStatus(unsigned long long stationId, bool value);
    bool CheckEventNotifiedStatus(unsigned long long stationId) const;
    void CallJoinEventNotYetNotified();
    void ClearDisconnectedStationId();
    bool DisconnectionEventListenerForJointSession(void* event) const;
    bool GetSessionId(unsigned long long stationId, unsigned int* sessionId) const;
    void SetSessionHost(unsigned long long stationId, bool value);
    bool GetSessionHost(unsigned long long stationId, bool* value) const;
    unsigned short GetStationIdNum(unsigned int sessionId) const;
    unsigned char GetSessionIdRank(unsigned int sessionId) const;
    unsigned char GetSessionIdArray(unsigned int* sessionIds) const;
    void SetStationIndex(unsigned long long stationId, StationIndex index);
    bool GetStationIndex(unsigned long long stationId, StationIndex* index) const;
    void Trace(unsigned long long value) const;
    unsigned int GetValidStationIdNum() const;
    void SetSessionId(unsigned long long stationId, unsigned int sessionId);
    bool CheckSessionHost(unsigned int sessionId) const;

private:
    static Entry* entryOf(common::ListNode* node) { return static_cast<Entry*>(node); }
    static const Entry* entryOf(const common::ListNode* node) { return static_cast<const Entry*>(node); }

    Entry* findMutable(unsigned long long stationId)
    {
        common::ListNode* head = m_elements.Sentinel();
        for (common::ListNode* node = head->next; node != head; node = node->next)
        {
            Entry* entry = entryOf(node);
            if (entry->value.stationId == stationId)
                return entry;
        }
        return 0;
    }

    const Entry* find(unsigned long long stationId) const
    {
        const common::ListNode* head = m_elements.Sentinel();
        for (const common::ListNode* node = head->next; node != head; node = node->next)
        {
            const Entry* entry = entryOf(node);
            if (entry->value.stationId == stationId)
                return entry;
        }
        return 0;
    }

    ElementList m_elements;
    unsigned long long* m_storage;
};

}
}
}

#endif
