#include "nn/pia/chat/HandsFree.hpp"
#include <string.h>
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "platform/wiiu/pead.hpp"
#include "vendor/VCTAudio.hpp"

namespace
{

bool isValidEcMode(TMC_HF_EC_MODE mode)
{
    return mode == TMC_HF_EC_MODE_0 || mode == TMC_HF_EC_MODE_1 || mode == TMC_HF_EC_MODE_2;
}

bool isValidNsLevel(int level)
{
    return level >= 0 && level < 3;
}

bool isValidAgcLevel(int level)
{
    return level >= 0 && level < 5;
}

void updateFunctionalBits(unsigned int bits, bool enabled)
{
    if (enabled)
    {
        nn::pia::common::g_sessionStateMonitoringContent.m_metric215 &= ~bits;
    }
    else
    {
        nn::pia::common::g_sessionStateMonitoringContent.m_metric215 |= bits;
    }
}

}

namespace nn
{
namespace pia
{
namespace chat
{

namespace
{
const int kAlreadyInitializedResult = 0x10c02;
const int kInvalidParameterResult = 0x10c07;
const int kAllocationFailureResult = 0x10c01;
const int kNotInitializedResult = 0x10c0c;
const int kProcessingFailureResult = 0xc0b;
const unsigned int kWorkspaceSize = 0x251c0;
const int16_t noiseLevels[3] = {6, 10, 14};
const int16_t agcParameters[5][2] = {{0, 0}, {10, -14}, {14, -18}, {17, -21}, {19, -23}};

}

HandsFree::HandsFree() : m_handle(0), m_ecMode(1), m_noiseSuppressorLevel(1), m_agcLevel(1), m_workspace(0)
{
    memset(&m_config, 0, sizeof(m_config));
}

HandsFree::~HandsFree()
{
    Finalize();
}

void HandsFree::Finalize()
{
    if (m_handle != 0)
    {
        TMC_HF_Close(m_handle);
        m_handle = 0;
    }
    if (m_workspace != 0)
    {
        common::DeleteArray(m_workspace);
        m_workspace = 0;
    }
}

Result HandsFree::Initialize(TMC_HF_EC_MODE ecMode, int noiseSuppressorLevel, int agcLevel)
{
    if (m_handle != 0)
        return kAlreadyInitializedResult;
    if (!isValidEcMode(ecMode) || !isValidNsLevel(noiseSuppressorLevel) || !isValidAgcLevel(agcLevel))
        return kInvalidParameterResult;

    m_workspace = common::NewArray<unsigned int>(kWorkspaceSize / sizeof(unsigned int));
    if (m_workspace == 0)
        return kAllocationFailureResult;
    m_ecMode = static_cast<int32_t>(ecMode);
    m_noiseSuppressorLevel = noiseSuppressorLevel;
    m_agcLevel = agcLevel;
    m_config.workspace = m_workspace;
    m_config.flags = 7;
    m_config.echoCancellerMode = static_cast<int32_t>(ecMode);
    m_config.noiseSuppressorLevel = noiseLevels[noiseSuppressorLevel];
    m_config.agcParameterA = agcParameters[agcLevel][0];
    m_config.agcParameterB = agcParameters[agcLevel][1];
    m_config.inputMode = 0;
    m_config.outputMode = 0;
    m_handle = TMC_HF_Init(&m_config);
    if (m_config.error != 0)
        return kProcessingFailureResult;

    common::g_sessionStateMonitoringContent.m_metric221[0] = static_cast<unsigned char>(m_config.echoCancellerMode);
    common::g_sessionStateMonitoringContent.m_metric221[1] = static_cast<unsigned char>(m_config.noiseSuppressorLevel);
    common::g_sessionStateMonitoringContent.m_metric221[2] = static_cast<unsigned char>(m_config.agcParameterA);
    common::g_sessionStateMonitoringContent.m_metric221[3] = static_cast<unsigned char>(-m_config.agcParameterB);
    common::g_sessionStateMonitoringContent.m_metric215 = 0xffffffff;
    updateFunctionalBits(2, true);
    updateFunctionalBits(4, true);
    updateFunctionalBits(8, true);
    return 0;
}

Result HandsFree::Process(short* output, const short* inputLeft, const short* inputRight, const short* inputReference)
{
    if (m_handle == 0)
        return kNotInitializedResult;
    if (output == 0 || inputLeft == 0 || inputRight == 0 || inputReference == 0)
        return kInvalidParameterResult;

    uint32_t frames = VCT_GetAudioFrameLength() >> 3;
    uint32_t outputOffset = 0;
    uint32_t inputOffset = 0;
    while (frames-- != 0)
    {
        const uint32_t result = TMC_HF_Proc(m_handle, inputLeft + outputOffset, inputRight + inputOffset,
                                            inputReference + inputOffset, output + outputOffset);
        if (result != 0)
            return kProcessingFailureResult;
        inputOffset += 256;
        outputOffset += 64;
    }
    return 0;
}

}
}
}
