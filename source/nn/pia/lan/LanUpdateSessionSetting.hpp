#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LAN_UPDATE_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LAN_UPDATE_SESSION_SETTING_HPP
#include "nn/pia/Common.hpp"
#include "nn/pia/session/UpdateSessionSetting.hpp"
#include "nn/pia/lan/LanCreateSessionSetting.hpp"
namespace nn
{
namespace pia
{
namespace lan
{
class LanUpdateSessionSetting : public session::UpdateSessionSetting
{
public:
    LanUpdateSessionSetting();
    virtual ~LanUpdateSessionSetting();
    unsigned short GetMinParticipantNum() const;
    unsigned short GetMaxParticipantNum() const;
    LanSessionType GetSessionType() const;
    unsigned int GetAttribute(unsigned int index) const;
    Result SetApplicationData(const void* data, unsigned int size);
    Result GetApplicationData(void* output, unsigned int size) const;
    unsigned int GetApplicationDataSize() const;
    void SetMinParticipantNum(unsigned short value);
    bool IsSetMinParticipantNum() const;
    void SetMaxParticipantNum(unsigned short value);
    bool IsSetMaxParticipantNum() const;
    void SetSessionType(LanSessionType value);
    bool IsSetSessionType() const;
    Result SetAttribute(unsigned int index, unsigned int value);
    bool IsSetAttribute(unsigned int index) const;
    void Copy(const LanUpdateSessionSetting& other);
    LanUpdateSessionSetting(const LanUpdateSessionSetting& other);
    LanUpdateSessionSetting& operator=(const LanUpdateSessionSetting& other);
    void Reset();
    void Trace(unsigned long long value) const;

private:
    unsigned short m_minParticipantNum;
    unsigned short m_maxParticipantNum;
    LanSessionType m_sessionType;
    unsigned int m_attributes[6];
    unsigned char m_applicationData[0x180];
    unsigned int m_applicationDataSize;
};
}
}
}
#endif
