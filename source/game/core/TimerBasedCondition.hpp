#ifndef GAME_CORE_TIMERBASEDCONDITION_HPP
#define GAME_CORE_TIMERBASEDCONDITION_HPP

#include <vector>

#include "game/core/TimeCondition.hpp"

class MasterGameMode;
class XmlObject;

class TimerBasedCondition : public TimeCondition
{
public:
    TimerBasedCondition(const XmlObject& object);
    TimerBasedCondition(int offset);
    virtual ~TimerBasedCondition() {}
    virtual unsigned char GetMessage() const;

    virtual bool UpdateMessage(MasterGameMode* gameMode, int message) const;
    virtual unsigned int GetTimerID() const = 0;
    virtual bool ShouldBlock() const = 0;
    virtual void SetUpTimer(MasterGameMode* gameMode, int message) const;
    virtual bool CanStartTimer(MasterGameMode* gameMode) const;
};

#endif
