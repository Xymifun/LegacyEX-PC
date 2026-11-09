#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_STATE_MONITORING_CONTENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_STATE_MONITORING_CONTENT_HPP

#include "nn/pia/common/MonitoringContentHeader.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class SessionStateMonitoringContent : public MonitoringContentHeader
{
public:
    void Initialize();
    void Cleanup();
    static unsigned int GetSerializedSize();
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    bool IsBroken() const;
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Trace(unsigned long long value) const;

    unsigned int m_schedulerDispatchCount;
    unsigned int m_metric1[19];
    unsigned int m_metric20[16];
    unsigned long long m_metric36[16];
    unsigned int m_metric52[16];
    unsigned int m_metric68[16];
    unsigned long long m_metric84[16];
    unsigned int m_metric100[15];
    unsigned char m_metric115[15];
    unsigned short m_metric130[15];
    unsigned short m_metric145[15];
    unsigned char m_sendThreadSleepMilliSecond;
    unsigned char m_receiveThreadSleepMilliSecond;
    unsigned short m_metric162;
    unsigned int m_metric163;
    unsigned short m_metric164;
    unsigned int m_metric165;
    unsigned int m_receiveThreadMetric;
    unsigned int m_metric167[5];
    unsigned int m_metric172[4];
    unsigned int m_metric176;
    unsigned int m_metric177;
    unsigned char m_metric178;
    unsigned int m_metric179;
    unsigned char m_metric180;
    unsigned char m_metric181;
    unsigned int m_metric182;
    unsigned int m_metric183;
    unsigned short m_metric184;
    unsigned short m_metric185;
    unsigned int m_metric186;
    unsigned char m_metric187[6];
    unsigned int m_metric193;
    unsigned char m_metric194;
    unsigned int m_metric195;
    unsigned char m_metric196;
    unsigned char m_metric197;
    unsigned int m_metric198;
    unsigned int m_metric199;
    unsigned char m_metric200[4];
    unsigned char m_metric201[5];
    unsigned int m_metric202;
    unsigned short m_metric203;
    unsigned char m_metric204;
    unsigned int m_metric205;
    unsigned int m_metric206;
    unsigned short m_metric207;
    unsigned char m_metric208;
    unsigned short m_metric209;
    unsigned short m_metric210;
    unsigned int m_metric211;
    unsigned short m_metric212;
    unsigned short m_metric213;
    unsigned char m_metric214;
    unsigned int m_metric215;
    unsigned int m_metric216;
    unsigned int m_metric217;
    unsigned int m_metric218;
    unsigned long long m_metric219;
    unsigned long long m_metric220;
    unsigned char m_metric221[4];
    unsigned int m_metric222;
    unsigned char m_metric223[3];
    unsigned int m_leaveProcessingTime;
    unsigned char m_metric225[3];
    unsigned char m_metric226[16];
    unsigned int m_marker;
};

extern SessionStateMonitoringContent g_sessionStateMonitoringContent;

}
}
}

#endif
