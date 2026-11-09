#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_API_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_API_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

struct DebugSetting
{
    enum
    {
        cNatTraversalResultNotEmulated = 0x10c13
    };

    DebugSetting() : natTraversalResult(cNatTraversalResultNotEmulated)
    {
        options[0] = 0;
        options[1] = 0;
        options[2] = 0;
        options[3] = 0;
    }

    unsigned char options[4];
    Result natTraversalResult;
};

Result Initialize();
void Finalize();
Result BeginSetup();
Result EndSetup();
bool IsBehindNat(const transport::StationLocation& location);
bool IsPublic(const transport::StationLocation& location);
unsigned char IsGlobal(const transport::StationLocation& location);
bool IsEdmMapping(const transport::StationLocation& location);
bool IsEimMapping(const transport::StationLocation& location);
unsigned char IsInitialized();
unsigned char IsInSetupMode();
DebugSetting* GetDebugSetting();
void EmbedModuleInfo();

}
}
}

#endif
