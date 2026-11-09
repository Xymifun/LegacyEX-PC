#ifndef GAME_ENTITY_MOB_MOBEFFECT_HPP
#define GAME_ENTITY_MOB_MOBEFFECT_HPP

#include <functional>
#include <string>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/AttributeModifier.hpp"
#include "game/core/ColourTable.hpp"
#include "game/world/block/IdMapper.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/util/ResourceLocation.hpp"

class BaseAttributeMap;
class DamageSource;
class Entity;
class LivingEntity;
class MobEffectInstance;

class MobEffect
{
public:
    enum EMobEffectIcon
    {
        EMobEffectIcon_None,
        EMobEffectIcon_Blindness,
        EMobEffectIcon_FireResistance,
        EMobEffectIcon_Haste,
        EMobEffectIcon_Hunger,
        EMobEffectIcon_Invisibility,
        EMobEffectIcon_JumpBoost,
        EMobEffectIcon_MiningFatigue,
        EMobEffectIcon_Nausea,
        EMobEffectIcon_NightVision,
        EMobEffectIcon_Poison,
        EMobEffectIcon_Regeneration,
        EMobEffectIcon_Resistance,
        EMobEffectIcon_Slowness,
        EMobEffectIcon_Speed,
        EMobEffectIcon_Strength,
        EMobEffectIcon_WaterBreathing,
        EMobEffectIcon_Weakness,
        EMobEffectIcon_Wither,
        EMobEffectIcon_HealthBoost,
        EMobEffectIcon_Absorption,
        EMobEffectIcon_Glowing,
        EMobEffectIcon_Levitation,
        EMobEffectIcon_Luck,
        EMobEffectIcon_Unluck,
        EMobEffectIcon_SlowFalling = 26,
        EMobEffectIcon_ConduitPower
    };

    static MappedRegistry<ResourceLocation, MobEffect*>* mobEffectRegistry;

    MobEffect(bool harmful, eMinecraftColour color);

    MobEffect* setIcon(EMobEffectIcon icon);
    MobEffect* setDescriptionId(int descriptionId);
    MobEffect* setDurationModifier(double durationModifier);
    static void staticCtor();
    static unsigned int GetEffectCount();
    static MobEffect* byId(int id);
    static int getId(MobEffect* effect);
    static MobEffect* byName(const std::wstring& name);

    virtual void applyEffectTick(const boost::shared_ptr<LivingEntity>& entity, int amplifier);
    virtual void applyInstantenousEffect(const boost::shared_ptr<Entity>& source,
                                         const boost::shared_ptr<Entity>& indirectSource,
                                         const boost::shared_ptr<LivingEntity>& entity, int amplifier, double health);
    virtual bool isDurationEffectTick(int duration, int amplifier);
    virtual bool isInstantenous();
    int getDescriptionId(int auxiliaryValue);
    bool hasIcon() const;
    unsigned int getIcon() const;
    bool isHarmful() const;
    virtual double getDurationModifier() const;
    virtual MobEffect* setBeneficial();
    virtual bool isBeneficial() const;
    virtual eMinecraftColour getColor() const;
    virtual MobEffect* addAttributeModifier(Attribute* attribute, eMODIFIER_ID id, double amount, int operation);
    virtual boost::unordered_map<Attribute*, AttributeModifier*>* getAttributeModifiers();
    virtual void removeAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity, BaseAttributeMap* attributes,
                                          int amplifier);
    virtual void addAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity, BaseAttributeMap* attributes,
                                       int amplifier);
    virtual double getAttributeModifierValue(int amplifier, AttributeModifier* modifier);
    virtual ~MobEffect() {}
    static std::wstring formatDuration(MobEffectInstance* effect, float durationFactor);

private:
    boost::unordered_map<Attribute*, AttributeModifier*> attributeModifiers;
    int descriptionId;
    unsigned int icon;
    bool harmful;
    double durationModifier;
    bool beneficial;
    eMinecraftColour color;
};

#endif
