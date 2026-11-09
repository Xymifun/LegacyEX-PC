#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_BEGIN_MONITORING_DATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_BEGIN_MONITORING_DATA_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/MonitoringDataHeader.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class SessionBeginMonitoringData : public MonitoringDataHeader
{
public:
    static unsigned int GetSerializedSize();
    void Initialize();
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    void SetSdkVersion(unsigned int version);
    void SetNexVersion(unsigned char major, unsigned char minor, unsigned char patch);
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Cleanup();
    void Trace(unsigned long long value) const;

    SessionBeginMonitoringContent m_content;
};

}
}
}

#endif
