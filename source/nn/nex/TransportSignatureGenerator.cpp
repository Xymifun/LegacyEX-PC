#include "nn/nex/TransportSignatureGenerator.hpp"

#include <string.h>

#include "nn/nex/BitStream.hpp"
#include "nn/nex/Buffer.hpp"
#include "nn/nex/HMACChecksum.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int ADDRESS_LENGTH = 4;
const unsigned int PORT_LENGTH = 2;
const unsigned int TIME_LENGTH = 8;

}

TransportSignatureGenerator::TransportSignatureGenerator()
{
    m_checksum = new HMACChecksum;
    Buffer buffer;
    BitStream stream(&buffer);
    uint64_t uiTime = Time::GetTime();
    stream.Append(reinterpret_cast<const unsigned char*>(&uiTime), TIME_LENGTH, 1);
    Buffer checksum;
    m_checksum->ComputeChecksum(buffer, &checksum);
    Key key(checksum.GetData(), checksum.GetLength());
    m_checksum->SetKey(key);
}

TransportSignatureGenerator::~TransportSignatureGenerator()
{
}

unsigned int TransportSignatureGenerator::ComputeSourceSignature(unsigned int uiAddress, unsigned short uiPortNumber)
{
    unsigned char aData[ADDRESS_LENGTH + PORT_LENGTH];
    unsigned int uiNetworkAddress = ((uiAddress & 0xFF) << 24) | (((uiAddress >> 8) & 0xFF) << 16)
                                    | (((uiAddress >> 16) & 0xFF) << 8) | ((uiAddress >> 24) & 0xFF);
    unsigned short uiNetworkPortNumber = static_cast<unsigned short>(((uiPortNumber & 0xFF) << 8)
                                                                     + ((uiPortNumber & 0xFF00) >> 8));
    memcpy(aData, &uiNetworkAddress, ADDRESS_LENGTH);
    memcpy(aData + ADDRESS_LENGTH, &uiNetworkPortNumber, PORT_LENGTH);
    return m_checksum->ComputeChecksumForTransport(aData, ADDRESS_LENGTH + PORT_LENGTH);
}

}
}
