#include "nn/pia/transport/SequenceIdController.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

namespace
{
static short signed16(unsigned short bits)
{
    if (bits <= 0x7fff)
        return static_cast<short>(bits);
    return static_cast<short>(static_cast<int>(bits) - 0x10000);
}
}

SequenceIdController::SequenceIdController()
{
}

SequenceIdController::~SequenceIdController()
{
}

Result SequenceIdController::Startup()
{
    m_missingSequenceCount = 0;
    m_lastReceivedSequence = 0;
    m_sequenceCount = 0;
    m_profileBroadcast = 0;
    m_profileEnabled = false;
    m_profileLost = 0;
    m_profileBase = 0;
    m_nextSequence = 0;
    return 0;
}

unsigned short SequenceIdController::GetNextSendSequenceId()
{
    m_nextSequence = static_cast<unsigned short>(m_nextSequence + 1u);
    if (m_nextSequence == 0)
        m_nextSequence = 1;
    return m_nextSequence;
}

bool SequenceIdController::CheckReceivedSequenceId(unsigned short sequenceId)
{
    if (sequenceId == 0)
    {
        if (m_profileEnabled)
        {
            if (m_profileBroadcast != 0xffff)
            {
                m_profileBroadcast = static_cast<unsigned short>(m_profileBroadcast + 1u);
                return true;
            }
            m_profileEnabled = false;
            m_profileBroadcast = 0xffff;
        }
        return true;
    }

    const unsigned short deltaBits = static_cast<unsigned short>(sequenceId - m_lastReceivedSequence);
    const short delta = signed16(deltaBits);
    int distance = static_cast<int>(delta);
    if (distance < 1)
        return false;

    const unsigned int wrapDistance = (0x10000u - static_cast<unsigned int>(m_lastReceivedSequence)) & 0xffffu;
    if (wrapDistance != 0 && static_cast<int>(wrapDistance) < distance)
        distance = static_cast<int>(signed16(static_cast<unsigned short>(delta - 1)));

    const unsigned int received = m_sequenceCount;
    if (m_profileEnabled)
    {
        const unsigned short base = m_profileBase;
        const unsigned int next = static_cast<unsigned int>(base) + static_cast<unsigned int>(distance);
        if (next < 0x10000u)
        {
            m_profileBase = static_cast<unsigned short>(next);
            m_profileLost = static_cast<unsigned short>(static_cast<unsigned int>(m_profileLost)
                                                        + static_cast<unsigned int>(distance) - 1u);
        }
        else
        {
            m_profileEnabled = false;
            m_profileBase = 0xffff;
            m_profileLost = static_cast<unsigned short>(static_cast<unsigned int>(m_profileLost)
                                                        - static_cast<unsigned int>(base) - 1u);
        }
    }

    if (received != 0xffffffffu)
    {
        const unsigned int next = received + static_cast<unsigned int>(distance);
        if (received <= next)
        {
            m_lastReceivedSequence = sequenceId;
            m_sequenceCount = next;
            m_missingSequenceCount += static_cast<unsigned int>(distance) - 1u;
            return true;
        }
        m_sequenceCount = 0xffffffffu;
        m_missingSequenceCount += ~received;
    }
    m_lastReceivedSequence = sequenceId;
    return true;
}

void SequenceIdController::Trace(unsigned long long value) const
{
    (void)value;
}

void SequenceIdController::Cleanup()
{
}

}
}
}
