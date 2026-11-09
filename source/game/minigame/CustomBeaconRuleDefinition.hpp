#ifndef GAME_MINIGAME_CUSTOMBEACONRULEDEFINITION_HPP
#define GAME_MINIGAME_CUSTOMBEACONRULEDEFINITION_HPP

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"

class AABB;
class GameRuleSaveInterface;
class MutableBlockPos;
class ServerLevel;

class CustomBeaconRuleDefinition : public GameRuleDefinition
{
public:
    CustomBeaconRuleDefinition();
    CustomBeaconRuleDefinition(int x, int y, int z);
    virtual ~CustomBeaconRuleDefinition();

    void processCustomBeacon(ServerLevel* level);
    void deactivateCustomBeacon(ServerLevel* level);
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    int getX();
    int getY();
    int getZ();
    virtual AABB* getBoundingVolume();
    virtual void moveBoundingVolume(int x, int y, int z);
    virtual ConsoleGameRules::EGameRuleType getActionType();
    BeaconBlockEntity::eBEAM_DIRECTION getDirection();
    void setDirection(BeaconBlockEntity::eBEAM_DIRECTION direction);
    int getLength();
    void setLength(int length);

private:
    int mLength;
    MutableBlockPos* mPosition;
    BeaconBlockEntity::eBEAM_DIRECTION mDirection;
};

#endif
