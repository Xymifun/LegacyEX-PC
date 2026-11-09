#include "game/core/MinecartHopper.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>

#include <boost/pointer_cast.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/HopperMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HopperBlockEntity.hpp"
#include "game/world/generation/Player.hpp"

MinecartHopper::MinecartHopper(Level* level) : MinecartContainer(level), Hopper()
{
    _init();
}

MinecartHopper::MinecartHopper(Level* level, double x, double y, double z) : MinecartContainer(level, x, y, z), Hopper()
{
    _init();
}

MinecartHopper::~MinecartHopper()
{
}

Entity* MinecartHopper::create(Level* level)
{
    return new MinecartHopper(level);
}

unsigned int MinecartHopper::getContainerSize()
{
    return 5;
}

not_null_ptr<ItemInstance> MinecartHopper::getItem(unsigned int slot)
{
    return MinecartContainer::getItem(slot);
}

not_null_ptr<ItemInstance> MinecartHopper::removeItem(unsigned int slot, int count)
{
    return MinecartContainer::removeItem(slot, count);
}

not_null_ptr<ItemInstance> MinecartHopper::removeItemNoUpdate(int slot)
{
    return MinecartContainer::removeItemNoUpdate(slot);
}

void MinecartHopper::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    MinecartContainer::setItem(slot, item);
}

int MinecartHopper::getMaxStackSize()
{
    return MinecartContainer::getMaxStackSize();
}

void MinecartHopper::setChanged()
{
    (void)mItems;
}

bool MinecartHopper::stillValid(boost::shared_ptr<Player> player)
{
    return MinecartContainer::stillValid(player);
}

void MinecartHopper::startOpen(boost::shared_ptr<Player> player)
{
    MinecartContainer::startOpen(player);
}

void MinecartHopper::stopOpen(boost::shared_ptr<Player> player)
{
    MinecartContainer::stopOpen(player);
}

bool MinecartHopper::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return MinecartContainer::canPlaceItem(slot, item);
}

AbstractContainerMenu* MinecartHopper::createMenu(boost::shared_ptr<Inventory> inventory,
                                                  boost::shared_ptr<Player> player)
{
    return new HopperMenu(inventory, boost::static_pointer_cast<MinecartHopper>(shared_from_this()), player);
}

int MinecartHopper::getTypeId(bool large)
{
    (void)large;
    return ClientboundContainerOpenPacket::HOPPER;
}

Level* MinecartHopper::getLevel()
{
    return mLevel;
}

double MinecartHopper::getLevelX()
{
    return mX;
}

double MinecartHopper::getLevelY()
{
    return mY + 0.5;
}

double MinecartHopper::getLevelZ()
{
    return mZ;
}

void MinecartHopper::_init()
{
    fjDerivedCtorCalls();
    mPosition = BlockPos::ZERO;
    mEnabled = true;
    mCooldown = -1;
}

void MinecartHopper::addDataWalkers(DataFixerUpper* fixer)
{
    MinecartContainer::addDataWalkers(fixer, EntityType::MINECART_HOPPER);
}

const Minecart::Type* MinecartHopper::getType()
{
    return Minecart::Type::HOPPER;
}

const BlockState* MinecartHopper::getDefaultDisplayBlockState()
{
    return Blocks::hopper->defaultBlockState();
}

int MinecartHopper::getDefaultDisplayOffset()
{
    return 1;
}

bool MinecartHopper::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (!player->isAllowedToInteract(shared_from_this()))
        return false;
    if (!mLevel->mIsClientSide)
    {
        player->openContainer(boost::static_pointer_cast<MinecartHopper>(shared_from_this()));
    }
    return true;
}

void MinecartHopper::activateMinecart(int x, int y, int z, bool powered)
{
    bool enabled = !powered;
    if (isEnabled() != enabled)
        setEnabled(enabled);
}

bool MinecartHopper::isEnabled()
{
    return mEnabled;
}

void MinecartHopper::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

void MinecartHopper::setCooldown(int cooldown)
{
    mCooldown = cooldown;
}

bool MinecartHopper::isOnCooldown()
{
    return mCooldown > 0;
}

void MinecartHopper::tick()
{
    PIXBeginNamedEvent(0.0f, "Minecart hopper tick");
    Minecart::tick();
    if (!mLevel->mIsClientSide && isAlive() && isEnabled())
    {
        BlockPos position(shared_from_this());
        if (position == mPosition)
            --mCooldown;
        else
            setCooldown(0);
        if (!isOnCooldown())
        {
            setCooldown(0);
            if (suckInItems())
                setCooldown(4);
        }
    }
    PIXEndNamedEvent();
}

bool MinecartHopper::suckInItems()
{
    if (HopperBlockEntity::suckInItems(this))
        return true;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(
        typeid(ItemEntity), mBoundingBox->grow(0.25, 0.0, 0.25), EntitySelector::ENTITY_STILL_ALIVE, false);
    if (!entities->empty())
    {
        boost::shared_ptr<ItemEntity> item = boost::static_pointer_cast<ItemEntity>(entities->at(0));
        HopperBlockEntity::addItem(NULL, this, item);
    }
    delete entities;
    return false;
}

void MinecartHopper::destroy(const DamageSource* source)
{
    MinecartContainer::destroy(source);
    if (mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
        spawnAtLocation(Item::byBlock(Blocks::hopper), 1, 0.0f);
}

void MinecartHopper::addAdditonalSaveData(CompoundTag* tag)
{
    MinecartContainer::addAdditonalSaveData(tag);
    tag->putInt(L"TransferCooldown", mCooldown);
    tag->putBoolean(L"Enabled", mEnabled);
}

void MinecartHopper::readAdditionalSaveData(CompoundTag* tag)
{
    MinecartContainer::readAdditionalSaveData(tag);
    mCooldown = tag->getInt(L"TransferCooldown");
    mEnabled = tag->contains(L"Enabled") ? tag->getBoolean(L"Enabled") : true;
}

unsigned long long MinecartHopper::GetType()
{
    return EntityType::MINECART_HOPPER;
}
