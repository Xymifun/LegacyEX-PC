#include "game/core/LastKillTimeCondition.hpp"

LastKillTimeCondition::LastKillTimeCondition(const XmlObject& object) : TimeCondition(object)
{
}

LastKillTimeCondition::~LastKillTimeCondition()
{
}

int LastKillTimeCondition::GetValue(MasterGameMode* gameMode) const
{
    return 0;
}
