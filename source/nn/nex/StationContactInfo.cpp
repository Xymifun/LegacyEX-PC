#include "nn/nex/StationContactInfo.hpp"

#include "nn/nex/String.hpp"
#include "nn/nex/TraceLog.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int STATION_URL_PUBLIC_TYPE_BIT = 1;
const unsigned int TRACE_INDENT = 2;

inline bool IsPublicURL(const StationURL& rStationURL)
{
    return ((rStationURL.GetType() >> STATION_URL_PUBLIC_TYPE_BIT) & 1) != 0;
}

}

StationContactInfo::StationContactInfo() : m_uiConnectionID(0)
{
}

StationContactInfo::StationContactInfo(const qList<StationURL>& rURLs)
{
    m_uiConnectionID = 0;
    for (qList<StationURL>::const_iterator url = rURLs.begin(); url != rURLs.end(); ++url)
    {
        AddURL(*url);
    }
}

StationContactInfo::~StationContactInfo()
{
    m_urls.clear();
}

bool StationContactInfo::SetConnectionID(unsigned int uiConnectionID)
{
    if (m_uiConnectionID == 0 || uiConnectionID == m_uiConnectionID)
    {
        m_uiConnectionID = uiConnectionID;
        return true;
    }
    return false;
}

unsigned int StationContactInfo::GetConnectionID() const
{
    return m_uiConnectionID;
}

bool StationContactInfo::AddURL(const StationURL& rStationURL)
{
    if (rStationURL.GetRVConnectionID() == 0)
    {
        m_urls.push_back(rStationURL);
        return true;
    }
    if (SetConnectionID(rStationURL.GetRVConnectionID()))
    {
        m_urls.push_back(rStationURL);
        return true;
    }
    return false;
}

void StationContactInfo::SortAndFilterTarget(StationContactInfo& rTarget) const
{
    bool bMatch = false;
    int nbPublic = 0;
    for (qList<StationURL>::iterator target = rTarget.m_urls.begin(); target != rTarget.m_urls.end() && !bMatch;
         ++target)
    {
        const StationURL& rTargetURL = *target;
        if (IsPublicURL(rTargetURL))
        {
            for (qList<StationURL>::const_iterator url = m_urls.begin(); url != m_urls.end() && !bMatch; ++url)
            {
                if (String::IsEqual(url->GetAddress(), rTargetURL.GetAddress()))
                    bMatch = true;
            }
            ++nbPublic;
        }
    }
    if (nbPublic == 0)
        return;
    qList<StationURL> sorted;
    if (bMatch)
    {
        for (qList<StationURL>::iterator target = rTarget.m_urls.begin(); target != rTarget.m_urls.end(); ++target)
        {
            const StationURL& rTargetURL = *target;
            if (!IsPublicURL(rTargetURL))
                sorted.push_back(rTargetURL);
        }
    }
    for (qList<StationURL>::iterator target = rTarget.m_urls.begin(); target != rTarget.m_urls.end(); ++target)
    {
        const StationURL& rTargetURL = *target;
        if (IsPublicURL(rTargetURL))
            sorted.push_back(rTargetURL);
    }
    rTarget.m_urls = sorted;
}

void StationContactInfo::Trace(unsigned long long uiFilter)
{
    TraceLog::GetInstance()->GetOutputFormat()->IncreaseIndent(TRACE_INDENT);
    for (qList<StationURL>::iterator url = m_urls.begin(); url != m_urls.end(); ++url)
    {
        url->Trace(uiFilter);
    }
    TraceLog::GetInstance()->GetOutputFormat()->DecreaseIndent(TRACE_INDENT);
}

}
}
