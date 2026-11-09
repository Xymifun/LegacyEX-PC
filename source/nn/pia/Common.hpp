#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_HPP

#include <cstdarg>
#include <stdint.h>

namespace nn
{
namespace pia
{
class Result
{
public:
    Result() {}
    Result(int code) : m_code(code) {}
    Result(const Result& other) : m_code(other.m_code) {}
    operator int() const { return m_code; }

    int GetErrorCode() const;
    static void SetNexNetworkErrorCode(unsigned int code);
    static void ClearNexNetworkErrorCode();

    int m_code;
};

class ResultAlreadyInitialized : public Result
{
public:
    ResultAlreadyInitialized() : Result(0x10c02) {}
};

class ResultInvalidArgument : public Result
{
public:
    ResultInvalidArgument() : Result(0x10c07) {}
};

class ResultInvalidState : public Result
{
public:
    ResultInvalidState() : Result(0x10c08) {}
};

class ResultNotInitialized : public Result
{
public:
    ResultNotInitialized() : Result(0x10c0c) {}
};

enum StationIndex
{
    cStationIndexNum = 32,
    cStationIndexInvalid = 0xfd,
    cStationIndexSentinelFE = 0xfe,
    cStationIndexSentinelFF = 0xff
};

namespace common
{

Result Initialize(void* workMemory, unsigned int workMemorySize);
Result BeginSetup();
Result EndSetup();
void Finalize();
unsigned int GetMemoryUsage();
void SetProgramInformation(unsigned int sdkVersion, unsigned char nexMajorVersion, unsigned char nexMinorVersion,
                           unsigned char nexMicroVersion);
void PrintDebugInfo();
void VPrintf(const char* format, va_list args);
void Printf(const char* format, ...);
unsigned char IsInitialized();
unsigned char IsInSetupMode();
void EmbedModuleInfo();

unsigned int hashWithMd5(unsigned int value);
unsigned int hashWithMd5(unsigned long long value);

void serializeU64(unsigned char* buffer, unsigned long long value);
void serializeU16(unsigned char* buffer, unsigned short value);
void serializeU32(unsigned char* buffer, unsigned int value);
unsigned short deserializeU16(const unsigned char* buffer);
void serializeU8(unsigned char* buffer, unsigned char value);
unsigned long long deserializeU64(const unsigned char* buffer);
unsigned char deserializeU8(const unsigned char* buffer);
unsigned int deserializeU32(const unsigned char* buffer);

bool isValidSourceStationIndex(StationIndex index);
bool isValidDestinationStationIndex(StationIndex index);

void dump(unsigned long long traceFlag, const void* data, unsigned int size);

}
}
}

#endif
