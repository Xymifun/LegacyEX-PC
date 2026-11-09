#include "game/world/block/DropperBlockEntity.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/ItemListDataWalker.hpp"

DropperBlockEntity::~DropperBlockEntity()
{
}

BlockEntity* DropperBlockEntity::create()
{
    return new DropperBlockEntity();
}

int DropperBlockEntity::getTypeId(bool)
{
    return ClientboundContainerOpenPacket::DROPPER;
}

void DropperBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::DROPPER, L"Items"));
}

boost::shared_ptr<BlockEntity> DropperBlockEntity::clone()
{
    boost::shared_ptr<DropperBlockEntity> result(new DropperBlockEntity());
    BlockEntity::clone(result);
    result->mCustomName->set(mCustomName->raw(), std::wstring(mCustomName->getUUID()), mCustomName->isChatCensored(),
                             mCustomName->isUUIDLocal());
    return result;
}

unsigned long long DropperBlockEntity::GetType()
{
    return BlockEntityType::DROPPER;
}
