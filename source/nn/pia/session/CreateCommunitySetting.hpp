#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATE_COMMUNITY_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATE_COMMUNITY_SETTING_HPP


namespace nn
{
namespace pia
{
namespace session
{

class CreateCommunitySetting
{
public:
    CreateCommunitySetting();
    virtual ~CreateCommunitySetting();
    unsigned short GetMinParticipantNum() const;
    unsigned short GetMaxParticipantNum() const;
    void Reset();
    void SetMinParticipantNum(unsigned short participantNum);
    void SetMaxParticipantNum(unsigned short participantNum);

private:
    unsigned short m_minParticipantNum;
    unsigned short m_maxParticipantNum;
};

}
}
}

#endif
