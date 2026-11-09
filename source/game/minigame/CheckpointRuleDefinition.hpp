#ifndef GAME_MINIGAME_CHECKPOINTRULEDEFINITION_HPP
#define GAME_MINIGAME_CHECKPOINTRULEDEFINITION_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/minigame/NamedAreaRuleDefinition.hpp"

class CustomBeaconRuleDefinition;
class ServerLevel;
class ServerPlayer;
class UpdatePlayerRuleDefinition;

class CheckpointRuleDefinition : public NamedAreaRuleDefinition
{
public:
    CheckpointRuleDefinition();
    CheckpointRuleDefinition(int x, int y, int z, int xRot, int yRot, int sequenceId);
    virtual ~CheckpointRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children, ConsoleGameRules::EGameRuleType type);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void WriteXMLData(std::string& xml);
    virtual void WriteAttributesAsXML(std::string& xml);

    uint32_t getSequenceID();
    void setPlayerPosition(boost::shared_ptr<ServerPlayer> player);
    double getElytraBoost();
    UpdatePlayerRuleDefinition* GetSpawnPoint();
    bool hasSpawnPoint();
    void setSpawnPoint(boost::shared_ptr<Player> player);
    void processBeacons(ServerLevel* level);
    void deactivateBeacons(ServerLevel* level);
    std::vector<CustomBeaconRuleDefinition*>* getBeacons();
    void addBeacon(CustomBeaconRuleDefinition* beacon);

private:
    UpdatePlayerRuleDefinition* mSpawnPoint;
    int mSequenceID;
    double mElytraBoost;
    std::vector<CustomBeaconRuleDefinition*> mBeacons;
};

#endif
