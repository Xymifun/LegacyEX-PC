#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATPROPERTIES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATPROPERTIES_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class NATProperties : public RootObject
{
public:
    enum MappingProperties
    {
        MAPPING_UNKNOWN = 0,
        MAPPING_ENDPOINT_INDEPENDENT = 1,
        MAPPING_ENDPOINT_DEPENDENT = 2
    };

    enum FilteringProperties
    {
        FILTERING_UNKNOWN = 0,
        FILTERING_ENDPOINT_INDEPENDENT = 1,
        FILTERING_ENDPOINT_DEPENDENT = 2
    };

    enum PortIncreaseProperties
    {
    };

    struct AllProperties
    {
        MappingProperties m_mapping;
        FilteringProperties m_filtering;
        PortIncreaseProperties m_portIncrease;
    };

    NATProperties();
    virtual ~NATProperties();

    void SetInterfaceAddress(const String& rAddress);
    String GetInterfaceAddress() const;
    void SetInterfacePort(unsigned short uiPort);
    unsigned short GetInterfacePort() const;

    void SetPublicAddress(const String& rAddress);
    String GetPublicAddress() const;
    void SetPublicPort(unsigned short uiPort);
    unsigned short GetPublicPort() const;

    void SetNATMapping(MappingProperties mapping);
    MappingProperties GetNATMapping() const;
    void SetNATFiltering(FilteringProperties filtering);
    FilteringProperties GetNATFiltering() const;

    void SetNATMappingPortIncrement(unsigned int uiIncrement);
    unsigned int GetNATMappingPortIncrement() const;
    PortIncreaseProperties GetPortIncreaseProperties() const;

    AllProperties GetAllProperties();

    static String ConvertMappingPropertiesToStr(MappingProperties mapping);
    static String ConvertFilteringPropertiesToStr(FilteringProperties filtering);
    static String ConvertPortIncreasePropertiesToStr(PortIncreaseProperties portIncrease);
    static unsigned int ConvertAllPropertiesToInteger(const AllProperties& rProperties);
    static AllProperties ConvertIntegerToAllProperties(unsigned int uiValue);

private:
    friend class JobPerformNATCheck;
    friend class NATTraversalEngine;
    friend class JobCheckNATProperties;

    PortIncreaseProperties m_portIncrease;
    String m_interfaceAddress;
    unsigned short m_uiInterfacePort;
    String m_publicAddress;
    unsigned short m_uiPublicPort;
    MappingProperties m_mapping;
    FilteringProperties m_filtering;
    unsigned int m_uiMappingPortIncrement;
    bool m_bMappingPortIncrementValid;
};

}
}

#endif
