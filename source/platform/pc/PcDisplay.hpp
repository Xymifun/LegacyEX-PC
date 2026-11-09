#ifndef PLATFORM_PC_PCDISPLAY_HPP
#define PLATFORM_PC_PCDISPLAY_HPP

#include "platform/pc/PcOs.hpp"

enum PcDisplayMode
{
    ePcDisplay_Window,
    ePcDisplay_Hidden,
    ePcDisplay_Headless
};

void PcSetDisplayMode(PcDisplayMode mode);
PcDisplayMode PcGetDisplayMode();
void PcRequestScreenshot(const char* path);
void PcSetRenderHeight(int height);
void PcSetWindowSize(int width, int height);
void PcSetLetterbox(bool letterbox);
void PcSetWideWorldPass(bool wide);
void PcSetWideUiPass(bool wide);
void PcSetCoverUiPass(bool cover);
float PcGetUiWidthScale();
void PcClearOutsideUiArea();
float PcGetWorldAspect();
bool PcBeginExternalTarget(unsigned int framebuffer, int width, int height);
void PcEndExternalTarget();
void PcCopyToMainTarget(unsigned int framebuffer, int width, int height);
void PcScaleTarget(unsigned int source, int sourceWidth, int sourceHeight, unsigned int destination,
                   int destinationWidth, int destinationHeight);
void PcResyncRendererState();
void PcNeutralGlState();
void PcDrawPanel(unsigned int framebuffer, int width, int height, unsigned int texture, const float* matrix,
                 bool opaque, const float* source, const float* cuts, int cutCount);
void PcClearMainTarget(float red, float green, float blue, float alpha);
void PcClearDepth(unsigned int framebuffer);
void PcDrawBoxes(unsigned int framebuffer, int width, int height, const float* matrices, const float* colours,
                 int count);
void PcCopyMainTargetTo(unsigned int framebuffer);
void PcSetWindowUnderlay(unsigned int framebuffer, int width, int height);
void PcBindMainTargetAfterCallback();
void PcMakeFrustumMatrix(float left, float right, float bottom, float top, float nearDistance, float farDistance,
                         float* matrix);
void PcSetStartFullscreen(bool fullscreen);
void PcGetRenderSize(int* width, int* height);
const char* PcGetGlRendererName();
bool PcDisplayPollEvent(PcOsWindowEvent* event);

#endif
