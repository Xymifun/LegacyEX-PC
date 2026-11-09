#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SECURITYCONTEXTMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SECURITYCONTEXTMANAGER_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/ThreadVariable.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class SecurityContext : public RootObject
{
public:
    SecurityContext(unsigned int uiPID, unsigned int uiCID, unsigned int uiAddress)
        : m_uiPID(uiPID),
          m_uiCID(uiCID),
          m_uiAddress(uiAddress)
    {
    }

    virtual ~SecurityContext();

    unsigned int GetPID() const { return m_uiPID; }

    unsigned int GetCID() const { return m_uiCID; }

    unsigned int GetAddress() const { return m_uiAddress; }

private:
    unsigned int m_uiPID;
    unsigned int m_uiCID;
    unsigned int m_uiAddress;
};

class SecurityContextManager : public RootObject
{
public:
    typedef qList<SecurityContext> SecurityContextList;

    SecurityContextManager();
    virtual ~SecurityContextManager() {}

    static SecurityContextManager* GetInstance();

    void Push(unsigned int uiPID, unsigned int uiCID, unsigned int uiAddress);
    void Pop();

    unsigned int GetCurrentPID() const;
    unsigned int GetCurrentCID() const;
    unsigned int GetCurrentAddress() const;

    static unsigned int StaticGetCurrentPID();
    static unsigned int StaticGetCurrentCID();

private:
    ThreadVariable<SecurityContextList> m_securityContexts;
};

}
}

#endif
