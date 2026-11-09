#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <typeinfo>

#include "game/client/render/ParticleType.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/BodyControl.hpp"
#include "game/core/CensoredString.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/LootTables.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Path.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/Sensing.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/SwordItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/Items.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/Difficulty.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/PIXNamedEvents.hpp"

const EntityDataAccessor<unsigned char>* Mob::DATA_MOB_FLAGS_ID = NULL;

Mob::Mob(Level* level)
    : LivingEntity(level),
      mGoalSelector(),
      mTargetSelector(),
      mTarget(),
      mHandItems(),
      mArmorItems(),
      mAdditionalItems(),
      mAdditionalDropChances(NULL),
      mAdditionalItemCount(0),
      mPathfindingMalus(),
      mCustomName()
{
    MemSect(0x39);
    _init();
    MemSect(0);
    MemSect(0x3a);
    registerAttributes();
    MemSect(0);
    mLookControl = new LookControl(this);
    mMoveControl = new MoveControl(this);
    mJumpControl = new JumpControl(this);
    mBodyControl = NULL;
    mNavigation = NULL;
    mSensing = new Sensing(this);
    mHandDropChances[0] = 0.085f;
    mHandDropChances[1] = 0.085f;
    mArmorDropChances[0] = 0.085f;
    mArmorDropChances[1] = 0.085f;
    mArmorDropChances[2] = 0.085f;
    mArmorDropChances[3] = 0.085f;
}

Mob::~Mob()
{
    delete mLookControl;
    mLookControl = NULL;
    delete mMoveControl;
    mMoveControl = NULL;
    delete mJumpControl;
    mJumpControl = NULL;
    delete mBodyControl;
    mBodyControl = NULL;
    delete mNavigation;
    mNavigation = NULL;
    delete mSensing;
    mSensing = NULL;
    delete[] mAdditionalItems.items.data;
    delete[] mAdditionalDropChances;
    delete[] mHandItems.items.data;
    delete[] mArmorItems.items.data;
}

unsigned long long Mob::GetType()
{
    return EntityType::MOB;
}

void Mob::staticCtor()
{
    DATA_MOB_FLAGS_ID = SynchedEntityData::defineId(EntityType::MOB, EntityType::LIVING_ENTITY,
                                                    EntityDataSerializers::BYTE);
}

void Mob::_init()
{
    mAmbientSoundTime = 0;
    mMoveSpeed = 0.0f;
    mExperienceReward = 0;
    mTarget.reset();
    mSensing = NULL;
    mAdditionalItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(5, ItemInstance::EMPTY);
    mAdditionalDropChances = new float[5];
    for (int i = 0; i < 5; ++i)
        mAdditionalDropChances[i] = 0.0f;
    mCanPickUpLoot = false;
    mPersistenceRequired = false;
    mAdditionalItemCount = 5;
    mCustomName = boost::shared_ptr<CensoredString>(new CensoredString());
    mLootTable = NULL;
    mLootTableSeed = 0;
    mHandItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(2, ItemInstance::EMPTY);
    mArmorItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY);
    mLeader = false;
    mFlockSize = 0;
    mCategories |= EntityCategory::eEntityCategory_Mob;
    mMaxFlockSize = 1;
    mFollowsLeader = false;
}

void Mob::registerAttributes()
{
    LivingEntity::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(16.0);
}

void Mob::fjDerivedCtorCalls()
{
    LivingEntity::fjDerivedCtorCalls();
    mBodyControl = createBodyControl();
    mNavigation = createNavigation(mLevel);
    registerGoals();
}

void Mob::defineSynchedData()
{
    LivingEntity::defineSynchedData();
    mEntityData->createDateItem(DATA_MOB_FLAGS_ID, (unsigned char)0);
}

void Mob::registerGoals()
{
}

PathNavigation* Mob::createNavigation(Level* level)
{
    return new GroundPathNavigation(this, mLevel, true);
}

float Mob::getPathfindingMalus(const BlockPathTypes* type)
{
    boost::unordered_map<const BlockPathTypes*, float>::const_iterator it = mPathfindingMalus.find(type);
    if (it != mPathfindingMalus.end())
        return it->second;
    return type->getMalus();
}

void Mob::setPathfindingMalus(const BlockPathTypes* type, float malus)
{
    mPathfindingMalus[type] = malus;
}

BodyControl* Mob::createBodyControl()
{
    return new BodyControl(this);
}

LookControl* Mob::getLookControl()
{
    return mLookControl;
}

MoveControl* Mob::getMoveControl()
{
    return mMoveControl;
}

JumpControl* Mob::getJumpControl()
{
    return mJumpControl;
}

PathNavigation* Mob::getNavigation()
{
    return mNavigation;
}

Sensing* Mob::getSensing()
{
    return mSensing;
}

boost::shared_ptr<LivingEntity> Mob::getTarget()
{
    return mTarget.lock();
}

void Mob::setTarget(boost::shared_ptr<LivingEntity> target)
{
    mTarget = target;
}

void Mob::setLevel(Level* level)
{
    Entity::setLevel(level);
    mNavigation->setLevel(level);
    mGoalSelector.setLevel(level);
    mTargetSelector.setLevel(level);
}

bool Mob::canAttackType(unsigned long long type)
{
    return type != EntityType::GHAST;
}

void Mob::ate()
{
}

int Mob::getAmbientSoundInterval()
{
    return 80;
}

void Mob::playAmbientSound()
{
    MemSect(0x1f);
    const SoundEvent* sound = getAmbientSound();
    if (sound != NULL)
        playSound(sound, getSoundVolume(), getVoicePitch());
    MemSect(0);
}

void Mob::baseTick()
{
    LivingEntity::baseTick();
    if (!isAlive())
        return;

    int roll = mRandom->nextInt(1000);
    int soundTime = mAmbientSoundTime;
    ++mAmbientSoundTime;
    if (roll < soundTime)
    {
        resetAmbientSoundTime();
        playAmbientSound();
    }
}

void Mob::playHurtSound(const DamageSource* source)
{
    resetAmbientSoundTime();
    LivingEntity::playHurtSound(source);
}

void Mob::resetAmbientSoundTime()
{
    mAmbientSoundTime = -getAmbientSoundInterval();
}

int Mob::getExperienceReward(boost::shared_ptr<Player>)
{
    int reward = mExperienceReward;
    if (reward <= 0)
        return reward;

    const float dropChanceLimit = 1.0f;

    if (!mArmorItems[0]->isEmpty() && mArmorDropChances[0] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mArmorItems[1]->isEmpty() && mArmorDropChances[1] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mArmorItems[2]->isEmpty() && mArmorDropChances[2] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mArmorItems[3]->isEmpty() && mArmorDropChances[3] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mHandItems[0]->isEmpty() && mHandDropChances[0] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;
    if (!mHandItems[1]->isEmpty() && mHandDropChances[1] <= dropChanceLimit)
        reward += mRandom->nextInt(3) + 1;

    return reward;
}

void Mob::spawnAnim()
{
    if (!mLevel->mIsClientSide)
    {
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::SPAWN_ANIM, 0);
        return;
    }

    for (int i = 0; i < 20; ++i)
    {
        double xa = mRandom->nextGaussian() * 0.02;
        double ya = mRandom->nextGaussian() * 0.02;
        double za = mRandom->nextGaussian() * 0.02;
        mLevel->addParticle(PARTICLE_EXPLODE, mX + mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth - xa * 10.0,
                            mY + mRandom->nextFloat() * mBbHeight - ya * 10.0,
                            mZ + mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth - za * 10.0, xa, ya, za,
                            arrayWithLength<int>());
    }
}

void Mob::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::SPAWN_ANIM)
    {
        spawnAnim();
        return;
    }
    LivingEntity::handleEntityEvent(event, data);
}

void Mob::setLeader(bool leader)
{
    mLeader = leader;
}

void Mob::tick()
{
    PIXBeginNamedEvent(0.0f, "Mob tick");
    LivingEntity::tick();
    if (!mLevel->mIsClientSide)
    {
        tickLeash();
        if (mTickCount % 5 == 0)
        {
            bool hasNonMobPassenger = !Entity::instanceof(getControllingPassenger(), EntityType::MOB);
            bool vehicleIsBoat = Entity::instanceof(getVehicle(), EntityType::BOAT);
            mGoalSelector.setControlFlag(1, hasNonMobPassenger);
            mGoalSelector.setControlFlag(4, hasNonMobPassenger && !vehicleIsBoat);
            mGoalSelector.setControlFlag(2, hasNonMobPassenger);
        }
        if (mLeader)
        {
            if (mRandom->nextInt(200) == 1)
            {
                AABB* searchBox = mBoundingBox->grow(8.0);
                std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(*this),
                                                                                               searchBox);
                if (entities->size() < 2)
                {
                    setLeader(false);
                    mFlockSize = 0;
                }
                delete entities;
            }
        }
        if (isVehicle() && isUnderLiquid(Material::WATER))
            underwaterEjectRider();
    }
    PIXEndNamedEvent();
}

float Mob::tickHeadTurn(float, float headTurnDelta)
{
    mBodyControl->clientTick();
    return headTurnDelta;
}

const SoundEvent* Mob::getAmbientSound()
{
    return NULL;
}

Item* Mob::getDeathLoot()
{
    return NULL;
}

void Mob::dropDeathLoot(bool, int lootingLevel)
{
    Item* item = getDeathLoot();
    if (item == NULL)
        return;
    int count = mRandom->nextInt(3);
    if (lootingLevel > 0)
        count += mRandom->nextInt(lootingLevel + 1);
    for (int i = 0; i < count; ++i)
        spawnAtLocation(item, 1);
}

void Mob::dropEquipment(bool hitByPlayer, int lootingLevel)
{
    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        not_null_ptr<ItemInstance> item = getItemSlot(slot);
        double chance;
        switch (slot->getType())
        {
        case EquipmentSlot::HAND:
            chance = mHandDropChances[slot->getIndex()];
            break;
        case EquipmentSlot::ARMOR:
            chance = mArmorDropChances[slot->getIndex()];
            break;
        default:
            chance = 0.0;
            break;
        }
        bool guaranteed = chance > 1.0;
        if (!item->isEmpty() && !EnchantmentHelper::hasVanishingCurse(item) && (hitByPlayer || guaranteed)
            && mRandom->nextFloat() - lootingLevel * 0.01f < chance)
        {
            if (!guaranteed && item->isDamageableItem())
            {
                int bound = item->getMaxDamage() - 3;
                bound = bound < 1 ? 1 : bound;
                item->setAuxValue(item->getMaxDamage() - mRandom->nextInt(mRandom->nextInt(bound) + 1));
            }
            spawnAtLocation(item, 0.0f);
        }
    }
}

void Mob::addDataWalkers(DataFixerUpper* fixer, unsigned long long entityType)
{
    std::vector<std::wstring> itemNames;
    itemNames.push_back(L"ArmorItems");
    itemNames.push_back(L"HandItems");
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity, (DataWalker*)new ItemListDataWalker(entityType, itemNames));
}

bool Mob::isLeftHanded()
{
    return (mEntityData->get(DATA_MOB_FLAGS_ID) & 2) != 0;
}

void Mob::addAdditonalSaveData(CompoundTag* tag)
{
    LivingEntity::addAdditonalSaveData(tag);

    tag->putBoolean(L"CanPickUpLoot", canPickUpLoot());
    tag->putBoolean(L"PersistenceRequired", mPersistenceRequired);

    ListTag* armorItems = new ListTag();
    for (int i = 0; i < 4; ++i)
    {
        not_null_ptr<ItemInstance> armorItem = mArmorItems[i];
        CompoundTag* itemTag = new CompoundTag();
        if (!armorItem->isEmpty())
            armorItem->save(itemTag);
        armorItems->add(itemTag);
    }
    tag->put(L"ArmorItems", armorItems);

    ListTag* handItems = new ListTag();
    for (int i = 0; i < 2; ++i)
    {
        not_null_ptr<ItemInstance> handItem = mHandItems[i];
        CompoundTag* itemTag = new CompoundTag();
        if (!handItem->isEmpty())
            handItem->save(itemTag);
        handItems->add(itemTag);
    }
    tag->put(L"HandItems", handItems);

    ListTag* armorDropChances = new ListTag();
    for (unsigned int i = 0; i < 4; ++i)
        armorDropChances->add(new FloatTag(mArmorDropChances[i]));
    tag->put(L"ArmorDropChances", armorDropChances);

    ListTag* handDropChances = new ListTag();
    for (unsigned int i = 0; i < 2; ++i)
        handDropChances->add(new FloatTag(mHandDropChances[i]));
    tag->put(L"HandDropChances", handDropChances);

    tag->putBoolean(L"Leashed", mLeashed);

    boost::shared_ptr<Entity> leashHolder = getLeashHolder();
    if (leashHolder)
    {
        CompoundTag* leashTag = new CompoundTag();

        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, leashHolder->GetType()))
        {
            leashTag->putString(L"UUID", leashHolder->getUUID());
        }
        else if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, leashHolder->GetType()))
        {
            boost::shared_ptr<HangingEntity> hanging = boost::dynamic_pointer_cast<HangingEntity>(leashHolder);
            BlockPos pos = hanging->getPos();
            leashTag->putInt(L"X", pos.x);
            leashTag->putInt(L"Y", pos.y);
            leashTag->putInt(L"Z", pos.z);
        }

        tag->put(L"Leash", leashTag);
    }

    tag->putBoolean(L"LeftHanded", isLeftHanded());

    if (mLootTable != NULL)
    {
        tag->putString(L"DeathLootTable", mLootTable->toString());
        if (mLootTableSeed != 0)
            tag->putLong(L"DeathLootTableSeed", mLootTableSeed);
    }

    if (isNoAi())
        tag->putBoolean(L"NoAI", isNoAi());

    if (mLootTable != NULL)
    {
        tag->putString(L"DeathLootTable", mLootTable->toString());
        if (mLootTableSeed != 0)
            tag->putLong(L"DeathLootTableSeed", mLootTableSeed);
    }
}

void Mob::setPersistenceRequired(bool required)
{
    mPersistenceRequired = required;
}

bool Mob::isPersistenceRequired()
{
    return mPersistenceRequired;
}

void Mob::readAdditionalSaveData(CompoundTag* tag)
{
    LivingEntity::readAdditionalSaveData(tag);

    if (tag->contains(L"CanPickUpLoot", Tag::TAG_BYTE))
        setCanPickUpLoot(tag->getBoolean(L"CanPickUpLoot"));

    setPersistenceRequired(tag->getBoolean(L"PersistenceRequired"));

    if (tag->contains(L"ArmorItems", Tag::TAG_LIST))
    {
        ListTag* items = tag->getList(L"ArmorItems", Tag::TAG_COMPOUND);
        for (int i = 0; i < 4; ++i)
            mArmorItems[i] = not_null_ptr<ItemInstance>(new ItemInstance(items->getCompound(i)));
    }

    if (tag->contains(L"HandItems", Tag::TAG_LIST))
    {
        ListTag* items = tag->getList(L"HandItems", Tag::TAG_COMPOUND);
        for (int i = 0; i < 2; ++i)
            mHandItems[i] = not_null_ptr<ItemInstance>(new ItemInstance(items->getCompound(i)));
    }

    if (tag->contains(L"ArmorDropChances", Tag::TAG_LIST))
    {
        ListTag* chances = tag->getList(L"ArmorDropChances", Tag::TAG_FLOAT);
        for (int i = 0; i < chances->size(); ++i)
            mArmorDropChances[i] = chances->getFloat(i);
    }

    if (tag->contains(L"HandDropChances", Tag::TAG_LIST))
    {
        ListTag* chances = tag->getList(L"HandDropChances", Tag::TAG_FLOAT);
        for (int i = 0; i < chances->size(); ++i)
            mHandDropChances[i] = chances->getFloat(i);
    }

    mLeashed = tag->getBoolean(L"Leashed");
    if (mLeashed && tag->contains(L"Leash", Tag::TAG_COMPOUND))
    {
        mLeashInfoTag = tag->getCompound(L"Leash")->copy();
    }

    setLeftHanded(tag->getBoolean(L"LeftHanded"));

    if (tag->contains(L"DeathLootTable", Tag::TAG_STRING))
    {
        mLootTable = new ResourceLocation(tag->getString(L"DeathLootTable"));
        mLootTableSeed = tag->getLong(L"DeathLootTableSeed");
    }

    setNoAi(tag->getBoolean(L"NoAI"));

    if (tag->contains(L"DeathLootTable"))
    {
        mLootTable = new ResourceLocation(tag->getString(L"DeathLootTable"));
        mLootTableSeed = tag->getLong(L"DeathLootTableSeed");
    }
}

ResourceLocation* Mob::getDefaultLootTable()
{
    return NULL;
}

ResourceLocation* Mob::getDefaultGearTable()
{
    return NULL;
}

void Mob::dropAllDeathLoot(bool hitByPlayer, int lootingLevel, const DamageSource* source)
{
    ResourceLocation* lootTableLocation = mLootTable;
    if (lootTableLocation == NULL)
    {
        lootTableLocation = getDefaultLootTable();
    }
    if (lootTableLocation != NULL)
    {
        LootTable* lootTable = mLevel->getLootTables()->get(lootTableLocation);
        mLootTable = NULL;

        LootTableContext::Builder* builder = (new LootTableContext::Builder(static_cast<ServerLevel*>(mLevel)))
                                                 ->withLootedEntity(shared_from_this())
                                                 ->withDamageSource(source);
        if (hitByPlayer && mLastHurtByPlayer)
        {
            builder->withPlayer(mLastHurtByPlayer)->withLuck(mLastHurtByPlayer->getLuck());
        }

        Random seededRandom(mLootTableSeed);
        LootTableContext context = builder->build();
        Random* random = mLootTableSeed == 0 ? mRandom : &seededRandom;

        std::vector<not_null_ptr<ItemInstance> >* items = lootTable->getRandomItems(random, context);
        for (std::vector<not_null_ptr<ItemInstance> >::iterator it = items->begin(); it != items->end(); ++it)
        {
            spawnAtLocation(*it, 0.0f);
        }

        dropEquipment(hitByPlayer, lootingLevel);
        delete items;
        delete builder;
    }
    else
    {
        LivingEntity::dropAllDeathLoot(hitByPlayer, lootingLevel, source);
    }
}

void Mob::setZza(float zza)
{
    mZza = zza;
}

void Mob::setYya(float yya)
{
    mYya = yya;
}

void Mob::setXxa(float xxa)
{
    mXxa = xxa;
}

void Mob::setSpeed(float speed)
{
    LivingEntity::setSpeed(speed);
    setZza(speed);
}

void Mob::aiStep()
{
    LivingEntity::aiStep();

    if (mLevel->mIsClientSide || !canPickUpLoot() || mDead)
        return;

    if (!mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
        return;

    AABB* searchBox = mBoundingBox->grow(1.0, 0.0, 1.0);
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(ItemEntity), searchBox);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(*it);
        if (itemEntity->mRemoved || itemEntity->getItem()->isEmpty() || itemEntity->hasPickUpDelay())
            continue;
        pickUpItem(itemEntity);
    }
    delete entities;
}

const EquipmentSlot* Mob::getEquipmentSlotForItem(not_null_ptr<ItemInstance> item)
{
    if (Block::byItem(item->getItem()) != Blocks::carved_pumpkin && item->getItem() != Items::skull)
    {
        ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item->getItem());
        if (armorItem != NULL)
        {
            return armorItem->mSlot;
        }

        return item->getItem() == Items::elytra ? EquipmentSlot::CHEST : EquipmentSlot::MAINHAND;
    }

    return EquipmentSlot::HEAD;
}

void Mob::pickUpItem(boost::shared_ptr<ItemEntity> entity)
{
    not_null_ptr<ItemInstance> item = entity->getItem();
    const EquipmentSlot* slot = getEquipmentSlotForItem(item);
    bool replace = true;
    not_null_ptr<ItemInstance> current = getItemSlot(slot);

    if (!current->isEmpty())
    {
        if (slot->getType() == EquipmentSlot::HAND)
        {
            SwordItem* newSword = dynamic_cast<SwordItem*>(item->getItem());
            SwordItem* currentSword = dynamic_cast<SwordItem*>(current->getItem());
            BowItem* newBow = dynamic_cast<BowItem*>(item->getItem());
            BowItem* currentBow = dynamic_cast<BowItem*>(current->getItem());
            if (newSword != NULL && currentSword == NULL)
            {
                replace = true;
            }
            else if (newSword != NULL && currentSword != NULL)
            {
                if (newSword->getTierDamage() == currentSword->getTierDamage())
                {
                    replace = item->getAuxValue() > current->getAuxValue() || (item->hasTag() && !current->hasTag());
                }
                else
                {
                    replace = newSword->getTierDamage() > currentSword->getTierDamage();
                }
            }
            else if (newBow != NULL && currentBow != NULL)
            {
                replace = item->hasTag() && !current->hasTag();
            }
            else
            {
                replace = false;
            }
        }
        else
        {
            ArmorItem* newArmor = dynamic_cast<ArmorItem*>(item->getItem());
            ArmorItem* currentArmor = dynamic_cast<ArmorItem*>(current->getItem());
            if (newArmor != NULL && currentArmor == NULL)
            {
                replace = true;
            }
            else if (newArmor != NULL && currentArmor != NULL && !EnchantmentHelper::hasBindingCurse(current))
            {
                if (newArmor->mDefense == currentArmor->mDefense)
                {
                    replace = item->getAuxValue() > current->getAuxValue() || (item->hasTag() && !current->hasTag());
                }
                else
                {
                    replace = newArmor->mDefense > currentArmor->mDefense;
                }
            }
            else
            {
                replace = false;
            }
        }
    }

    if (!replace || !canHoldItem(item))
        return;

    double dropChance;
    switch (slot->getType())
    {
    case EquipmentSlot::HAND:
        dropChance = mHandDropChances[slot->getIndex()];
        break;
    case EquipmentSlot::ARMOR:
        dropChance = mArmorDropChances[slot->getIndex()];
        break;
    default:
        dropChance = 0.0;
        break;
    }

    if (!current->isEmpty() && (double)(mRandom->nextFloat() - 0.1f) < dropChance)
        spawnAtLocation(current, 0.0f);

    setItemSlot(slot, item);
    switch (slot->getType())
    {
    case EquipmentSlot::HAND:
        mHandDropChances[slot->getIndex()] = 2.0f;
        break;
    case EquipmentSlot::ARMOR:
        mArmorDropChances[slot->getIndex()] = 2.0f;
        break;
    }

    setPersistenceRequired(true);
    take(entity, item->getCount());
    entity->remove();
}

bool Mob::canHoldItem(not_null_ptr<ItemInstance>)
{
    return true;
}

bool Mob::removeWhenFarAway()
{
    return !hasCustomName();
}

void Mob::checkDespawn()
{
    if (mPersistenceRequired)
    {
        mNoActionTime = 0;
        return;
    }

    boost::shared_ptr<Entity> player = mLevel->getNearestPlayer(shared_from_this(), -1.0, -1.0);
    if (player)
    {
        double dx = player->mX - mX;
        double dy = player->mY - mY;
        double dz = player->mZ - mZ;
        double distSq = dx * dx + dy * dy + dz * dz;

        if (removeWhenFarAway() && distSq > 16384.0)
            remove();

        if (mNoActionTime > 600 && mRandom->nextInt(800) == 0 && distSq > 1024.0 && removeWhenFarAway())
            remove();
        else if (distSq < 1024.0)
            mNoActionTime = 0;
    }
}

void Mob::serverAiStep()
{
    PIXBeginNamedEvent(0.0f, "Tick target selector for %I64d", GetType());
    MemSect(0x33);
    ++mNoActionTime;

    PIXBeginNamedEvent(0.0f, "Check despawn");
    checkDespawn();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Tick sensing");
    mSensing->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Tick target selector");
    mTargetSelector.tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Tick goal selectors");
    mGoalSelector.tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Tick navigation");
    mNavigation->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Tick server ai mob step");
    customServerAiStep();
    PIXEndNamedEvent();

    if (isPassenger() && eTYPE_DERIVED_FROM(EntityType::MOB, getVehicle()->GetType()))
    {
        boost::shared_ptr<Mob> vehicleMob = boost::static_pointer_cast<Mob>(getVehicle());

        PathNavigation* navigation = getNavigation();
        Path* currentPath = navigation->getPath();
        Path* clonedPath = currentPath != NULL ? currentPath->clone() : NULL;
        vehicleMob->getNavigation()->moveTo(clonedPath, 1.5);

        vehicleMob->getMoveControl()->copyFrom(getMoveControl());
    }

    PIXBeginNamedEvent(0.0f, "Tick look");
    mLookControl->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Tick move");
    mMoveControl->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Tick jump");
    mJumpControl->tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Consider extra wandering");
    considerForExtraWandering(isDespawnProtected());
    PIXEndNamedEvent();

    MemSect(0);
    PIXEndNamedEvent();
}

void Mob::customServerAiStep()
{
}

int Mob::getMaxHeadXRot()
{
    return 40;
}

int Mob::getMaxHeadYRot()
{
    return 10;
}

float Mob::rotlerp(float from, float to, float max)
{
    float delta = Mth::wrapDegrees(to - from);
    if (delta > max)
        delta = max;
    if (delta < -max)
        delta = -max;
    return from + delta;
}

void Mob::lookAt(boost::shared_ptr<Entity> entity, float maxHeadXRot, float maxHeadYRot)
{
    double xd = entity->mX - mX;
    double zd = entity->mZ - mZ;
    double yd;

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
    {
        boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
        yd = (living->mY + living->getEyeHeight()) - (mY + getEyeHeight());
    }
    else
    {
        yd = (entity->mBoundingBox->mMinY + entity->mBoundingBox->mMaxY) / 2.0 - (mY + getEyeHeight());
    }

    double sd = Mth::sqrt(xd * xd + zd * zd);
    float yRotD = (float)(atan2(zd, xd) * 57.2957763671875) - 90.0f;
    float xRotD = (float)-(atan2(yd, sd) * 57.2957763671875);
    mXRot = rotlerp(mXRot, xRotD, maxHeadYRot);
    mYRot = rotlerp(mYRot, yRotD, maxHeadXRot);
}

bool Mob::checkSpawnRules()
{
    BlockPos below(BlockPos(this).below());
    const BlockState* state = mLevel->getBlockState(below);
    return state->isValidSpawn(shared_from_this());
}

bool Mob::checkSpawnObstruction()
{
    if (!mLevel->isUnobstructed(mBoundingBox, shared_from_this()))
        return false;
    return mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty()
           && !mLevel->containsAnyLiquid_NoLoad(mBoundingBox);
}

float Mob::getSizeScale()
{
    return 1.0f;
}

float Mob::getHeadSizeScale()
{
    return 1.0f;
}

int Mob::getMaxSpawnClusterSize()
{
    return 4;
}

int Mob::getMaxFallDistance()
{
    if (!getTarget())
        return 3;

    int fallDistance = (int)(getHealth() - getMaxHealth() * 0.33f);
    int difficultyAdjustment = 12 - mLevel->getDifficulty()->getId() * 4;
    fallDistance -= difficultyAdjustment;
    if (fallDistance < 0)
        fallDistance = 0;
    return fallDistance + 3;
}

NonNullList<not_null_ptr<ItemInstance> > Mob::getHandSlots()
{
    return mHandItems;
}

NonNullList<not_null_ptr<ItemInstance> > Mob::getArmorSlots()
{
    return mArmorItems;
}

void Mob::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    switch (slot->getType())
    {
    case EquipmentSlot::HAND:
        mHandItems[slot->getIndex()] = item;
        break;
    case EquipmentSlot::ARMOR:
        mArmorItems[slot->getIndex()] = item;
        break;
    }
}

not_null_ptr<ItemInstance> Mob::getItemSlot(const EquipmentSlot* slot)
{
    if (slot->getType() == EquipmentSlot::HAND)
        return mHandItems[slot->getIndex()];
    if (slot->getType() == EquipmentSlot::ARMOR)
        return mArmorItems[slot->getIndex()];
    return ItemInstance::EMPTY;
}

Item* Mob::getEquipmentForSlot(const EquipmentSlot* slot, int tier)
{
    if (slot == EquipmentSlot::HEAD)
    {
        if (tier == 0)
            return Items::leatherHelmet;
        if (tier == 1)
            return Items::goldenHelmet;
        if (tier == 2)
            return Items::chainmailHelmet;
        if (tier == 3)
            return Items::ironHelmet;
        if (tier == 4)
            return Items::diamondHelmet;
    }
    else if (slot == EquipmentSlot::CHEST)
    {
        if (tier == 0)
            return Items::leatherChestplate;
        if (tier == 1)
            return Items::goldenChestplate;
        if (tier == 2)
            return Items::chainmailChestplate;
        if (tier == 3)
            return Items::ironChestplate;
        if (tier == 4)
            return Items::diamondChestplate;
    }
    else if (slot == EquipmentSlot::LEGS)
    {
        if (tier == 0)
            return Items::leatherLeggings;
        if (tier == 1)
            return Items::goldenLeggings;
        if (tier == 2)
            return Items::chainmailLeggings;
        if (tier == 3)
            return Items::ironLeggings;
        if (tier == 4)
            return Items::diamondLeggings;
    }
    else if (slot == EquipmentSlot::FEET)
    {
        if (tier == 0)
            return Items::leatherBoots;
        if (tier == 1)
            return Items::goldenBoots;
        if (tier == 2)
            return Items::chainmailBoots;
        if (tier == 3)
            return Items::ironBoots;
        if (tier == 4)
            return Items::diamondBoots;
    }
    return NULL;
}

void Mob::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    if (getDefaultGearTable() != NULL)
    {
        LootTableContext::Builder* builder = (new LootTableContext::Builder(static_cast<ServerLevel*>(mLevel)))
                                                 ->withLootedEntity(shared_from_this())
                                                 ->withDamageSource(NULL);
        LootTableContext context = builder->build();

        std::vector<not_null_ptr<ItemInstance> >* items =
            BuiltInLootTables::getLootTable(getDefaultGearTable())->getRandomItems(mRandom, context);
        for (std::vector<not_null_ptr<ItemInstance> >::iterator it = items->begin(); it != items->end(); ++it)
        {
            ArmorItem* armorItem = dynamic_cast<ArmorItem*>((*it)->getItem());
            Item* item = (*it)->getItem();
            if (armorItem != NULL)
            {
                setItemSlot(armorItem->getSlot(), *it);
            }
            else if (item->allowOffhand())
            {
                setItemSlot(EquipmentSlot::OFFHAND, *it);
            }
            else
            {
                setItemSlot(EquipmentSlot::MAINHAND, *it);
            }
        }
        delete items;
        delete builder;
        return;
    }

    if (mRandom->nextFloat() >= 0.15f * difficulty.getSpecialMultiplier())
        return;

    int tier = mRandom->nextInt(2);
    float stopChance = mLevel->getDifficulty() == Difficulty::HARD ? 0.1f : 0.25f;
    if (mRandom->nextFloat() < 0.095f)
        ++tier;
    if (mRandom->nextFloat() < 0.095f)
        ++tier;
    if (mRandom->nextFloat() < 0.095f)
        ++tier;

    bool firstArmorSlot = true;

    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        if (slot->getType() != EquipmentSlot::ARMOR)
            continue;

        if (!firstArmorSlot && mRandom->nextFloat() < stopChance)
            break;

        firstArmorSlot = false;

        not_null_ptr<ItemInstance> equipped = getItemSlot(slot);
        if (!equipped->isEmpty())
            continue;

        Item* equipment = getEquipmentForSlot(slot, tier);
        if (equipment == NULL)
            continue;

        setItemSlot(slot, not_null_ptr<ItemInstance>(new ItemInstance(equipment)));
    }
}

void Mob::populateDefaultEquipmentEnchantments(const DifficultyInstance& difficulty)
{
    float specialMultiplier = difficulty.getSpecialMultiplier();

    if (!getMainHandItem()->isEmpty() && mRandom->nextFloat() < 0.25f * specialMultiplier)
    {
        setItemSlot(EquipmentSlot::MAINHAND, EnchantmentHelper::enchantItem(
                                                 mRandom, getMainHandItem(),
                                                 (int)(specialMultiplier * (float)mRandom->nextInt(18) + 5.0f), false));
    }

    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        if (slot->getType() != EquipmentSlot::ARMOR)
            continue;

        not_null_ptr<ItemInstance> item = getItemSlot(slot);
        if (!item->isEmpty() && mRandom->nextFloat() < 0.5f * specialMultiplier)
        {
            setItemSlot(slot, EnchantmentHelper::enchantItem(
                                  mRandom, item, (int)(specialMultiplier * (float)mRandom->nextInt(18) + 5.0f), false));
        }
    }
}

MobGroupData* Mob::finalizeMobSpawn(const DifficultyInstance&, MobGroupData* data, const std::wstring&)
{
    if (mRandom->nextFloat() < 0.05f)
        setLeftHanded(true);
    else
        setLeftHanded(false);
    return data;
}

void Mob::finalizeSpawnEggSpawn(int)
{
}

bool Mob::canBeControlledByRider()
{
    return false;
}

void Mob::setDropChance(const EquipmentSlot* slot, float chance)
{
    switch (slot->getType())
    {
    case EquipmentSlot::HAND:
        mHandDropChances[slot->getIndex()] = chance;
        break;
    case EquipmentSlot::ARMOR:
        mArmorDropChances[slot->getIndex()] = chance;
        break;
    }
}

bool Mob::canPickUpLoot()
{
    return mCanPickUpLoot;
}

void Mob::setCanPickUpLoot(bool canPickUp)
{
    mCanPickUpLoot = canPickUp;
}

bool Mob::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> heldItem = player->getItemInHand(hand);
    Item* heldItemType = heldItem->getItem();

    if (heldItemType == Items::lead)
    {
        if (!mLevel->mIsClientSide && isLeashedToPlayer(player))
        {
            dropLeash(true, !player->mAbilities.mInstabuild);
            return true;
        }
        if (canBeLeashed(player) && player->isAllowedToAttackAnimals())
        {
            setLeashedTo(player, true);
            heldItem->shrink(1);
            return true;
        }
    }

    if (mobInteract(player, hand))
        return true;

    if (!mLevel->mIsClientSide && isLeashedToPlayer(player))
    {
        dropLeash(true, !player->mAbilities.mInstabuild);
        return true;
    }

    return Entity::interact(player, hand);
}

bool Mob::mobInteract(const boost::shared_ptr<Player>&, InteractionHand::EInteractionHand)
{
    return false;
}

void Mob::underwaterEjectRider()
{
    if (breathesWater())
        return;
    ejectPassengers();
}

bool Mob::canBeLeashed(const boost::shared_ptr<Player>&)
{
    return !isLeashed() && (shared_from_this()->GetType() & EntityType::ENEMY) != EntityType::ENEMY;
}

bool Mob::startRiding(boost::shared_ptr<Entity> entity, bool force)
{
    bool riding = Entity::startRiding(entity, force);
    if (riding && isLeashed())
        dropLeash(true, true);
    return riding;
}

unsigned int Mob::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (isLeashedToPlayer(holder.m_pPlayer))
        return IDS_TOOLTIPS_UNLEASH;
    return Entity::GetInteractTooltip(holder);
}

bool Mob::shouldRender(double distance)
{
    AABB* box = mBoundingBox;
    int minX = Mth::floor(box->mMinX);
    int minY = Mth::floor(box->mMinY);
    int minZ = Mth::floor(box->mMinZ);
    int maxX = Mth::floor(box->mMaxX);
    int maxY = Mth::floor(box->mMaxY);
    int maxZ = Mth::floor(box->mMaxZ);

    if (!mLevel->reallyHasChunksAt(BlockPos(minX, minY, minZ), BlockPos(maxX, maxY, maxZ)))
        return false;

    return Entity::shouldRender(distance);
}

bool Mob::isValidSlotForItem(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    const EquipmentSlot* itemSlot = getEquipmentSlotForItem(item);
    return itemSlot == slot || (itemSlot == EquipmentSlot::MAINHAND && slot == EquipmentSlot::OFFHAND)
           || (itemSlot == EquipmentSlot::OFFHAND && slot == EquipmentSlot::MAINHAND);
}

bool Mob::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
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

    if (!item->isEmpty() && !isValidSlotForItem(equipmentSlot, item) && equipmentSlot != EquipmentSlot::HEAD)
    {
        return false;
    }

    setItemSlot(equipmentSlot, item);
    return true;
}

bool Mob::isControlledByLocalInstance()
{
    return canBeControlledByRider() && Entity::isControlledByLocalInstance();
}

bool Mob::isEffectiveAi()
{
    return LivingEntity::isEffectiveAi() && !isNoAi();
}

void Mob::setNoAi(bool noAi)
{
    unsigned char flags = mEntityData->get(DATA_MOB_FLAGS_ID);
    mEntityData->set(DATA_MOB_FLAGS_ID, noAi ? (unsigned char)(flags | 1) : (unsigned char)(flags & 0xfe), true);
}

void Mob::setLeftHanded(bool leftHanded)
{
    unsigned char flags = mEntityData->get(DATA_MOB_FLAGS_ID);
    mEntityData->set(DATA_MOB_FLAGS_ID, leftHanded ? (unsigned char)(flags | 2) : (unsigned char)(flags & 0xfd), true);
}

bool Mob::isNoAi()
{
    return (mEntityData->get(DATA_MOB_FLAGS_ID) & 1) != 0;
}

const HumanoidArm* Mob::getMainArm()
{
    return isLeftHanded() ? HumanoidArm::LEFT : HumanoidArm::RIGHT;
}

std::wstring Mob::getName()
{
    return LivingEntity::getName();
}

int Mob::maxFlockSize()
{
    return mMaxFlockSize;
}

bool Mob::shouldFollow()
{
    return mLeader && mFlockSize < maxFlockSize();
}

void Mob::setFollowsLeader(bool follows)
{
    mFollowsLeader = follows;
}

bool Mob::followsLeader()
{
    return mFollowsLeader;
}

bool Mob::isLeader()
{
    return mLeader;
}
