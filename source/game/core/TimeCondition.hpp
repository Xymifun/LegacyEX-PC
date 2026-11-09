#ifndef GAME_CORE_TIMECONDITION_HPP
#define GAME_CORE_TIMECONDITION_HPP

#include <vector>

#include "game/core/IntegerCondition.hpp"

class MasterGameMode;
class XmlObject;

class TimeCondition : public IntegerCondition
{
public:
    TimeCondition(const XmlObject& object);
    TimeCondition(int offset);
    virtual ~TimeCondition() {}

    virtual void AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const;
    virtual unsigned int GetThreshold(MasterGameMode* gameMode) const;
    virtual bool IsSatisfied(MasterGameMode* gameMode) const;

private:
    int m_offset;
};

#endif
