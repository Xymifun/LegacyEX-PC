#include "nn/nex/NATProperties.hpp"

namespace nn
{
namespace nex
{

NATProperties::NATProperties()
    : m_interfaceAddress(L""),
      m_uiInterfacePort(0),
      m_publicAddress(L""),
      m_uiPublicPort(0),
      m_mapping(static_cast<MappingProperties>(0)),
      m_filtering(static_cast<FilteringProperties>(0)),
      m_uiMappingPortIncrement(0),
      m_bMappingPortIncrementValid(false)
{
}

NATProperties::~NATProperties()
{
}

void NATProperties::SetInterfaceAddress(const String& rAddress)
{
    m_interfaceAddress = rAddress;
}

void NATProperties::SetInterfacePort(unsigned short uiPort)
{
    m_uiInterfacePort = uiPort;
}

unsigned short NATProperties::GetInterfacePort() const
{
    return m_uiInterfacePort;
}

void NATProperties::SetPublicAddress(const String& rAddress)
{
    m_publicAddress = rAddress;
}

String NATProperties::GetPublicAddress() const
{
    return m_publicAddress;
}

void NATProperties::SetPublicPort(unsigned short uiPort)
{
    m_uiPublicPort = uiPort;
}

unsigned short NATProperties::GetPublicPort() const
{
    return m_uiPublicPort;
}

void NATProperties::SetNATMapping(MappingProperties mapping)
{
    m_mapping = mapping;
}

NATProperties::MappingProperties NATProperties::GetNATMapping() const
{
    return m_mapping;
}

void NATProperties::SetNATFiltering(FilteringProperties filtering)
{
    m_filtering = filtering;
}

NATProperties::FilteringProperties NATProperties::GetNATFiltering() const
{
    return m_filtering;
}

void NATProperties::SetNATMappingPortIncrement(unsigned int uiIncrement)
{
    m_uiMappingPortIncrement = uiIncrement;
}

unsigned int NATProperties::GetNATMappingPortIncrement() const
{
    return m_uiMappingPortIncrement;
}

}
}
