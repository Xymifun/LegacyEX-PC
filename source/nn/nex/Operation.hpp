#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_OPERATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_OPERATION_HPP

#include "nn/nex/StateMachine.hpp"
#include "nn/nex/UserContext.hpp"

namespace nn
{
namespace nex
{

class Operation : public StateMachine::QEvent
{
public:
    enum Event
    {
    };

    explicit Operation(unsigned int flags);
    virtual ~Operation() {}
    virtual unsigned short GetSignal() const;
    virtual unsigned int GetType() const = 0;
    virtual const wchar_t* GetClassNameString() const = 0;
    virtual void ForceImplOperationCommonMethodsMacro() = 0;

    UserContext GetUserData();
    void SetUserData(UserContext context);
    void AbortOperation();
    const wchar_t* GetEventString(Event event) const;

protected:
    bool m_signalState;
    UserContext m_userContext;
    unsigned int m_flags;
};

}
}

#endif
