#include "game/item/PotionUtils.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/AttributeModifier.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/Potions.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"

const std::wstring PotionUtils::TAG_CUSTOM_POTION_EFFECTS(L"CustomPotionEffects");
const std::wstring PotionUtils::TAG_CUSTOM_POTION_COLOR(L"CustomPotionColor");
const std::wstring PotionUtils::TAG_POTION(L"Potion");
boost::unordered_map<std::wstring, PotionUtils::PotionCacheEntry> PotionUtils::potionCache;
OSFastMutex PotionUtils::potionCacheLock;
bool PotionUtils::potionCacheLockInitialised = false;

not_null_ptr<ItemInstance> PotionUtils::setPotion(not_null_ptr<ItemInstance> item, Potion* potion)
{
    ResourceLocation key = Potion::potionRegistry->getKey(potion);
    if (potion == Potions::EMPTY)
    {
        if (item->hasTag())
        {
            CompoundTag* tag = item->getTag();
            tag->remove(TAG_POTION);
            if (tag->isEmpty())
                item->setTag(NULL);
        }
    }
    else
    {
        CompoundTag* tag = item->hasTag() ? item->getTag() : new CompoundTag();
        tag->putString(TAG_POTION, key.toString());
        item->setTag(tag);
    }
    return item;
}

Potion* PotionUtils::getPotion(CompoundTag* tag)
{
    if (tag == NULL)
        return Potions::WATER;
    return Potion::byName(tag->getString(TAG_POTION));
}

void PotionUtils::getCustomEffects(CompoundTag* tag, std::vector<MobEffectInstance*>* effects)
{
    if (tag == NULL || !tag->contains(TAG_CUSTOM_POTION_EFFECTS, Tag::TAG_LIST))
        return;
    ListTag* list = tag->getList(TAG_CUSTOM_POTION_EFFECTS, Tag::TAG_COMPOUND);
    for (int i = 0; i < list->size(); ++i)
    {
        MobEffectInstance* effect = MobEffectInstance::load(list->getCompound(i));
        if (effect != NULL)
            effects->push_back(effect);
    }
}

std::vector<MobEffectInstance*>* PotionUtils::getAllEffects(CompoundTag* tag)
{
    std::vector<MobEffectInstance*>* effects = new std::vector<MobEffectInstance*>();
    std::vector<MobEffectInstance*>& potionEffects = getPotion(tag)->getEffects();
    effects->insert(effects->end(), potionEffects.begin(), potionEffects.end());
    getCustomEffects(tag, effects);
    return effects;
}

std::vector<MobEffectInstance*>* PotionUtils::getMobEffects(not_null_ptr<ItemInstance> item)
{
    return getAllEffects(item->getTag());
}

bool PotionUtils::hasEffect(not_null_ptr<ItemInstance> item, MobEffect* effect)
{
    std::vector<MobEffectInstance*>* effects = getMobEffects(item);
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
    {
        if ((*it)->getEffect() == effect)
        {
            delete effects;
            return true;
        }
    }
    delete effects;
    return false;
}

Potion* PotionUtils::getPotion(not_null_ptr<ItemInstance> item)
{
    return getPotion(item->getTag());
}

void PotionUtils::addPotionTooltip(not_null_ptr<ItemInstance> item, std::vector<HtmlString>* lines,
                                   float durationFactor, boost::shared_ptr<Player> player)
{
    std::vector<MobEffectInstance*>* effects = getMobEffects(item);
    if (effects != NULL && !effects->empty())
    {
        boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*> modifiers;
        if (effects->empty())
        {
            std::wstring none(L10N::GetString(IDS_POTION_EFFECT_EMPTY));
            lines->push_back(HtmlString(none, eMinecraftColour_HTMLColor_7, false, false, 0));
        }
        else
        {
            for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
            {
                MobEffectInstance* effect = *it;
                std::wstring text(L10N::GetString(effect->getDescriptionId()));
                MobEffect* mobEffect = effect->getEffect();
                boost::unordered_map<Attribute*, AttributeModifier*>* effectModifiers =
                    mobEffect->getAttributeModifiers();
                if (effectModifiers != NULL && !effectModifiers->empty())
                {
                    for (boost::unordered_map<Attribute*, AttributeModifier*>::iterator mod = effectModifiers->begin();
                         mod != effectModifiers->end(); ++mod)
                    {
                        AttributeModifier* base = mod->second;
                        AttributeModifier* scaled = new AttributeModifier(
                            mobEffect->getAttributeModifierValue(effect->getAmplifier(), base), base->getOperation());
                        modifiers.insert(boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>::value_type(
                            mod->first->getId(), scaled));
                    }
                }
                if (effect->getAmplifier() > 0)
                {
                    std::wstring amplifierText(L"");
                    switch (effect->getAmplifier())
                    {
                    case 1:
                        amplifierText = L" ";
                        amplifierText += L10N::GetString(IDS_POTION_POTENCY_1);
                        text += amplifierText;
                        break;
                    case 2:
                        amplifierText = L" ";
                        amplifierText += L10N::GetString(IDS_POTION_POTENCY_2);
                        text += amplifierText;
                        break;
                    case 3:
                        amplifierText = L" ";
                        amplifierText += L10N::GetString(IDS_POTION_POTENCY_3);
                        text += amplifierText;
                        break;
                    case 4:
                        amplifierText = L" ";
                        amplifierText += L10N::GetString(IDS_POTION_POTENCY_4);
                        text += amplifierText;
                        break;
                    case 5:
                        amplifierText = L" ";
                        amplifierText += L10N::GetString(IDS_POTION_POTENCY_5);
                        text += amplifierText;
                        break;
                    default:
                        amplifierText = L10N::GetString(IDS_POTION_POTENCY_0);
                        text += amplifierText;
                        break;
                    }
                }
                if (effect->getDuration() > 20)
                {
                    text += L" (" + MobEffect::formatDuration(effect, durationFactor) + L")";
                }
                eMinecraftColour colour = eMinecraftColour_HTMLColor_7;
                if (mobEffect->isHarmful())
                    colour = eMinecraftColour_HTMLColor_c;
                lines->push_back(HtmlString(text, colour, false, false, 0));
            }
        }
        delete effects;
        if (!modifiers.empty())
        {
            lines->push_back(HtmlString(L"", eMinecraftColour_NOTSET, false, false, 0));
            lines->push_back(HtmlString(L10N::GetString(IDS_POTION_EFFECTS_WHENDRANK), eMinecraftColour_HTMLColor_5,
                                        false, false, 0));
            for (boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>::iterator it = modifiers.begin();
                 it != modifiers.end(); ++it)
            {
                lines->push_back(it->second->getHoverText(it->first, item, player));
                delete it->second;
            }
        }
    }
}

bool PotionUtils::isFoil(not_null_ptr<ItemInstance> item)
{
    if (!potionCacheLockInitialised)
    {
        InitializeCriticalSection(&potionCacheLock);
        potionCacheLockInitialised = true;
    }

    bool foil = false;
    if (item->getTag() != NULL && item->getTag()->contains(TAG_POTION))
    {
        std::wstring potionName = item->getTag()->getString(TAG_POTION);
        EnterCriticalSection(&potionCacheLock);
        boost::unordered_map<std::wstring, PotionCacheEntry>::iterator cached = potionCache.find(potionName);
        if (cached != potionCache.end())
        {
            foil = cached->second.isFoil;
        }
        else
        {
            std::vector<MobEffectInstance*>* effects = getMobEffects(item);
            foil = effects != NULL && !effects->empty();
            delete effects;
            potionCache[potionName].isFoil = foil;
        }
        LeaveCriticalSection(&potionCacheLock);
    }
    return foil;
}

std::vector<MobEffectInstance*>* PotionUtils::getAllEffects(Potion* potion, std::vector<MobEffectInstance*>* effects)
{
    std::vector<MobEffectInstance*>* result = new std::vector<MobEffectInstance*>();
    std::vector<MobEffectInstance*>& potionEffects = potion->getEffects();
    result->insert(result->begin(), potionEffects.begin(), potionEffects.end());
    result->insert(result->begin(), effects->begin(), effects->end());
    return result;
}

std::vector<MobEffectInstance*>* PotionUtils::getCustomEffects(CompoundTag* tag)
{
    std::vector<MobEffectInstance*>* effects = new std::vector<MobEffectInstance*>();
    getCustomEffects(tag, effects);
    return effects;
}

std::vector<MobEffectInstance*>* PotionUtils::getCustomEffects(not_null_ptr<ItemInstance> item)
{
    return getCustomEffects(item->getTag());
}

int PotionUtils::getColor(std::vector<MobEffectInstance*>* effects)
{
    if (effects->empty())
        return 0x385dc6;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    int count = 0;
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
    {
        MobEffectInstance* effect = *it;
        if (effect->isVisible())
        {
            int color = gApp.GetHTMLColor(effect->getEffect()->getColor());
            int amplifier = effect->getAmplifier() + 1;
            red += static_cast<float>(amplifier * ((color >> 16) & 0xff)) / 255.0f;
            green += static_cast<float>(amplifier * ((color >> 8) & 0xff)) / 255.0f;
            blue += static_cast<float>(amplifier * (color & 0xff)) / 255.0f;
            count += amplifier;
        }
    }
    if (count == 0)
        return 0;
    red = red / count * 255.0f;
    green = green / count * 255.0f;
    blue = blue / count * 255.0f;
    return static_cast<int>(red) << 16 | static_cast<int>(green) << 8 | static_cast<int>(blue);
}

int PotionUtils::getColor(const not_null_ptr<ItemInstance>& item)
{
    CompoundTag* tag = item->getTag();
    if (tag != NULL && tag->contains(TAG_CUSTOM_POTION_COLOR, Tag::TAG_ANY_NUMERIC))
        return tag->getInt(TAG_CUSTOM_POTION_COLOR);
    return getPotion(item) == Potions::EMPTY ? 0xf800f8 : getColor(getMobEffects(item));
}

int PotionUtils::getColor(Potion* potion)
{
    if (potion == Potions::EMPTY)
        return 0xf800f8;
    return getColor(&potion->getEffects());
}

std::wstring PotionUtils::getPotionID(not_null_ptr<ItemInstance> item)
{
    CompoundTag* tag = item->getTag();
    if (tag == NULL)
        return L"";
    std::wstring name = tag->getString(TAG_POTION);
    std::wstring::size_type separator = name.find(L":");
    return std::wstring(name, separator + 1, std::wstring::npos);
}

not_null_ptr<ItemInstance> PotionUtils::setCustomEffects(not_null_ptr<ItemInstance> item,
                                                         std::vector<MobEffectInstance*>* effects)
{
    if (effects->empty())
        return item;
    CompoundTag* tag = item->getTag() == NULL ? new CompoundTag() : item->getTag()->copy();
    ListTag* list = tag->getList(TAG_CUSTOM_POTION_EFFECTS, Tag::TAG_LIST);
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
        list->add((*it)->save(new CompoundTag()));
    tag->put(TAG_CUSTOM_POTION_EFFECTS, list);
    item->setTag(tag);
    return item;
}

not_null_ptr<ItemInstance> PotionUtils::setPotionAndCustomEffects(not_null_ptr<ItemInstance> target,
                                                                  not_null_ptr<ItemInstance> source)
{
    Potion* potion = getPotion(source->getTag());
    target = setPotion(target, potion);
    target = setCustomEffects(target, getCustomEffects(source->getTag()));
    return target;
}

void PotionUtils::setPotionName(not_null_ptr<ItemInstance> item, const std::wstring& name)
{
    CompoundTag* tag = item->getTag() == NULL ? new CompoundTag() : item->getTag()->copy();
    tag->putString(TAG_POTION, name);
    item->setTag(tag);
}
