#include "game/minigame/CompleteAllRuleDefinition.hpp"

#include "game/core/GameRule.hpp"
#include "game/network/Connection.hpp"
#include "game/network/UpdateGameRuleProgressPacket.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/ItemInstance.hpp"

std::wstring CompleteAllRuleDefinition::generateDescriptionString(const std::wstring& description, void* value,
                                                                  int count)
{
    const int* values = static_cast<const int*>(value);
    std::wstring text = description;

    text = replaceAll(text, L"{*progress*}", _toString(values[1]));
    text = replaceAll(text, L"{*goal*}", _toString(values[0]));
    return text;
}

void CompleteAllRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    CompoundGameRuleDefinition::getChildren(children);
}

void CompleteAllRuleDefinition::updateStatus(GameRule* rule)
{
    struct ProgressData
    {
        int goal;
        int progress;
    };

    int goal = 0;
    int progress = 0;
    for (GameRule::ParameterMap::iterator it = rule->mParameters.begin(); it != rule->mParameters.end(); ++it)
    {
        if (it->second.bIsGameRule)
        {
            goal += it->second.pGameRule->getGameRuleDefinition()->getGoal();
            progress += it->second.pGameRule->getGameRuleDefinition()->getProgress(it->second.pGameRule);
        }
    }

    if (rule->mConnection != NULL)
    {
        ProgressData data;
        data.goal = goal;
        data.progress = progress;

        int icon = -1;
        int auxValue = 0;
        if (mCompletedChild != NULL)
        {
            icon = mCompletedChild->getIcon();
            auxValue = mCompletedChild->getAuxValue();
            mCompletedChild = NULL;
        }
        boost::shared_ptr<UpdateGameRuleProgressPacket> packet(new UpdateGameRuleProgressPacket(
            getActionType(), mDescriptionName, icon, auxValue, 0, &data, sizeof(data)));
        rule->mConnection->send(packet);
    }
}

bool CompleteAllRuleDefinition::onUseBlock(GameRule* rule, int blockId, int x, int y, int z)
{
    bool result = CompoundGameRuleDefinition::onUseBlock(rule, blockId, x, y, z);
    if (result != 0)
    {
        updateStatus(rule);
    }
    return result;
}

bool CompleteAllRuleDefinition::onCollectItem(GameRule* rule, not_null_ptr<ItemInstance> item)
{
    bool result = CompoundGameRuleDefinition::onCollectItem(rule, item);
    if (result)
    {
        updateStatus(rule);
    }
    return result;
}

ConsoleGameRules::EGameRuleType CompleteAllRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CompleteAll;
}
