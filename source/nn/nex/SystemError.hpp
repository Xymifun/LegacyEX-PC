#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMERROR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMERROR_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class SystemError : public RootObject
{
public:
    SystemError(wchar_t* file, unsigned int line);
    ~SystemError();

    void SetErrorInfo(unsigned int errorCode, unsigned int errorDetail);
    void Trace(unsigned long long traceMask);

    static void SignalError(wchar_t* file, unsigned int line, unsigned int errorCode, unsigned int errorDetail);
    static void GetErrorString(unsigned int errorCode, wchar_t* buffer, unsigned int bufferSize);
    static void ClearLast();
    static unsigned int GetLast();
    static unsigned int GetLastExt();
    static bool IsError();
    static bool IsError(unsigned int errorCode);
    static bool IsWarning(unsigned int errorCode);
    static bool IsInformational(unsigned int errorCode);
    static bool IsSuccess(unsigned int errorCode);
    static void TraceLast(unsigned long long traceMask);
    static void EraseErrorElements(unsigned int threadId);
    static void EraseErrorElements();
    static void EraseAllErrorElements();
};

}
}

#endif
