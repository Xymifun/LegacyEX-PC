#include "game/world/generation/ZombieVillager.hpp"

#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

const EntityDataAccessor<bool>* ZombieVillager::DATA_CONVERTING_ID = NULL;
const EntityDataAccessor<int>* ZombieVillager::DATA_PROFESSION_ID = NULL;

ZombieVillager::ZombieVillager(Level* level) : Zombie(level, false)
{
    fjDerivedCtorCalls();
    mConversionTime = 0;
    mCategories |= EntityCategory::eEntityCategory_Undead | EntityCategory::eEntityCategory_Monster;
}

Entity* ZombieVillager::create(Level* level)
{
    return new ZombieVillager(level);
}

void ZombieVillager::setProfession(int profession)
{
    mEntityData->set(DATA_PROFESSION_ID, profession, true);
}

void ZombieVillager::staticCtor()
{
    DATA_CONVERTING_ID = SynchedEntityData::defineId<bool>(EntityType::ZOMBIE_VILLAGER, EntityType::ZOMBIE,
                                                           EntityDataSerializers::BOOLEAN);
    DATA_PROFESSION_ID = SynchedEntityData::defineId<int>(EntityType::ZOMBIE_VILLAGER, EntityType::ZOMBIE,
                                                          EntityDataSerializers::INT);
}

void ZombieVillager::defineSynchedData()
{
    Zombie::defineSynchedData();
    mEntityData->createDateItem(DATA_CONVERTING_ID, false);
    mEntityData->createDateItem(DATA_PROFESSION_ID, 0);
}

int ZombieVillager::getProfession()
{
    int profession = mEntityData->get(DATA_PROFESSION_ID) % 6;
    return profession < 0 ? 0 : profession;
}

void ZombieVillager::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::ZOMBIE_VILLAGER);
}

bool ZombieVillager::isConverting()
{
    return getEntityData()->get(DATA_CONVERTING_ID);
}

void ZombieVillager::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"Profession"), getProfession());
    tag->putInt(std::wstring(L"ConversionTime"), isConverting() ? mConversionTime : -1);
    if (!mConversionPlayer.empty())
        tag->putString(std::wstring(L"ConversionPlayer"), mConversionPlayer);
}

void ZombieVillager::startConverting(const std::wstring& player, int conversionTime)
{
    mConversionPlayer = player;
    mConversionTime = conversionTime;
    getEntityData()->set(DATA_CONVERTING_ID, true, true);
    removeEffect(MobEffects::WEAKNESS);
    addEffect(new MobEffectInstance(MobEffects::DAMAGE_BOOST, conversionTime,
                                    std::min(mLevel->getDifficulty()->getId() - 1, 0)),
              boost::shared_ptr<Entity>());
    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::ZOMBIE_CONVERTING, 0);
}

void ZombieVillager::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setProfession(tag->getInt(std::wstring(L"Profession")));
    if (tag->contains(std::wstring(L"ConversionTime"), 99) && tag->getInt(std::wstring(L"ConversionTime")) >= 0)
    {
        startConverting(tag->contains(std::wstring(L"ConversionPlayer"))
                            ? tag->getString(std::wstring(L"ConversionPlayer"))
                            : std::wstring(L""),
                        tag->getInt(std::wstring(L"ConversionTime")));
    }
}

MobGroupData* ZombieVillager::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                               const std::wstring& dataString)
{
    setProfession(mLevel->mRandom->nextInt(6));
    return Zombie::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

int ZombieVillager::getConversionProgress()
{
    int progress = 1;
    if (mRandom->nextFloat() < 0.01f)
    {
        BlockPos pos(0, 0, 0);
        int checked = 0;
        for (int x = (int)mX - 4; x < (int)mX + 4 && checked < 14; ++x)
        {
            for (int y = (int)mY - 4; y < (int)mY + 4 && checked < 14; ++y)
            {
                for (int z = (int)mZ - 4; z < (int)mZ + 4 && checked < 14; ++z)
                {
                    pos.x = x;
                    pos.y = y;
                    pos.z = z;
                    const Block* block = mLevel->getBlockState(pos)->getBlock();
                    if (block == Blocks::iron_bars || block == Blocks::bed)
                    {
                        if (mRandom->nextFloat() < 0.3f)
                            ++progress;
                        ++checked;
                    }
                }
            }
        }
    }
    return progress;
}

void ZombieVillager::tick()
{
    if (!mLevel->mIsClientSide && isConverting())
    {
        mConversionTime -= getConversionProgress();
        if (mConversionTime < 1)
            finishConversion();
    }
    Zombie::tick();
}

bool ZombieVillager::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (item->getItem() == Items::goldenApple && item->getAuxValue() == 0 && hasEffect(MobEffects::WEAKNESS))
    {
        if (!player->mAbilities.mInstabuild)
            item->shrink(1);
        if (!mLevel->mIsClientSide)
            startConverting(player->getUUID(), mRandom->nextInt(2401) + 3600);
        return true;
    }
    return false;
}

unsigned int ZombieVillager::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    static Item* goldenApple = Items::goldenApple;
    if (Item::getId(holder.m_pItemInstance->getItem()) == ItemId::GOLDEN_APPLE && isWeakened()
        && !goldenApple->isFoil(holder.m_pItemInstance))
    {
        return IDS_TOOLTIPS_CURE;
    }
    return Mob::GetInteractTooltip(holder);
}

bool ZombieVillager::removeWhenFarAway()
{
    return !isConverting();
}

void ZombieVillager::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::ZOMBIE_CONVERTING)
    {
        if (!isSilent())
        {
            mLevel->playLocalSound(mX + 0.5, mY + 0.5, mZ + 0.5, SoundEvents::ZOMBIE_VILLAGER_CURE, getSoundSource(),
                                   mRandom->nextFloat() + 1.0f, mRandom->nextFloat() * 0.7f + 0.3f, false, 16.0f);
        }
        return;
    }
    Mob::handleEntityEvent(event, data);
}

void ZombieVillager::finishConversion()
{
    boost::shared_ptr<Villager> villager(new Villager(mLevel));
    villager->copyPosition(shared_from_this());
    villager->setProfession(getProfession());
    villager->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(villager)), NULL, false, std::wstring(L""));
    villager->setRewardPlayersInVillage();
    if (isBaby())
        villager->setAge(-24000);
    mLevel->removeEntity(shared_from_this());
    villager->setNoAi(isNoAi());
    if (hasCustomName())
    {
        villager->setCustomName(getCustomName(), getOwnerUUID(), wasNamedByRestrictedPlayer());
        villager->setCustomNameVisible(isCustomNameVisible());
    }
    mLevel->addEntity(villager);
    boost::shared_ptr<Player> player = mLevel->getPlayerByUUID(mConversionPlayer);
    if (player != NULL)
    {
        bool isServerPlayer = eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, player->GetType());
        if (isServerPlayer)
            player->awardStat(GenericStats::zombieDoctor(), GenericStats::param_zombieDoctor());
    }
    villager->addEffect(new MobEffectInstance(MobEffects::CONFUSION, 200, 0), boost::shared_ptr<Entity>());
    mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_ZOMBIE_CONVERTED,
                       BlockPos((int)mX, (int)mY, (int)mZ), 0);
}

float ZombieVillager::getVoicePitch()
{
    if (isBaby())
        return (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 2.0f;
    return (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f;
}

const SoundEvent* ZombieVillager::getAmbientSound()
{
    return SoundEvents::ZOMBIE_VILLAGER_AMBIENT;
}

const SoundEvent* ZombieVillager::getHurtSound(const DamageSource*)
{
    return SoundEvents::ZOMBIE_VILLAGER_HURT;
}

const SoundEvent* ZombieVillager::getDeathSound()
{
    return SoundEvents::ZOMBIE_VILLAGER_DEATH;
}

const SoundEvent* ZombieVillager::getStepSound()
{
    return SoundEvents::ZOMBIE_VILLAGER_STEP;
}

ResourceLocation* ZombieVillager::getDefaultLootTable()
{
    return &BuiltInLootTables::ZOMBIE_VILLAGER;
}

ResourceLocation* ZombieVillager::getDefaultGearTable()
{
    return NULL;
}

not_null_ptr<ItemInstance> ZombieVillager::getSkull()
{
    return ItemInstance::EMPTY;
}

unsigned long long ZombieVillager::GetType()
{
    return EntityType::ZOMBIE_VILLAGER;
}
