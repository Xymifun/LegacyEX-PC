#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_RVCONNECTIONDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_RVCONNECTIONDATA_HPP

#include <stdint.h>

#include "nn/nex/DateTime.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_RVConnectionData : public RootObject
{
public:
    _DDL_RVConnectionData() : m_uiStructureVersion(1) {}

    virtual ~_DDL_RVConnectionData() {}

    _DDL_RVConnectionData& operator=(const _DDL_RVConnectionData& rSource);
    bool operator==(const _DDL_RVConnectionData& rSource) const;

    static void Add(Message* pMessage, const _DDL_RVConnectionData& rSource);
    static void Extract(Message* pMessage, _DDL_RVConnectionData* pDestination);

    uint8_t m_uiStructureVersion;
    StationURL m_RegularProtocolsURL;
    qList<unsigned char> m_SpecialProtocols;
    StationURL m_SpecialProtocolsURL;
    DateTime m_Time;
};

}
}

#endif
