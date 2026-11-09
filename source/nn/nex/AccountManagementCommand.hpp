#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTCOMMAND_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTCOMMAND_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/SandboxConnectionInfo.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class AccountManagementProtocolClient;
class ProtocolCallContext;

class AccountManagementCommand : public RootObject
{
public:
    AccountManagementCommand(const DebugString& rDescription, const String& strPassword);

    virtual ~AccountManagementCommand();

    bool Invoke(ProtocolCallContext* pCallContext);

    const DebugString& GetDescription() const;

    DebugString m_description;
    String m_strPassword;
    String m_strKey;

protected:
    virtual bool InvokeImpl(ProtocolCallContext* pCallContext) = 0;
};

class LookupOrCreateAccountCommand : public AccountManagementCommand
{
public:
    LookupOrCreateAccountCommand(const DebugString& rDescription, AccountManagementProtocolClient* pProtocolClient,
                                 const String& strPrincipalName, const String& strPassword, const String& strEmail,
                                 unsigned int uiGroups, AnyObjectHolder<Data, String>* pCustomData,
                                 unsigned int* pPrincipalID);

    virtual ~LookupOrCreateAccountCommand();

protected:
    virtual bool InvokeImpl(ProtocolCallContext* pCallContext);

private:
    AccountManagementProtocolClient* m_pProtocolClient;
    String m_strPrincipalName;
    String m_strEmail;
    unsigned int m_uiGroups;
    AnyObjectHolder<Data, String> m_oCustomData;
    unsigned int* m_pPrincipalID;
};

class GuestCreateAccountCommand : public AccountManagementCommand
{
public:
    GuestCreateAccountCommand(const DebugString& rDescription, const SandboxConnectionInfo& oSandboxConnectionInfo,
                              const String& strPrincipalName, const String& strPassword, const String& strEmail,
                              unsigned int uiGroups);
    virtual ~GuestCreateAccountCommand();

protected:
    virtual bool InvokeImpl(ProtocolCallContext* pCallContext);

    SandboxConnectionInfo m_oSandboxConnectionInfo;
    String m_strPrincipalName;
    String m_strEmail;
    unsigned int m_uiGroups;
};

class GuestCustomCreateAccountCommand : public GuestCreateAccountCommand
{
public:
    GuestCustomCreateAccountCommand(const DebugString& rDescription,
                                    const SandboxConnectionInfo& oSandboxConnectionInfo, const String& strPrincipalName,
                                    const String& strPassword, const String& strEmail, unsigned int uiGroups,
                                    AnyObjectHolder<Data, String>* pCustomData, unsigned int* pPrincipalID);
    virtual ~GuestCustomCreateAccountCommand();

protected:
    virtual bool InvokeImpl(ProtocolCallContext* pCallContext);

private:
    AnyObjectHolder<Data, String>* m_pCustomData;
    unsigned int* m_pPrincipalID;
};

}
}

#endif
