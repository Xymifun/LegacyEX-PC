#ifndef GAME_ENTITY_MOB_MOBEFFECTINSTANCE_HPP
#define GAME_ENTITY_MOB_MOBEFFECTINSTANCE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/MobEffect.hpp"

class CompoundTag;
class LivingEntity;

class MobEffectInstance
{
    friend class Potion;
    friend class ThrownPotion;

public:
    MobEffectInstance(MobEffect* effect);
    MobEffectInstance(MobEffect* effect, int duration);
    MobEffectInstance(MobEffect* effect, int duration, int amplifier);
    MobEffectInstance(MobEffect* effect, int duration, int amplifier, bool ambient, bool visible);
    MobEffectInstance(MobEffectInstance* effect);

    int getDuration() const;
    bool isNoCounter();
    void _init(MobEffect* effect, int duration, int amplifier, bool visible);
    void update(MobEffectInstance* effect);
    MobEffect* getEffect() const;
    int getAmplifier() const;
    bool isAmbient() const;
    bool isVisible() const;
    void applyEffect(const boost::shared_ptr<LivingEntity>& entity);
    void tickDownDuration();
    bool tick(const boost::shared_ptr<LivingEntity>& entity);
    int getDescriptionId();
    bool compareTo(const MobEffectInstance* effect) const;
    CompoundTag* save(CompoundTag* tag);
    static MobEffectInstance* load(CompoundTag* tag);
    void setNoCounter(bool noCounter);
    static int getColorValue(const std::vector<MobEffectInstance*>* effects);

    void setSplash(bool splash);
    std::wstring toString();
    bool equals(const MobEffectInstance* effect) const;
    static bool effectsEqual(const MobEffectInstance* first, const MobEffectInstance* second);
    int hashCode();

private:
    MobEffect* effect;
    int duration;
    int amplifier;
    unsigned char reserved;
    bool ambient;
    bool noCounter;
    bool visible;
    int durationEffectId;
};

#endif
