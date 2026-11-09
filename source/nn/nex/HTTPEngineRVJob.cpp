#include "nn/nex/HTTPEngineRVJob.hpp"

#include "nn/nex/DebugString.hpp"
#include "nn/nex/HTTPEngineCurlBase.hpp"

namespace nn
{
namespace nex
{

HTTPEngineRVJob::HTTPEngineRVJob(HTTPEngineCurlBase* pEngine)
    : PeriodicJob(JOB_TYPE_2, DebugString(L"")),
      m_pEngine(pEngine)
{
}

HTTPEngineRVJob::~HTTPEngineRVJob()
{
}

void HTTPEngineRVJob::Execute()
{
    m_pEngine->PerformTransfer();
}

}
}
