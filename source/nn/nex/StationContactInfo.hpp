#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STATIONCONTACTINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STATIONCONTACTINFO_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class StationContactInfo : public RootObject
{
public:
    StationContactInfo();
    StationContactInfo(const qList<StationURL>& rURLs);
    virtual ~StationContactInfo();

    bool SetConnectionID(unsigned int uiConnectionID);
    unsigned int GetConnectionID() const;
    bool AddURL(const StationURL& rStationURL);
    void SortAndFilterTarget(StationContactInfo& rTarget) const;
    void Trace(unsigned long long uiFilter);

    const qList<StationURL>& GetURLs() const { return m_urls; }

private:
    unsigned int m_uiConnectionID;
    qList<StationURL> m_urls;
};

}
}

#endif
