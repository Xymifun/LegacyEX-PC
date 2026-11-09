#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ENDPOINTUNIQUEID_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ENDPOINTUNIQUEID_HPP

#include "nn/nex/InetAddress.hpp"

namespace nn
{
namespace nex
{

class EndPointUniqueID
{
public:
    EndPointUniqueID() : m_address(), m_uiStreamID(0) {}

    EndPointUniqueID(const InetAddress& rAddress, unsigned char uiStreamID) : m_address(), m_uiStreamID(uiStreamID)
    {
        m_address = rAddress;
    }

    bool operator<(const EndPointUniqueID& rOther) const
    {
        return m_address < rOther.m_address || (m_address == rOther.m_address && m_uiStreamID < rOther.m_uiStreamID);
    }

private:
    InetAddress m_address;
    unsigned char m_uiStreamID;
};

}
}

#endif
