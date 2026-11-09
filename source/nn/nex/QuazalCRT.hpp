#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QUAZALCRT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QUAZALCRT_HPP

extern "C"
{
int* QuazalCRTAlloc(int iSize);
void QuazalCRTFree(int iAddress);
int QuazalCRTCalloc(int iCount, int iSize);
}

#endif
