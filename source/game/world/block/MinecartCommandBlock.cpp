#include "game/world/block/MinecartCommandBlock.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/OverwritableWalker.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

const EntityDataAccessor<std::wstring>* MinecartCommandBlock::DATA_COMMAND_ID = NULL;
const EntityDataAccessor<std::wstring>* MinecartCommandBlock::DATA_LAST_OUTPUT_ID = NULL;

void MinecartCommandBlock::staticCtor()
{
    DATA_COMMAND_ID = SynchedEntityData::defineId<std::wstring>(EntityType::MINECART_COMMAND_BLOCK,
                                                                EntityType::MINECART, EntityDataSerializers::STRING);
    DATA_LAST_OUTPUT_ID = SynchedEntityData::defineId<std::wstring>(
        EntityType::MINECART_COMMAND_BLOCK, EntityType::MINECART, EntityDataSerializers::STRING);
}

void MinecartCommandBlock::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
    dataFixerUpper->addWalker(DataFixTypes::eDataFixTypes_Entity,
                              new OverwritableWalker(EntityType::MINECART_COMMAND_BLOCK, L"Control"));
}

void MinecartCommandBlock::defineSynchedData()
{
    Minecart::defineSynchedData();
}

BaseCommandBlock* MinecartCommandBlock::getCommandBlock()
{
    return mCommandBlock;
}

void MinecartCommandBlock::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    mCommandBlock->load(tag);
    getEntityData()->set(DATA_COMMAND_ID, getCommandBlock()->getCommand(), true);
}

void MinecartCommandBlock::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    mCommandBlock->save(tag);
}

const Minecart::Type* MinecartCommandBlock::getType()
{
    return Minecart::Type::COMMAND_BLOCK;
}

const BlockState* MinecartCommandBlock::getDefaultDisplayBlockState()
{
    return Blocks::command_block->defaultBlockState();
}

void MinecartCommandBlock::activateMinecart(int x, int y, int z, bool powered)
{
    if (powered && mTickCount - mLastActivatedTick >= 4)
    {
        getCommandBlock()->performCommand(mLevel);
        mLastActivatedTick = mTickCount;
    }
}

bool MinecartCommandBlock::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    mCommandBlock->usedBy(player);
    return false;
}

void MinecartCommandBlock::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    Entity::onSyncedDataUpdated(accessor);
    if (accessor->getId() == DATA_LAST_OUTPUT_ID->getId())
    {
        mCommandBlock->setLastOutput(getEntityData()->get(DATA_LAST_OUTPUT_ID));
    }
    else if (accessor->getId() == DATA_COMMAND_ID->getId())
    {
        mCommandBlock->setCommand(getEntityData()->get(DATA_COMMAND_ID));
    }
}

bool MinecartCommandBlock::onlyOpCanSetNbt()
{
    return true;
}

MinecartCommandBlock::MinecartBaseCommandBlock::MinecartBaseCommandBlock(
    const boost::shared_ptr<MinecartCommandBlock>& minecart)
    : BaseCommandBlock(),
      mMinecart(minecart)
{
}

void MinecartCommandBlock::MinecartBaseCommandBlock::onUpdated()
{
    boost::shared_ptr<MinecartCommandBlock> minecart(mMinecart.lock());
    minecart->getEntityData()->set(MinecartCommandBlock::DATA_COMMAND_ID, getCommand(), true);
    minecart->getEntityData()->set(MinecartCommandBlock::DATA_LAST_OUTPUT_ID, getLastOutput(), true);
}

unsigned int MinecartCommandBlock::MinecartBaseCommandBlock::getType()
{
    return 1;
}

void MinecartCommandBlock::MinecartBaseCommandBlock::writeCoordinates(DataOutputStream* output)
{
    output->writeInt(mMinecart.lock()->getId());
}

BlockPos MinecartCommandBlock::MinecartBaseCommandBlock::getCommandSenderBlockPosition()
{
    boost::shared_ptr<MinecartCommandBlock> minecart(mMinecart.lock());
    return BlockPos(Vec3::newTemp(minecart->mX, minecart->mY + 0.5, minecart->mZ));
}

Vec3* MinecartCommandBlock::MinecartBaseCommandBlock::getCommandSenderWorldPosition()
{
    boost::shared_ptr<MinecartCommandBlock> minecart(mMinecart.lock());
    return Vec3::newTemp(minecart->mX, minecart->mY, minecart->mZ);
}

Level* MinecartCommandBlock::MinecartBaseCommandBlock::getCommandSenderWorld()
{
    return mMinecart.lock()->mLevel;
}

boost::shared_ptr<Entity> MinecartCommandBlock::MinecartBaseCommandBlock::getCommandSenderEntity()
{
    boost::shared_ptr<MinecartCommandBlock> minecart(mMinecart.lock());
    return boost::shared_ptr<Entity>(minecart);
}

MinecraftServer* MinecartCommandBlock::MinecartBaseCommandBlock::getServer()
{
    return mMinecart.lock()->mLevel->getServer();
}
