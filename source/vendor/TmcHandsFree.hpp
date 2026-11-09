#ifndef SOURCE_VENDOR_TMCHANDSFREE_HPP
#define SOURCE_VENDOR_TMCHANDSFREE_HPP

enum TMC_HF_EC_MODE
{
    TMC_HF_EC_MODE_0 = 0,
    TMC_HF_EC_MODE_1 = 1,
    TMC_HF_EC_MODE_2 = 2
};

extern "C" void* TMC_HF_Init(void* config);
extern "C" unsigned int TMC_HF_Proc(void* handle, const short* inputLeft, const short* inputRight,
                                    const short* inputReference, short* output);
extern "C" unsigned int TMC_HF_Close(void* handle);

#endif
