#ifndef GAME_WORLD_GENERATION_TIMESINCEPLAYERSCONDITION_HPP
#define GAME_WORLD_GENERATION_TIMESINCEPLAYERSCONDITION_HPP

#include "game/core/TimerBasedCondition.hpp"

class MasterGameMode;
class XmlObject;

class TimeSincePlayersCondition : public TimerBasedCondition
{
public:
    TimeSincePlayersCondition(const XmlObject& object);
    TimeSincePlayersCondition(int offset);
    virtual ~TimeSincePlayersCondition();

    virtual bool IsSatisfied(MasterGameMode* gameMode) const;
    virtual unsigned char GetMessage() const;
    virtual int GetValue(MasterGameMode* gameMode) const;
    virtual unsigned int GetTimerID() const;
    virtual bool ShouldBlock() const;
    virtual bool CanStartTimer(MasterGameMode* gameMode) const;

private:
    unsigned int m_playerCount;
};

#endif
