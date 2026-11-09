#ifndef GAME_CORE_SHOWDOWNTIMECONDITION_HPP
#define GAME_CORE_SHOWDOWNTIMECONDITION_HPP

#include "game/core/TimerBasedCondition.hpp"

class MasterGameMode;
class XmlObject;

class ShowdownTimeCondition : public TimerBasedCondition
{
public:
    ShowdownTimeCondition(const XmlObject& object);
    ShowdownTimeCondition(int offset);
    virtual ~ShowdownTimeCondition();

    virtual int GetValue(MasterGameMode* gameMode) const;
    virtual unsigned int GetTimerID() const;
    virtual bool ShouldBlock() const;
};

#endif
