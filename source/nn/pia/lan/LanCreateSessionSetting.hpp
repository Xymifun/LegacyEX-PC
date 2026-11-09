#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_CREATE_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_CREATE_SESSION_SETTING_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

enum LanSessionType
{
    cLanSessionTypeInvalid = 0
};

enum LanCreateSettingFlag
{
    cLanSettingGameMode = 1u,
    cLanSettingSessionType = 2u,
    cLanSettingApplicationData = 0x100u,
    cLanSettingOpenSession = 0x200u
};

static const unsigned int cLanAttributeCount = 6;
static const unsigned int cLanAttributeFlagShift = 2;
static const unsigned int cLanApplicationDataCapacity = 0x180;

class LanCreateSessionSetting : public session::CreateSessionSetting
{
public:
    LanCreateSessionSetting();
    virtual ~LanCreateSessionSetting();
    void SetGameMode(unsigned int value);
    unsigned int GetGameMode() const;
    bool IsSetGameMode() const;
    void SetSessionType(LanSessionType value);
    LanSessionType GetSessionType() const;
    bool IsSetSessionType() const;
    Result SetAttribute(unsigned int index, unsigned int value);
    unsigned int GetAttribute(unsigned int index) const;
    bool IsSetAttribute(unsigned int index) const;
    virtual Result SetApplicationData(const void* data, unsigned int size);
    virtual Result GetApplicationData(void* output, unsigned int size) const;
    virtual unsigned int GetApplicationDataSize() const;
    virtual bool IsSetApplicationData() const;
    void SetOpenSession(bool open);
    bool IsOpenSession() const;
    bool IsSetOpenSession() const;
    virtual void Reset();
    virtual void Trace(unsigned long long value) const;
    void Copy(const LanCreateSessionSetting& other);
    LanCreateSessionSetting& operator=(const LanCreateSessionSetting& other);

private:
    unsigned int gameMode;
    LanSessionType sessionType;
    unsigned int attributes[cLanAttributeCount];
    unsigned char applicationData[cLanApplicationDataCapacity];
    unsigned int applicationDataSize;
    bool openSession;
    unsigned int stateFlags;
};

}
}
}

#endif
