#include "vendor/VCTQuality.hpp"

namespace
{
long long g_qualityAlertClearStartTick = 0;
unsigned int g_qualityAlertDuration = 1800;
unsigned short g_qualityAlertSamplingPacketNum = 40;
unsigned char g_qualityAlertEnabled = 1;
unsigned char g_qualityAlertStatus = 0;
unsigned char g_qualityAlertThresholdRate = 20;
}

void VCTi_InitQualityAlertParam()
{
    g_qualityAlertDuration = 1800;
    g_qualityAlertStatus = 0;
    g_qualityAlertSamplingPacketNum = 40;
    g_qualityAlertThresholdRate = 20;
}

void VCTi_SetQualityAlertStatus(unsigned char status)
{
    g_qualityAlertStatus = status;
}

void VCTi_SetQualityAlertClearStartTick(long long tick)
{
    g_qualityAlertClearStartTick = tick;
}

bool VCT_GetQualityAlertStatus()
{
    return g_qualityAlertStatus != 0;
}

void VCT_EnableQualityAlert(bool enabled)
{
    g_qualityAlertEnabled = enabled ? 1 : 0;
}

unsigned char VCTi_GetQualityAlertStatus()
{
    return g_qualityAlertStatus;
}

long long VCTi_GetQualityAlertClearStartTick()
{
    return g_qualityAlertClearStartTick;
}

unsigned char VCT_IsEnableQualityAlert()
{
    return g_qualityAlertEnabled;
}

unsigned short VCT_GetQualityAlertSamplingPacketNum()
{
    return g_qualityAlertSamplingPacketNum;
}

unsigned char VCT_GetQualityAlertThresholdRate()
{
    return g_qualityAlertThresholdRate;
}

unsigned int VCT_GetQualityAlertDuration()
{
    return g_qualityAlertDuration;
}
