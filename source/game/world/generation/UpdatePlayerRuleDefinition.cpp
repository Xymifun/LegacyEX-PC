#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"

#include "game/core/StringConversion.hpp"

#include <algorithm>
#include <string>

#include "game/core/FoodData.hpp"
#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/IGameRuleAttribute.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/AddItemRuleDefinition.hpp"
#include "game/item/Inventory.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/core/StringFormat.hpp"

UpdatePlayerRuleDefinition::UpdatePlayerRuleDefinition()
{
    mHealth = 0;
    mFoodLevel = 0;
    mSpawnPosition = NULL;
    mDimension = 0;
    mYRot = 0.0f;
    mXRot = 0.0f;

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_dimension, &mDimension);
    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_yRot, &mYRot)->setOptional(true);
    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_xRot, &mXRot)->setOptional(true);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_food, &mHealth)->setOptional(true);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_health, &mFoodLevel)->setOptional(true);
}

UpdatePlayerRuleDefinition::UpdatePlayerRuleDefinition(boost::shared_ptr<Player> player)
{
    mHealth = 0;
    mFoodLevel = 0;
    mDimension = 0;
    mSpawnPosition = new MutableBlockPos();
    mSpawnPosition->setX(static_cast<int>(player->mX));
    mSpawnPosition->setY(static_cast<int>(player->mY));
    mSpawnPosition->setZ(static_cast<int>(player->mZ));
    mYRot = player->mYRot;
    mXRot = player->mXRot;
}

UpdatePlayerRuleDefinition::~UpdatePlayerRuleDefinition()
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        delete *it;
    }
    delete mSpawnPosition;
}

double UpdatePlayerRuleDefinition::SanitisedYPos(double x, double y, double z, boost::shared_ptr<Player> player) const
{
    ServerLevel* level = gMinecraftServer->getLevel(0);
    double testY = y;
    double halfHeight = static_cast<double>(player->mBbHeight) * 0.5;

    while (level->getBlockState(BlockPos(x, testY + halfHeight, z))->getMaterial() != Material::AIR)
    {
        testY -= 0.1;
    }

    while (level->getBlockState(BlockPos(x, testY - halfHeight, z))->getMaterial() != Material::AIR)
    {
        testY += 0.1;
    }

    if (std::max(y, testY) - std::min(y, testY) > 5.0)
    {
        return y;
    }
    return testY;
}

void UpdatePlayerRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* UpdatePlayerRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* rule = NULL;
    if (type == ConsoleGameRules::eGameRuleType_AddItem)
    {
        rule = new AddItemRuleDefinition();
        mItems.push_back(static_cast<AddItemRuleDefinition*>(rule));
    }
    return rule;
}

void UpdatePlayerRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 3);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnX, _toString<int>(mSpawnPosition->getX()));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnY, _toString<int>(mSpawnPosition->getY()));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnZ, _toString<int>(mSpawnPosition->getZ()));
}

void UpdatePlayerRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"spawnX") == 0)
    {
        if (mSpawnPosition == NULL)
        {
            mSpawnPosition = new MutableBlockPos();
        }
        mSpawnPosition->setX(_fromString<int>(value));
        return;
    }
    if (name.compare(L"spawnY") == 0)
    {
        if (mSpawnPosition == NULL)
        {
            mSpawnPosition = new MutableBlockPos();
        }
        mSpawnPosition->setY(_fromString<int>(value));
        return;
    }
    if (name.compare(L"spawnZ") == 0)
    {
        if (mSpawnPosition == NULL)
        {
            mSpawnPosition = new MutableBlockPos();
        }
        mSpawnPosition->setZ(_fromString<int>(value));
        return;
    }
    GameRuleDefinition::addAttribute(name, value);
}

void UpdatePlayerRuleDefinition::postProcessPlayer(boost::shared_ptr<Player> player)
{
    if (attributeHasValue(ConsoleGameRules::eGameRuleAttr_health))
    {
        player->setHealth(static_cast<float>(mHealth));
    }
    if (attributeHasValue(ConsoleGameRules::eGameRuleAttr_food))
    {
        player->getFoodData()->setFoodLevel(mFoodLevel);
    }

    double x = player->mX;
    double y = player->mY;
    double z = player->mZ;
    float yRot = player->mYRot;
    float xRot = player->mXRot;

    if (mSpawnPosition != NULL)
    {
        player->mDimension = mDimension;
        x = static_cast<double>(mSpawnPosition->getX()) + 0.5;
        z = static_cast<double>(mSpawnPosition->getZ()) + 0.5;

        if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
        {
            y = static_cast<double>(mSpawnPosition->getY());
        }
        else if (Minecraft::InMiniGame(eMiniGameId_Lobby, true) || Minecraft::InMiniGame(eMiniGameId_Battle, true))
        {
            y = SanitisedYPos(x, mSpawnPosition->getY() + 0.5 + player->mBbHeight * 0.5f, z, player);
        }
        else
        {
            y = SanitisedYPos(x, static_cast<double>(mSpawnPosition->getY()), z, player);
        }
    }

    if (attributeHasValue(ConsoleGameRules::eGameRuleAttr_yRot))
    {
        yRot = mYRot;
    }
    if (attributeHasValue(ConsoleGameRules::eGameRuleAttr_xRot))
    {
        xRot = mXRot;
    }
    if (mSpawnPosition != NULL || attributeHasValue(ConsoleGameRules::eGameRuleAttr_yRot)
        || attributeHasValue(ConsoleGameRules::eGameRuleAttr_xRot))
    {
        player->absMoveTo(x, y, z, yRot, xRot);
    }

    for (std::vector<AddItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        AddItemRuleDefinition* item = *it;
        item->addItemToContainer(player->mInventory, -1);
    }
}

int UpdatePlayerRuleDefinition::getX() const
{
    return mSpawnPosition->x;
}

int UpdatePlayerRuleDefinition::getY() const
{
    return mSpawnPosition->y;
}

int UpdatePlayerRuleDefinition::getZ() const
{
    return mSpawnPosition->z;
}

float UpdatePlayerRuleDefinition::getXRot() const
{
    return mXRot;
}

float UpdatePlayerRuleDefinition::getYRot() const
{
    return mYRot;
}

void UpdatePlayerRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    xml.append(formatstr(" spawnX=\"%d\"", mSpawnPosition->getX()));
    xml.append(formatstr(" spawnY=\"%d\"", mSpawnPosition->getY()));
    xml.append(formatstr(" spawnZ=\"%d\"", mSpawnPosition->getZ()));

    if (attributeHasValue(ConsoleGameRules::eGameRuleAttr_yRot))
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_yRot, xml);
    }
    if (attributeHasValue(ConsoleGameRules::eGameRuleAttr_xRot))
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_xRot, xml);
    }
    if (attributeHasValue(ConsoleGameRules::eGameRuleAttr_food))
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_food, xml);
    }
    if (attributeHasValue(ConsoleGameRules::eGameRuleAttr_health))
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_health, xml);
    }
}

ConsoleGameRules::EGameRuleType UpdatePlayerRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_UpdatePlayer;
}
