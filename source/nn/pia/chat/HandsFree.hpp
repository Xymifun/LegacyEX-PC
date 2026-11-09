#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_HANDSFREE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_HANDSFREE_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"
#include "vendor/TmcHandsFree.hpp"

namespace nn
{
namespace pia
{
namespace chat
{

class HandsFree
{
public:
    HandsFree();
    ~HandsFree();

    void Finalize();
    Result Initialize(TMC_HF_EC_MODE ecMode, int noiseSuppressorLevel, int agcLevel);
    Result Process(short* output, const short* inputLeft, const short* inputRight, const short* inputReference);

    void EnableEchoCanceller(bool enabled);
    bool IsEnableEchoCanceller() const;
    void SetEchoCancellerMode(TMC_HF_EC_MODE mode);
    void EnableNoiseSuppressor(bool enabled);
    bool IsEnableNoiseSuppressor() const;
    void SetNoiseSuppressorLevel(int level);
    void EnableAgc(bool enabled);
    bool IsEnableAgc() const;
    void SetAGCLevel(int level);
    void Trace(unsigned long long value) const;

private:
    struct TmcConfig
    {
        void* workspace;
        uint16_t flags;
        uint16_t neutral;
        int32_t echoCancellerMode;
        int16_t noiseSuppressorLevel;
        int16_t agcParameterA;
        int16_t agcParameterB;
        int32_t inputMode;
        int32_t outputMode;
        int32_t error;
    };

    void* m_handle;
    int32_t m_ecMode;
    int32_t m_noiseSuppressorLevel;
    int32_t m_agcLevel;
    unsigned int* m_workspace;
    TmcConfig m_config;
};

}
}
}

#endif
