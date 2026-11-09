#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATESESSIONSETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATESESSIONSETTING_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class CreateSessionSetting
{
public:
    CreateSessionSetting();
    virtual ~CreateSessionSetting();
    virtual void SetMaxParticipantNum(uint16_t value);
    virtual Result SetApplicationData(const void* data, unsigned int size) = 0;
    virtual Result GetApplicationData(void* data, unsigned int size) const = 0;
    virtual unsigned int GetApplicationDataSize() const = 0;
    virtual bool IsSetApplicationData() const = 0;
    virtual void Reset();
    virtual void Trace(uint64_t value) const;

    uint16_t GetMinParticipantNum() const;
    uint8_t IsSetMinParticipantNum() const;
    uint16_t GetMaxParticipantNum() const;
    uint8_t IsSetMaxParticipantNum() const;
    void Copy(const CreateSessionSetting& other);

    void SetMinParticipantNum(uint16_t value);

private:
    uint16_t mMinParticipantNum;
    uint8_t mMinParticipantNumSet;
    uint8_t mReserved;
    uint16_t mMaxParticipantNum;
    uint8_t mMaxParticipantNumSet;
};

}
}
}

#endif
