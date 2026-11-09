#ifndef GAME_CORE_CONDITIONCONNECTIVE_HPP
#define GAME_CORE_CONDITIONCONNECTIVE_HPP

#include <string>
#include <utility>
#include <vector>

#include "game/core/GameCondition.hpp"

class MasterGameMode;
class XmlObject;

class ConditionConnective : public GameCondition
{
public:
    ConditionConnective(const XmlObject& object);
    virtual ~ConditionConnective();

    static std::pair<bool, int> parseType(std::wstring name);
    static GameCondition* parseCondition(const XmlObject& object);

    GameCondition* FirstConditionWithValue(bool value, MasterGameMode* gameMode) const;
    const std::vector<GameCondition*>& GetConditions() const { return m_conditions; }
    virtual bool IsSatisfied(MasterGameMode* gameMode) const;
    virtual unsigned char GetMessage() const;
    virtual void AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const;
    virtual bool UpdateMessage(MasterGameMode* gameMode, int message) const;

private:
    int m_type;
    std::vector<GameCondition*> m_conditions;
};

#endif
