#include "game/entity/mob/MobEffect.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cwchar>

#include <boost/shared_ptr.hpp>

#include "game/core/BaseAttributeMap.hpp"
#include "game/entity/mob/AbsoptionMobEffect.hpp"
#include "game/entity/mob/AttackDamageMobEffect.hpp"
#include "game/entity/mob/HealthBoostMobEffect.hpp"
#include "game/entity/mob/InstantenousMobEffect.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"

MappedRegistry<ResourceLocation, MobEffect*>* MobEffect::mobEffectRegistry =
    new MappedRegistry<ResourceLocation, MobEffect*>();
static unsigned int effectCount;

MobEffect::MobEffect(bool isHarmful, eMinecraftColour effectColor)
{
    harmful = isHarmful;
    color = effectColor;
    descriptionId = -1;
    icon = 0;
    beneficial = false;

    if (isHarmful)
        durationModifier = 0.5;
    else
        durationModifier = 1.0;
}

MobEffect* MobEffect::setIcon(EMobEffectIcon effectIcon)
{
    icon = (unsigned int)effectIcon;
    return this;
}

MobEffect* MobEffect::setDescriptionId(int id)
{
    descriptionId = id;
    return this;
}

MobEffect* MobEffect::setDurationModifier(double modifier)
{
    durationModifier = modifier;
    return this;
}

void MobEffect::staticCtor()
{
    effectCount = 0;

    mobEffectRegistry->registerMapping(
        ++effectCount, ResourceLocation(L"minecraft", L"speed"),
        (new MobEffect(false, eMinecraftColour_Effect_MovementSpeed))
            ->setDescriptionId(IDS_EFFECT_MOVESPEED)
            ->setIcon(EMobEffectIcon_Speed)
            ->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED, eMODIFIER_ID_PotionMoveSpeed, 0.2f, 2)
            ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"slowness"),
                                       (new MobEffect(true, eMinecraftColour_Effect_MovementSlowDown))
                                           ->setDescriptionId(IDS_EFFECT_MOVESLOWDOWN)
                                           ->setIcon(EMobEffectIcon_Slowness)
                                           ->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED,
                                                                  eMODIFIER_ID_PotionMoveSlowdown, -0.15f, 2));

    mobEffectRegistry->registerMapping(
        ++effectCount, ResourceLocation(L"minecraft", L"haste"),
        (new MobEffect(false, eMinecraftColour_Effect_DigSpeed))
            ->setDescriptionId(IDS_EFFECT_DIGSPEED)
            ->setDurationModifier(1.5)
            ->setIcon(EMobEffectIcon_Haste)
            ->setBeneficial()
            ->addAttributeModifier(SharedMonsterAttributes::ATTACK_SPEED, eMODIFIER_ID_PotionDigSpeed, 0.1f, 2));

    mobEffectRegistry->registerMapping(
        ++effectCount, ResourceLocation(L"minecraft", L"mining_fatigue"),
        (new MobEffect(true, eMinecraftColour_Effect_DigSlowdown))
            ->setDescriptionId(IDS_EFFECT_DIGSLOWDOWN)
            ->setIcon(EMobEffectIcon_MiningFatigue)
            ->addAttributeModifier(SharedMonsterAttributes::ATTACK_SPEED, eMODIFIER_ID_PotionDigSpeed, -0.1f, 2));

    mobEffectRegistry->registerMapping(
        ++effectCount, ResourceLocation(L"minecraft", L"strength"),
        (new AttackDamageMobEffect(false, eMinecraftColour_Effect_DamageBoost, 3.0))
            ->setDescriptionId(IDS_EFFECT_DAMAGEBOOST)
            ->setIcon(EMobEffectIcon_Strength)
            ->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE, eMODIFIER_ID_PotionDamageBoost, 0.0, 0)
            ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"instant_health"),
                                       (new InstantenousMobEffect(false, eMinecraftColour_Effect_Heal))
                                           ->setDescriptionId(IDS_EFFECT_HEAL)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"instant_damage"),
                                       (new InstantenousMobEffect(true, eMinecraftColour_Effect_Harm))
                                           ->setDescriptionId(IDS_EFFECT_HARM)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"jump_boost"),
                                       (new MobEffect(false, eMinecraftColour_Effect_Jump))
                                           ->setDescriptionId(IDS_EFFECT_JUMP)
                                           ->setIcon(EMobEffectIcon_JumpBoost)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"nausea"),
                                       (new MobEffect(true, eMinecraftColour_Effect_Confusion))
                                           ->setDescriptionId(IDS_EFFECT_CONFUSION)
                                           ->setDurationModifier(0.25)
                                           ->setIcon(EMobEffectIcon_Nausea));

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"regeneration"),
                                       (new MobEffect(false, eMinecraftColour_Effect_Regeneration))
                                           ->setDescriptionId(IDS_EFFECT_REGENERATION)
                                           ->setDurationModifier(0.25)
                                           ->setIcon(EMobEffectIcon_Regeneration)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"resistance"),
                                       (new MobEffect(false, eMinecraftColour_Effect_DamageResistance))
                                           ->setDescriptionId(IDS_EFFECT_RESISTANCE)
                                           ->setIcon(EMobEffectIcon_Resistance)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"fire_resistance"),
                                       (new MobEffect(false, eMinecraftColour_Effect_FireResistance))
                                           ->setDescriptionId(IDS_EFFECT_FIRERESISTANCE)
                                           ->setIcon(EMobEffectIcon_FireResistance)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"water_breathing"),
                                       (new MobEffect(false, eMinecraftColour_Effect_WaterBreathing))
                                           ->setDescriptionId(IDS_EFFECT_WATERBREATHING)
                                           ->setIcon(EMobEffectIcon_WaterBreathing)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"invisibility"),
                                       (new MobEffect(false, eMinecraftColour_Effect_Invisiblity))
                                           ->setDescriptionId(IDS_EFFECT_INVISIBILITY)
                                           ->setIcon(EMobEffectIcon_Invisibility)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"blindness"),
                                       (new MobEffect(true, eMinecraftColour_Effect_Blindness))
                                           ->setDescriptionId(IDS_EFFECT_BLINDNESS)
                                           ->setDurationModifier(0.25)
                                           ->setIcon(EMobEffectIcon_Blindness));

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"night_vision"),
                                       (new MobEffect(false, eMinecraftColour_Effect_NightVision))
                                           ->setDescriptionId(IDS_EFFECT_NIGHTVISION)
                                           ->setIcon(EMobEffectIcon_NightVision)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"hunger"),
                                       (new MobEffect(true, eMinecraftColour_Effect_Hunger))
                                           ->setDescriptionId(IDS_EFFECT_HUNGER)
                                           ->setIcon(EMobEffectIcon_Hunger));

    mobEffectRegistry->registerMapping(
        ++effectCount, ResourceLocation(L"minecraft", L"weakness"),
        (new AttackDamageMobEffect(true, eMinecraftColour_Effect_Weakness, -4.0))
            ->setDescriptionId(IDS_EFFECT_WEAKNESS)
            ->setIcon(EMobEffectIcon_Weakness)
            ->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE, eMODIFIER_ID_PotionWeakness, 0.0, 0));

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"poison"),
                                       (new MobEffect(true, eMinecraftColour_Effect_Poison))
                                           ->setDescriptionId(IDS_EFFECT_POISON)
                                           ->setDurationModifier(0.25)
                                           ->setIcon(EMobEffectIcon_Poison));

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"wither"),
                                       (new MobEffect(true, eMinecraftColour_Effect_Wither))
                                           ->setDescriptionId(IDS_EFFECT_WITHER)
                                           ->setIcon(EMobEffectIcon_Wither)
                                           ->setDurationModifier(0.25));

    mobEffectRegistry->registerMapping(
        ++effectCount, ResourceLocation(L"minecraft", L"health_boost"),
        (new HealthBoostMobEffect(false, eMinecraftColour_Effect_HealthBoost))
            ->setDescriptionId(IDS_EFFECT_HEALTHBOOST)
            ->setIcon(EMobEffectIcon_HealthBoost)
            ->addAttributeModifier(SharedMonsterAttributes::MAX_HEALTH, eMODIFIER_ID_PotionHealthBoost, 4.0, 0)
            ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"absorption"),
                                       (new AbsoptionMobEffect(false, eMinecraftColour_Effect_Absorption))
                                           ->setDescriptionId(IDS_EFFECT_ABSORPTION)
                                           ->setIcon(EMobEffectIcon_Absorption)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"saturation"),
                                       (new InstantenousMobEffect(false, eMinecraftColour_Effect_Saturation))
                                           ->setDescriptionId(IDS_EFFECT_SATURATION)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"glowing"),
                                       (new MobEffect(false, eMinecraftColour_Effect_Glowing))
                                           ->setDescriptionId(IDS_EFFECT_GLOWING)
                                           ->setIcon(EMobEffectIcon_Glowing));

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"levitation"),
                                       (new MobEffect(true, eMinecraftColour_Effect_Levitation))
                                           ->setDescriptionId(IDS_EFFECT_LEVITATION)
                                           ->setIcon(EMobEffectIcon_Levitation));

    mobEffectRegistry->registerMapping(
        ++effectCount, ResourceLocation(L"minecraft", L"luck"),
        (new MobEffect(false, eMinecraftColour_Effect_Luck))
            ->setDescriptionId(IDS_EFFECT_LUCK)
            ->setIcon(EMobEffectIcon_Luck)
            ->setBeneficial()
            ->addAttributeModifier(SharedMonsterAttributes::LUCK, eMODIFIER_ID_PotionLuck, 1.0, 0));

    mobEffectRegistry->registerMapping(
        ++effectCount, ResourceLocation(L"minecraft", L"unluck"),
        (new MobEffect(true, eMinecraftColour_Effect_BadLuck))
            ->setDescriptionId(IDS_EFFECT_UNLUCK)
            ->setIcon(EMobEffectIcon_Unluck)
            ->addAttributeModifier(SharedMonsterAttributes::LUCK, eMODIFIER_ID_PotionLuck, -1.0, 0));

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"conduit_power"),
                                       (new MobEffect(false, eMinecraftColour_Effect_ConduitPower))
                                           ->setDescriptionId(IDS_EFFECT_CONDUITPOWER)
                                           ->setIcon(EMobEffectIcon_ConduitPower)
                                           ->setBeneficial());

    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"slow_falling"),
                                       (new MobEffect(false, eMinecraftColour_Effect_SlowFall))
                                           ->setDescriptionId(IDS_EFFECT_SLOWFALLING)
                                           ->setIcon(EMobEffectIcon_SlowFalling)
                                           ->setBeneficial());
}

unsigned int MobEffect::GetEffectCount()
{
    return effectCount;
}

MobEffect* MobEffect::byId(int id)
{
    return mobEffectRegistry->byId(id);
}

int MobEffect::getId(MobEffect* effect)
{
    return mobEffectRegistry->getId(effect);
}

MobEffect* MobEffect::byName(const std::wstring& name)
{
    return mobEffectRegistry->get(ResourceLocation(name));
}

void MobEffect::applyEffectTick(const boost::shared_ptr<LivingEntity>& entity, int amplifier)
{
    if (this == MobEffects::REGENERATION)
    {
        if (entity->getHealth() < entity->getMaxHealth())
        {
            entity->heal(1.0f);
        }
    }
    else if (this == MobEffects::POISON)
    {
        if (entity->getHealth() > 1.0f)
        {
            entity->hurt(DamageSource::POISON, 1.0f);
        }
    }
    else if (this == MobEffects::WITHER)
    {
        entity->hurt(DamageSource::WITHER, 1.0f);
    }
    else if (this == MobEffects::HUNGER && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
        player->causeFoodExhaustion(FoodConstants::ExhaustionMine() * (float)(amplifier + 1));
    }
    else if (this == MobEffects::SATURATION && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        if (!entity->mLevel->mIsClientSide)
        {
            boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
            player->getFoodData()->eat(amplifier + 1, FoodConstants::FOOD_SATURATION_MAX);
        }
    }
    else if ((this == MobEffects::HEAL && !entity->isInvertedHealAndHarm())
             || (this == MobEffects::HARM && entity->isInvertedHealAndHarm()))
    {
        entity->heal((float)std::max(4 << amplifier, 0));
    }
    else if ((this == MobEffects::HARM && !entity->isInvertedHealAndHarm())
             || (this == MobEffects::HEAL && entity->isInvertedHealAndHarm()))
    {
        entity->hurt(DamageSource::INSTANT_DAMAGE, (float)(6 << amplifier));
    }
}

void MobEffect::applyInstantenousEffect(const boost::shared_ptr<Entity>& source,
                                        const boost::shared_ptr<Entity>& indirectSource,
                                        const boost::shared_ptr<LivingEntity>& entity, int amplifier, double health)
{
    if ((this == MobEffects::HEAL && !entity->isInvertedHealAndHarm())
        || (this == MobEffects::HARM && entity->isInvertedHealAndHarm()))
    {
        float amount = (float)(health * (double)(4 << amplifier) + 0.5);
        entity->heal(amount);
    }
    else if ((this == MobEffects::HARM && !entity->isInvertedHealAndHarm())
             || (this == MobEffects::HEAL && entity->isInvertedHealAndHarm()))
    {
        float amount = (float)(health * (double)(6 << amplifier) + 0.5);
        if (source == NULL)
        {
            entity->hurt(DamageSource::MAGIC, amount);
        }
        else
        {
            DamageSource* damage = DamageSource::CreateIndirectMagic(source, indirectSource,
                                                                     DamageSource::INSTANT_DAMAGE);
            entity->hurt(damage, amount);
            delete damage;
        }
    }
}

bool MobEffect::isDurationEffectTick(int duration, int amplifier)
{
    if (this == MobEffects::REGENERATION)
    {
        int interval = 50 >> amplifier;
        if (interval > 0)
        {
            return duration % interval == 0;
        }
        return true;
    }
    if (this == MobEffects::POISON)
    {
        int interval = 25 >> amplifier;
        if (interval > 0)
        {
            return duration % interval == 0;
        }
        return true;
    }
    if (this == MobEffects::WITHER)
    {
        int interval = 40 >> amplifier;
        if (interval > 0)
        {
            return duration % interval == 0;
        }
        return true;
    }
    if (this == MobEffects::HUNGER)
    {
        return true;
    }
    return false;
}

bool MobEffect::isInstantenous()
{
    return false;
}

int MobEffect::getDescriptionId(int)
{
    return descriptionId;
}

bool MobEffect::hasIcon() const
{
    return icon != 0;
}

unsigned int MobEffect::getIcon() const
{
    return icon;
}

bool MobEffect::isHarmful() const
{
    return harmful;
}

double MobEffect::getDurationModifier() const
{
    return durationModifier;
}

MobEffect* MobEffect::setBeneficial()
{
    beneficial = true;
    return this;
}

bool MobEffect::isBeneficial() const
{
    return beneficial;
}

eMinecraftColour MobEffect::getColor() const
{
    return color;
}

MobEffect* MobEffect::addAttributeModifier(Attribute* attribute, eMODIFIER_ID id, double amount, int operation)
{
    attributeModifiers.insert(boost::unordered_map<Attribute*, AttributeModifier*>::value_type(
        attribute, new AttributeModifier(id, amount, operation)));
    return this;
}

boost::unordered_map<Attribute*, AttributeModifier*>* MobEffect::getAttributeModifiers()
{
    return &attributeModifiers;
}

void MobEffect::removeAttributeModifiers(const boost::shared_ptr<LivingEntity>&, BaseAttributeMap* attributes, int)
{
    for (boost::unordered_map<Attribute*, AttributeModifier*>::iterator it = attributeModifiers.begin();
         it != attributeModifiers.end(); ++it)
    {
        AttributeInstance* instance = attributes->getInstance(it->first);
        if (instance)
        {
            instance->removeModifier(it->second);
        }
    }
}

void MobEffect::addAttributeModifiers(const boost::shared_ptr<LivingEntity>&, BaseAttributeMap* attributes,
                                      int amplifier)
{
    for (boost::unordered_map<Attribute*, AttributeModifier*>::iterator it = attributeModifiers.begin();
         it != attributeModifiers.end(); ++it)
    {
        AttributeInstance* instance = attributes->getInstance(it->first);
        if (instance)
        {
            AttributeModifier* modifier = it->second;
            instance->removeModifier(modifier);
            instance->addModifier(new AttributeModifier(
                modifier->getId(), getAttributeModifierValue(amplifier, modifier), modifier->getOperation()));
        }
    }
}

double MobEffect::getAttributeModifierValue(int amplifier, AttributeModifier* modifier)
{
    return modifier->getAmount() * (double)(amplifier + 1);
}

std::wstring MobEffect::formatDuration(MobEffectInstance* effect, float durationFactor)
{
    if (effect->isNoCounter())
    {
        return L"**:**";
    }
    int ticks = (int)std::floor((float)effect->getDuration() * durationFactor);
    int seconds = ticks / 20;
    int minutes = seconds / 60;
    seconds %= 60;
    wchar_t buffer[8];
    std::memset(buffer, 0, sizeof(buffer));
    if (seconds < 10)
        std::swprintf(buffer, 8, L"%d:0%d", minutes, seconds);
    else
        std::swprintf(buffer, 8, L"%d:%d", minutes, seconds);
    return std::wstring(buffer);
}
