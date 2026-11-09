#include "game/core/GameRule.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/network/Connection.hpp"

GameRule::GameRule(GameRuleDefinition* pDefinition, Connection* pConnection)
{
    mDefinition = pDefinition;
    mConnection = pConnection;
}

GameRule::~GameRule()
{
    for (ParameterMap::iterator it = mParameters.begin(); it != mParameters.end(); ++it)
    {
        if (it->second.bIsGameRule)
        {
            delete it->second.pGameRule;
        }
    }
}

GameRule::_ValueType GameRule::getParameter(const std::wstring& name)
{
    ParameterMap::iterator it = mParameters.find(name);
    return mParameters[name];
}

void GameRule::setParameter(const std::wstring& name, _ValueType value)
{
    ParameterMap::iterator it = mParameters.find(name);
    mParameters[name] = value;
}

GameRuleDefinition* GameRule::getGameRuleDefinition()
{
    return mDefinition;
}

void GameRule::onUseBlock(int blockId, int x, int y, int z)
{
    mDefinition->onUseBlock(this, blockId, x, y, z);
}

void GameRule::onCollectItem(not_null_ptr<ItemInstance> item)
{
    mDefinition->onCollectItem(this, item);
}

void GameRule::write(DataOutputStream* pOutput)
{
    pOutput->writeInt(mParameters.size());

    for (ParameterMap::iterator it = mParameters.begin(); it != mParameters.end(); ++it)
    {
        std::wstring name = it->first;
        _ValueType value = it->second;

        pOutput->writeUTF(it->first);
        bool isRule = value.bIsGameRule;
        pOutput->writeBoolean(isRule);

        if (isRule)
        {
            value.pGameRule->write(pOutput);
        }
        else
        {
            pOutput->writeLong(value.lValue);
        }
    }
}

void GameRule::read(DataInputStream* pInput)
{
    int count = pInput->readInt();

    for (int i = 0; i < count; ++i)
    {
        std::wstring name = pInput->readUTF();
        _ValueType value = getParameter(name);

        if (pInput->readBoolean())
        {
            value.pGameRule->read(pInput);
        }
        else
        {
            value.bIsGameRule = false;
            value.lValue = pInput->readLong();
            setParameter(name, value);
        }
    }
}
