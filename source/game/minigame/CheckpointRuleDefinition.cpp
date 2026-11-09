#include "game/minigame/CheckpointRuleDefinition.hpp"

#include "game/core/GameRuleManager.hpp"
#include "game/minigame/CustomBeaconRuleDefinition.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"
#include "game/core/StringFormat.hpp"

CheckpointRuleDefinition::CheckpointRuleDefinition()
{
    mSpawnPoint = NULL;
    mSequenceID = -1;
    mElytraBoost = -1.0;
    mBeacons.clear();
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_id, &mSequenceID);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_speedBoost, &mElytraBoost);
}

CheckpointRuleDefinition::~CheckpointRuleDefinition()
{
    delete mSpawnPoint;
}

uint32_t CheckpointRuleDefinition::getSequenceID()
{
    return mSequenceID;
}

void CheckpointRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    if (mSpawnPoint != NULL)
    {
        children->push_back(mSpawnPoint);
    }

    for (std::vector<CustomBeaconRuleDefinition*>::iterator it = mBeacons.begin(); it != mBeacons.end(); ++it)
    {
        children->push_back(*it);
    }
}

void CheckpointRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children,
                                           ConsoleGameRules::EGameRuleType type)
{
    if (mSpawnPoint != NULL && mSpawnPoint->getActionType() == type)
    {
        children->push_back(mSpawnPoint);
    }

    for (std::vector<CustomBeaconRuleDefinition*>::iterator it = mBeacons.begin(); it != mBeacons.end(); ++it)
    {
        if ((*it)->getActionType() == type)
        {
            children->push_back(*it);
        }
    }
}

GameRuleDefinition* CheckpointRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* rule = NULL;
    if (type == ConsoleGameRules::eGameRuleType_UpdatePlayer)
    {
        rule = mSpawnPoint = new UpdatePlayerRuleDefinition();
    }
    else if (type == ConsoleGameRules::eGameRuleType_CustomBeacon)
    {
        rule = new CustomBeaconRuleDefinition();
        mBeacons.push_back(static_cast<CustomBeaconRuleDefinition*>(rule));
    }

    return rule;
}

void CheckpointRuleDefinition::setPlayerPosition(boost::shared_ptr<ServerPlayer> player)
{
    player->mConnection->teleport(static_cast<double>(mSpawnPoint->getX()), static_cast<double>(mSpawnPoint->getY()),
                                  static_cast<double>(mSpawnPoint->getZ()), mSpawnPoint->getYRot(),
                                  mSpawnPoint->getXRot(), true, true);
    player->mXOld = static_cast<double>(mSpawnPoint->getX());
    player->mYOld = static_cast<double>(mSpawnPoint->getY());
    player->mZOld = static_cast<double>(mSpawnPoint->getZ());
}

double CheckpointRuleDefinition::getElytraBoost()
{
    return mElytraBoost;
}

UpdatePlayerRuleDefinition* CheckpointRuleDefinition::GetSpawnPoint()
{
    return mSpawnPoint;
}

bool CheckpointRuleDefinition::hasSpawnPoint()
{
    return mSpawnPoint != NULL;
}

void CheckpointRuleDefinition::setSpawnPoint(boost::shared_ptr<Player> player)
{
    delete mSpawnPoint;
    mSpawnPoint = new UpdatePlayerRuleDefinition(player);
}

void CheckpointRuleDefinition::WriteXMLData(std::string& xml)
{
    ConsoleGameRules::EGameRuleType type = getActionType();
    xml += formatstr("<%ls ", GameRuleManager::sTagNames[type]);
    WriteAttributesAsXML(xml);

    if (mSpawnPoint == NULL)
    {
        xml += "/>\n";
    }
    else
    {
        xml += ">\n";
        mSpawnPoint->WriteXMLData(xml);
        xml += formatstr("</%ls>\n", GameRuleManager::sTagNames[type]);
    }
}

void CheckpointRuleDefinition::processBeacons(ServerLevel* level)
{
    if (mBeacons.empty())
    {
        return;
    }

    for (std::vector<CustomBeaconRuleDefinition*>::iterator it = mBeacons.begin(); it != mBeacons.end(); ++it)
    {
        (*it)->processCustomBeacon(level);
    }
}

void CheckpointRuleDefinition::deactivateBeacons(ServerLevel* level)
{
    if (mBeacons.empty())
    {
        return;
    }

    for (std::vector<CustomBeaconRuleDefinition*>::iterator it = mBeacons.begin(); it != mBeacons.end(); ++it)
    {
        (*it)->deactivateCustomBeacon(level);
    }
}

void CheckpointRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_id, xml);
    if (mElytraBoost > -1.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_speedBoost, xml);
    }
    NamedAreaRuleDefinition::WriteAttributesAsXML(xml);
}

ConsoleGameRules::EGameRuleType CheckpointRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_Checkpoint;
}
