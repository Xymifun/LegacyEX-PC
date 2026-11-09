#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STATEMACHINE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STATEMACHINE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/VirtualRootObject.hpp"

namespace nn
{
namespace nex
{

class StateMachine : public VirtualRootObject
{
public:
    virtual ~StateMachine();

    class QEvent : public RootObject
    {
    public:
        virtual ~QEvent() {}

        virtual unsigned short GetSignal() const = 0;

    protected:
        QEvent() : m_unknownByte(false) {}

        bool m_unknownByte;
    };

    class QSimpleEvent : public QEvent
    {
    public:
        virtual ~QSimpleEvent();
        virtual unsigned short GetSignal() const;

    protected:
        unsigned short m_signal;
    };
};

}
}

#endif
