#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORENEWARRIVALCHECKTOKEN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORENEWARRIVALCHECKTOKEN_HPP

#include <stddef.h>
#include <stdint.h>

#include "nn/nex/MD5Checksum.hpp"

namespace nn
{
namespace nex
{

class DataStoreNewArrivalCheckToken
{
public:
    enum
    {
        NOTIFICATION_URL_SIZE = 0x140
    };

    static bool Validate(const DataStoreNewArrivalCheckToken& rToken);

    void UpdateChecksum()
    {
        MD5Checksum checksum;
        m_uiChecksum = checksum.ComputeChecksumForTransport(reinterpret_cast<const unsigned char*>(this),
                                                            offsetof(DataStoreNewArrivalCheckToken, m_uiChecksum));
    }

    unsigned int m_uiReserved0;
    unsigned int m_uiFailureCount;
    unsigned int m_uiSequence;
    unsigned int m_uiReserved1;
    uint64_t m_uiLastNotificationId;
    char m_szNotificationUrl[NOTIFICATION_URL_SIZE];
    unsigned int m_uiChecksum;
    unsigned int m_uiReserved2;
};

}
}

#endif
