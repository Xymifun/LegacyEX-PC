#ifndef SOURCE_VENDOR_VCT_QUALITY_HPP
#define SOURCE_VENDOR_VCT_QUALITY_HPP

#include "vendor/VCTAudio.hpp"

void VCTi_InitQualityAlertParam();
void VCTi_SetQualityAlertStatus(unsigned char status);
void VCTi_SetQualityAlertClearStartTick(long long tick);
bool VCT_GetQualityAlertStatus();
void VCT_EnableQualityAlert(bool enabled);
unsigned char VCTi_GetQualityAlertStatus();
long long VCTi_GetQualityAlertClearStartTick();
unsigned char VCT_IsEnableQualityAlert();
unsigned short VCT_GetQualityAlertSamplingPacketNum();
unsigned char VCT_GetQualityAlertThresholdRate();
unsigned int VCT_GetQualityAlertDuration();

#endif
