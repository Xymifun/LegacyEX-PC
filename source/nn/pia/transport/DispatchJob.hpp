#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_DISPATCH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_DISPATCH_JOB_HPP

#include "nn/pia/common/Job.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class DispatchJob : public common::Job
{
public:
    virtual ~DispatchJob() {}
    virtual ExecuteResult ExecuteCore();
};

}
}
}

#endif
