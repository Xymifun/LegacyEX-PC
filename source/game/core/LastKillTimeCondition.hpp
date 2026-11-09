#ifndef GAME_CORE_LASTKILLTIMECONDITION_HPP
#define GAME_CORE_LASTKILLTIMECONDITION_HPP

#include "game/core/TimeCondition.hpp"

class MasterGameMode;
class XmlObject;

class LastKillTimeCondition : public TimeCondition
{
public:
    LastKillTimeCondition(const XmlObject& object);
    LastKillTimeCondition(int offset);
    virtual ~LastKillTimeCondition();

    virtual int GetValue(MasterGameMode* gameMode) const;
};

#endif
