#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_AUTHENTICATIONINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_AUTHENTICATIONINFO_HPP

#include "nn/nex/_DDL_AuthenticationInfo.hpp"

namespace nn
{
namespace nex
{

class AuthenticationInfo : public _DDL_AuthenticationInfo
{
public:
    AuthenticationInfo() : _DDL_AuthenticationInfo()
    {
        m_Name = L"";
        m_uiValue1 = 3;
        m_uiValue3 = 2006;
        m_uiValue2 = 1;
    }

    virtual ~AuthenticationInfo();
};

}
}

#endif
