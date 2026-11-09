#ifndef GAME_CORE_GAMECONDITION_HPP
#define GAME_CORE_GAMECONDITION_HPP

#include <vector>

class MasterGameMode;

class GameCondition
{
public:
    virtual ~GameCondition() {}
    virtual bool IsSatisfied(MasterGameMode* gameMode) const = 0;
    virtual unsigned char GetMessage() const = 0;
    virtual void AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const = 0;
    virtual bool UpdateMessage(MasterGameMode* gameMode, int message) const;
};

#endif
