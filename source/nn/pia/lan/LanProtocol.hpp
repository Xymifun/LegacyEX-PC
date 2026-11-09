#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANPROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANPROTOCOL_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/CryptoSecurePseudoRandomGenerator.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/lan/LanFacade.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class Socket;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{
class StationConnectionInfo;
}
}
}

namespace nn
{
namespace pia
{
namespace lan
{

class LanInputStream;
class LanMatchmakeUpdateJob;
class LanOutputStream;

class LanPayload
{
public:
    LanPayload() {}
    LanPayload(const unsigned char* data, unsigned short size, unsigned int type)
        : m_data(data),
          m_size(size),
          m_type(type)
    {
    }

    const unsigned char* m_data;
    unsigned short m_size;
    unsigned int m_type;
};

class LanProtocol : public transport::Protocol
{
public:
    enum
    {
        cProtocolType = 0x44,
        cProtocolId = cProtocolType << 8
    };

    class ReceivedFragmentData
    {
    public:
        ReceivedFragmentData() : m_fragmentSize(800), m_source() { Clear(); }

        void Clear();
        void Receive(const void* data, unsigned int size, unsigned int id, unsigned short sequence, unsigned char index,
                     unsigned char count, const common::StationAddress& source);
        bool IsReceivedAll() const;

        unsigned int m_fragmentSize;
        unsigned char m_data[0x5dc];
        unsigned int m_id;
        unsigned short m_sequence;
        unsigned char m_receivedMask;
        unsigned char m_fragmentCount;
        common::StationAddress m_source;
    };

    LanProtocol();
    virtual ~LanProtocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result Startup();
    virtual void Cleanup();
    virtual Result Dispatch();
    virtual unsigned int IsProtocolFilteringEnabled() const;
    virtual Result Initialize();
    virtual void Finalize();

    void ResetSelfKeepAliveTime();
    bool IsCryptoEnabled() const;
    Result CalculateCryptionKeyForSession(const unsigned char* input, unsigned int inputSize, void* output,
                                          unsigned int outputSize);
    Result RegisterSessionKey();
    transport::StationConnectionInfo& GetLocalStationConnectionInfo();
    void SendMessage(LanPayload* payload, const common::StationAddress& address, bool isPriority, bool field602,
                     unsigned long long traceLevel);
    void SendHostMessage(const common::StationAddress& address);
    void ParseGetHostMessage(const LanPayload* payload, const common::StationAddress& source);
    unsigned long long GetLocalPrincipalId() const;
    void ParseHostMessage(const LanPayload* payload, const common::StationAddress& source);
    void SendSessionMessage(const common::StationAddress& address);
    void ParseGetSessionMessage(const LanPayload* payload, const common::StationAddress& source);
    void ParseSessionMessage(const LanPayload* payload, const common::StationAddress& source);
    void ParseSelfKeepAlive(const LanPayload* payload, const common::StationAddress& source);
    void ParseMessage(const LanPayload* payload, const common::StationAddress& source);
    void ReceiveMessages();
    void SendSelfKeepAliveMessage();
    Result Bind(const LanFacade::LanNetworkSetting& setting);
    void Unbind();
    void RegisterCryptoSetting(const common::CryptoSetting& setting);
    void UnregisterCryptoSetting();
    Result BindRandomPort();
    Result StartLanSession();
    void StopLanSession();
    unsigned int RequestBrowseMessage(unsigned char type);
    bool IsBrowseMessageCompleted() const;
    bool RequestUpdateHostMessage();
    void SendGetHostMessage();
    bool RequestGetHostMessage();
    bool IsGetHostMessageCompleted() const;
    bool RequestUpdateSessionMessage();
    bool IsUpdateSessionMessageCompleted() const;
    void SendGetSessionMessage();
    bool RequestGetSessionMessage();
    bool IsGetSessionMessageCompleted() const;
    Result GetSessionMessageResult() const;
    Result CalculateCryptionKeyForBrowse(const unsigned char* input, unsigned int inputSize, void* output,
                                         unsigned int outputSize);
    Result CalculateCryptionKeyForBrowseReply(const unsigned char* input, unsigned int inputSize, void* output,
                                              unsigned int outputSize);
    Result CalculateTokenHash(const unsigned char* input, unsigned int inputSize, void* output,
                              unsigned int outputSize);
    unsigned char IsCryptoSettingRegistered() const;
    void SetRandomSeed(unsigned long long first, unsigned long long second);
    void RequestRegisterSessionKey();

    unsigned int GetProtocolIdentifier() const { return m_address; }
    bool IsRelayEnabled() const { return m_unknown24 != 0; }

    friend class LanFacade;
    friend class LanInputStream;
    friend class LanMatchmakeUpdateJob;
    friend class LanOutputStream;

private:
    unsigned char m_started;
    unsigned int m_address;
    unsigned int m_broadcastAddress;
    unsigned short m_port;
    unsigned short m_filterPort;
    unsigned char m_unknown24;
    common::Socket* m_socket;
    common::Socket* m_broadcastSocket;
    unsigned char m_broadcastSocketBound;
    LanOutputStream* m_outputStream;
    LanInputStream* m_inputStream;
    LanMatchmakeUpdateJob* m_matchmakeUpdateJob;
    unsigned char m_messageBuffer[0x5d8];
    unsigned char m_unknown618;
    unsigned char m_sessionInfoBuffer[0x5d8];
    ReceivedFragmentData m_receivedSessionInfo;
    unsigned short m_sessionMessageId;
    common::CallContext m_getHostContext;
    common::Time m_getHostDeadline;
    common::CallContext m_getSessionContext;
    common::Time m_getSessionDeadline;
    common::CallContext m_updateSessionContext;
    common::Time m_selfKeepAliveTime;
    unsigned char m_randomSeedSet;
    common::CryptoSetting m_cryptoSetting;
    unsigned char m_sessionKeyRegistered;
    unsigned char m_cryptoSettingRegistered;
    unsigned char m_sessionKeyRequested;
    common::CryptoSecurePseudoRandomGenerator m_random;
};

}
}
}

#endif
