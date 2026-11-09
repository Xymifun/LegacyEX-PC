#ifndef GAME_WORLD_GENERATION_PLAYERSENTEREDVOLUMECONDITION_HPP
#define GAME_WORLD_GENERATION_PLAYERSENTEREDVOLUMECONDITION_HPP

#include <string>

#include "game/core/IntegerCondition.hpp"
#include "game/core/maybe.hpp"

class PlayersEnteredVolumeCondition : public IntegerCondition
{
public:
    PlayersEnteredVolumeCondition(const XmlObject& object);
    PlayersEnteredVolumeCondition(int threshold, EOperator op, const std::wstring& volumeName, bool useTargetAreas);

    virtual int GetValue(MasterGameMode* gameMode) const;
    int GetThresholdPlayerOverride(MasterGameMode* gameMode, int playerCount) const;
    virtual unsigned int GetThreshold(MasterGameMode* gameMode) const;
    virtual void AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const;
    bool IsUsingPrimaryTargetAreas() const;
    virtual bool UpdateMessage(MasterGameMode* gameMode, int message) const;

private:
    maybe<std::wstring> m_volumeName;
    maybe<bool> m_useTargetAreas;
    maybe<bool> m_useRatio;
};

#endif
