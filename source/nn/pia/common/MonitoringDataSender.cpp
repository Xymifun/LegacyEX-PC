#include "nn/pia/common/MonitoringDataSender.hpp"

namespace nn
{
namespace pia
{
namespace common
{

void MonitoringDataSender::RequestSendReport(DataType dataType)
{
    RequestSendReportImpl(dataType);
}

}
}
}
