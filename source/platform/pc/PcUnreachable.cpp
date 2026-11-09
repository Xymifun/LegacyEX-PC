#ifdef _WIN32

#include <stdio.h>
#include <stdlib.h>

#define PC_STRINGIFY_TEXT(text) #text
#define PC_STRINGIFY(text) PC_STRINGIFY_TEXT(text)
#define PC_LABEL_PREFIX PC_STRINGIFY(__USER_LABEL_PREFIX__)

namespace
{
void ReportUnreachable(const char* name)
{
    fprintf(stderr, "[pc] unreachable Wii U function called: %s\n", name);
    fflush(stderr);
    abort();
}
}

extern "C" void PcUnreachableNex0() asm(PC_LABEL_PREFIX "_ZN2nn3nex10RootObjectdlEPv");

extern "C" void PcUnreachableNex0()
{
    ReportUnreachable("nn::nex::RootObject::operator delete(void*)");
}

extern "C" void PcUnreachableNex1() asm(PC_LABEL_PREFIX "_ZN2nn3nex11SystemError11SignalErrorEPwjjj");

extern "C" void PcUnreachableNex1()
{
    ReportUnreachable("nn::nex::SystemError::SignalError");
}

extern "C" void PcUnreachableNex2() asm(PC_LABEL_PREFIX "_ZN2nn3nex12StringStreamC1Ev");

extern "C" void PcUnreachableNex2()
{
    ReportUnreachable("nn::nex::StringStream::StringStream()");
}

extern "C" void PcUnreachableNex3() asm(PC_LABEL_PREFIX "_ZN2nn3nex12StringStreamD1Ev");

extern "C" void PcUnreachableNex3()
{
    ReportUnreachable("nn::nex::StringStream::~StringStream()");
}

extern "C" void PcUnreachableNex4() asm(PC_LABEL_PREFIX "_ZN2nn3nex13MemoryManager4FreeEPv");

extern "C" void PcUnreachableNex4()
{
    ReportUnreachable("nn::nex::MemoryManager::Free");
}

extern "C" void PcUnreachableNex5() asm(PC_LABEL_PREFIX "_ZN2nn3nex13MemoryManager8AllocateEj");

extern "C" void PcUnreachableNex5()
{
    ReportUnreachable("nn::nex::MemoryManager::Allocate");
}

extern "C" void PcUnreachableNex6() asm(PC_LABEL_PREFIX "_ZN2nn3nex16RefCountedObject10ReleaseRefEv");

extern "C" void PcUnreachableNex6()
{
    ReportUnreachable("nn::nex::RefCountedObject::ReleaseRef");
}

extern "C" void PcUnreachableNex7() asm(PC_LABEL_PREFIX "_ZN2nn3nex18JobConnectEndPoint19ReportConnectResultENS0_7qResultE");

extern "C" void PcUnreachableNex7()
{
    ReportUnreachable("nn::nex::JobConnectEndPoint::ReportConnectResult");
}

extern "C" void PcUnreachableNex8() asm(PC_LABEL_PREFIX "_ZN2nn3nex18ThreadVariableRoot11GetThreadIDEv");

extern "C" void PcUnreachableNex8()
{
    ReportUnreachable("nn::nex::ThreadVariableRoot::GetThreadID");
}

extern "C" void PcUnreachableNex9() asm(PC_LABEL_PREFIX "_ZN2nn3nex3Job10SetToReadyEv");

extern "C" void PcUnreachableNex9()
{
    ReportUnreachable("nn::nex::Job::SetToReady");
}

extern "C" void PcUnreachableNex10() asm(PC_LABEL_PREFIX "_ZN2nn3nex9BadEvents6SignalENS1_3_IDE");

extern "C" void PcUnreachableNex10()
{
    ReportUnreachable("nn::nex::BadEvents::Signal");
}

extern "C" void PcUnreachableNex11() asm(PC_LABEL_PREFIX "_ZN2nn3nexlsERNS0_12StringStreamERKNS0_17NotificationEventE");

extern "C" void PcUnreachableNex11()
{
    ReportUnreachable("nn::nex::operator<<");
}

extern "C" unsigned char PcUnreachableNexInstanceTable[64] asm(PC_LABEL_PREFIX "_ZN2nn3nex15g_instanceTableE");
unsigned char PcUnreachableNexInstanceTable[64];

extern "C" unsigned char PcUnreachableNexUsingInstantiationContext[64] asm(PC_LABEL_PREFIX "_ZN2nn3nex27g_usingInstantiationContextE");
unsigned char PcUnreachableNexUsingInstantiationContext[64];

extern "C" unsigned char PcUnreachableNexInstantiationContextVariable[64] asm(PC_LABEL_PREFIX "_ZN2nn3nex30g_instantiationContextVariableE");
unsigned char PcUnreachableNexInstantiationContextVariable[64];

extern "C" unsigned char PcUnreachableNexBadEventsInstance[64] asm(PC_LABEL_PREFIX "_ZN2nn3nex9BadEvents11s_pInstanceE");
unsigned char PcUnreachableNexBadEventsInstance[64];

#endif
