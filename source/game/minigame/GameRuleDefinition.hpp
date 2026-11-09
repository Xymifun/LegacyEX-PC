#ifndef GAME_MINIGAME_GAMERULEDEFINITION_HPP
#define GAME_MINIGAME_GAMERULEDEFINITION_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/core/GameRulesInstance.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/item/not_null_ptr.hpp"

class AABB;
class Block;
class BlockState;
class Connection;
class DataOutputStream;
class GameRule;
class GameRules_BlockAttribute;
class GameRules_BlockStateAttribute;
class GameRules_BoolAttribute;
class GameRules_DoubleAttribute;
class GameRules_FloatAttribute;
class GameRules_HexAttribute;
class GameRules_IntAttribute;
class GameRules_StringAttribute;
class GameRules_UnsignedCharAttribute;
class GrantPermissionsRuleDefinition;
class IGameRuleAttribute;
class ItemInstance;
class Level;
class LevelRuleset;
class MiniGameDef;
class Player;

class GameRuleDefinition
{
public:
    typedef boost::unordered_map<ConsoleGameRules::EGameRuleAttr, IGameRuleAttribute*> AttributeMap;
    typedef boost::unordered_map<ConsoleGameRules::EGameRuleType, GameRuleDefinition*> DefinitionMap;

    GameRuleDefinition();
    virtual ~GameRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType() = 0;
    virtual AABB* getBoundingVolume();
    virtual void moveBoundingVolume(int x, int y, int z);
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children, ConsoleGameRules::EGameRuleType type);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual void onAttributesAdded();
    virtual void populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule);
    virtual int getGoal();
    virtual int getProgress(GameRule* gameRule);
    virtual int getIcon();
    virtual int getAuxValue();
    virtual bool onUseBlock(GameRule* gameRule, int blockId, int x, int y, int z);
    virtual bool onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item);
    virtual bool evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef);
    virtual void postProcessPlayer(boost::shared_ptr<Player> player);
    virtual void WriteXMLData(std::string& xml);
    virtual void WriteAttributesAsXML(std::string& xml);

    void write(GameRuleSaveInterface* saveInterface);
    void write(DataOutputStream* output, boost::unordered_map<int, int>* tagKeys);

    void findDistinctTagsAndAttributes(boost::unordered_set<int>& tagTypes, boost::unordered_set<int>& attributes);
    void distinctTagTypes(boost::unordered_set<ConsoleGameRules::EGameRuleType>& tagTypes);
    void distinctAttributes(boost::unordered_set<ConsoleGameRules::EGameRuleAttr>& attributes);

    bool attributeHasValue(ConsoleGameRules::EGameRuleAttr attribute);

    bool getComplete(GameRule* gameRule);
    void setComplete(GameRule* gameRule, bool complete);

    std::vector<GameRuleDefinition*>* enumerate();
    DefinitionMap* enumerateMap();

    void WriteAllAttributesAsXML(std::string& xml);
    void WriteAttributeAsXML(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

    static GameRule* generateNewGameRulesInstance(GameRulesInstance::EGameRulesInstanceType type, LevelRuleset* ruleset,
                                                  Connection* connection);
    static std::wstring generateDescriptionString(ConsoleGameRules::EGameRuleType type, const std::wstring& description,
                                                  void* data, int count);

protected:
    friend class GrantPermissionsRuleDefinition;
    friend class LevelRuleset;
    GameRules_StringAttribute* addStringAttribute(ConsoleGameRules::EGameRuleAttr attribute, std::wstring* value);
    GameRules_IntAttribute* addIntAttribute(ConsoleGameRules::EGameRuleAttr attribute, int* value);
    GameRules_HexAttribute* addHexAttribute(ConsoleGameRules::EGameRuleAttr attribute, int* value);
    GameRules_UnsignedCharAttribute* addUnsignedCharAttribute(ConsoleGameRules::EGameRuleAttr attribute,
                                                              unsigned char* value);
    GameRules_FloatAttribute* addFloatAttribute(ConsoleGameRules::EGameRuleAttr attribute, float* value);
    GameRules_DoubleAttribute* addDoubleAttribute(ConsoleGameRules::EGameRuleAttr attribute, double* value);
    GameRules_BoolAttribute* addBoolAttribute(ConsoleGameRules::EGameRuleAttr attribute, bool* value);
    GameRules_BlockAttribute* addBlockAttribute(ConsoleGameRules::EGameRuleAttr attribute, Block** value);
    GameRules_BlockStateAttribute* addBlockStateAttribute(ConsoleGameRules::EGameRuleAttr attribute,
                                                          const BlockState** value);

    unsigned int mReserved;
    std::wstring mDescriptionName;
    std::wstring mPromptName;
    int mDataTag;
    AttributeMap mAttributes;
};

#endif
