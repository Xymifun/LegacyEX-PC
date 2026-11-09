#include "game/minigame/GameRuleDefinition.hpp"

#include "game/core/GameRule.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/core/GameRules_BoolAttribute.hpp"
#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_HexAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/GameRules_StringAttribute.hpp"
#include "game/core/GameRules_UnsignedCharAttribute.hpp"
#include "game/core/IGameRuleAttribute.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/GameRules_BlockAttribute.hpp"
#include "game/world/block/GameRules_BlockStateAttribute.hpp"
#include "game/data/DosGameRuleSaveInterface.hpp"
#include "game/data/TagCollectorSaveInterface.hpp"
#include "game/minigame/CompleteAllRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/core/StringFormat.hpp"

GameRuleDefinition::GameRuleDefinition()
{
    mDescriptionName = L"";
    mPromptName = L"";
    mDataTag = 0;
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_descriptionName, &mDescriptionName);
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_promptName, &mPromptName);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_dataTag, &mDataTag);
}

GameRuleDefinition::~GameRuleDefinition()
{
    for (AttributeMap::iterator it = mAttributes.begin(); it != mAttributes.end(); ++it)
    {
        delete it->second;
    }
}

GameRules_StringAttribute* GameRuleDefinition::addStringAttribute(ConsoleGameRules::EGameRuleAttr attribute,
                                                                  std::wstring* value)
{
    GameRules_StringAttribute* newAttribute = new GameRules_StringAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

GameRules_IntAttribute* GameRuleDefinition::addIntAttribute(ConsoleGameRules::EGameRuleAttr attribute, int* value)
{
    AttributeMap::iterator it = mAttributes.find(attribute);
    if (it != mAttributes.end())
    {
        delete it->second;
    }

    GameRules_IntAttribute* newAttribute = new GameRules_IntAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

GameRules_HexAttribute* GameRuleDefinition::addHexAttribute(ConsoleGameRules::EGameRuleAttr attribute, int* value)
{
    AttributeMap::iterator it = mAttributes.find(attribute);
    if (it != mAttributes.end())
    {
        delete it->second;
    }

    GameRules_HexAttribute* newAttribute = new GameRules_HexAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

GameRules_UnsignedCharAttribute* GameRuleDefinition::addUnsignedCharAttribute(ConsoleGameRules::EGameRuleAttr attribute,
                                                                              unsigned char* value)
{
    GameRules_UnsignedCharAttribute* newAttribute = new GameRules_UnsignedCharAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

GameRules_FloatAttribute* GameRuleDefinition::addFloatAttribute(ConsoleGameRules::EGameRuleAttr attribute, float* value)
{
    GameRules_FloatAttribute* newAttribute = new GameRules_FloatAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

GameRules_DoubleAttribute* GameRuleDefinition::addDoubleAttribute(ConsoleGameRules::EGameRuleAttr attribute,
                                                                  double* value)
{
    GameRules_DoubleAttribute* newAttribute = new GameRules_DoubleAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

GameRules_BoolAttribute* GameRuleDefinition::addBoolAttribute(ConsoleGameRules::EGameRuleAttr attribute, bool* value)
{
    GameRules_BoolAttribute* newAttribute = new GameRules_BoolAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

GameRules_BlockAttribute* GameRuleDefinition::addBlockAttribute(ConsoleGameRules::EGameRuleAttr attribute,
                                                                Block** value)
{
    GameRules_BlockAttribute* newAttribute = new GameRules_BlockAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

GameRules_BlockStateAttribute* GameRuleDefinition::addBlockStateAttribute(ConsoleGameRules::EGameRuleAttr attribute,
                                                                          const BlockState** value)
{
    GameRules_BlockStateAttribute* newAttribute = new GameRules_BlockStateAttribute(value);
    mAttributes[attribute] = newAttribute;
    return newAttribute;
}

void GameRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    int attributeId = IGameRuleAttribute::attributeIdFromName(name);
    ConsoleGameRules::EGameRuleAttr attribute = static_cast<ConsoleGameRules::EGameRuleAttr>(attributeId);

    if (attributeId != -1 && attributeId < ConsoleGameRules::eGameRuleAttr_Count
        && mAttributes.find(attribute) != mAttributes.end())
    {
        mAttributes[attribute]->setValue(value);
    }
}

bool GameRuleDefinition::attributeHasValue(ConsoleGameRules::EGameRuleAttr attribute)
{
    AttributeMap::iterator it = mAttributes.find(attribute);
    if (it != mAttributes.end())
    {
        return it->second->hasValue();
    }

    return false;
}

void GameRuleDefinition::write(GameRuleSaveInterface* saveInterface)
{
    saveInterface->startTag(getActionType());
    writeAttributes(saveInterface, 0);

    std::vector<GameRuleDefinition*> children;
    getChildren(&children);

    saveInterface->startChildren(static_cast<int>(children.size()));

    for (std::vector<GameRuleDefinition*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        (*it)->write(saveInterface);
    }

    saveInterface->finishTag();
}

void GameRuleDefinition::write(DataOutputStream* output, boost::unordered_map<int, int>* tagKeys)
{
    DosGameRuleSaveInterface saveInterface(output, tagKeys);
    write(&saveInterface);
}

void GameRuleDefinition::findDistinctTagsAndAttributes(boost::unordered_set<int>& tagTypes,
                                                       boost::unordered_set<int>& attributes)
{
    TagCollectorSaveInterface saveInterface(&tagTypes, &attributes);
    write(&saveInterface);
}

void GameRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    unsigned int attributeCount = 0;

    for (AttributeMap::iterator it = mAttributes.begin(); it != mAttributes.end(); ++it)
    {
        if (!it->second->isOptional() || it->second->hasValue())
        {
            ++attributeCount;
        }
    }

    saveInterface->startAttributes(extraAttributeCount + attributeCount);

    for (AttributeMap::iterator it = mAttributes.begin(); it != mAttributes.end(); ++it)
    {
        if (!it->second->isOptional() || it->second->hasValue())
        {
            it->second->save(it->first, saveInterface);
        }
    }
}

void GameRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
}

void GameRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children, ConsoleGameRules::EGameRuleType type)
{
}

GameRuleDefinition* GameRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    return NULL;
}

void GameRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule)
{
    GameRule::_ValueType value;
    value.bValue = false;

    gameRule->setParameter(L"bComplete", value);
}

bool GameRuleDefinition::getComplete(GameRule* gameRule)
{
    return gameRule->getParameter(L"bComplete").bValue;
}

void GameRuleDefinition::setComplete(GameRule* gameRule, bool complete)
{
    GameRule::_ValueType value = gameRule->getParameter(L"bComplete");
    value.bValue = complete;

    gameRule->setParameter(L"bComplete", value);
}

std::vector<GameRuleDefinition*>* GameRuleDefinition::enumerate()
{
    std::vector<GameRuleDefinition*>* definitions = new std::vector<GameRuleDefinition*>();

    definitions->push_back(this);
    getChildren(definitions);

    return definitions;
}

GameRule* GameRuleDefinition::generateNewGameRulesInstance(GameRulesInstance::EGameRulesInstanceType type,
                                                           LevelRuleset* ruleset, Connection* connection)
{
    GameRule* rule = new GameRulesInstance(ruleset, connection);

    ruleset->populateGameRule(type, rule);

    return rule;
}

std::wstring GameRuleDefinition::generateDescriptionString(ConsoleGameRules::EGameRuleType type,
                                                           const std::wstring& description, void* data, int count)
{
    std::wstring text = description;

    if (type == 27)
    {
        text = CompleteAllRuleDefinition::generateDescriptionString(description, data, count);
    }

    return text;
}

void GameRuleDefinition::WriteXMLData(std::string& xml)
{
    ConsoleGameRules::EGameRuleType type = getActionType();

    if (type == 19 && !static_cast<TargetAreaRuleDefinition*>(this)->mPrimary)
    {
        type = ConsoleGameRules::eGameRuleType_ScoreRing;
    }

    xml += formatstr("<%ls", GameRuleManager::sTagNames[type]);

    WriteAttributesAsXML(xml);

    xml += "/>\n";
}

void GameRuleDefinition::WriteAllAttributesAsXML(std::string& xml)
{
    for (AttributeMap::iterator it = mAttributes.begin(); it != mAttributes.end(); ++it)
    {
        if (!it->second->isOptional() || it->second->hasValue())
        {
            it->second->writeAsXml(it->first, xml);
        }
    }
}

void GameRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    WriteAllAttributesAsXML(xml);
}

void GameRuleDefinition::WriteAttributeAsXML(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml)
{
    AttributeMap::iterator it = mAttributes.find(attribute);
    if (it != mAttributes.end())
    {
        if (!it->second->isOptional() || it->second->hasValue())
        {
            it->second->writeAsXml(it->first, xml);
        }
    }
}

AABB* GameRuleDefinition::getBoundingVolume()
{
    return NULL;
}

void GameRuleDefinition::moveBoundingVolume(int x, int y, int z)
{
}

void GameRuleDefinition::onAttributesAdded()
{
}

int GameRuleDefinition::getGoal()
{
    return 0;
}

int GameRuleDefinition::getProgress(GameRule* gameRule)
{
    return 0;
}

int GameRuleDefinition::getIcon()
{
    return -1;
}

int GameRuleDefinition::getAuxValue()
{
    return 0;
}

bool GameRuleDefinition::onUseBlock(GameRule* gameRule, int blockId, int x, int y, int z)
{
    return false;
}

bool GameRuleDefinition::onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item)
{
    return false;
}

bool GameRuleDefinition::evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef)
{
    return true;
}

void GameRuleDefinition::postProcessPlayer(boost::shared_ptr<Player> player)
{
}
