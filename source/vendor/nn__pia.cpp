#include "nn__pia.hpp"

#include "nn/middleware.h"

namespace nn
{
namespace pia
{

namespace
{
unsigned int g_nexNetworkErrorCode = 0;
unsigned int g_nexNetworkErrorCodeSecondary = 0;
}

void Assert(const char*, const char*, unsigned int, const char*, ...)
{
}


void EmbedMiddlewareInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_5_9_0]");
}

void Result::SetNexNetworkErrorCode(unsigned int code)
{
    g_nexNetworkErrorCode = code;
}

void Result::ClearNexNetworkErrorCode()
{
    g_nexNetworkErrorCode = 0;
}

int Result::GetErrorCode() const
{
    if (m_code == 0)
        return 0;
    if (m_code == 0xe437)
    {
        if (g_nexNetworkErrorCode != 0)
            return g_nexNetworkErrorCode;
    }
    else if (m_code == 0xe46c)
    {
        if (g_nexNetworkErrorCodeSecondary != 0)
            return g_nexNetworkErrorCodeSecondary;
    }
    else if (m_code != 0xa46e)
    {
        if (m_code == 0x10c01)
            return 0x120161;
        if (m_code == 0x10c02)
            return 0x120162;
        if (m_code == 0x10c04)
            return 0x120163;
        if (m_code == 0x2c03)
            return 0x120164;
        if (m_code == 0x6c05)
            return 0x120165;
        if (m_code == 0xc406)
            return 0x120166;
        if (m_code == 0x10c07)
            return 0x120167;
        if (m_code == 0x10c08)
            return 0x120168;
        if (m_code == 0x2c09)
            return 0x120169;
        if (m_code == 0x2c0a)
            return 0x12016a;
        if (m_code == 0xc0b)
            return 0x12016b;
        if (m_code == 0x10c0c)
            return 0x12016c;
        if (m_code == 0x4c0d)
            return 0x12016d;
        if (m_code == 0x6c0e)
            return 0x12016e;
        if (m_code == 0x10c0f)
            return 0x12016f;
        if (m_code == 0xc10)
            return 0x120170;
        if (m_code == 0x4c11)
            return 0x120171;
        if (m_code == 0x10c13)
            return 0x120173;
        if (m_code == 0x10c14)
            return 0x1201c5;
        if (m_code == 0x6434)
            return 0x120229;
        if (m_code == 0xc35)
            return 0x12022a;
        if (m_code == 0x6436)
            return 0x12022b;
        if (m_code == 0xc16)
            return 0x12028e;
        if (m_code == 0x2c18)
            return 0x120290;
        if (m_code == 0x6c19)
            return 0x120291;
        if (m_code == 0x6c1b)
            return 0x120294;
        if (m_code == 0x6c1c)
            return 0x120295;
        if (m_code == 0x6c1d)
            return 0x120296;
        if (m_code == 0x641e)
            return 0x120297;
        if (m_code == 0x641f)
            return 0x120298;
        if (m_code == 0xc420)
            return 0x120299;
        if (m_code == 0xc421)
            return 0x12029a;
        if (m_code == 0x6422)
            return 0x12029b;
        if (m_code == 0xc23)
            return 0x1202f1;
        if (m_code == 0xc24)
            return 0x1202f2;
        if (m_code == 0x10c25)
            return 0x1202f3;
        if (m_code == 0x2c26)
            return 0x1202f4;
        if (m_code == 0xc27)
            return 0x1202f5;
        if (m_code == 0x6838)
            return 0x120355;
        if (m_code == 0x6439)
            return 0x120356;
        if (m_code == 0x6c3d)
            return 0x12035a;
        if (m_code == 0x10c3e)
            return 0x12035b;
        if (m_code == 0x643f)
            return 0x12035e;
        if (m_code == 0x6442)
            return 0x120361;
        if (m_code == 0x6445)
            return 0x120362;
        if (m_code == 0x6443)
            return 0x120363;
        if (m_code == 0x6444)
            return 0x120364;
        if (m_code == 0x6846)
            return 0x120365;
        if (m_code == 0x6847)
            return 0x120366;
        if (m_code == 0x6848)
            return 0x120367;
        if (m_code == 0x6849)
            return 0x120368;
        if (m_code == 0x644a)
            return 0x120369;
        if (m_code == 0x6c4b)
            return 0x12037d;
        if (m_code == 0x6c4c)
            return 0x12037e;
        if (m_code == 0x6c4d)
            return 0x12037f;
        if (m_code == 0x6c4e)
            return 0x120380;
        if (m_code == 0x6c51)
            return 0x120381;
        if (m_code == 0x6c52)
            return 0x120382;
        if (m_code == 0x6c50)
            return 0x120383;
        if (m_code == 0x6c53)
            return 0x120384;
        if (m_code == 0x6c54)
            return 0x120385;
        if (m_code == 0x6c55)
            return 0x120386;
        if (m_code == 0x8c56)
            return 0x120387;
        if (m_code == 0x6c57)
            return 0x120388;
        if (m_code == 0x6c59)
            return 0x12038a;
        if (m_code == 0x6c5b)
            return 0x12038b;
        if (m_code == 0xa44f)
            return 0x120391;
        if (m_code == 0xac5a)
            return 0x120392;
        if (m_code == 0x6c5f)
            return 0x12039b;
        if (m_code == 0x6c60)
            return 0x12039c;
        if (m_code == 0x6c61)
            return 0x12039d;
        if (m_code == 0x6c62)
            return 0x12039e;
        if (m_code == 0x6c63)
            return 0x12039f;
        if (m_code == 0x6c66)
            return 0x1203a0;
        if (m_code == 0x2c64)
            return 0x1203a1;
        if (m_code == 0x6c65)
            return 0x1203a2;
        if (m_code == 0xa471)
            return 0x1203a3;
        if (m_code == 0x6467)
            return 0x1203a7;
        if (m_code == 0x646a)
            return 0x1203a8;
        if (m_code == 0x6468)
            return 0x1203a9;
        if (m_code == 0x6469)
            return 0x1203aa;
        if (m_code == 0xa472)
            return 0x1203ae;
        if (m_code == 0xa473)
            return 0x1203af;
        if (m_code == 0xa474)
            return 0x1203b0;
        if (m_code == 0xa475)
            return 0x1203b1;
        if (m_code == 0x4c2c)
            return 0x1203ba;
        if (m_code == 0x10c32)
            return 0x1203be;
        if (m_code == 0x646f)
            return 0x12041d;
        if (m_code == 0x6470)
            return 0x12041e;
        if (m_code == 0xec6b)
            return 0x120549;
        if (m_code == 0xac6d)
            return 0x12054b;
    }
    return 0x120160;
}

}
}
