#include "nn/nex/StreamTable.hpp"

#include <cwchar>

#include "nn/nex/ConnectionOrientedStream.hpp"

namespace nn
{
namespace nex
{

StreamTable::StreamTable() : m_streams()
{
}

StreamTable::~StreamTable()
{
}

ConnectionOrientedStream* StreamTable::FindStream(const StationURL* pStationURL)
{
    for (StreamList::const_iterator it = m_streams.begin(); it != m_streams.end(); ++it)
    {
        if ((*it)->ResponsibleForURL(pStationURL))
            return *it;
    }
    return 0;
}

ConnectionOrientedStream* StreamTable::FindStream(wchar_t* pType)
{
    for (StreamList::const_iterator it = m_streams.begin(); it != m_streams.end(); ++it)
    {
        if (std::wcscmp((*it)->GetURLType(), pType) == 0)
            return *it;
    }
    return 0;
}

}
}
