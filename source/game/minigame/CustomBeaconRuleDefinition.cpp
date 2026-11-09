#include "game/minigame/CustomBeaconRuleDefinition.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/IGameRuleAttribute.hpp"
#include "game/core/StringConversion.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/PlayerList.hpp"

static const wchar_t* sBeamDirectionNames[7] = {L"none", L"plus_y", L"minus_y", L"plus_x", L"minus_x", L"plus_z", L"minus_z"};

CustomBeaconRuleDefinition::CustomBeaconRuleDefinition()
{
    mPosition = NULL;
    mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NONE;
    mLength = 0;
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_beam_length, &mLength);
}

CustomBeaconRuleDefinition::~CustomBeaconRuleDefinition()
{
    delete mPosition;
}

void CustomBeaconRuleDefinition::processCustomBeacon(ServerLevel* level)
{
    BlockPos position(mPosition->getX(), mPosition->getY(), mPosition->getZ());
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::dynamic_pointer_cast<BeaconBlockEntity>(
        level->getBlockEntity(position));
    if (beacon && mDirection != BeaconBlockEntity::eBEAM_DIRECTION_NONE)
    {
        beacon->setBeamDirection(mDirection);
        beacon->setBeamLength(mLength);
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->broadcastAll(beacon->getUpdatePacket());
    }
}

void CustomBeaconRuleDefinition::deactivateCustomBeacon(ServerLevel* level)
{
    BlockPos position(mPosition->getX(), mPosition->getY(), mPosition->getZ());
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::dynamic_pointer_cast<BeaconBlockEntity>(
        level->getBlockEntity(position));
    if (beacon && mDirection != BeaconBlockEntity::eBEAM_DIRECTION_NONE)
    {
        beacon->setBeamDirection(BeaconBlockEntity::eBEAM_DIRECTION_NONE);
        beacon->setBeamLength(0);
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->broadcastAll(beacon->getUpdatePacket());
    }
}

void CustomBeaconRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 3);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnX, _toString(mPosition->getX()));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnY, _toString(mPosition->getY()));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnZ, _toString(mPosition->getZ()));
}

void CustomBeaconRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(gRuleAttributeNames[ConsoleGameRules::eGameRuleAttr_spawnX]) == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos();
        }
        int x = _fromString<int>(value);
        mPosition->setX(x);
        return;
    }
    if (name.compare(gRuleAttributeNames[ConsoleGameRules::eGameRuleAttr_spawnY]) == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos();
        }
        int y = _fromString<int>(value);
        mPosition->setY(y);
        return;
    }
    if (name.compare(gRuleAttributeNames[ConsoleGameRules::eGameRuleAttr_spawnZ]) == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos();
        }
        int z = _fromString<int>(value);
        mPosition->setZ(z);
        return;
    }
    if (name.compare(gRuleAttributeNames[ConsoleGameRules::eGameRuleAttr_beam_direction]) == 0)
    {
        if (value.compare(sBeamDirectionNames[BeaconBlockEntity::eBEAM_DIRECTION_NONE]) == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NONE;
        }
        else if (value.compare(sBeamDirectionNames[BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_Y]) == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_Y;
        }
        else if (value.compare(sBeamDirectionNames[BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_Y]) == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_Y;
        }
        else if (value.compare(sBeamDirectionNames[BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_X]) == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_X;
        }
        else if (value.compare(sBeamDirectionNames[BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_X]) == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_X;
        }
        else if (value.compare(sBeamDirectionNames[BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_Z]) == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_Z;
        }
        else if (value.compare(sBeamDirectionNames[BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_Z]) == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_Z;
        }
        return;
    }
    GameRuleDefinition::addAttribute(name, value);
}

int CustomBeaconRuleDefinition::getX()
{
    return mPosition->x;
}

int CustomBeaconRuleDefinition::getY()
{
    return mPosition->y;
}

int CustomBeaconRuleDefinition::getZ()
{
    return mPosition->z;
}

AABB* CustomBeaconRuleDefinition::getBoundingVolume()
{
    return AABB::newTemp(getX(), getY(), getZ(), getX() + 1, getY() + 1, getZ() + 1);
}

void CustomBeaconRuleDefinition::moveBoundingVolume(int x, int y, int z)
{
    mPosition->setX(mPosition->getX() + x);
    mPosition->setY(mPosition->getY() + y);
    mPosition->setZ(mPosition->getZ() + z);
}

ConsoleGameRules::EGameRuleType CustomBeaconRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CustomBeacon;
}
