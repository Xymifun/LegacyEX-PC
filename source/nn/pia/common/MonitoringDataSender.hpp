#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_DATA_SENDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_DATA_SENDER_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class MonitoringDataSender
{
public:
    enum DataType
    {
        cDataTypeSessionBegin = 0,
        cDataTypeSessionEnd = 1,
        cDataType2 = 2
    };

    virtual ~MonitoringDataSender() {}
    virtual Result Startup() = 0;
    virtual void Cleanup() = 0;
    virtual void SetSendSessionBeginReportRequested(bool requested) = 0;
    virtual unsigned char IsSendSessionBeginReportRequested() const = 0;
    virtual unsigned int IsEnableMonitoring() const = 0;
    virtual unsigned int IsPreparedForCleanup() const = 0;

    void RequestSendReport(DataType dataType);
    static Result GetMonitoringServerURL(char* buffer, int size);

protected:
    virtual void RequestSendReportImpl(DataType dataType) = 0;
    virtual void Trace(unsigned long long value) const = 0;
};

}
}
}

#endif
