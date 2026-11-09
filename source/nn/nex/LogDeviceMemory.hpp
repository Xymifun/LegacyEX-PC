#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOGDEVICEMEMORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOGDEVICEMEMORY_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/LogDevice.hpp"

namespace nn
{
namespace nex
{

class LogDeviceMemory : public LogDevice
{
public:
    enum
    {
        MAX_LINE_LENGTH = 0x100
    };

    LogDeviceMemory(char* pBuffer, int iSize, const char* pEndMarker, char** ppEndPosition);
    virtual ~LogDeviceMemory();
    virtual void Output(const wchar_t* pText);

    void SetBuffer(char* pBuffer, int iSize, char** ppEndPosition);

private:
    char* m_pBuffer;
    int m_iSize;
    const char* m_pEndMarker;
    char** m_ppEndPosition;
    ForcedCriticalSection m_criticalSection;
    int m_iStartPosition;
    int m_iWritePosition;
    bool m_bWrapped;
    char m_line[MAX_LINE_LENGTH + 2];
};

}
}

#endif
