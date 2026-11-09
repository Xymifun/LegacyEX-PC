#include "platform/pc/PcGlide.hpp"

namespace
{
bool s_practice[4];
}

bool PcGlidePractice(int pad)
{
    return pad >= 0 && pad < 4 && s_practice[pad];
}

void PcGlideSetPractice(int pad, bool practice)
{
    if (pad >= 0 && pad < 4)
        s_practice[pad] = practice;
}

void PcGlideClearPractice()
{
    for (int pad = 0; pad < 4; ++pad)
        s_practice[pad] = false;
}
