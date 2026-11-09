#include "game/world/block/ComparatorBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include "game/data/CompoundTag.hpp"

BlockEntity* ComparatorBlockEntity::create()
{
    return new ComparatorBlockEntity();
}

unsigned int ComparatorBlockEntity::getOutputSignal()
{
    return (unsigned int)mOutputSignal;
}

void ComparatorBlockEntity::setOutputSignal(int signal)
{
    mOutputSignal = signal;
}

boost::shared_ptr<BlockEntity> ComparatorBlockEntity::clone()
{
    boost::shared_ptr<ComparatorBlockEntity> result(new ComparatorBlockEntity());
    BlockEntity::clone(result);
    result->mOutputSignal = mOutputSignal;
    return result;
}

CompoundTag* ComparatorBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putInt(L"OutputSignal", mOutputSignal);
    return tag;
}

void ComparatorBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mOutputSignal = tag->getInt(L"OutputSignal");
}

unsigned long long ComparatorBlockEntity::GetType()
{
    return BlockEntityType::COMPARATOR;
}
