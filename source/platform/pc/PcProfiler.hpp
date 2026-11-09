#ifndef PLATFORM_PC_PCPROFILER_HPP
#define PLATFORM_PC_PCPROFILER_HPP

void PcProfilerEnable(double slowFrameMilliseconds, bool syncGpu);
bool PcProfilerEnabled();
bool PcProfilerSyncGpu();
void PcProfilerBegin(const char* name);
void PcProfilerEnd();
void PcProfilerFrameEnd();

#endif
