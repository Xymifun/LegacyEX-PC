#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHLEAVESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHLEAVESESSIONJOB_HPP

#include "nn/pia/session/LeaveSessionJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchLeaveSessionJob : public session::LeaveSessionJob
{
public:
    LanMatchLeaveSessionJob();
    ~LanMatchLeaveSessionJob();
    bool IsNeedSkipLeaveCurrentMatchmakeSession();
    void Trace(unsigned long long value) const;
};

}
}
}

#endif
