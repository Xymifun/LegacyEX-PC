#include "nn/pia/transport/DispatchJob.hpp"

#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

DispatchJob::ExecuteResult DispatchJob::ExecuteCore()
{
    g_piaTransport->dispatch();
    return MakeExecuteResult(cExecuteKindNextStep);
}

}
}
}
