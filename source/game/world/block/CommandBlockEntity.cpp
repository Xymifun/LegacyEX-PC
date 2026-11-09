#include "game/world/block/CommandBlockEntity.hpp"

#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CommandBlock.hpp"

CommandBlockEntity::AnonymousCommandBlock::AnonymousCommandBlock(CommandBlockEntity* entity)
    : BaseCommandBlock(),
      mEntity(entity)
{
}

BlockPos CommandBlockEntity::AnonymousCommandBlock::getCommandSenderBlockPosition()
{
    return mEntity->mPos;
}

Vec3* CommandBlockEntity::AnonymousCommandBlock::getCommandSenderWorldPosition()
{
    return Vec3::newTemp((double)mEntity->mPos.getX() + 0.5, (double)mEntity->mPos.getY() + 0.5,
                         (double)mEntity->mPos.getZ() + 0.5);
}

Level* CommandBlockEntity::AnonymousCommandBlock::getCommandSenderWorld()
{
    return mEntity->getLevel();
}

void CommandBlockEntity::AnonymousCommandBlock::setCommand(const std::wstring& command)
{
    BaseCommandBlock::setCommand(command);
    mEntity->setChanged();
}

void CommandBlockEntity::AnonymousCommandBlock::onUpdated()
{
    const BlockState* state = mEntity->mLevel->getBlockState(mEntity->mPos);
    mEntity->getLevel()->sendBlockUpdated(mEntity->mPos, state, state, 3, true);
}

unsigned int CommandBlockEntity::AnonymousCommandBlock::getType()
{
    return 0;
}

void CommandBlockEntity::AnonymousCommandBlock::writeCoordinates(DataOutputStream* output)
{
    output->writeInt(mEntity->mPos.getX());
    output->writeInt(mEntity->mPos.getY());
    output->writeInt(mEntity->mPos.getZ());
}

CommandBlockEntity::CommandBlockEntity()
    : BlockEntity(),
      mPowered(false),
      mAutomatic(false),
      mConditionMet(false),
      mCommandBlock(new AnonymousCommandBlock(this))
{
}

CommandBlockEntity::CommandBlockEntity(CommandBlock::Mode mode)
    : BlockEntity(),
      mPowered(false),
      mAutomatic(mode == CommandBlock::CHAIN),
      mConditionMet(false),
      mCommandBlock(new AnonymousCommandBlock(this))
{
}

BlockEntity* CommandBlockEntity::create()
{
    return new CommandBlockEntity();
}

BaseCommandBlock* CommandBlockEntity::getCommandBlock()
{
    return mCommandBlock;
}

bool CommandBlockEntity::isPowered()
{
    return mPowered;
}

bool CommandBlockEntity::wasConditionMet()
{
    return mConditionMet;
}

bool CommandBlockEntity::isAutomatic()
{
    return mAutomatic;
}

CompoundTag* CommandBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCommandBlock->save(tag);
    tag->putBoolean(L"powered", isPowered());
    tag->putBoolean(L"conditionMet", wasConditionMet());
    tag->putBoolean(L"auto", isAutomatic());
    return tag;
}

void CommandBlockEntity::setAutomatic(bool automatic)
{
    mAutomatic = automatic;
}

void CommandBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCommandBlock->load(tag);
    mPowered = tag->getBoolean(L"powered");
    mConditionMet = tag->getBoolean(L"conditionMet");
    setAutomatic(tag->getBoolean(L"auto"));
}

boost::shared_ptr<Packet> CommandBlockEntity::getUpdatePacket()
{
    CompoundTag* tag = save(new CompoundTag());
    return boost::shared_ptr<ClientboundBlockEntityDataPacket>(
        new ClientboundBlockEntityDataPacket(mPos, 2, mLevel->mDimension->getType()->getId(), tag));
}

bool CommandBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

unsigned int CommandBlockEntity::getMode()
{
    Block* block = getBlock();
    if (block != Blocks::command_block)
    {
        if (block == Blocks::repeating_command_block)
        {
            return 1;
        }
        if (block == Blocks::chain_command_block)
        {
            return 0;
        }
    }
    return 2;
}

unsigned char CommandBlockEntity::isConditional()
{
    const BlockState* state = mLevel->getBlockState(getBlockPos());
    Block* block = state->getBlock();
    if (CommandBlock::isCommandBlock(block))
    {
        return state->getValue(CommandBlock::CONDITIONAL);
    }
    return 0;
}

void CommandBlockEntity::clearRemoved()
{
    mBlock = NULL;
    BlockEntity::clearRemoved();
}

boost::shared_ptr<BlockEntity> CommandBlockEntity::clone()
{
    boost::shared_ptr<CommandBlockEntity> result(new CommandBlockEntity());
    result->getCommandBlock()->setCommand(getCommandBlock()->getCommand());
    result->getCommandBlock()->setName(getCommandBlock()->getName(), true);
    result->setAutomatic(isAutomatic());
    BlockEntity::clone(result);
    return result;
}

void CommandBlockEntity::updateNamedPosition(const std::wstring& name, const std::wstring& value)
{
    std::wstring token = L"<";
    token += name;
    token += L">";
    std::wstring replacement = L"<";
    replacement += value;
    replacement += L">";
    std::wstring command = mCommandBlock->getCommand();
    std::wstring updated;
    std::wstring::size_type position = 0;
    bool changed = false;
    std::wstring::size_type found;
    while ((found = command.find(token, position)) != std::wstring::npos)
    {
        changed = true;
        updated.append(command, position, found - position);
        updated.append(replacement);
        position = found + token.length();
    }
    updated.append(command, position, std::wstring::npos);
    mCommandBlock->setCommand(updated);
    if (changed)
    {
        mCommandBlock->setCommand(updated);
    }
}

unsigned long long CommandBlockEntity::GetType()
{
    return BlockEntityType::COMMAND;
}
