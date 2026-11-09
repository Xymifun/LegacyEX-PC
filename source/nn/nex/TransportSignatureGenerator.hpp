#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTSIGNATUREGENERATOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTSIGNATUREGENERATOR_HPP

#include "nn/nex/Holder.hpp"
#include "nn/nex/KeyedChecksumAlgorithm.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class TransportSignatureGenerator : public RootObject
{
public:
    TransportSignatureGenerator();
    ~TransportSignatureGenerator();

    unsigned int ComputeSourceSignature(unsigned int uiAddress, unsigned short uiPortNumber);

private:
    unsigned int m_unknown0;
    Holder<KeyedChecksumAlgorithm> m_checksum;
};

}
}

#endif
