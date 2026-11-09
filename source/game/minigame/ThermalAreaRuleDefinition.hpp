#ifndef GAME_MINIGAME_THERMALAREARULEDEFINITION_HPP
#define GAME_MINIGAME_THERMALAREARULEDEFINITION_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/minigame/NamedAreaRuleDefinition.hpp"

class Entity;

class ThermalAreaRuleDefinition : public NamedAreaRuleDefinition
{
public:
    ThermalAreaRuleDefinition();
    ThermalAreaRuleDefinition(int, int, int, int, int, int);
    virtual ~ThermalAreaRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual void WriteAttributesAsXML(std::string& xml);

    bool getConditionsMet(boost::shared_ptr<Entity> entity);
    void resetConditions();
    void updateUseCount();
    bool setBoostMods(int& boostMod, int& boostDirection);
    unsigned int getBoostDirection();

    bool isEnabled() const { return mEnabled; }
    double getLiftForceModifier() const { return mEnabled ? mLiftForceModifier : 1.0; }
    double getLiftSpeed() const { return mEnabled ? mStaticLift : 0.0; }
    double getMaxHeight() const { return mEnabled ? mTargetHeight : 0.0; }
    double getElytraSpeedBoost() const { return mEnabled ? mSpeedBoost : 0.0; }

protected:
    double mLiftForceModifier;
    double mStaticLift;
    double mTargetHeight;
    double mSpeedBoost;
    int mBoostDirection;
    int mConditionType;
    int mConditionValue0;
    int mConditionValue1;
    bool mEnabled;
    int mUseCount;
};

#endif
