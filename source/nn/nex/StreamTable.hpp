#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STREAMTABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STREAMTABLE_HPP

#include <list>
#include <stdint.h>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class ConnectionOrientedStream;
class StationURL;

class StreamTable : public RootObject
{
public:
    typedef std::list<ConnectionOrientedStream*, MemAllocator<ConnectionOrientedStream*> > StreamList;

    StreamTable();
    virtual ~StreamTable();

    ConnectionOrientedStream* FindStream(const StationURL* pStationURL);
    ConnectionOrientedStream* FindStream(wchar_t* pType);

    StreamList& GetStreams() { return m_streams; }

private:
    uint32_t m_unresolved0;
    StreamList m_streams;
};

}
}

#endif
