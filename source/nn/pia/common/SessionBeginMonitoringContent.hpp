#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_BEGIN_MONITORING_CONTENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SESSION_BEGIN_MONITORING_CONTENT_HPP

#include "nn/pia/common/MonitoringContentHeader.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class SessionBeginMonitoringContent : public MonitoringContentHeader
{
public:
    void Initialize();
    void SetSdkVersion(unsigned int version);
    void SetNexVersion(unsigned char major, unsigned char minor, unsigned char patch);
    void Cleanup();
    bool IsBroken() const;
    static unsigned int GetSerializedSize();
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Trace(unsigned long long value) const;

    unsigned int m_piaVersion;
    unsigned int m_sdkVersion;
    unsigned int m_nexVersion;
    unsigned char m_environment;
    unsigned char m_platform;
    unsigned char m_platformRevision;
    unsigned char m_country;
    unsigned char m_region;
    unsigned int m_metric9;
    unsigned int m_metric10;
    unsigned long long m_metric11;
    unsigned char m_metric12;
    unsigned int m_metric13;
    unsigned int m_metric14;
    unsigned char m_metric15;
    unsigned char m_metric16;
    unsigned int m_publicAddress;
    unsigned int m_localAddress;
    unsigned char m_natMapping;
    unsigned char m_natFiltering;
    unsigned char m_natPortDelta;
    unsigned char m_natTypeFlags;
    unsigned short m_metric23;
    unsigned short m_metric24;
    unsigned short m_metric25;
    unsigned int m_metric26;
    unsigned int m_principalIdHash;
    unsigned int m_metric28;
    unsigned int m_gameServerId;
    unsigned char m_threadMode;
    unsigned short m_metric31;
    unsigned int m_metric32;
    unsigned int m_metric33;
    unsigned int m_metric34;
    unsigned int m_metric35;
    unsigned int m_metric36;
    unsigned short m_metric37;
    unsigned int m_metric38;
    unsigned short m_metric39;
    unsigned int m_metric40;
    unsigned short m_metric41;
    unsigned short m_metric42;
    unsigned short m_metric43;
    unsigned int m_metric44;
    unsigned char m_metric45;
    unsigned char m_metric46;
    unsigned char m_metric47;
    unsigned int m_joinResult;
    unsigned int m_joinProcessingTime;
    unsigned char m_metric50;
    unsigned int m_metric51[15];
    unsigned char m_metric52;
    unsigned int m_metric53[8];
    unsigned short m_metric54;
    unsigned char m_metric55;
    unsigned char m_metric56;
    unsigned int m_metric57;
    unsigned short m_metric58;
    unsigned short m_metric59;
    unsigned short m_metric60;
    unsigned short m_metric61;
    unsigned short m_metric62;
    unsigned short m_metric63;
    unsigned short m_metric64;
    unsigned short m_metric65;
    unsigned char m_metric66;
    unsigned short m_metric67;
    unsigned short m_publicPort;
    unsigned short m_privatePort;
    unsigned char m_metric70;
    unsigned int m_metric71;
    unsigned char m_metric72;
    unsigned char m_metric73;
    unsigned char m_metric74;
    unsigned int m_metric75;
    unsigned int m_metric76;
    unsigned int m_metric77;
    unsigned char m_metric78;
    unsigned char m_metric79;
    unsigned int m_metric80;
    unsigned int m_metric81;
    unsigned int m_metric82[6];
    unsigned short m_metric83;
    unsigned int m_metric84;
    unsigned char m_metric85;
    unsigned char m_metric86;
    unsigned char m_metric87;
    unsigned char m_metric88;
    unsigned char m_metric89[6];
    unsigned int m_metric90[6];
    unsigned int m_metric91[6];
    unsigned char m_metric92;
    unsigned char m_metric93;
    unsigned char m_metric94;
    unsigned int m_metric95;
    unsigned char m_metric96;
    unsigned char m_metric97;
    unsigned char m_metric98;
    unsigned char m_metric99;
    unsigned char m_metric100[6];
    unsigned int m_metric101[6];
    unsigned int m_metric102[6];
    unsigned char m_metric103;
    unsigned char m_metric104;
    unsigned char m_metric105;
    unsigned char m_joinRetryCount;
    unsigned char m_metric107;
    unsigned char m_metric108;
    unsigned int m_metric109;
    unsigned int m_metric110;
    unsigned int m_metric111;
    unsigned char m_metric112;
    unsigned char m_metric113;
    unsigned int m_metric114;
    unsigned int m_metric115;
    unsigned int m_metric116;
    unsigned char m_metric117;
    unsigned char m_metric118;
    unsigned char m_metric119;
    unsigned char m_retryReasonACount;
    unsigned char m_connectionInfoRetryCount;
    unsigned char m_ownerChangeRetryCount;
    unsigned char m_sessionInfoRetryCount;
    unsigned char m_retryReasonBCount;
    unsigned int m_joinTargetIdHash;
    unsigned int m_metric126;
    unsigned char m_metric127;
    unsigned char m_metric128;
    unsigned char m_joinSessionState;
    unsigned char m_metric130;
    unsigned char m_joinMeshState;
    unsigned char m_metric132;
    unsigned short m_metric133;
    unsigned short m_metric134;
    unsigned int m_metric135;
    unsigned int m_metric136;
    unsigned short m_metric137;
    unsigned short m_metric138;
    unsigned int m_metric139[15];
    unsigned int m_metric140[15];
    unsigned int m_metric141[15];
    unsigned int m_metric142[15];
    unsigned int m_metric143[15];
    unsigned char m_metric144[15];
    unsigned char m_metric145[15];
    unsigned char m_metric146[15];
    unsigned char m_metric147[15];
    unsigned char m_metric148;
    unsigned char m_metric149;
    unsigned short m_metric150;
    unsigned short m_metric151;
    unsigned short m_metric152;
    unsigned short m_metric153;
    unsigned short m_metric154;
    unsigned char m_metric155[15];
    unsigned int m_metric156;
    unsigned short m_metric157;
    unsigned char m_metric158;
    unsigned char m_metric159;
    unsigned char m_metric160;
    unsigned char m_metric161[16];
    unsigned char m_metric162[16];
    unsigned char m_metric163[16];
    unsigned char m_metric164[16];
    unsigned char m_metric165[150];
    unsigned char m_metric166[150];
    unsigned char m_metric167[16];
    unsigned char m_metric168[16];
    unsigned char m_metric169[150];
    unsigned char m_metric170[16];
    unsigned char m_metric171[16];
    unsigned char m_metric172[16];
    unsigned char m_metric173[16];
    unsigned char m_metric174[16];
    unsigned char m_metric175[16];
    unsigned char m_metric176[16];
    unsigned char m_metric177[16];
    unsigned char m_metric178[16];
    unsigned char m_metric179[16];
    unsigned char m_metric180[16];
    unsigned int m_metric181;
    unsigned int m_metric182;
    unsigned char m_metric183[300];
    unsigned char m_metric184[8];
    unsigned int m_integrityMarker;
};

extern SessionBeginMonitoringContent g_sessionBeginMonitoringContent;

}
}
}

#endif
