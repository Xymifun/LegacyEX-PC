#ifndef GAME_CORE_ROUNDTIMECONDITION_HPP
#define GAME_CORE_ROUNDTIMECONDITION_HPP

#include "game/core/TimerBasedCondition.hpp"

class XmlObject;

class RoundTimeCondition : public TimerBasedCondition
{
public:
    RoundTimeCondition(const XmlObject& object);
    RoundTimeCondition(int offset);
    virtual ~RoundTimeCondition();

    virtual int GetValue(MasterGameMode* gameMode) const;
    virtual unsigned int GetThreshold(MasterGameMode* gameMode) const;
    virtual unsigned int GetTimerID() const;
    virtual bool ShouldBlock() const;
};

#endif
