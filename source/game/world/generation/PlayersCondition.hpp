#ifndef GAME_WORLD_GENERATION_PLAYERSCONDITION_HPP
#define GAME_WORLD_GENERATION_PLAYERSCONDITION_HPP

#include "game/core/IntegerCondition.hpp"

class PlayersCondition : public IntegerCondition
{
public:
    PlayersCondition(const XmlObject& object);
    virtual ~PlayersCondition();

    virtual bool IsSatisfied(MasterGameMode* gameMode) const;
    virtual void AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const;
    virtual int GetValue(MasterGameMode* gameMode) const;

private:
    bool m_activeOnly;
    bool m_readyOnly;
};

#endif
