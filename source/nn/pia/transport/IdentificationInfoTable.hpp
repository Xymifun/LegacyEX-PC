#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_IDENTIFICATION_INFO_TABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_IDENTIFICATION_INFO_TABLE_HPP

#include "nn/pia.h"
#include "nn/pia/Common.hpp"
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class IdentificationInfo
{
public:
    struct Field20
    {
        unsigned char m_bytes[64];
    };

    Station::IdentificationToken m_field00;
    Field20 m_field20;
    common::FixedString<20> m_field60[1];
    common::FixedString<10> m_fieldC8[1];
    unsigned char m_field108;
    unsigned char m_field109;
    unsigned char m_playerCount;
    unsigned char m_participantCount;
    unsigned int m_field10c;
    unsigned long long m_principalId[1];
};

class IdentificationInfoTable : public IdentificationInfo
{
public:
    IdentificationInfoTable();
    ~IdentificationInfoTable();
    static Result CreateInstance();
    static void DestroyInstance();
    static IdentificationInfoTable* GetInstance();
    void Finalize();
    Result Initialize(unsigned short count);
    void ClearIdentificationInfo(IdentificationInfo* info);
    void SetLocalIdentificationInfo(const IdentificationInfo* info);
    Result GetLocalIdentificationInfo(IdentificationInfo* info);
    void ClearLocalIdentificationInfo();
    Result AddToTable(const Station* station, const IdentificationInfo* info, unsigned char kind);
    IdentificationInfo* GetIdentificationInfoPtr(const Station* station);
    Result GetIdentificationInfo(const Station* station, IdentificationInfo* info);
    IdentificationInfo* GetIdentificationInfoPtr(unsigned long long principalId);
    Result GetIdentificationInfo(unsigned long long principalId, IdentificationInfo* info);
    Result GetPlayerCount(const Station* station, unsigned char* count);
    Result GetParticipantCount(const Station* station, unsigned char* count);
    void EraseFromTable(const Station* station);
    void ClearTable();
    void Trace(unsigned long long value) const;

private:
    struct Record
    {
        IdentificationInfo m_info;
        unsigned char m_state;
        const Station* m_station;
    };

    typedef common::FixedPooledList<Record, 32> RecordPool;

    RecordPool m_pool;
    unsigned short m_count;
    common::CriticalSection m_criticalSection;
};

}
}
}

#endif
