#include "nn/pia/lan/LanFacade.hpp"

#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/lan/LanSupport.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultAlreadyCreated = 0x10c0f,
    cResultOutOfMemory = 0x10c01
};
}

LanFacade* g_lanFacade = 0;

unsigned char* g_lanFacadeStorage = 0;

LanFacade::LanFacade() : m_lanProtocol(0), m_networkSetting()
{
}

LanFacade::~LanFacade()
{
    m_lanProtocol = 0;
}

Result LanFacade::Initialize()
{
    return 0;
}

Result LanFacade::Startup(const common::CryptoSetting& cryptoSetting)
{
    if (m_lanProtocol == 0)
        return cResultInvalidState;
    m_lanProtocol->RegisterCryptoSetting(cryptoSetting);
    return 0;
}

Result LanFacade::Bind(const LanNetworkSetting& setting)
{
    if (setting.m_field00 == 0)
        return cResultInvalidArgument;

    m_networkSetting = setting;
    if (m_lanProtocol != 0)
        return m_lanProtocol->Bind(setting);
    return 0;
}

void LanFacade::Unbind()
{
    if (m_lanProtocol == 0)
        return;
    m_lanProtocol->Unbind();
}

void LanFacade::Cleanup()
{
    if (m_lanProtocol == 0)
        return;
    m_lanProtocol->UnregisterCryptoSetting();
}

void LanFacade::RegisterLanProtocol(LanProtocol* protocol)
{
    m_lanProtocol = protocol;
    Result result = m_lanProtocol->Initialize();
    result = m_lanProtocol->Bind(m_networkSetting);
}

void LanFacade::UnregisterLanProtocol()
{
    if (m_lanProtocol != 0)
        m_lanProtocol->Finalize();
    m_lanProtocol = 0;
}

LanProtocol* LanFacade::GetLanProtocol()
{
    return m_lanProtocol;
}

Result LanFacade::StartLanSession()
{
    return m_lanProtocol->StartLanSession();
}

void LanFacade::StopLanSession()
{
    m_lanProtocol->StopLanSession();
}

unsigned int LanFacade::RequestBrowseMessage(unsigned char type)
{
    return m_lanProtocol->RequestBrowseMessage(type);
}

bool LanFacade::IsBrowseMessageCompleted() const
{
    return m_lanProtocol->IsBrowseMessageCompleted();
}

unsigned char LanFacade::GetApplicationCommunicationVersion() const
{
    return m_lanProtocol->m_unknown24;
}

unsigned long long LanFacade::GetLocalPrincipalId() const
{
    return m_lanProtocol->GetLocalPrincipalId();
}

transport::StationConnectionInfo& LanFacade::GetLocalStationConnectionInfo()
{
    return m_lanProtocol->GetLocalStationConnectionInfo();
}

Result LanFacade::CreateInstance()
{
    if (IsInitialized() == 0)
        return cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return cResultInvalidState;
    if (g_lanFacade != 0)
        return cResultAlreadyCreated;

    g_lanFacadeStorage = common::NewArray<unsigned char>(sizeof(LanFacade), 8);
    if (g_lanFacadeStorage == 0)
        return cResultOutOfMemory;

    g_lanFacade = new (g_lanFacadeStorage) LanFacade();
    g_lanFacade->Initialize();
    return 0;
}

void LanFacade::DestroyInstance()
{
    if (g_lanFacade != 0)
    {
        g_lanFacade->~LanFacade();
        g_lanFacade = 0;
    }
    if (g_lanFacadeStorage != 0)
    {
        common::DeleteArray(g_lanFacadeStorage);
        g_lanFacadeStorage = 0;
    }
}

LanFacade* LanFacade::GetInstance()
{
    return g_lanFacade;
}

}
}
}
