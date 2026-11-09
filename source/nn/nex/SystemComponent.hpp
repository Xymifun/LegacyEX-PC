#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCOMPONENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCOMPONENT_HPP

#include <cwchar>

#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class SystemComponent : public RefCountedObject
{
public:
    enum _State
    {
        STATE_CREATED = 1,
        STATE_INITIALIZING = 2,
        STATE_READY = 4,
        STATE_IN_USE = 8,
        STATE_TERMINATING = 16,
        STATE_TERMINATION_PENDING = 32,
        STATE_TERMINATED = 64,
        STATE_INITIALIZATION_FAILED = 128,
        STATE_TERMINATION_FAILED = 256
    };

    class Use : public RootObject
    {
    public:
        Use(SystemComponent* pComponent, const wchar_t* pReason);
        virtual ~Use();

    private:
        SystemComponent* m_pComponent;
        const wchar_t* m_pReason;
        bool m_bInUse;
    };

    explicit SystemComponent(const String& rName);
    virtual ~SystemComponent();

    virtual const wchar_t* GetType() const;
    virtual bool IsAKindOf(const wchar_t* pType) const { return std::wcscmp(L"SystemComponent", pType) == 0; }

    virtual void EnforceDeclareSysComponentMacro() = 0;
    virtual void StateTransition(_State state) {}

    virtual void OnInitialize() { TestState(); }

    virtual void OnTerminate() { TestState(); }

    virtual bool BeginInitialization();

    virtual bool EndInitialization() { return true; }

    virtual bool BeginTermination();

    virtual bool EndTermination() { return true; }

    virtual bool ValidTransition(_State state);
    virtual bool UseIsAllowed();

    virtual _State TestState() { return static_cast<_State>(m_uiState); }

    virtual void DoWork() {}

    void SetParent(SystemComponent* pParent);
    bool SetState(_State state, bool bForce);
    _State Initialize();
    _State Terminate();
    qResult WaitForTerminatedState(unsigned int uiTimeout);
    qResult WaitForReadyState(unsigned int uiTimeout);
    void SetName(const String& rName);
    bool IncrementInUseCount(const wchar_t* pReason);
    void DecrementInUseCount(const wchar_t* pReason);
    static const wchar_t* GetStateString(_State state);

protected:
    friend class SystemComponentGroup;

    String m_Name;
    unsigned int m_uiState;
    int m_iInUseCount;
    SystemComponent* m_pParent;
};

}
}

#endif
