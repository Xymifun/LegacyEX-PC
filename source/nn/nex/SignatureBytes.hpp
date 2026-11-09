#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SIGNATUREBYTES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SIGNATUREBYTES_HPP

namespace nn
{
namespace nex
{

class SignatureBytes
{
public:
    enum
    {
        MaxLength = 16
    };

    SignatureBytes() : m_uiLength(0) {}

    unsigned char m_bytes[MaxLength];
    unsigned int m_uiLength;
};

}
}

#endif
