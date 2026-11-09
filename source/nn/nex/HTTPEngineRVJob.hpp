#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINERVJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINERVJOB_HPP

#include "nn/nex/PeriodicJob.hpp"

namespace nn
{
namespace nex
{

class HTTPEngineCurlBase;

class HTTPEngineRVJob : public PeriodicJob
{
public:
    explicit HTTPEngineRVJob(HTTPEngineCurlBase* pEngine);
    virtual ~HTTPEngineRVJob();

    virtual void Execute();

private:
    HTTPEngineCurlBase* m_pEngine;
};

}
}

#endif
