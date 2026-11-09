#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_SEQUENCE_ID_CONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_SEQUENCE_ID_CONTROLLER_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class TransportAnalyzer;

class SequenceIdController
{
public:
    SequenceIdController();
    virtual ~SequenceIdController();
    Result Startup();
    unsigned short GetNextSendSequenceId();
    bool CheckReceivedSequenceId(unsigned short sequenceId);
    void StartProfile();
    void StopProfile();
    virtual void Trace(unsigned long long value) const;
    void Cleanup();
    unsigned int CountReceivedToAllPacket();

private:
    friend class TransportAnalyzer;

    unsigned short m_nextSequence;
    unsigned short m_lastReceivedSequence;
    unsigned short m_profileBase;
    unsigned short m_profileLost;
    unsigned short m_profileBroadcast;
    bool m_profileEnabled;
    unsigned int m_sequenceCount;
    unsigned int m_missingSequenceCount;
};

}
}
}

#endif
