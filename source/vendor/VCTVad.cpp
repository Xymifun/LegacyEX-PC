#include "vendor/VCTAudio.hpp"
#include <cmath>

struct VADState
{
    unsigned int m_level;
    unsigned int m_releaseTime;
    unsigned int m_frameLength;
    bool m_active;
};

static unsigned int s_vad_54 = 0;
static unsigned int s_vad_58 = 0;
static unsigned int s_vad_5c = 0;
static unsigned int s_vadClampGain = 0x31;
static unsigned int s_vadEnableInvalidation = 0;
static unsigned short s_vadActiveGain = 0x15e;
static unsigned short s_vadInactiveGain = 0x118;
static VADState s_vad = {0, 5, 0x2d0, false};

static unsigned int s_vadBss[4];

static inline unsigned int VADAverage()
{
    unsigned int sum = 0;
    for (int i = 0; i < 4; ++i)
    {
        sum += s_vadBss[i];
    }
    return (unsigned int)std::sqrt((double)(sum >> 2));
}

unsigned int VCT_GetVADReleaseTime()
{
    return s_vad.m_releaseTime;
}

void VCT_ResetVAD()
{
    for (int i = 0; i < 4; ++i)
    {
        s_vadBss[i] = 0x1000000;
    }
    s_vad_5c = 0;
    s_vad.m_active = false;
    s_vad.m_level = 0;
    s_vad_54 = 0;
    s_vad_58 = 0;
}

bool VCT_GetVADActivity()
{
    return s_vad.m_active;
}

unsigned int VCTi_GetVADStatus(unsigned int energy, unsigned int threshold, unsigned char* pStatus)
{
    s_vad.m_level = threshold >> 8;
    if (pStatus != 0)
    {
        *pStatus = (unsigned char)(threshold >> 8);
    }
    if (energy < s_vadClampGain)
    {
        return 0;
    }
    unsigned int status;
    if (!s_vad.m_active)
    {
        unsigned int average = VADAverage();
        if (threshold != 0 && threshold >= average * s_vadActiveGain / 100)
        {
            average = VADAverage();
            s_vadBss[0] = energy;
            s_vad.m_active = true;
            s_vad_58 = 1;
            s_vad_5c = average * s_vadInactiveGain / 100;
            status = 1;
        }
        else
        {
            s_vadBss[s_vad_58] = energy;
            s_vad_58 = (s_vad_58 + 1) & 3;
            status = 0;
        }
    }
    else
    {
        s_vadBss[s_vad_58] = energy;
        s_vad_58 = (s_vad_58 + 1) & 3;
        unsigned int average = VADAverage();
        unsigned int count;
        if (average > s_vad_5c)
        {
            count = 0;
        }
        else
        {
            count = s_vad_54 + 1;
            if (count > s_vad.m_releaseTime)
            {
                s_vad_58 = 0;
                s_vad_54 = 0;
                s_vad.m_active = false;
                return 3;
            }
        }
        s_vad_54 = count;
        status = 2;
    }
    return status;
}

void vct_init_vad()
{
    s_vad.m_releaseTime = 5;
    s_vad.m_frameLength = VCTi_GetAudioFrameLength() * 5;
    VCT_ResetVAD();
}

unsigned short VCT_GetVADActiveGain()
{
    return s_vadActiveGain;
}

unsigned short VCT_GetVADInactiveGain()
{
    return s_vadInactiveGain;
}

unsigned int VCT_GetVADClampGain()
{
    return s_vadClampGain;
}

unsigned int VCT_IsEnableVADInvalidation()
{
    return s_vadEnableInvalidation;
}
