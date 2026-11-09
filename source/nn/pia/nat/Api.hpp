#ifndef SOURCE_NN_PIA_NAT_API_HPP
#define SOURCE_NN_PIA_NAT_API_HPP

namespace nn
{
namespace pia
{
namespace nat
{

struct DebugSetting
{
    unsigned char options[4];
    unsigned int value;
};

const DebugSetting* GetDebugSetting();

enum
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultCancelled = 0x6c05,
    cResultProbeRequestTimedOut = 0x644a,
    cResultPortDetectionFailed = 0x6434
};

enum
{
    cTraceError = 1,
    cTraceBrief = 0x800,
    cTraceDetail = 0x1000,
    cTraceVerbose = 0x2000
};

enum
{
    cNatServerPort = 0x8237,
    cProbeMessageSize = 0x10,
    cDefaultSprayCount = 0x32,
    cPortDetectionProbeRequest = 0x65,
    cNatMonitoringEntryNum = 15
};

enum
{
    cInvalidIndex = 0xff,
    cPortMax = 0xffff,
    cCounterMax = 0xffff,
    cInvalidId = 0xffffffff
};

extern unsigned int g_natLocalConnectionId;
extern int g_natClearProbeDelayMsec;
extern int g_natProbeDurationMsec;
extern unsigned char g_natProbeSprayCount;
extern int g_natDispatchIntervalMsec;

}
}
}

#endif
