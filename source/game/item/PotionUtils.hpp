#ifndef GAME_ITEM_POTIONUTILS_HPP
#define GAME_ITEM_POTIONUTILS_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "game/core/CriticalSection.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"

class CompoundTag;
class HtmlString;
class ItemInstance;
class MobEffect;
class MobEffectInstance;
class Player;
class Potion;

class PotionUtils
{
public:
    static const std::wstring TAG_POTION;
    static const std::wstring TAG_CUSTOM_POTION_EFFECTS;
    static const std::wstring TAG_CUSTOM_POTION_COLOR;
    static not_null_ptr<ItemInstance> setPotion(not_null_ptr<ItemInstance> item, Potion* potion);
    static Potion* getPotion(CompoundTag* tag);
    static void getCustomEffects(CompoundTag* tag, std::vector<MobEffectInstance*>* effects);
    static std::vector<MobEffectInstance*>* getAllEffects(CompoundTag* tag);
    static std::vector<MobEffectInstance*>* getMobEffects(not_null_ptr<ItemInstance> item);
    static bool hasEffect(not_null_ptr<ItemInstance> item, MobEffect* effect);
    static Potion* getPotion(not_null_ptr<ItemInstance> item);
    static void addPotionTooltip(not_null_ptr<ItemInstance> item, std::vector<HtmlString>* lines, float durationFactor,
                                 boost::shared_ptr<Player> player);
    static bool isFoil(not_null_ptr<ItemInstance> item);
    static std::vector<MobEffectInstance*>* getAllEffects(Potion* potion, std::vector<MobEffectInstance*>* effects);
    static std::vector<MobEffectInstance*>* getCustomEffects(CompoundTag* tag);
    static std::vector<MobEffectInstance*>* getCustomEffects(not_null_ptr<ItemInstance> item);
    static int getColor(std::vector<MobEffectInstance*>* effects);
    static int getColor(const not_null_ptr<ItemInstance>& item);
    static int getColor(Potion* potion);
    static std::wstring getPotionID(not_null_ptr<ItemInstance> item);
    static not_null_ptr<ItemInstance> setCustomEffects(not_null_ptr<ItemInstance> item,
                                                       std::vector<MobEffectInstance*>* effects);
    static not_null_ptr<ItemInstance> setPotionAndCustomEffects(not_null_ptr<ItemInstance> target,
                                                                not_null_ptr<ItemInstance> source);
    static void setPotionName(not_null_ptr<ItemInstance> item, const std::wstring& name);

private:
    struct PotionCacheEntry
    {
        PotionCacheEntry() : isFoil(false) {}

        bool isFoil;
    };

    static boost::unordered_map<std::wstring, PotionCacheEntry> potionCache;
    static OSFastMutex potionCacheLock;
    static bool potionCacheLockInitialised;
};

#endif
