#include "game/item/AreaEffectCloud.hpp"
#include "game/core/EntityType.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/core/Mth.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"

const EntityDataAccessor<float>* AreaEffectCloud::DATA_RADIUS = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_COLOR = NULL;
const EntityDataAccessor<bool>* AreaEffectCloud::DATA_WAITING = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_PARTICLE = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_PARTICLE_PARAM_1 = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_PARTICLE_PARAM_2 = NULL;
const EntityDataAccessor<int>* AreaEffectCloud::DATA_PARTICLE_RATE = NULL;

AreaEffectCloud::AreaEffectCloud(Level* level) : Entity(level, true), mOwner(), mOwnerUUID()
{
    _init();
}

AreaEffectCloud::AreaEffectCloud(Level* level, double x, double y, double z)
    : Entity(level, true),
      mOwner(),
      mOwnerUUID()
{
    _init();
    setPos(x, y, z);
}

AreaEffectCloud::~AreaEffectCloud()
{
    std::vector<MobEffectInstance*>::iterator end = mEffects.end();
    for (std::vector<MobEffectInstance*>::iterator it = mEffects.begin(); it != end; ++it)
    {
        delete *it;
    }
}

void AreaEffectCloud::staticCtor()
{
    DATA_RADIUS = SynchedEntityData::defineId<float>(EntityType::AREA_EFFECT_CLOUD, EntityType::ENTITY,
                                                     EntityDataSerializers::FLOAT);
    DATA_COLOR = SynchedEntityData::defineId<int>(EntityType::AREA_EFFECT_CLOUD, EntityType::ENTITY,
                                                  EntityDataSerializers::INT);
    DATA_WAITING = SynchedEntityData::defineId<bool>(EntityType::AREA_EFFECT_CLOUD, EntityType::ENTITY,
                                                     EntityDataSerializers::BOOLEAN);
    DATA_PARTICLE = SynchedEntityData::defineId<int>(EntityType::AREA_EFFECT_CLOUD, EntityType::ENTITY,
                                                     EntityDataSerializers::INT);
    DATA_PARTICLE_PARAM_1 = SynchedEntityData::defineId<int>(EntityType::AREA_EFFECT_CLOUD, EntityType::ENTITY,
                                                             EntityDataSerializers::INT);
    DATA_PARTICLE_PARAM_2 = SynchedEntityData::defineId<int>(EntityType::AREA_EFFECT_CLOUD, EntityType::ENTITY,
                                                             EntityDataSerializers::INT);
    DATA_PARTICLE_RATE = SynchedEntityData::defineId<int>(EntityType::AREA_EFFECT_CLOUD, EntityType::ENTITY,
                                                          EntityDataSerializers::INT);
}

void AreaEffectCloud::setRadius(float radius)
{
    double x = mX;
    double y = mY;
    double z = mZ;
    setSize(radius * 2.0f, 0.5f);
    setPos(x, y, z);
    if (!mLevel->mIsClientSide)
    {
        getEntityData()->set(DATA_RADIUS, radius, true);
    }
}

void AreaEffectCloud::_init()
{
    fjDerivedCtorCalls();
    mFixedColor = false;
    mDurationOnUse = 0;
    mRadiusOnUse = 0.0f;
    mRadiusPerTick = 0.0f;
    mOwnerUUID = L"";
    mPotion = Potions::EMPTY;
    mDuration = 600;
    mWaitTime = 20;
    mReapplicationDelay = 20;
    mNoPhysics = true;
    mFireImmune = true;
    setRadius(3.0f);
}

void AreaEffectCloud::updateColor()
{
    if (mPotion == Potions::EMPTY && mEffects.empty())
    {
        getEntityData()->set(DATA_COLOR, 0, true);
    }
    else
    {
        std::vector<MobEffectInstance*>* allEffects = PotionUtils::getAllEffects(mPotion, &mEffects);
        getEntityData()->set(DATA_COLOR, PotionUtils::getColor(allEffects), true);
        delete allEffects;
    }
}

float AreaEffectCloud::getRadius()
{
    return getEntityData()->get(DATA_RADIUS);
}

boost::shared_ptr<LivingEntity> AreaEffectCloud::getOwner()
{
    ServerLevel* serverLevel = dynamic_cast<ServerLevel*>(mLevel);
    if (mOwner.lock() == NULL && !mOwnerUUID.empty() && serverLevel != NULL)
    {
        boost::shared_ptr<Entity> entity = serverLevel->getEntityByUUID(mOwnerUUID);
        if (entity != NULL && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
        {
            mOwner = boost::static_pointer_cast<LivingEntity>(entity);
        }
    }
    return mOwner.lock();
}

void AreaEffectCloud::applyMobEffects()
{
    float radius = getRadius();
    if (mTickCount % 5 != 0)
    {
        return;
    }

    for (boost::unordered_map<int, int>::iterator it = mReapplicationDelayMap.begin();
         it != mReapplicationDelayMap.end();)
    {
        if (mTickCount >= it->second)
        {
            it = mReapplicationDelayMap.erase(it);
        }
        else
        {
            ++it;
        }
    }

    std::vector<MobEffectInstance*> effects;
    for (std::vector<MobEffectInstance*>::iterator it = mPotion->getEffects().begin();
         it != mPotion->getEffects().end(); ++it)
    {
        MobEffectInstance* source = *it;
        effects.push_back(new MobEffectInstance(source->getEffect(), source->getDuration() / 4, source->getAmplifier(),
                                                source->isAmbient(), source->isVisible()));
    }
    effects.insert(effects.end(), mEffects.begin(), mEffects.end());

    if (effects.empty())
    {
        mReapplicationDelayMap.clear();
        return;
    }

    std::vector<boost::shared_ptr<Entity> >* nearby = mLevel->getEntitiesOfClass(typeid(LivingEntity), mBoundingBox);

    if (!nearby->empty())
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = nearby->begin(); it != nearby->end(); ++it)
        {
            boost::shared_ptr<LivingEntity> target = boost::static_pointer_cast<LivingEntity>(*it);

            if (mReapplicationDelayMap.find(target->getId()) != mReapplicationDelayMap.end()
                || !target->isAffectedByPotions())
            {
                continue;
            }

            double dx = target->mX - mX;
            double dz = target->mZ - mZ;
            if (dx * dx + dz * dz <= radius * radius)
            {
                mReapplicationDelayMap[target->getId()] = mTickCount + mReapplicationDelay;

                for (std::vector<MobEffectInstance*>::iterator effectIt = effects.begin(); effectIt != effects.end();
                     ++effectIt)
                {
                    MobEffectInstance* instance = *effectIt;
                    if (instance->getEffect()->isInstantenous())
                        instance->getEffect()->applyInstantenousEffect(
                            boost::static_pointer_cast<LivingEntity>(shared_from_this()), getOwner(), target,
                            instance->getAmplifier(), 0.5);
                    else
                        target->addEffect(new MobEffectInstance(instance), boost::shared_ptr<Entity>());

                    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType());
                    if (isPlayer && instance->getEffect() == MobEffects::WATER_BREATHING)
                        boost::static_pointer_cast<Player>(target)->ResetTimerAchievement(
                            Player::eTimerAchievement_SleepWithTheFishes);
                }

                if (mRadiusOnUse != 0.0f)
                {
                    radius += mRadiusOnUse;
                    if (radius < 0.5f)
                    {
                        remove();
                        delete nearby;
                        return;
                    }
                    setRadius(radius);
                }

                if (mDurationOnUse != 0)
                {
                    mDuration += mDurationOnUse;
                    if (mDuration < 1)
                    {
                        remove();
                        delete nearby;
                        return;
                    }
                }
            }
        }
    }

    delete nearby;
}

bool AreaEffectCloud::isWaiting()
{
    return getEntityData()->get(DATA_WAITING);
}

const ParticleType* AreaEffectCloud::getParticle()
{
    return ParticleType::byId(static_cast<ePARTICLE_TYPE>(getEntityData()->get(DATA_PARTICLE)));
}

int AreaEffectCloud::getColor()
{
    return getEntityData()->get(DATA_COLOR);
}

int AreaEffectCloud::getParticleParam1()
{
    return getEntityData()->get(DATA_PARTICLE_PARAM_1);
}

int AreaEffectCloud::getParticleParam2()
{
    return getEntityData()->get(DATA_PARTICLE_PARAM_2);
}

void AreaEffectCloud::generateParticles()
{
    bool waiting = isWaiting();
    float radius = getRadius();
    const ParticleType* particleType = getParticle();

    arrayWithLength<int> params;
    if (particleType->getParamCount() >= 1)
    {
        params = arrayWithLength<int>(particleType->getParamCount(), true);
        if (params.length != 0)
        {
            params[0] = getParticleParam1();
            if (params.length > 1)
            {
                params[1] = getParticleParam2();
            }
        }
    }

    if (waiting)
    {
        if (mRandom->nextBoolean())
        {
            for (int i = 0; i < 2; ++i)
            {
                float angle = mRandom->nextFloat() * 6.2831855f;
                float dist = std::sqrt(mRandom->nextFloat()) * 0.2f;
                float dx = std::cos(angle) * dist;
                float dz = std::sin(angle) * dist;

                if (particleType == PARTICLE_MOB_SPELL)
                {
                    int color = mRandom->nextBoolean() ? 0xffffff : getColor();
                    int red = (color >> 16) & 0xff;
                    int green = (color >> 8) & 0xff;
                    int blue = color & 0xff;
                    mLevel->addAlwaysVisibleParticle(static_cast<ePARTICLE_TYPE>(PARTICLE_MOB_SPELL->getId()), mX + dx,
                                                     mY, mZ + dz, static_cast<double>(red / 255.0f),
                                                     static_cast<double>(green / 255.0f),
                                                     static_cast<double>(blue / 255.0f), params);
                }
                else
                {
                    mLevel->addAlwaysVisibleParticle(static_cast<ePARTICLE_TYPE>(particleType->getId()), mX + dx, mY,
                                                     mZ + dz, 0.0, 0.0, 0.0, params);
                }
            }
        }
    }
    else
    {
        int rate = getEntityData()->get(DATA_PARTICLE_RATE);
        float count = rate >= 1 ? static_cast<float>(rate) : 3.1415927f * radius * radius;

        for (int i = 0; i < count; ++i)
        {
            float angle = mRandom->nextFloat() * 6.2831855f;
            float dist = std::sqrt(mRandom->nextFloat()) * radius;
            float dx = std::cos(angle) * dist;
            float dz = std::sin(angle) * dist;

            if (particleType == PARTICLE_MOB_SPELL)
            {
                int color = getColor();
                int red = (color >> 16) & 0xff;
                int green = (color >> 8) & 0xff;
                int blue = color & 0xff;
                mLevel->addAlwaysVisibleParticle(static_cast<ePARTICLE_TYPE>(PARTICLE_MOB_SPELL->getId()), mX + dx, mY,
                                                 mZ + dz, static_cast<double>(red / 255.0f),
                                                 static_cast<double>(green / 255.0f),
                                                 static_cast<double>(blue / 255.0f), params);
            }
            else
            {
                mLevel->addAlwaysVisibleParticle(static_cast<ePARTICLE_TYPE>(particleType->getId()), mX + dx, mY,
                                                 mZ + dz, (0.5 - mRandom->nextDouble()) * 0.15, 0.009999999776482582,
                                                 (0.5 - mRandom->nextDouble()) * 0.15, params);
            }
        }
    }

    delete[] params.data;
}

void AreaEffectCloud::suckInNearbyPlayers()
{
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
        shared_from_this(), mBoundingBox->expand(4.0, 4.0, 4.0)->expand(-4.0, -4.0, -4.0), NULL);
    std::vector<boost::shared_ptr<Entity> >::iterator end = entities->end();
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != end; ++it)
    {
        bool isPlayer = ((*it)->GetType() & EntityType::PLAYER) == EntityType::PLAYER;
        if (isPlayer)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(*it);
            if (!player->isSpectator() && !(player->isCreative() && player->mAbilities.mFlying) && player != getOwner())
            {
                double pull = -(Mth::clamp((4.0 - (*it)->distanceTo(mX, mY, mZ)) * 0.25, 0.0, 1.0) * 0.05);
                double dx = (*it)->mX - mX;
                double dy = ((*it)->mY + (*it)->getEyeHeight()) - mY;
                double dz = (*it)->mZ - mZ;
                double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                if (distance == 0.0)
                {
                    continue;
                }
                double pushX = dx / distance * pull;
                double pushY = dy / distance * pull;
                double pushZ = dz / distance * pull;
                player->mXd += pushX;
                player->mYd += pushY;
                player->mZd += pushZ;
            }
        }
    }
}

void AreaEffectCloud::defineSynchedData()
{
    getEntityData()->define(DATA_COLOR, 0);
    getEntityData()->define(DATA_RADIUS, 0.5f);
    getEntityData()->define(DATA_WAITING, false);
    getEntityData()->define(DATA_PARTICLE, static_cast<int>(PARTICLE_MOB_SPELL->getId()));
    getEntityData()->define(DATA_PARTICLE_PARAM_1, 0);
    getEntityData()->define(DATA_PARTICLE_PARAM_2, 0);
    getEntityData()->define(DATA_PARTICLE_RATE, 0);
}

void AreaEffectCloud::setPotion(Potion* potion)
{
    mPotion = potion;
    if (!mFixedColor)
        updateColor();
}

void AreaEffectCloud::addEffect(MobEffectInstance* effect)
{
    mEffects.push_back(effect);
    if (!mFixedColor)
        updateColor();
}

void AreaEffectCloud::setFixedColor(int color)
{
    mFixedColor = 1;
    getEntityData()->set(DATA_COLOR, color, true);
}

void AreaEffectCloud::setParticle(const ParticleType* particle)
{
    getEntityData()->set(DATA_PARTICLE, static_cast<int>(particle->getId()), true);
}

void AreaEffectCloud::setParticleParam1(int value)
{
    getEntityData()->set(DATA_PARTICLE_PARAM_1, value, true);
}

void AreaEffectCloud::setParticleParam2(int value)
{
    getEntityData()->set(DATA_PARTICLE_PARAM_2, value, true);
}

void AreaEffectCloud::setParticleSpawnRateOnTick(int value)
{
    if (!mLevel->mIsClientSide)
    {
        getEntityData()->set(DATA_PARTICLE_RATE, value, true);
    }
}

void AreaEffectCloud::setWaiting(bool waiting)
{
    getEntityData()->set(DATA_WAITING, waiting, true);
}

int AreaEffectCloud::getDuration()
{
    return mDuration;
}

void AreaEffectCloud::setDuration(int duration)
{
    mDuration = duration;
}

void AreaEffectCloud::setDurationOnUse(int duration)
{
    mDurationOnUse = duration;
}

void AreaEffectCloud::tick()
{
    Entity::tick();
    bool waiting = isWaiting();
    float radius = getRadius();
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades())
    {
        suckInNearbyPlayers();
    }
    if (mLevel->mIsClientSide)
    {
        generateParticles();
    }
    else
    {
        if (mTickCount >= mWaitTime + mDuration)
        {
            remove();
            return;
        }
        bool shouldWait = mTickCount < mWaitTime;
        if (waiting != shouldWait)
            setWaiting(shouldWait);
        if (!shouldWait)
        {
            if (mRadiusPerTick != 0.0f)
            {
                radius += mRadiusPerTick;
                if (radius < 0.5f)
                {
                    remove();
                    return;
                }
                setRadius(radius);
            }
            if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades())
            {
                applyMobEffects();
            }
        }
    }
}

void AreaEffectCloud::setRadiusOnUse(float radius)
{
    mRadiusOnUse = radius;
}

void AreaEffectCloud::setRadiusPerTick(float radius)
{
    mRadiusPerTick = radius;
}

void AreaEffectCloud::setWaitTime(int waitTime)
{
    mWaitTime = waitTime;
}

void AreaEffectCloud::setOwner(boost::shared_ptr<LivingEntity> owner)
{
    mOwner = owner;
    mOwnerUUID = owner == NULL ? std::wstring(L"") : owner->getUUID();
}

void AreaEffectCloud::readAdditionalSaveData(CompoundTag* tag)
{
    mTickCount = tag->getInt(L"Age");
    mDuration = tag->getInt(L"Duration");
    mWaitTime = tag->getInt(L"WaitTime");
    mReapplicationDelay = tag->getInt(L"ReapplicationDelay");
    mDurationOnUse = tag->getInt(L"DurationOnUse");
    mRadiusOnUse = tag->getFloat(L"RadiusOnUse");
    mRadiusPerTick = tag->getFloat(L"RadiusPerTick");
    setRadius(tag->getFloat(L"Radius"));
    mOwnerUUID = tag->getString(L"OwnerUUID");
    if (tag->contains(L"Particle", Tag::TAG_INT))
    {
        const ParticleType* particle = ParticleType::byId(static_cast<ePARTICLE_TYPE>(tag->getInt(L"Particle")));
        if (particle != NULL)
        {
            setParticle(particle);
            setParticleParam1(tag->getInt(L"ParticleParam1"));
            setParticleParam2(tag->getInt(L"ParticleParam2"));
        }
    }
    if (tag->contains(L"Color", Tag::TAG_ANY_NUMERIC))
        setFixedColor(tag->getInt(L"Color"));
    if (tag->contains(PotionUtils::TAG_POTION, Tag::TAG_STRING))
        setPotion(PotionUtils::getPotion(tag));
    if (tag->contains(L"Effects", Tag::TAG_LIST))
    {
        ListTag* effects = tag->getList(L"Effects", Tag::TAG_COMPOUND);
        mEffects.clear();
        for (int i = 0; i < effects->size(); ++i)
        {
            MobEffectInstance* effect = MobEffectInstance::load(effects->getCompound(i));
            if (effect != NULL)
                addEffect(effect);
        }
    }
}

void AreaEffectCloud::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"Age", mTickCount);
    tag->putInt(L"Duration", mDuration);
    tag->putInt(L"WaitTime", mWaitTime);
    tag->putInt(L"ReapplicationDelay", mReapplicationDelay);
    tag->putInt(L"DurationOnUse", mDurationOnUse);
    tag->putFloat(L"RadiusOnUse", mRadiusOnUse);
    tag->putFloat(L"RadiusPerTick", mRadiusPerTick);
    tag->putFloat(L"Radius", getRadius());
    tag->putInt(L"Particle", getParticle()->getId());
    tag->putInt(L"ParticleParam1", getParticleParam1());
    tag->putInt(L"ParticleParam2", getParticleParam2());
    if (!mOwnerUUID.empty())
        tag->putString(L"OwnerUUID", mOwnerUUID);
    if (mFixedColor)
        tag->putInt(L"Color", getColor());
    if (mPotion != Potions::EMPTY && mPotion != NULL)
        tag->putString(PotionUtils::TAG_POTION, Potion::potionRegistry->getKey(mPotion).toString());
    if (!mEffects.empty())
    {
        ListTag* effects = new ListTag();
        for (std::vector<MobEffectInstance*>::iterator it = mEffects.begin(); it != mEffects.end(); ++it)
        {
            MobEffectInstance* effect = *it;
            effects->add(effect->save(new CompoundTag()));
        }
        tag->put(L"Effects", effects);
    }
}

void AreaEffectCloud::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor->getId() == DATA_RADIUS->getId())
        setRadius(getRadius());
    Entity::onSyncedDataUpdated(accessor);
}

int AreaEffectCloud::getPistonPushReaction()
{
    return 3;
}

unsigned long long AreaEffectCloud::GetType()
{
    return EntityType::AREA_EFFECT_CLOUD;
}

Entity* AreaEffectCloud::create(Level* level)
{
    return new AreaEffectCloud(level);
}
