#include "game/core/MinecartChest.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <boost/pointer_cast.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Direction.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

MinecartChest::MinecartChest(Level* level) : MinecartContainer(level)
{
    fjDerivedCtorCalls();
}

MinecartChest::MinecartChest(Level* level, double x, double y, double z) : MinecartContainer(level, x, y, z)
{
    fjDerivedCtorCalls();
}

MinecartChest::~MinecartChest()
{
}

Entity* MinecartChest::create(Level* level)
{
    return new MinecartChest(level);
}

unsigned int MinecartChest::getContainerSize()
{
    return 27;
}

AbstractContainerMenu* MinecartChest::createMenu(boost::shared_ptr<Inventory> inventory,
                                                 boost::shared_ptr<Player> player)
{
    unpackLootTable(player);

    return new ChestMenu(boost::dynamic_pointer_cast<Container>(inventory),
                         boost::static_pointer_cast<MinecartChest>(shared_from_this()), player);
}

int MinecartChest::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::CONTAINER;
}

void MinecartChest::addDataWalkers(DataFixerUpper* fixer)
{
    MinecartContainer::addDataWalkers(fixer, EntityType::MINECART_CHEST);
}

unsigned int MinecartChest::getContainerType()
{
    return 16;
}

void MinecartChest::destroy(const DamageSource* source)
{
    MinecartContainer::destroy(source);

    if (mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
    {
        spawnAtLocation(Item::byBlock(Blocks::chest), 1, 0.0f);
    }
}

const Minecart::Type* MinecartChest::getType()
{
    return Minecart::Type::CHEST;
}

const BlockState* MinecartChest::getDefaultDisplayBlockState()
{
    return Blocks::chest->defaultBlockState()->setValue(ChestBlock::FACING, Direction::NORTH);
}

int MinecartChest::getDefaultDisplayOffset()
{
    return 8;
}

unsigned long long MinecartChest::GetType()
{
    return EntityType::MINECART_CHEST;
}
