#ifndef PLATFORM_PC_PCGLIDE_HPP
#define PLATFORM_PC_PCGLIDE_HPP

const unsigned long long kPcGlideHoldMilliseconds = 400;
const int kPcCheckpointStartKeepTime = -2;

bool PcGlidePractice(int pad);
void PcGlideSetPractice(int pad, bool practice);
void PcGlideClearPractice();

#endif
