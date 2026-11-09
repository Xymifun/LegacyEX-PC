#include "nn/nex/DataStoreNewArrivalCheckToken.hpp"

#include <string.h>

#include <string>

namespace nn
{
namespace nex
{

bool DataStoreNewArrivalCheckToken::Validate(const DataStoreNewArrivalCheckToken& rToken)
{
    MD5Checksum checksum;
    if (rToken.m_uiChecksum
        != checksum.ComputeChecksumForTransport(reinterpret_cast<const unsigned char*>(&rToken),
                                                offsetof(DataStoreNewArrivalCheckToken, m_uiChecksum)))
        return false;

    if (std::char_traits<char>::find(rToken.m_szNotificationUrl, NOTIFICATION_URL_SIZE, '\0') == 0)
        return false;

    if (strlen(rToken.m_szNotificationUrl) == 0)
        return false;

    return true;
}

}
}
