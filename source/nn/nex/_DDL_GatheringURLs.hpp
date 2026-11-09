#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GATHERINGURLS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GATHERINGURLS_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_GatheringURLs : public RootObject
{
public:
    _DDL_GatheringURLs() : m_uiStructureVersion(0), m_StationURLs() {}

    virtual ~_DDL_GatheringURLs() {}

    _DDL_GatheringURLs& operator=(const _DDL_GatheringURLs& rSource);

    static void Add(Message* pMessage, const _DDL_GatheringURLs& rSource);
    static void Extract(Message* pMessage, _DDL_GatheringURLs* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiGatheringID;
    qList<StationURL> m_StationURLs;
};

}
}

#endif
