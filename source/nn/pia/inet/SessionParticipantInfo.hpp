#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_SESSION_PARTICIPANT_INFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_SESSION_PARTICIPANT_INFO_HPP

#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class SessionParticipantInfo
{
public:
    enum
    {
        cParticipantNum = 32
    };

    struct Participant
    {
        unsigned long long m_principalId;
        unsigned long long m_stationId;
        unsigned char m_participantType;
        short m_participationCount;
        short m_participationEvent;
        bool m_leaving;
        common::Time m_updateTime;
    };

    typedef common::PooledList<Participant> ParticipantList;

    SessionParticipantInfo();
    ~SessionParticipantInfo();
    void Cleanup();
    void RemoveObsoleteEntry();

    unsigned int m_sessionId;
    common::FixedPooledList<Participant, cParticipantNum> m_participantList;
};

}
}
}

#endif
