#include "game/item/ItemFrame.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cstring>
#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/core/GenericStats.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Minecraft.hpp"

const EntityDataAccessor<not_null_ptr<ItemInstance> >* ItemFrame::DATA_ITEM = NULL;
const EntityDataAccessor<int>* ItemFrame::DATA_ROTATION = NULL;

ItemFrame::ItemFrame(Level* level) : HangingEntity(level)
{
    Entity::fjDerivedCtorCalls();
    _init();
}

ItemFrame::ItemFrame(Level* level, const BlockPos& pos, const Direction* direction) : HangingEntity(level, pos)
{
    Entity::fjDerivedCtorCalls();
    _init();
    setDirection(direction);
}

ItemFrame::~ItemFrame()
{
}

Entity* ItemFrame::create(Level* level)
{
    return new ItemFrame(level);
}

void ItemFrame::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity, new ItemDataWalker(EntityType::ITEM_FRAME, L"Item"));
}

void ItemFrame::staticCtor()
{
    DATA_ITEM = SynchedEntityData::defineId(EntityType::ITEM_FRAME, EntityType::HANGING_ENTITY,
                                            EntityDataSerializers::ITEM_INSTANCE);
    DATA_ROTATION = SynchedEntityData::defineId(EntityType::ITEM_FRAME, EntityType::HANGING_ENTITY,
                                                EntityDataSerializers::INT);
}

void ItemFrame::_init()
{
    mDropChance = 1.0f;
}

void ItemFrame::defineSynchedData()
{
    getEntityData()->define(DATA_ITEM, ItemInstance::EMPTY);
    getEntityData()->define(DATA_ROTATION, 0);
}

float ItemFrame::getPickRadius()
{
    return 0.0f;
}

not_null_ptr<ItemInstance> ItemFrame::getItem()
{
    return getEntityData()->get(DATA_ITEM);
}

void ItemFrame::setItem(not_null_ptr<ItemInstance> item, bool updateNeighbours)
{
    if (!item->isEmpty())
    {
        item = item->copy();
        item->setCount(1);
        item->setFramed(boost::static_pointer_cast<ItemFrame>(shared_from_this()));
    }
    getEntityData()->set(DATA_ITEM, item, true);
    getEntityData()->markDirty(DATA_ITEM);
    if (!item->isEmpty())
        playSound(SoundEvents::ITEMFRAME_ADD_ITEM, 1.0f, 1.0f);
    if (updateNeighbours)
    {
        mLevel->updateNeighbourForOutputSignal(mPos, Blocks::air);
    }
}

void ItemFrame::setItem(not_null_ptr<ItemInstance> item)
{
    setItem(item, true);
}

bool ItemFrame::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
    {
        return false;
    }
    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker && eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType())
        && !boost::dynamic_pointer_cast<Player>(attacker)->isAllowedToHurtEntity(shared_from_this()))
    {
        return false;
    }
    if (!source->isExplosion() && !getItem()->isEmpty())
    {
        if (!mLevel->mIsClientSide)
        {
            dropItem(source->getEntity(), false);
            playSound(SoundEvents::ITEMFRAME_REMOVE_ITEM, 1.0f, 1.0f);
            setItem(ItemInstance::EMPTY);
        }
        return true;
    }
    return HangingEntity::hurt(source, damage);
}

int ItemFrame::getWidth()
{
    return 12;
}

int ItemFrame::getHeight()
{
    return 12;
}

bool ItemFrame::shouldRenderAtSqrDistance(double distanceSqr)
{
    return HangingEntity::shouldRenderAtSqrDistance(distanceSqr);
}

void ItemFrame::dropItem(boost::shared_ptr<Entity> breaker)
{
    playSound(SoundEvents::ITEMFRAME_BREAK, 1.0f, 1.0f);
    dropItem(breaker, true);
}

void ItemFrame::removeFramedMap(not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty())
    {
        Item* type = item->getItem();
        if (type == Items::filledMap)
        {
            boost::shared_ptr<MapItemSavedData> data =
                static_cast<MapItem*>(Items::filledMap)->getSavedData(item, mLevel);
            boost::shared_ptr<ItemFrame> frame = item->getFrame();
            if (frame)
            {
                data->fjRemoveDecorationByEntityId(frame->getId());
            }
        }
        item->setFramed(boost::shared_ptr<ItemFrame>());
    }
}

void ItemFrame::dropItem(boost::shared_ptr<Entity> breaker, bool dropSelf)
{
    if (!mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
    {
        return;
    }
    mLevel->updateObservingBlocksAt(mPos, Blocks::air);
    not_null_ptr<ItemInstance> item = getItem();
    if (breaker && eTYPE_DERIVED_FROM(EntityType::PLAYER, breaker->GetType()))
    {
        if (boost::static_pointer_cast<Player>(breaker)->mAbilities.mInstabuild)
        {
            removeFramedMap(item);
            return;
        }
    }
    if (dropSelf)
    {
        spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Items::itemFrame)), 0.0f);
    }
    if (!item->isEmpty() && mRandom->nextFloat() < mDropChance)
    {
        removeFramedMap(item);
        item = item->copy();
        spawnAtLocation(item, 0.0f);
    }
}

void ItemFrame::playPlacementSound()
{
    playSound(SoundEvents::ITEMFRAME_PLACE, 1.0f, 1.0f);
}

void ItemFrame::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor->getId() == DATA_ITEM->getId())
    {
        not_null_ptr<ItemInstance> item = getItem();
        if (!item->isEmpty() && item->getFrame() != shared_from_this())
        {
            item->setFramed(boost::static_pointer_cast<ItemFrame>(shared_from_this()));
        }
    }
}

int ItemFrame::getRotation()
{
    return getEntityData()->get(DATA_ROTATION);
}

void ItemFrame::setRotation(int rotation, bool update)
{
    getEntityData()->set(DATA_ROTATION, rotation % 8, true);
    if (update)
    {
        mLevel->updateNeighbourForOutputSignal(mPos, Blocks::air);
    }
}

void ItemFrame::setRotation(int rotation)
{
    setRotation(rotation, true);
}

void ItemFrame::addAdditonalSaveData(CompoundTag* tag)
{
    if (!getItem()->isEmpty())
    {
        tag->put(L"Item", getItem()->save(new CompoundTag()));
        tag->putByte(L"ItemRotation", (unsigned char)getRotation());
        tag->putFloat(L"ItemDropChance", mDropChance);
    }
    HangingEntity::addAdditonalSaveData(tag);
}

void ItemFrame::readAdditionalSaveData(CompoundTag* tag)
{
    CompoundTag* itemTag = tag->fjSafeGetCompoundTag(L"Item");
    if (itemTag != NULL && !itemTag->isEmpty())
    {
        setItem(not_null_ptr<ItemInstance>(new ItemInstance(itemTag)), false);
        setRotation(tag->getByte(L"ItemRotation"), false);
        if (tag->contains(L"ItemDropChance", Tag::TAG_ANY_NUMERIC))
            mDropChance = tag->getFloat(L"ItemDropChance");
        if (tag->contains(L"Direction"))
            setRotation(getRotation() * 2, false);
    }
    HangingEntity::readAdditionalSaveData(tag);
}

boost::shared_ptr<ItemFrame> ItemFrame::fjGetItemFrameForPos(Level* level, const BlockPos& pos)
{
    AABB* area = AABB::newTemp(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)->expand(0.5, 0.5, 0.5)->move(pos);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(ItemFrame), area);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<ItemFrame> frame = boost::static_pointer_cast<ItemFrame>(*it);
        if (frame->getPos() == pos)
        {
            delete entities;
            return frame;
        }
    }
    delete entities;
    return boost::shared_ptr<ItemFrame>();
}

bool ItemFrame::fjTryAwardMapRoom(const boost::shared_ptr<Player>& player)
{
    struct MapRoomMatch
    {
        bool valid;
        int xCenter;
        int zCenter;
        int rotation;
    };

    if (mLevel->mDimension->isNaturalDimension() && getItem()->getItem() == Items::filledMap)
    {
        MapRoomMatch match;
        std::memset(&match, 0, sizeof(MapRoomMatch));
        const Direction* direction = getDirection();
        BlockPos adjacentPos = mPos.relative(direction->getCounterClockWise(), 0).relative(Direction::UP, 0);
        {
            boost::shared_ptr<ItemFrame> adjacent = fjGetItemFrameForPos(mLevel, adjacentPos);
            if (adjacent && adjacent->getDirection() == getDirection()
                && adjacent->getItem()->getItem() == Items::filledMap)
            {
                boost::shared_ptr<MapItemSavedData> data =
                    static_cast<MapItem*>(Items::filledMap)->getSavedData(adjacent->getItem(), mLevel);
                if (data->mDimension == 0 && data->mScale == 3 && data->fjIsCompletelyExplored())
                {
                    MapRoomMatch candidate = {true, 0, 0, 0};
                    candidate.xCenter = data->mXCenter;
                    candidate.zCenter = data->mZCenter;
                    candidate.rotation = adjacent->getRotation() % 4;
                    match = candidate;
                }
            }
        }
        if (match.valid)
        {
            player->awardStat(GenericStats::mapRoom(), GenericStats::param_noArgs());
        }
    }
    return false;
}

bool ItemFrame::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (!player->isAllowedToInteract(shared_from_this()))
    {
        return false;
    }
    not_null_ptr<ItemInstance> heldItem = player->getItemInHand(hand);
    if (!mLevel->mIsClientSide)
    {
        if (getItem()->isEmpty() && !Minecraft::InMiniGame(eMiniGameId_Any, true))
        {
            if (!heldItem->isEmpty())
            {
                setItem(heldItem);
                mLevel->updateObservingBlocksAt(BlockPos(this), Blocks::air);
                if (!player->mAbilities.mInstabuild)
                {
                    heldItem->shrink(1);
                }
                fjTryAwardMapRoom(player);
            }
        }
        else
        {
            playSound(SoundEvents::ITEMFRAME_ROTATE_ITEM, 1.0f, 1.0f);
            setRotation(getRotation() + 1);
            fjTryAwardMapRoom(player);
            mLevel->updateObservingBlocksAt(BlockPos(this), Blocks::air);
        }
    }
    return true;
}

unsigned int ItemFrame::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (!getItem()->isEmpty())
    {
        return IDS_TOOLTIPS_ROTATE;
    }
    if (!holder.m_pItemInstance->isEmpty() && !Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        return IDS_TOOLTIPS_PLACE;
    }
    return IDS_NONE;
}

unsigned int ItemFrame::GetHitTooltip(const EntityTooltipDataHolder& holder)
{
    return holder.m_bAllowedToHurt ? IDS_TOOLTIPS_MINE : IDS_NONE;
}

int ItemFrame::getAnalogOutput()
{
    if (getItem()->isEmpty())
    {
        return 0;
    }
    return getRotation() % 8 + 1;
}

unsigned long long ItemFrame::GetType()
{
    return EntityType::ITEM_FRAME;
}
