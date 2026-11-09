#ifndef GAME_CORE_GAMERULES_DOUBLEATTRIBUTE_HPP
#define GAME_CORE_GAMERULES_DOUBLEATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class GameRules_DoubleAttribute : public IGameRuleAttribute
{
public:
    GameRules_DoubleAttribute(double* value);
    virtual ~GameRules_DoubleAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

    GameRules_DoubleAttribute* setMin(double min);
    GameRules_DoubleAttribute* setMax(double max);

private:
    double* mValue;
    double mMin;
    double mMax;
};

#endif
