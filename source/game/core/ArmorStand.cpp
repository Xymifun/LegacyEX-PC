#include "game/core/ArmorStand.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Minecart.hpp"

#include <cmath>

#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/DataWalker.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemId.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/Items.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/EntityDataAccessor_Base.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/Abilities.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

const EntityDataAccessor<unsigned char>* ArmorStand::DATA_FLAGS = NULL;
const EntityDataAccessor<int>* ArmorStand::DATA_POSE = NULL;

static ArmorStand::Pose POSE_DEFAULT(Vec3::newPermanent(0.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                     Vec3::newPermanent(-15.0, 0.0, 10.0), Vec3::newPermanent(-10.0, 0.0, -10.0),
                                     Vec3::newPermanent(1.0, 0.0, 1.0), Vec3::newPermanent(-1.0, 0.0, -1.0),
                                     Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_NONE(Vec3::newPermanent(0.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                  Vec3::newPermanent(0.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                  Vec3::newPermanent(0.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                  Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_SOLEMN(Vec3::newPermanent(15.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 2.0),
                                    Vec3::newPermanent(-60.0, -20.0, -10.0), Vec3::newPermanent(-30.0, 15.0, 15.0),
                                    Vec3::newPermanent(1.0, 0.0, 1.0), Vec3::newPermanent(-1.0, 0.0, -1.0),
                                    Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_ATHENA(Vec3::newPermanent(-5.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 2.0),
                                    Vec3::newPermanent(-60.0, 20.0, -10.0), Vec3::newPermanent(10.0, 0.0, -5.0),
                                    Vec3::newPermanent(3.0, 3.0, 3.0), Vec3::newPermanent(-3.0, -3.0, -3.0),
                                    Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_BRANDISH(Vec3::newPermanent(-15.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, -2.0),
                                      Vec3::newPermanent(-110.0, 50.0, 0.0), Vec3::newPermanent(20.0, 0.0, -10.0),
                                      Vec3::newPermanent(-5.0, 3.0, 3.0), Vec3::newPermanent(5.0, -3.0, -3.0),
                                      Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_HONOR(Vec3::newPermanent(-15.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                   Vec3::newPermanent(-110.0, -35.0, 0.0), Vec3::newPermanent(-110.0, 35.0, 0.0),
                                   Vec3::newPermanent(-5.0, 3.0, 3.0), Vec3::newPermanent(5.0, -3.0, -3.0),
                                   Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_ENTERTAIN(Vec3::newPermanent(-15.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                       Vec3::newPermanent(-110.0, 35.0, 0.0), Vec3::newPermanent(-110.0, -35.0, 0.0),
                                       Vec3::newPermanent(-5.0, 3.0, 3.0), Vec3::newPermanent(5.0, -3.0, -3.0),
                                       Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_SALUTE(Vec3::newPermanent(0.0, 0.0, 0.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                    Vec3::newPermanent(-70.0, -40.0, 0.0), Vec3::newPermanent(10.0, 0.0, -5.0),
                                    Vec3::newPermanent(1.0, 0.0, 1.0), Vec3::newPermanent(-1.0, 0.0, -1.0),
                                    Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_RIDDLER(Vec3::newPermanent(16.0, 20.0, 0.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                     Vec3::newPermanent(246.0, 0.0, 89.0), Vec3::newPermanent(4.0, 8.0, 237.0),
                                     Vec3::newPermanent(8.0, 20.0, 4.0), Vec3::newPermanent(-14.0, -18.0, -16.0),
                                     Vec3::newPermanent(0.0, 180.0, 0.0));

static ArmorStand::Pose POSE_ZOMBIE(Vec3::newPermanent(-10.0, 0.0, -5.0), Vec3::newPermanent(0.0, 0.0, 0.0),
                                    Vec3::newPermanent(-100.0, 0.0, 0.0), Vec3::newPermanent(-105.0, 0.0, 0.0),
                                    Vec3::newPermanent(-46.0, 0.0, 0.0), Vec3::newPermanent(7.0, 0.0, 0.0),
                                    Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_CANCAN_A(Vec3::newPermanent(-5.0, 18.0, 0.0), Vec3::newPermanent(0.0, 22.0, 0.0),
                                      Vec3::newPermanent(0.0, 84.0, 111.0), Vec3::newPermanent(8.0, 0.0, -114.0),
                                      Vec3::newPermanent(0.0, 23.0, -13.0), Vec3::newPermanent(-111.0, 55.0, 0.0),
                                      Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_CANCAN_B(Vec3::newPermanent(-10.0, -20.0, 0.0), Vec3::newPermanent(0.0, -18.0, 0.0),
                                      Vec3::newPermanent(8.0, 90.0, 111.0), Vec3::newPermanent(0.0, 0.0, -112.0),
                                      Vec3::newPermanent(-119.0, -42.0, 0.0), Vec3::newPermanent(0.0, 0.0, 13.0),
                                      Vec3::newPermanent(0.0, 0.0, 0.0));

static ArmorStand::Pose POSE_HERO(Vec3::newPermanent(-4.0, 67.0, 0.0), Vec3::newPermanent(0.0, 8.0, 0.0),
                                  Vec3::newPermanent(-99.0, 63.0, 0.0), Vec3::newPermanent(16.0, 32.0, -8.0),
                                  Vec3::newPermanent(4.0, 63.0, 8.0), Vec3::newPermanent(0.0, -75.0, -8.0),
                                  Vec3::newPermanent(0.0, 0.0, 0.0));

const ArmorStand::RidableMinecartEntitySelector* ArmorStand::IS_RIDEABLE_MINECART =
    new ArmorStand::RidableMinecartEntitySelector();

static ArmorStand::Pose* POSES[13] = {&POSE_DEFAULT,  &POSE_NONE,      &POSE_SOLEMN, &POSE_ATHENA,  &POSE_BRANDISH,
                                      &POSE_HONOR,    &POSE_ENTERTAIN, &POSE_SALUTE, &POSE_RIDDLER, &POSE_ZOMBIE,
                                      &POSE_CANCAN_A, &POSE_CANCAN_B,  &POSE_HERO};

ArmorStand::Pose::Pose(const Vec3* headPose, const Vec3* bodyPose, const Vec3* rightArmPose, const Vec3* leftArmPose,
                       const Vec3* rightLegPose, const Vec3* leftLegPose, const Vec3* rightItemPose)
    : head(const_cast<Vec3*>(headPose)),
      body(const_cast<Vec3*>(bodyPose)),
      leftArm(const_cast<Vec3*>(leftArmPose)),
      rightArm(const_cast<Vec3*>(rightArmPose)),
      leftLeg(const_cast<Vec3*>(leftLegPose)),
      rightLeg(const_cast<Vec3*>(rightLegPose)),
      rightItem(const_cast<Vec3*>(rightItemPose))
{
}


ArmorStand::ArmorStand(Level* level) : LivingEntity(level), mHandItems(), mArmorItems()
{
    _init();
}

ArmorStand::ArmorStand(Level* level, double x, double y, double z) : LivingEntity(level), mHandItems(), mArmorItems()
{
    _init();
    setPos(x, y, z);
}

ArmorStand::~ArmorStand()
{
    delete[] mHandItems.items.data;
    delete[] mArmorItems.items.data;
}

void ArmorStand::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId<unsigned char>(EntityType::ARMOR_STAND, EntityType::LIVING_ENTITY,
                                                            EntityDataSerializers::BYTE);
    DATA_POSE = SynchedEntityData::defineId<int>(EntityType::ARMOR_STAND, EntityType::LIVING_ENTITY,
                                                 EntityDataSerializers::INT);
}

void ArmorStand::addDataWalkers(DataFixerUpper* fixer)
{
    std::vector<std::wstring> itemLists;
    itemLists.push_back(L"ArmorItems");
    itemLists.push_back(L"HandItems");
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     (DataWalker*)new ItemListDataWalker(EntityType::ARMOR_STAND, itemLists));
}

void ArmorStand::_init()
{
    fjDerivedCtorCalls();
    mDisabledSlots = 0;
    mInvisible = false;
    mLastHit = 0;
    mWasMarker = false;
    mNoPhysics = isNoGravity();
    setSize(0.5f, 1.975f);
    mHandItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(2, ItemInstance::EMPTY);
    mArmorItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY);
    mLastSignal = 0;
    setShowArms(true);
}

unsigned char ArmorStand::setBit(unsigned char value, int bit, bool enabled)
{
    if (enabled)
    {
        value |= bit;
    }
    else
    {
        value &= ~bit;
    }
    return value;
}

void ArmorStand::setShowArms(bool showArms)
{
    mEntityData->set(DATA_FLAGS, setBit(mEntityData->get(DATA_FLAGS), 4, showArms), true);
}
NonNullList<not_null_ptr<ItemInstance> > ArmorStand::getHandSlots()
{
    return mHandItems;
}
NonNullList<not_null_ptr<ItemInstance> > ArmorStand::getArmorSlots()
{
    return mArmorItems;
}
not_null_ptr<ItemInstance> ArmorStand::getItemSlot(const EquipmentSlot* slot)
{
    switch (slot->getType())
    {
    case EquipmentSlot::HAND:
        return mHandItems[slot->getIndex()];
    case EquipmentSlot::ARMOR:
        return mArmorItems[slot->getIndex()];
    }
    return ItemInstance::EMPTY;
}
void ArmorStand::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    switch (slot->getType())
    {
    case EquipmentSlot::HAND:
        playEquipSound(item);
        mHandItems.set(slot->getIndex(), item);
        break;
    case EquipmentSlot::ARMOR:
        playEquipSound(item);
        mArmorItems.set(slot->getIndex(), item);
        break;
    }
}
bool ArmorStand::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    const EquipmentSlot* equipmentSlot;
    if (slot == 98)
    {
        equipmentSlot = EquipmentSlot::MAINHAND;
    }
    else if (slot == 99)
    {
        equipmentSlot = EquipmentSlot::OFFHAND;
    }
    else if (slot == EquipmentSlot::HEAD->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::HEAD;
    }
    else if (slot == EquipmentSlot::CHEST->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::CHEST;
    }
    else if (slot == EquipmentSlot::LEGS->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::LEGS;
    }
    else if (slot == EquipmentSlot::FEET->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::FEET;
    }
    else
    {
        return false;
    }

    if (item->isEmpty() || Mob::isValidSlotForItem(equipmentSlot, item) || equipmentSlot == EquipmentSlot::HEAD)
    {
        setItemSlot(equipmentSlot, item);
        return true;
    }
    return false;
}
bool ArmorStand::isEffectiveAi()
{
    return LivingEntity::isEffectiveAi() && !isNoGravity();
}
bool ArmorStand::isSmall()
{
    return (mEntityData->get(DATA_FLAGS) & 1) != 0;
}
bool ArmorStand::isNoBasePlate()
{
    return (mEntityData->get(DATA_FLAGS) & 8) != 0;
}
bool ArmorStand::isMarker()
{
    return (mEntityData->get(DATA_FLAGS) & 0x10) != 0;
}
int ArmorStand::getPoseIndex()
{
    return mEntityData->get(DATA_POSE);
}
void ArmorStand::addAdditonalSaveData(CompoundTag* tag)
{
    LivingEntity::addAdditonalSaveData(tag);
    ListTag* armorItems = new ListTag();
    for (unsigned int index = 0; index < 4; ++index)
    {
        CompoundTag* itemTag = new CompoundTag();
        if (!mArmorItems[index]->isEmpty())
        {
            mArmorItems[index]->save(itemTag);
        }
        armorItems->add(itemTag);
    }
    tag->put(L"ArmorItems", armorItems);

    ListTag* handItems = new ListTag();
    for (unsigned int index = 0; index < 2; ++index)
    {
        CompoundTag* itemTag = new CompoundTag();
        if (!mHandItems[index]->isEmpty())
        {
            mHandItems[index]->save(itemTag);
        }
        handItems->add(itemTag);
    }
    tag->put(L"HandItems", handItems);
    tag->putBoolean(L"Invisible", isInvisible());
    tag->putBoolean(L"Small", isSmall());
    tag->putInt(L"DisabledSlots", mDisabledSlots);
    tag->putBoolean(L"NoGravity", isNoGravity());
    tag->putBoolean(L"NoBasePlate", isNoBasePlate());
    if (isMarker())
    {
        tag->putBoolean(L"Marker", isMarker());
    }

    {
        CompoundTag* pose = new CompoundTag();
        pose->putInt(L"PoseIndex", getPoseIndex());
        pose->putInt(L"LastSignal", mLastSignal);
        tag->put(L"Pose", pose);
    }
}
void ArmorStand::setSmall(bool small)
{
    mEntityData->set(DATA_FLAGS, setBit(mEntityData->get(DATA_FLAGS), 1, small), true);
    setSize(0.5f, 1.975f);
}
void ArmorStand::setNoBasePlate(bool noBasePlate)
{
    mEntityData->set(DATA_FLAGS, setBit(mEntityData->get(DATA_FLAGS), 8, noBasePlate), true);
}
void ArmorStand::setMarker(bool marker)
{
    mEntityData->set(DATA_FLAGS, setBit(mEntityData->get(DATA_FLAGS), 0x10, marker), true);
    setSize(0.5f, 1.975f);
}
void ArmorStand::setPoseIndex(int pose)
{
    mEntityData->set(DATA_POSE, pose, true);
}
void ArmorStand::readAdditionalSaveData(CompoundTag* tag)
{
    LivingEntity::readAdditionalSaveData(tag);
    if (tag->contains(L"ArmorItems", Tag::TAG_LIST))
    {
        ListTag* armorItems = tag->getList(L"ArmorItems", Tag::TAG_COMPOUND);
        for (int index = 0; index < 4; ++index)
        {
            mArmorItems[index] = not_null_ptr<ItemInstance>(new ItemInstance(armorItems->getCompound(index)));
        }
    }
    if (tag->contains(L"HandItems", Tag::TAG_LIST))
    {
        ListTag* handItems = tag->getList(L"HandItems", Tag::TAG_COMPOUND);
        for (int index = 0; index < 2; ++index)
        {
            mHandItems[index] = not_null_ptr<ItemInstance>(new ItemInstance(handItems->getCompound(index)));
        }
    }

    setInvisible(tag->getBoolean(L"Invisible"));
    setSmall(tag->getBoolean(L"Small"));
    mDisabledSlots = tag->getInt(L"DisabledSlots");
    setNoGravity(tag->getBoolean(L"NoGravity"));
    setNoBasePlate(tag->getBoolean(L"NoBasePlate"));
    setMarker(tag->getBoolean(L"Marker"));
    mWasMarker = !isMarker();
    mNoPhysics = isNoGravity();
    if (tag->contains(L"Pose"))
    {
        CompoundTag* pose = tag->getCompound(L"Pose");
        setPoseIndex(pose->getInt(L"PoseIndex"));
        mLastSignal = pose->getInt(L"LastSignal");
    }
}
bool ArmorStand::isPushable(boost::shared_ptr<Entity> entity)
{
    return false;
}
const EquipmentSlot* ArmorStand::getClickedSlot(const Vec3* hit)
{
    const EquipmentSlot* slot = EquipmentSlot::MAINHAND;
    bool small = isSmall();
    double y = small ? hit->y * 2.0 : hit->y;
    const EquipmentSlot* feet = EquipmentSlot::FEET;

    if (y >= 0.1 && y < 0.1 + (small ? 0.8 : 0.45) && hasItemInSlot(feet))
    {
        slot = EquipmentSlot::FEET;
    }
    else if (y >= 0.9 + (small ? 0.3 : 0.0) && y < 0.9 + (small ? 1.0 : 0.7) && hasItemInSlot(EquipmentSlot::CHEST))
    {
        slot = EquipmentSlot::CHEST;
    }
    else if (y >= 0.4 && y < 0.4 + (small ? 1.0 : 0.8) && hasItemInSlot(EquipmentSlot::LEGS))
    {
        slot = EquipmentSlot::LEGS;
    }
    else if (y >= 1.6 && hasItemInSlot(EquipmentSlot::HEAD))
    {
        slot = EquipmentSlot::HEAD;
    }
    return slot;
}

bool ArmorStand::isDisabled(const EquipmentSlot* slot)
{
    return (mDisabledSlots & (1 << slot->getFilterFlag())) != 0;
}

void ArmorStand::swapItem(const boost::shared_ptr<Player>& player, const EquipmentSlot* slot,
                          const not_null_ptr<ItemInstance>& item, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> equipped = getItemSlot(slot);
    if (!equipped->isEmpty() && (mDisabledSlots & (1 << (slot->getFilterFlag() + 8))) != 0)
    {
        return;
    }
    if (!equipped->isEmpty() && (mDisabledSlots & (1 << (slot->getFilterFlag() + 16))) != 0)
    {
        return;
    }

    if (player->mAbilities.mInstabuild && equipped->isEmpty() && !item->isEmpty())
    {
        not_null_ptr<ItemInstance> singleItem = item->copy();
        singleItem->setCount(1);
        setItemSlot(slot, singleItem);
        return;
    }

    if (!item->isEmpty() && item->getCount() > 1)
    {
        if (equipped->isEmpty())
        {
            not_null_ptr<ItemInstance> singleItem = item->copy();
            singleItem->setCount(1);
            setItemSlot(slot, singleItem);
            if (!player->mAbilities.mInstabuild)
            {
                item->shrink(1);
            }
            return;
        }
        if (player->mInventory->add(equipped) != 1)
        {
            return;
        }
        not_null_ptr<ItemInstance> singleItem = item->copy();
        singleItem->setCount(1);
        setItemSlot(slot, singleItem);
        if (!player->mAbilities.mInstabuild)
        {
            item->shrink(1);
        }
        return;
    }

    setItemSlot(slot, item);
    player->setItemInHand(hand, equipped);
}
bool ArmorStand::isShowArms()
{
    return (mEntityData->get(DATA_FLAGS) & 4) != 0;
}
InteractionResult::EInteractionResult ArmorStand::interactAt(const boost::shared_ptr<Player>& player, Vec3* hit,
                                                             InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> heldItem = player->getItemInHand(hand);

    if (!player->isAllowedToInteract(shared_from_this()))
    {
        return InteractionResult::eInteractionResult_Pass;
    }

    if (player->isSneaking())
    {
        setPoseIndex((getPoseIndex() + 1) % 13);
    }
    else
    {
        if (isMarker() || (heldItem->getItem() == Items::nameTag && heldItem->hasCustomHoverName()))
        {
            return InteractionResult::eInteractionResult_Pass;
        }

        if (mLevel->mIsClientSide || player->isSpectator())
        {
            return InteractionResult::eInteractionResult_Success;
        }

        const EquipmentSlot* equipmentSlot = Mob::getEquipmentSlotForItem(heldItem);
        if (heldItem->isEmpty())
        {
            const EquipmentSlot* clickedSlot = getClickedSlot(hit);
            const EquipmentSlot* useSlot = isDisabled(clickedSlot) ? equipmentSlot : clickedSlot;
            if (hasItemInSlot(useSlot))
            {
                swapItem(player, useSlot, heldItem, hand);
            }
        }
        else
        {
            if (isDisabled(equipmentSlot))
            {
                return InteractionResult::eInteractionResult_Fail;
            }
            if (equipmentSlot->getType() == EquipmentSlot::HAND && !isShowArms())
            {
                return InteractionResult::eInteractionResult_Fail;
            }
            swapItem(player, equipmentSlot, heldItem, hand);
        }
    }

    return InteractionResult::eInteractionResult_Success;
}
ArmorStand::EInteraction ArmorStand::test_interactAt(boost::shared_ptr<Player> player, Vec3* hit,
                                                     not_null_ptr<ItemInstance> item,
                                                     InteractionHand::EInteractionHand hand)
{
    if (!player->isAllowedToInteract(shared_from_this()))
        return eInteraction_None;
    if (isMarker())
        return eInteraction_None;
    if (player->isSpectator())
        return eInteraction_None;
    if (player->isSneaking())
        return eInteraction_ChangePose;

    const EquipmentSlot* slot = EquipmentSlot::MAINHAND;
    const bool holdingItem = !item->isEmpty();
    if (holdingItem)
    {
        ArmorItem* armor = dynamic_cast<ArmorItem*>(item->getItem());
        if (armor != NULL)
            slot = armor->mSlot;
        if (item->getItem() == Items::skull || item->getItem() == Item::byBlock(Blocks::carved_pumpkin))
            slot = EquipmentSlot::HEAD;
    }

    const EquipmentSlot* clickedSlot = getClickedSlot(hit);
    if (isDisabled(clickedSlot))
        clickedSlot = slot;
    if (isDisabled(slot))
        return eInteraction_None;
    if (holdingItem && slot == EquipmentSlot::MAINHAND && !isShowArms())
        return eInteraction_None;
    if (holdingItem && !getItemSlot(slot)->isEmpty())
    {
        return eInteraction_Swap;
    }
    else if (holdingItem)
    {
        return eInteraction_Place;
    }
    else if (!getItemSlot(clickedSlot)->isEmpty())
    {
        return eInteraction_Take;
    }
    return eInteraction_None;
}
unsigned int ArmorStand::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::NAME_TAG
        && tooltipData.m_pItemInstance->hasCustomHoverName())
    {
        if (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName().compare(getCustomName()) != 0)
            return IDS_TOOLTIPS_NAME;
        return tooltipData.m_pPlayer->isSneaking() ? IDS_TOOLTIPS_ARMOUR_STAND_POSE : IDS_NONE;
    }

    if (tooltipData.m_bAllowedToInteract)
    {
        HitResult* hitResult = Minecraft::GetInstance()->mHitResult;
        Vec3* hit = Vec3::newTemp(hitResult->mLocation->x - mX, hitResult->mLocation->y - mY,
                                  hitResult->mLocation->z - mZ);
        switch (test_interactAt(tooltipData.m_pPlayer, hit, tooltipData.m_pItemInstance,
                                InteractionHand::eInteractionHand_MainHand))
        {
        case eInteraction_Place:
            return IDS_TOOLTIPS_PLACE;
        case eInteraction_Take:
            return IDS_TOOLTIPS_PICKUP_GENERIC;
        case eInteraction_Swap:
            return IDS_TOOLTIPS_PICKUPPLACE;
        case eInteraction_ChangePose:
            return IDS_TOOLTIPS_ARMOUR_STAND_POSE;
        }
    }
    return IDS_NONE;
}
unsigned int ArmorStand::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToHurt ? IDS_TOOLTIPS_MINE : IDS_NONE;
}
void ArmorStand::playBrokenSound()
{
    mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::ARMORSTAND_BREAK, getSoundSource(), 1.0f,
                      1.0f, 16.0f);
}
void ArmorStand::brokenByAnything()
{
    playBrokenSound();
    for (int i = 0; i < 2; ++i)
    {
        not_null_ptr<ItemInstance> item = mHandItems[i];
        if (!item->isEmpty())
        {
            Block::popResource(mLevel, BlockPos(this).above(), item);
            mHandItems[i] = ItemInstance::EMPTY;
        }
    }
    for (int i = 0; i < 4; ++i)
    {
        not_null_ptr<ItemInstance> item = mArmorItems[i];
        if (!item->isEmpty())
        {
            Block::popResource(mLevel, BlockPos(this).above(), item);
            mArmorItems[i] = ItemInstance::EMPTY;
        }
    }
}
void ArmorStand::causeDamage(float damage)
{
    float remainingHealth = getHealth() - damage;
    if (remainingHealth <= 0.5f)
    {
        brokenByAnything();
        remove();
    }
    else
    {
        setHealth(remainingHealth);
    }
}
void ArmorStand::showBreakingParticles()
{
    ServerLevel* level = dynamic_cast<ServerLevel*>(mLevel);
    if (level == NULL)
        return;
    int blockData = Block::getIdWithData(Blocks::planks->defaultBlockState());
    arrayWithLength<int> data(&blockData, 1);
    level->sendParticles(PARTICLE_BLOCK_DUST, mX, mY + mBbHeight / 1.5, mZ, 10, mBbWidth / 4.0f, mBbHeight / 4.0f,
                         mBbWidth / 4.0f, 0.05, data);
}
void ArmorStand::brokenByPlayer()
{
    Block::popResource(mLevel, BlockPos(shared_from_this()),
                       not_null_ptr<ItemInstance>(new ItemInstance(Items::armorStand)));
    brokenByAnything();
}
bool ArmorStand::hurt(const DamageSource* source, float damage)
{
    boost::shared_ptr<Entity> sourceEntity = source->getEntity();
    if (sourceEntity && (sourceEntity->GetType() & EntityType::PLAYER) == EntityType::PLAYER
        && boost::dynamic_pointer_cast<Player>(sourceEntity)->isAllowedToHurtEntity(shared_from_this()) == 0)
    {
        return false;
    }

    if (mLevel->mIsClientSide)
    {
        return false;
    }

    if (DamageSource::OUT_OF_WORLD->equals(source))
    {
        remove();
        return false;
    }

    if (isInvulnerableTo(source) || mInvisible || isMarker())
    {
        return false;
    }

    if (source->isExplosion())
    {
        brokenByAnything();
        remove();
        return false;
    }

    if (DamageSource::IN_FIRE->equals(source))
    {
        if (!isOnFire())
        {
            setSecondsOnFire(5);
        }
        else
        {
            causeDamage(0.15f);
        }
        return false;
    }

    if (DamageSource::ON_FIRE->equals(source) && getHealth() > 0.5f)
    {
        causeDamage(4.0f);
        return false;
    }

    const bool isArrow = source->getMsgId() == ClientboundChatPacket::eChatPacketMessage_DeathArrow;
    const bool isTrident = source->getMsgId() == ClientboundChatPacket::eChatPacketMessage_DeathTrident;
    const bool isPlayer = source->getMsgId() == ClientboundChatPacket::eChatPacketMessage_DeathPlayer;
    if (!(isPlayer || isArrow || isTrident))
    {
        return false;
    }

    if ((source->getDirectEntity()->GetType() & EntityType::ARROW) == EntityType::ARROW)
    {
        source->getDirectEntity()->remove();
    }

    if ((source->getEntity()->GetType() & EntityType::PLAYER) == EntityType::PLAYER
        && !boost::dynamic_pointer_cast<Player>(source->getEntity())->mAbilities.mMayBuild)
    {
        return false;
    }

    if (source->isCreativePlayer())
    {
        playBrokenSound();
        showBreakingParticles();
        remove();
        return false;
    }

    long long gameTime = mLevel->getGameTime();
    if (gameTime - mLastHit <= 5 || isArrow || isTrident)
    {
        brokenByPlayer();
        showBreakingParticles();
        remove();
    }
    else
    {
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::ARMORSTAND_WOBBLE, 0);
        mLastHit = gameTime;
    }
    return false;
}
void ArmorStand::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::ARMORSTAND_WOBBLE)
    {
        mLevel->playLocalSound(mX, mY, mZ, SoundEvents::ARMORSTAND_HIT, getSoundSource(), 0.3f, 1.0f, false, 16.0f);
        mLastHit = mLevel->getGameTime();
    }
    else
    {
        LivingEntity::handleEntityEvent(event, data);
    }
}
bool ArmorStand::shouldRenderAtSqrDistance(double distanceSqr)
{
    double size = mBoundingBox->getSize();
    if (size != size || size == 0.0)
    {
        size = 4.0;
    }
    size *= 64.0;
    return distanceSqr < size * size;
}
void ArmorStand::setInvisible(bool invisible)
{
    mInvisible = invisible;
    Entity::setInvisible(invisible);
}
bool ArmorStand::isBaby()
{
    return isSmall();
}
void ArmorStand::kill()
{
    remove();
}
bool ArmorStand::ignoreExplosion()
{
    return isInvisible();
}
int ArmorStand::getPistonPushReaction()
{
    return isMarker() ? 3 : Entity::getPistonPushReaction();
}
bool ArmorStand::isAffectedByPotions()
{
    return false;
}
void ArmorStand::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor->getId() == DATA_FLAGS->getId())
    {
        setSize(0.5f, 1.975f);
    }
    LivingEntity::onSyncedDataUpdated(accessor);
}
bool ArmorStand::attackable()
{
    return false;
}
Vec3* ArmorStand::getHeadPose()
{
    return POSES[getPoseIndex()]->head;
}
Vec3* ArmorStand::getBodyPose()
{
    return POSES[getPoseIndex()]->body;
}
Vec3* ArmorStand::getLeftArmPose()
{
    return POSES[getPoseIndex()]->leftArm;
}
Vec3* ArmorStand::getRightArmPose()
{
    return POSES[getPoseIndex()]->rightArm;
}
Vec3* ArmorStand::getLeftLegPose()
{
    return POSES[getPoseIndex()]->leftLeg;
}
Vec3* ArmorStand::getRightLegPose()
{
    return POSES[getPoseIndex()]->rightLeg;
}
Vec3* ArmorStand::getRightItemPose()
{
    return POSES[getPoseIndex()]->rightItem;
}
bool ArmorStand::isPickable()
{
    return LivingEntity::isPickable() && !isMarker();
}
const HumanoidArm* ArmorStand::getMainArm()
{
    return HumanoidArm::RIGHT;
}
float ArmorStand::tickHeadTurn(float bodyRot, float headTurnDelta)
{
    mYBodyRotO = mYRotO;
    mYBodyRot = mYRot;
    return 0.0f;
}
double ArmorStand::getRidingHeight()
{
    return isMarker() ? 0.0 : 0.1;
}
float ArmorStand::getEyeHeight()
{
    return mBbHeight * (isBaby() ? 0.5f : 0.9f);
}
void ArmorStand::travel(float xxa, float yya, float zza)
{
    if (!isNoGravity())
    {
        LivingEntity::travel(xxa, yya, zza);
    }
}
void ArmorStand::setYBodyRot(float bodyRot)
{
    mYBodyRotO = mYRotO = bodyRot;
    mYHeadRotO = mYHeadRot = bodyRot;
}
void ArmorStand::setYHeadRot(float headRot)
{
    mYBodyRotO = mYRotO = headRot;
    mYHeadRotO = mYHeadRot = headRot;
}
bool ArmorStand::updatePoseForSignal(const BlockPos& pos, const Direction* direction)
{
    if (mLevel->hasSignal(pos.relative(direction), direction))
    {
        int signal = mLevel->getSignal(pos.relative(direction), direction);
        if (signal > 0)
        {
            if (signal != mLastSignal)
            {
                int pose = signal > 13 ? 13 : signal;
                int index = pose - 1;
                if (index != getPoseIndex())
                {
                    setPoseIndex(index);
                }
            }
            mLastSignal = signal;
            return true;
        }
    }
    return false;
}
void ArmorStand::updateBoundingBox(bool marker)
{
    if (marker)
    {
        setSize(0.0f, 0.0f);
    }
    else
    {
        setSize(0.5f, 1.975f);
    }
}
void ArmorStand::tick()
{
    PIXBeginNamedEvent(0.0f, "Armor stand tick");
    if (!mLevel->mIsClientSide)
    {
        BlockPos blockPos(getPos(0.0f));
        bool poseUpdated = false;
        for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
             ++it)
        {
            const Direction* direction = *it;
            if (!direction->getAxis()->isVertical())
            {
                if (updatePoseForSignal(blockPos.below(), direction))
                {
                    poseUpdated = true;
                    break;
                }
            }
        }
        if (!poseUpdated)
        {
            mLastSignal = 0;
        }
    }
    LivingEntity::tick();
    bool marker = isMarker();
    if (mWasMarker != marker)
    {
        updateBoundingBox(marker);
        mBlocksBuilding = !marker;
    }
    mWasMarker = marker;
    PIXEndNamedEvent();
}
const SoundEvent* ArmorStand::getHurtSound(const DamageSource*)
{
    return SoundEvents::ARMORSTAND_HIT;
}
const SoundEvent* ArmorStand::getDeathSound()
{
    return SoundEvents::ARMORSTAND_BREAK;
}
const SoundEvent* ArmorStand::getFallDamageSound(int)
{
    return SoundEvents::ARMORSTAND_FALL;
}
void ArmorStand::setSize(float width, float height)
{
    double x = mX;
    double y = mY;
    double z = mZ;
    float scale = isMarker() ? 0.0f : (isBaby() ? 0.5f : 1.0f);
    Entity::setSize(width * scale, height * scale);
    setPos(x, y, z);
}
void ArmorStand::defineSynchedData()
{
    LivingEntity::defineSynchedData();
    unsigned char flags = 0;
    mEntityData->createDateItem(DATA_FLAGS, flags);
    int pose = 0;
    mEntityData->createDateItem(DATA_POSE, pose);
}
void ArmorStand::doPush(boost::shared_ptr<Entity>)
{
}
void ArmorStand::pushEntities()
{
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(shared_from_this(), mBoundingBox,
                                                                            IS_RIDEABLE_MINECART, NULL);

    if (entities == NULL || entities->empty())
    {
        return;
    }

    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        if (distanceToSqr(entity) <= 0.2)
        {
            entity->push(shared_from_this());
        }
    }
}
void ArmorStand::updateInvisibilityStatus()
{
    setInvisible(mInvisible);
}
bool ArmorStand::generatesBubbles()
{
    return false;
}
void ArmorStand::setAirSupply(int airSupply)
{
}
unsigned long long ArmorStand::GetType()
{
    return EntityType::ARMOR_STAND;
}
void ArmorStand::thunderHit(const LightningBolt*)
{
}
Entity* ArmorStand::create(Level* level)
{
    return new ArmorStand(level);
}

bool ArmorStand::RidableMinecartEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    bool result = eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType())
                  && boost::static_pointer_cast<Minecart>(entity)->getType() == Minecart::Type::RIDEABLE;
    return result;
}

ArmorStand::RidableMinecartEntitySelector::~RidableMinecartEntitySelector()
{
}
