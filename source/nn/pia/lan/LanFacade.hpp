#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANFACADE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANFACADE_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/CryptoSetting.hpp"

namespace nn
{
namespace pia
{
namespace transport
{
class StationConnectionInfo;
}
}
}

namespace nn
{
namespace pia
{
namespace lan
{

class LanProtocol;

class LanFacade
{
public:
    class LanNetworkSetting
    {
    public:
        LanNetworkSetting() : m_field00(0), m_field04(cDefaultSubnetMask), m_field08(0) {}

        static const unsigned int cDefaultSubnetMask = 0xffffff00;

        unsigned int m_field00;
        unsigned int m_field04;
        unsigned char m_field08;
    };

    LanFacade();
    ~LanFacade();

    Result Initialize();
    void Finalize() {}
    Result Startup(const common::CryptoSetting& cryptoSetting);
    Result Bind(const LanNetworkSetting& setting);
    void Unbind();
    void Cleanup();
    void RegisterLanProtocol(LanProtocol* protocol);
    void UnregisterLanProtocol();
    LanProtocol* GetLanProtocol();
    const LanProtocol* GetLanProtocol() const { return m_lanProtocol; }
    Result StartLanSession();
    void StopLanSession();
    unsigned int RequestBrowseMessage(unsigned char type);
    bool IsBrowseMessageCompleted() const;
    unsigned char GetApplicationCommunicationVersion() const;
    unsigned long long GetLocalPrincipalId() const;
    transport::StationConnectionInfo& GetLocalStationConnectionInfo();
    void Trace(unsigned long long) const {}

    static Result CreateInstance();
    static void DestroyInstance();
    static LanFacade* GetInstance();

    LanProtocol* m_lanProtocol;
    LanNetworkSetting m_networkSetting;
};

extern LanFacade* g_lanFacade;

}
}
}

#endif
