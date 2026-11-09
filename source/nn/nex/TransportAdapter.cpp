#include "nn/nex/TransportAdapter.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/InterfaceInfo.hpp"
#include "nn/nex/InterfaceTable.hpp"
#include "nn/nex/StationContactInfo.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qProtectedList.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int RESULT_SUCCESS = 0x10001;
const unsigned long long INTERFACE_TRACE_FILTER = 0x400000;

}

TransportAdapter g_defaultTransportAdapter;
TransportAdapter* g_transportAdapter = 0;

TransportAdapter::TransportAdapter()
{
    m_pSocketDriver = 0;
    m_transportMode = TRANSPORT_MODE_0;
}

TransportAdapter::~TransportAdapter()
{
}

void TransportAdapter::SetTransportMode(ETransportMode mode)
{
    m_transportMode = mode;
}

bool TransportAdapter::GetLocalURLs(qList<StationURL>* pURLs)
{
    InterfaceTable table;
    table.Query();
    if (table.GetNbInterface() != 0)
    {
        for (unsigned int i = 0; i < table.GetNbInterface(); ++i)
        {
            if ((table[i].GetFlags() & InterfaceInfo::FLAG_LOOPBACK) == 0)
            {
                StationURL url(L"prudp:/");
                InetAddress address;
                address.SetNetworkAddress(table[i].GetAddress());
                url.SetInetAddress(&address);
                pURLs->push_back(url);
            }
        }
        if (pURLs->empty())
        {
            StationURL url(L"prudp:/");
            InetAddress address;
            address.SetNetworkAddress(table[0].GetAddress());
            url.SetInetAddress(&address);
            pURLs->push_back(url);
        }
        SetNetworkInterfaceInfo(&table[0]);
        return true;
    }
    return false;
}

void TransportAdapter::ResolveAddress(CallContext* pCallContext, StationURL*)
{
    pCallContext->InitiateCall();
    const int code = RESULT_SUCCESS;
    pCallContext->SignalSuccess(qResult(code));
}

void TransportAdapter::ReleaseAddress(const StationURL&)
{
}

void TransportAdapter::SortAndFilterTargetURLs(const qProtectedList<StationURL>& rTargetURLs, qList<StationURL>* pURLs)
{
    qList<StationURL> targetURLs = rTargetURLs.GetList();
    StationContactInfo targetInfo(targetURLs);
    StationContactInfo contactInfo(*pURLs);
    targetInfo.SortAndFilterTarget(contactInfo);
    *pURLs = contactInfo.GetURLs();
}

bool TransportAdapter::IsExportableURL(const StationURL&)
{
    return true;
}

unsigned int TransportAdapter::GetUniqueMachineID(const StationURL& rStationURL) const
{
    InetAddress address;
    address.SetAddress(rStationURL.GetAddress());
    return address.GetAddress();
}

void TransportAdapter::Load()
{
}

void TransportAdapter::Unload()
{
}

void TransportAdapter::Trace(unsigned long long)
{
}

bool TransportAdapter::SetNetworkInterfaceInfo(InterfaceInfo* pInterfaceInfo)
{
    m_localAddress.SetNetworkAddress(pInterfaceInfo->GetAddress());
    m_broadcastAddress.SetNetworkAddress(pInterfaceInfo->GetBroadcastAddress());
    m_multicastAddress.SetAddress(L"0.0.0.1");
    m_multicastAddress.SetPortNumber(0);
    Trace(INTERFACE_TRACE_FILTER);
    return true;
}

bool TransportAdapter::SetLocalAddress(const String& rAddress)
{
    m_localAddress.SetAddress(rAddress);
    return true;
}

bool TransportAdapter::SetPortNumber(unsigned short uiPortNumber)
{
    m_localAddress.SetPortNumber(uiPortNumber);
    return true;
}

bool TransportAdapter::IsSendMultiAddress(const InetAddress* pAddress)
{
    return pAddress->GetPortNumber() == 0;
}

}
}
