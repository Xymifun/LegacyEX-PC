#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/ClientScoreboard.hpp"
#include "game/core/IGameRuleAttribute.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/StringFormat.hpp"

static const wchar_t* sBoostDirectionNames[8] = {L"plus_x",      L"minus_x",      L"plus_z",      L"minus_z",
                                                 L"omni_plus_x", L"omni_minus_x", L"omni_plus_z", L"omni_minus_z"};

ThermalAreaRuleDefinition::ThermalAreaRuleDefinition()
    : mLiftForceModifier(1.0),
      mStaticLift(0.0),
      mTargetHeight(0.0),
      mSpeedBoost(0.0),
      mBoostDirection(4),
      mConditionType(0),
      mConditionValue0(0),
      mConditionValue1(0),
      mEnabled(true)
{
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_liftForceModifier, &mLiftForceModifier);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_staticLift, &mStaticLift);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_targetHeight, &mTargetHeight);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_speedBoost, &mSpeedBoost);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_condition_type, &mConditionType);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_condition_value_0, &mConditionValue0);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_condition_value_1, &mConditionValue1);
    mUseCount = 0;
}

ThermalAreaRuleDefinition::~ThermalAreaRuleDefinition()
{
}

bool ThermalAreaRuleDefinition::getConditionsMet(boost::shared_ptr<Entity> entity)
{
    switch (mConditionType)
    {
    case 1:
        if (mUseCount >= mConditionValue0)
        {
            return false;
        }
        break;
    case 2:
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            ClientScoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetClientScoreboard();
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            int lapsCompleted = scoreboard->GetLapsCompleted(player->getSUID(), -1);
            if (lapsCompleted < mConditionValue0 || lapsCompleted > mConditionValue1)
            {
                return false;
            }
        }
        break;
    }

    return true;
}

void ThermalAreaRuleDefinition::resetConditions()
{
    mUseCount = 0;
}

void ThermalAreaRuleDefinition::updateUseCount()
{
    ++mUseCount;
}

void ThermalAreaRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == gRuleAttributeNames[ConsoleGameRules::eGameRuleAttr_boostDirection])
    {
        if (value == sBoostDirectionNames[0])
        {
            mBoostDirection = 0;
            return;
        }
        if (value == sBoostDirectionNames[1])
        {
            mBoostDirection = 1;
            return;
        }
        if (value == sBoostDirectionNames[2])
        {
            mBoostDirection = 2;
            return;
        }
        if (value == sBoostDirectionNames[3])
        {
            mBoostDirection = 3;
            return;
        }
        if (value == sBoostDirectionNames[4])
        {
            mBoostDirection = 4;
            return;
        }
        if (value == sBoostDirectionNames[5])
        {
            mBoostDirection = 5;
            return;
        }
        if (value == sBoostDirectionNames[6])
        {
            mBoostDirection = 6;
            return;
        }
        if (value == sBoostDirectionNames[7])
        {
            mBoostDirection = 7;
            return;
        }
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

bool ThermalAreaRuleDefinition::setBoostMods(int& boostMod, int& boostDirection)
{
    switch (mBoostDirection)
    {
    case 0:
        boostMod = 1;
        boostDirection = 0;
        return true;
    case 1:
        boostMod = -1;
        boostDirection = 0;
        return true;
    case 2:
        boostMod = 0;
        boostDirection = 1;
        return true;
    case 3:
        boostMod = 0;
        boostDirection = -1;
        return true;
    default:
        return false;
    }
}

unsigned int ThermalAreaRuleDefinition::getBoostDirection()
{
    return static_cast<unsigned int>(mBoostDirection);
}

void ThermalAreaRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    if (!mName.empty())
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_name, xml);
    }
    if (mLiftForceModifier != 1.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_liftForceModifier, xml);
    }
    if (mStaticLift != 0.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_staticLift, xml);
    }
    if (mTargetHeight != 0.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_targetHeight, xml);
    }
    if (mSpeedBoost != 0.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_speedBoost, xml);
        xml.append(formatstr(" boostDirection=\"%ls\"", sBoostDirectionNames[mBoostDirection]));
    }
    if (mConditionType != 0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_condition_type, xml);
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_condition_value_0, xml);
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_condition_value_1, xml);
    }
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_x0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_y0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_z0, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_x1, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_y1, xml);
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_z1, xml);
    if (mDataTag != 0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_dataTag, xml);
    }
}

ConsoleGameRules::EGameRuleType ThermalAreaRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_ThermalArea;
}
