#ifndef GAME_CORE_POSTGAMETIMECONDITION_HPP
#define GAME_CORE_POSTGAMETIMECONDITION_HPP

#include "game/core/TimerBasedCondition.hpp"

class MasterGameMode;
class XmlObject;

class PostGameTimeCondition : public TimerBasedCondition
{
public:
    PostGameTimeCondition(const XmlObject& object);
    PostGameTimeCondition(int offset);
    virtual ~PostGameTimeCondition();

    virtual int GetValue(MasterGameMode* gameMode) const;
    virtual unsigned int GetTimerID() const;
    virtual bool ShouldBlock() const;
};

#endif
