#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_ADPCM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_ADPCM_HPP

#include "vendor/VCTAudio.hpp"

namespace nn
{
namespace pia
{
namespace chat
{

class Adpcm
{
public:
    struct State
    {
        short sample;
        unsigned char index;
        unsigned char reserved;
    };

    static void Decode(const unsigned char* pData, short* pOut, unsigned int uiSize, VCTCodec codec);
    static void Encode(unsigned char* pOut, const short* pAudio, unsigned int uiSize, State* pState, VCTCodec codec);
};

}
}
}

#endif
