#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALRELAYCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALRELAYCLIENT_HPP

#include "nn/nex/DateTime.hpp"
#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/NATTraversalResult.hpp"
#include "nn/nex/ServiceClient.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class Credentials;
class NATTraversalRelayProtocol;
class ProtocolCallContext;

class NATTraversalRelayClient : public ServiceClient
{
public:
    NATTraversalRelayClient();
    virtual ~NATTraversalRelayClient();

    virtual bool IsConnected() const;
    virtual void ConnectionStateHasChanged();
    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials);
    virtual NATTraversalRelayProtocol* CreateNATTraversalRelayProtocol();

    void Init();

    void RegisterRelayInterface(NATRelayInterface* pRelayInterface)
    {
        m_pRelayInterface = pRelayInterface;
        pRelayInterface->RegisterRelayClient(this);
    }

    void UnregisterRelayInterface()
    {
        if (m_pRelayInterface != 0)
        {
            m_pRelayInterface->UnregisterRelayClient();
            m_pRelayInterface = 0;
        }
    }

    void CallRequestProbeInitiation(ProtocolCallContext* pCallContext, const qList<StationURL>& rTargetList);
    void CallRequestProbeInitiationExt(ProtocolCallContext* pCallContext, const qList<StationURL>& rTargetList,
                                       const StationURL& rStationToProbe);
    void CallReportNATTraversalResult(ProtocolCallContext* pCallContext, const unsigned int& rCID, const bool& rResult,
                                      const unsigned int& rRTT);
    void CallReportNATProperties(ProtocolCallContext* pCallContext, const unsigned int& rNATMapping,
                                 const unsigned int& rNATFiltering, const unsigned int& rRTT);
    bool CallGetRelaySignatureKey(ProtocolCallContext* pCallContext, int* pRelayMode, DateTime* pCurrentUTCTime,
                                  String* pRelayAddress, unsigned short* pRelayPort, int* pRelayAddressType,
                                  unsigned int* pGameServerID);
    void CallReportNATTraversalResultDetail(ProtocolCallContext* pCallContext, const unsigned int& rCID,
                                            const bool& rResult, const NATTraversalResult& rResultDetail,
                                            unsigned int& rRTT);

private:
    friend class NATTraversalRelayProtocolImpl;
    NATRelayInterface* m_pRelayInterface;
    NATTraversalRelayProtocol* m_pRelayProtocol;
};

}
}

#endif
