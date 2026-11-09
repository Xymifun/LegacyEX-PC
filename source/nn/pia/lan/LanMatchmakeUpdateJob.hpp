#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMAKEUPDATEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMAKEUPDATEJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/lan/LanSessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanProtocol;

class LanMatchmakeUpdateJob : public common::StepSequenceJob
{
public:
    enum
    {
        cBrowsePort = 30000,
        cBrowseKeyParamSize = 0x10,
        cBrowseTokenSize = 0x100,
        cMessageBufferSize = 0x554
    };

    explicit LanMatchmakeUpdateJob(LanProtocol* protocol);
    virtual ~LanMatchmakeUpdateJob();
    virtual void Trace(unsigned long long value) const;
    virtual Result Startup(unsigned short* port);
    virtual void Cleanup();

    void NotifyDisconnectToMeshLayerController();
    Result SendBrowseReplyMessage(const common::InetAddress& address, unsigned char* keyParam,
                                  unsigned int keyParamSize, unsigned char* token, unsigned int tokenSize,
                                  bool isAuthenticated, bool omitSessionParam);
    Result SendBrowseMessage(unsigned char index);
    ExecuteResult Update();
    bool RequestBrowseMessage(unsigned char count);
    bool IsBrowseMessageCompleted() const;
    void RequestGenerateRandomSeed() { m_randomSeedPending = true; }

private:
    LanProtocol* m_protocol;
    common::Socket m_socket;
    unsigned char m_receiveBuffer[cMessageBufferSize];
    unsigned char m_sendBuffer[cMessageBufferSize];
    LanSessionSearchCriteria m_searchCriteria;
    unsigned char m_browseRequestCount;
    bool m_browseRequested;
    common::CallContext m_browseContext;
    common::Time m_browseDeadline;
    bool m_randomSeedPending;
    unsigned long long m_browseSequence;
    unsigned char m_browseKeyParam[cBrowseKeyParamSize];
    unsigned char m_browseToken[cBrowseTokenSize];
};

}
}
}

#endif
