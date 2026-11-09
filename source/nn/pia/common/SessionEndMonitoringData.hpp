#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_END_MONITORING_DATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_END_MONITORING_DATA_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/MonitoringDataHeader.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class SessionEndMonitoringData : public MonitoringDataHeader
{
public:
    static unsigned int GetSerializedSize();
    void Initialize(unsigned char dataType);
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Cleanup();
    void Trace(unsigned long long value) const;

    SessionBeginMonitoringContent m_beginContent;
    SessionStateMonitoringContent m_stateContent;
};

}
}
}

#endif
