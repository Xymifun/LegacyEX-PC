#ifndef GAME_CORE_TEAMSCONDITION_HPP
#define GAME_CORE_TEAMSCONDITION_HPP

#include "game/core/IntegerCondition.hpp"

class TeamsCondition : public IntegerCondition
{
public:
    TeamsCondition(const XmlObject& object) : IntegerCondition(object) {}

    virtual ~TeamsCondition();
    virtual void AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const;
    virtual int GetValue(MasterGameMode* gameMode) const;
};

#endif
