#include "game/entity/projectile/TippableArrow.hpp"

#include <algorithm>

#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/Potions.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/Level.hpp"

const EntityDataAccessor<int>* TippableArrow::DATA_EFFECT_COLOR_ID = NULL;

TippableArrow::TippableArrow(Level* level) : Arrow(level)
{
    init();
}

TippableArrow::TippableArrow(Level* level, double x, double y, double z) : Arrow(level, x, y, z)
{
    init();
}

TippableArrow::TippableArrow(Level* level, boost::shared_ptr<LivingEntity> owner) : Arrow(level, owner)
{
    init();
}

Entity* TippableArrow::create(Level* level)
{
    return new TippableArrow(level);
}

void TippableArrow::staticCtor()
{
    DATA_EFFECT_COLOR_ID = SynchedEntityData::defineId(EntityType::TIPPABLE_ARROW, EntityType::ARROW,
                                                       EntityDataSerializers::INT);
}

void TippableArrow::init()
{
    fjDerivedCtorCalls();
    mFixedColor = false;
    mPotion = Potions::EMPTY;
}

int TippableArrow::getCustomColor(const not_null_ptr<ItemInstance>& itemInstance)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag != NULL && tag->contains(PotionUtils::TAG_CUSTOM_POTION_COLOR, Tag::TAG_ANY_NUMERIC))
        return tag->getInt(PotionUtils::TAG_CUSTOM_POTION_COLOR);
    return -1;
}

void TippableArrow::updateColor()
{
    mFixedColor = false;
    SynchedEntityData* entityData = mEntityData.operator->();
    if (mPotion == Potions::EMPTY && mEffects.empty())
    {
        entityData->set(DATA_EFFECT_COLOR_ID, -1, true);
    }
    else
    {
        std::vector<MobEffectInstance*>* allEffects = PotionUtils::getAllEffects(mPotion, &mEffects);
        int color = PotionUtils::getColor(allEffects);
        delete allEffects;
        entityData->set(DATA_EFFECT_COLOR_ID, color, true);
    }
}

void TippableArrow::setFixedColor(int color)
{
    mFixedColor = true;
    mEntityData->set(DATA_EFFECT_COLOR_ID, color, true);
}

void TippableArrow::setEffectsFromItem(const not_null_ptr<ItemInstance>& itemInstance)
{
    if (itemInstance->getItem() == Items::tippedArrow)
    {
        mPotion = PotionUtils::getPotion(itemInstance->getTag());
        std::vector<MobEffectInstance*>* effects = PotionUtils::getCustomEffects(itemInstance);
        if (!effects->empty())
        {
            for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
                mEffects.push_back(*it);
        }

        int color = getCustomColor(itemInstance);
        if (color == -1)
            updateColor();
        else
            setFixedColor(color);
        delete effects;
    }
    else if (itemInstance->getItem() == Items::arrow)
    {
        mPotion = Potions::EMPTY;
        mEffects.clear();
        mEntityData->set(DATA_EFFECT_COLOR_ID, -1, true);
    }
}

void TippableArrow::addEffect(MobEffectInstance* effect)
{
    mEffects.push_back(effect);
    boost::shared_ptr<SynchedEntityData> entityDataRef = getEntityData();
    SynchedEntityData* entityData = entityDataRef.operator->();
    if (mPotion == Potions::EMPTY && mEffects.empty())
    {
        entityData->set(DATA_EFFECT_COLOR_ID, -1, true);
    }
    else
    {
        std::vector<MobEffectInstance*>* allEffects = PotionUtils::getAllEffects(mPotion, &mEffects);
        int color = PotionUtils::getColor(allEffects);
        delete allEffects;
        entityData->set(DATA_EFFECT_COLOR_ID, color, true);
    }
}

void TippableArrow::defineSynchedData()
{
    AbstractRotatedItemProjectile::defineSynchedData();
    mEntityData->createDateItem(DATA_EFFECT_COLOR_ID, -1);
}

int TippableArrow::getColor()
{
    return mEntityData->get(DATA_EFFECT_COLOR_ID);
}

void TippableArrow::makeParticle(int count)
{
    int color = getColor();
    if (color == -1 || count <= 0)
        return;

    double red = static_cast<double>((color >> 16) & 255) / 255.0;
    double green = static_cast<double>((color >> 8) & 255) / 255.0;
    double blue = static_cast<double>(color & 255) / 255.0;
    for (int i = 0; i < count; ++i)
    {
        mLevel->addParticle(PARTICLE_MOB_SPELL, mX + (mRandom->nextDouble() - 0.5) * mBbWidth,
                            mY + mRandom->nextDouble() * mBbHeight, mZ + (mRandom->nextDouble() - 0.5) * mBbWidth, red,
                            green, blue, arrayWithLength<int>());
    }
}

void TippableArrow::tick()
{
    AbstractRotatedItemProjectile::tick();
    if (mLevel->mIsClientSide)
    {
        if (mInGround)
        {
            if (mInGroundTime % 5 == 0)
                makeParticle(1);
        }
        else
        {
            makeParticle(2);
        }
    }
    else if (mInGround && mInGroundTime != 0 && !mEffects.empty() && mInGroundTime >= 600)
    {
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::TIPPED_ARROW_PARTICLES, 0);
        mPotion = Potions::EMPTY;
        mEffects.clear();
        mEntityData->set(DATA_EFFECT_COLOR_ID, -1, true);
    }
}

void TippableArrow::addDataWalkers(DataFixerUpper* fixer)
{
    Arrow::addDataWalkers(fixer, EntityType::TIPPABLE_ARROW);
}

void TippableArrow::addAdditonalSaveData(CompoundTag* tag)
{
    Arrow::addAdditonalSaveData(tag);
    if (mPotion != Potions::EMPTY && mPotion != NULL)
        tag->putString(PotionUtils::TAG_POTION, Potion::potionRegistry->getKey(mPotion).toString());
    if (mFixedColor)
        tag->putInt(L"Color", getColor());
    if (!mEffects.empty())
    {
        ListTag* list = new ListTag();
        for (std::vector<MobEffectInstance*>::iterator it = mEffects.begin(); it != mEffects.end(); ++it)
            list->add((*it)->save(new CompoundTag()));
        tag->put(PotionUtils::TAG_CUSTOM_POTION_EFFECTS, list);
    }
}

void TippableArrow::readAdditionalSaveData(CompoundTag* tag)
{
    Arrow::readAdditionalSaveData(tag);
    if (tag->contains(PotionUtils::TAG_POTION, Tag::TAG_STRING))
    {
        mPotion = PotionUtils::getPotion(tag);
    }

    std::vector<MobEffectInstance*>* customEffects = PotionUtils::getCustomEffects(tag);
    for (std::vector<MobEffectInstance*>::iterator it = customEffects->begin(); it != customEffects->end(); ++it)
    {
        addEffect(*it);
    }
    delete customEffects;

    if (mPotion != Potions::EMPTY || !mEffects.empty())
    {
        SynchedEntityData* entityData = mEntityData.operator->();
        if (mPotion == Potions::EMPTY && mEffects.empty())
        {
            entityData->set(DATA_EFFECT_COLOR_ID, -1, true);
        }
        else
        {
            std::vector<MobEffectInstance*>* allEffects = PotionUtils::getAllEffects(mPotion, &mEffects);
            int color = PotionUtils::getColor(allEffects);
            delete allEffects;
            entityData->set(DATA_EFFECT_COLOR_ID, color, true);
        }
    }

    if (tag->contains(L"Color", Tag::TAG_ANY_NUMERIC))
    {
        setFixedColor(tag->getInt(L"Color"));
    }
    else
    {
        updateColor();
    }
}

void TippableArrow::doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity)
{
    AbstractRotatedItemProjectile::doPostHurtEffects(entity);
    std::vector<MobEffectInstance*>& potionEffects = mPotion->getEffects();
    for (std::vector<MobEffectInstance*>::iterator it = potionEffects.begin(); it != potionEffects.end(); ++it)
    {
        MobEffectInstance* effect = *it;
        if (Minecraft::InMiniGame(eMiniGameId_Battle, false))
        {
            entity->addEffect(new MobEffectInstance(effect->getEffect(), std::max(effect->getDuration() / 3, 1),
                                                    effect->getAmplifier(), effect->isAmbient(), effect->isVisible()),
                              boost::shared_ptr<Entity>());
        }
        else
        {
            entity->addEffect(new MobEffectInstance(effect->getEffect(), std::max(effect->getDuration() / 8, 1),
                                                    effect->getAmplifier(), effect->isAmbient(), effect->isVisible()),
                              boost::shared_ptr<Entity>());
        }
    }
    if (!mEffects.empty())
    {
        for (std::vector<MobEffectInstance*>::iterator it = mEffects.begin(); it != mEffects.end(); ++it)
            entity->addEffect(*it, boost::shared_ptr<Entity>());
    }
}

not_null_ptr<ItemInstance> TippableArrow::getPickupItem()
{
    if (mEffects.empty() && mPotion == Potions::EMPTY)
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::arrow));

    not_null_ptr<ItemInstance> item(new ItemInstance(Items::tippedArrow));
    PotionUtils::setPotion(item, mPotion);
    PotionUtils::setCustomEffects(item, &mEffects);
    if (mFixedColor)
    {
        CompoundTag* tag = item->getTag();
        if (tag == NULL)
        {
            tag = new CompoundTag();
            item->setTag(tag);
        }
        tag->putInt(PotionUtils::TAG_CUSTOM_POTION_COLOR, getColor());
    }
    return item;
}

void TippableArrow::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::TIPPED_ARROW_PARTICLES)
    {
        int color = getColor();
        if (color != -1)
        {
            double red = static_cast<double>((color >> 16) & 255) / 255.0;
            double green = static_cast<double>((color >> 8) & 255) / 255.0;
            double blue = static_cast<double>(color & 255) / 255.0;
            for (int i = 0; i < 20; ++i)
            {
                mLevel->addParticle(PARTICLE_MOB_SPELL, mX + (mRandom->nextDouble() - 0.5) * mBbWidth,
                                    mY + mRandom->nextDouble() * mBbHeight,
                                    mZ + (mRandom->nextDouble() - 0.5) * mBbWidth, red, green, blue,
                                    arrayWithLength<int>());
            }
        }
    }
    else
    {
        Entity::handleEntityEvent(event, data);
    }
}

unsigned long long TippableArrow::GetType()
{
    return EntityType::TIPPABLE_ARROW;
}
