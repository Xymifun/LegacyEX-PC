#include "game/entity/mob/MobEffectInstance.hpp"

#include <string>
#include <vector>

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/minigame/MiniGameDef.hpp"

MobEffectInstance::MobEffectInstance(MobEffect* effect)
{
    _init(effect, 0, 0, true);
}

MobEffectInstance::MobEffectInstance(MobEffect* effect, int duration)
{
    _init(effect, duration, 0, true);
}

MobEffectInstance::MobEffectInstance(MobEffect* effect, int duration, int amplifier)
{
    _init(effect, duration, amplifier, true);
}

MobEffectInstance::MobEffectInstance(MobEffect* effect, int duration, int amplifier, bool ambient, bool visible)
{
    _init(effect, duration, amplifier, visible);
    this->ambient = ambient;
}

MobEffectInstance::MobEffectInstance(MobEffectInstance* effect)
    : effect(effect->effect),
      duration(effect->duration),
      amplifier(effect->amplifier),
      reserved(effect->reserved),
      ambient(effect->ambient),
      noCounter(effect->noCounter),
      visible(effect->visible),
      durationEffectId(effect->durationEffectId)
{
}

int MobEffectInstance::getDuration() const
{
    if (durationEffectId != -1)
    {
        return Minecraft::GetInstance()->GetMiniGame()->getPotionDuration(durationEffectId, duration);
    }
    return duration;
}

bool MobEffectInstance::isNoCounter()
{
    return noCounter;
}

void MobEffectInstance::_init(MobEffect* effect, int duration, int amplifier, bool visible)
{
    this->effect = effect;
    this->duration = duration;
    this->amplifier = amplifier;
    this->reserved = 0;
    this->durationEffectId = -1;
    this->noCounter = false;
    this->visible = visible;
    this->ambient = false;
}

void MobEffectInstance::update(MobEffectInstance* effect)
{
    if (amplifier <= effect->amplifier)
    {
        if (amplifier < effect->amplifier)
        {
            amplifier = effect->amplifier;
            duration = effect->duration;
            visible = effect->visible;
            return;
        }
        if (duration < effect->duration)
        {
            duration = effect->duration;
            visible = effect->visible;
            return;
        }
    }
    if (!effect->ambient && ambient)
    {
        ambient = false;
    }
    visible = effect->visible;
}

MobEffect* MobEffectInstance::getEffect() const
{
    return effect;
}

int MobEffectInstance::getAmplifier() const
{
    return amplifier;
}

bool MobEffectInstance::isAmbient() const
{
    return ambient;
}

bool MobEffectInstance::isVisible() const
{
    return visible;
}

void MobEffectInstance::applyEffect(const boost::shared_ptr<LivingEntity>& entity)
{
    if (duration < 1)
    {
        return;
    }
    effect->applyEffectTick(entity, amplifier);
}

void MobEffectInstance::tickDownDuration()
{
    --duration;
}

bool MobEffectInstance::tick(const boost::shared_ptr<LivingEntity>& entity)
{
    getDuration();
    if (duration > 0)
    {
        if (effect->isDurationEffectTick(duration, amplifier))
        {
            applyEffect(entity);
        }
        tickDownDuration();
    }
    return duration > 0;
}

int MobEffectInstance::getDescriptionId()
{
    return effect->getDescriptionId(-1);
}

bool MobEffectInstance::compareTo(const MobEffectInstance* other) const
{
    bool equal = false;
    bool ambient;
    bool otherAmbient;
    if (getDuration() > 32147 && other->getDuration() > 32147)
    {
        ambient = isAmbient();
        otherAmbient = other->isAmbient();
    }
    else
    {
        ambient = isAmbient();
        otherAmbient = other->isAmbient();
        if (!ambient || !otherAmbient)
        {
            if (ambient == otherAmbient && getDuration() == other->getDuration()
                && getEffect()->getColor() == other->getEffect()->getColor())
            {
                equal = true;
            }
            return equal;
        }
    }
    if (ambient == otherAmbient && getEffect()->getColor() == other->getEffect()->getColor())
    {
        equal = true;
    }
    return equal;
}

CompoundTag* MobEffectInstance::save(CompoundTag* tag)
{
    tag->putByte(L"Id", (unsigned char)MobEffect::getId(effect));
    tag->putByte(L"Amplifier", (unsigned char)getAmplifier());
    tag->putInt(L"Duration", getDuration());
    tag->putBoolean(L"Ambient", isAmbient());
    tag->putBoolean(L"ShowParticles", isVisible());
    return tag;
}

MobEffectInstance* MobEffectInstance::load(CompoundTag* tag)
{
    unsigned char id = tag->getByte(L"Id");
    MobEffect* effect = MobEffect::byId(id);
    if (effect == NULL)
    {
        return NULL;
    }
    int amplifier = tag->getByte(L"Amplifier");
    int duration = tag->getInt(L"Duration");
    bool ambient = tag->getBoolean(L"Ambient");
    bool visible = true;
    if (tag->contains(L"ShowParticles", Tag::TAG_BYTE))
    {
        visible = tag->getBoolean(L"ShowParticles");
    }
    if (amplifier < 0)
    {
        amplifier = 0;
    }
    return new MobEffectInstance(effect, duration, amplifier, ambient, visible);
}

void MobEffectInstance::setNoCounter(bool noCounter)
{
    this->noCounter = noCounter;
}

int MobEffectInstance::getColorValue(const std::vector<MobEffectInstance*>* effects)
{
    if (effects->empty())
    {
        return 0x385dc6;
    }
    int color = 0;
    int count = 0;
    for (unsigned int i = 0; i < effects->size(); ++i)
    {
        MobEffectInstance* effect = effects->at(i);
        if (effect->isVisible() && effect->getEffect() != NULL)
        {
            for (int j = 0; j <= effect->getAmplifier(); ++j)
            {
                eMinecraftColour effectColor = effect->getEffect()->getColor();
                color += Minecraft::GetInstance()->getColourTable()->getColour(effectColor);
                ++count;
            }
        }
    }
    if (count > 0)
    {
        color /= count;
    }
    return color;
}
