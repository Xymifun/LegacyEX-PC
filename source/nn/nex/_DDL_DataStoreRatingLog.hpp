#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGLOG_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORERATINGLOG_HPP

#include <stdint.h>

#include "nn/nex/DateTime.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreRatingLog : public RootObject
{
public:
    _DDL_DataStoreRatingLog() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreRatingLog() {}

    _DDL_DataStoreRatingLog& operator=(const _DDL_DataStoreRatingLog& rSource);

    bool operator==(const _DDL_DataStoreRatingLog& rSource) const
    {
        return m_bIsRated == rSource.m_bIsRated && m_uiPid == rSource.m_uiPid
               && m_iRatingValue == rSource.m_iRatingValue && m_LockExpirationTime == rSource.m_LockExpirationTime;
    }

    static void Add(Message* pMessage, const _DDL_DataStoreRatingLog& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreRatingLog* pDestination);

    uint8_t m_uiStructureVersion;
    bool m_bIsRated;
    uint32_t m_uiPid;
    int32_t m_iRatingValue;
    DateTime m_LockExpirationTime;
};

}
}

#endif
