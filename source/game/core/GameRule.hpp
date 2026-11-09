#ifndef GAME_CORE_GAMERULE_HPP
#define GAME_CORE_GAMERULE_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/item/not_null_ptr.hpp"

class Connection;
class DataInputStream;
class DataOutputStream;
class GameRuleDefinition;
class ItemInstance;

class GameRule
{
public:
    struct _ValueType
    {
        _ValueType() : lValue(0), bIsGameRule(false) {}

        union
        {
            bool bValue;
            int iValue;
            long long lValue;
            double dValue;
            GameRule* pGameRule;
        };

        bool bIsGameRule;
    };

    GameRule(GameRuleDefinition* pDefinition, Connection* pConnection);
    virtual ~GameRule();

    _ValueType getParameter(const std::wstring& name);
    void setParameter(const std::wstring& name, _ValueType value);
    GameRuleDefinition* getGameRuleDefinition();
    void onUseBlock(int blockId, int x, int y, int z);
    void onCollectItem(not_null_ptr<ItemInstance> item);
    void write(DataOutputStream* pOutput);
    void read(DataInputStream* pInput);

    friend class GameRuleDefinition;
    friend class CompoundGameRuleDefinition;
    friend class CollectItemRuleDefinition;
    friend class CompleteAllRuleDefinition;

private:
    typedef boost::unordered_map<std::wstring, _ValueType> ParameterMap;

    GameRuleDefinition* mDefinition;
    Connection* mConnection;
    ParameterMap mParameters;
};

#endif
