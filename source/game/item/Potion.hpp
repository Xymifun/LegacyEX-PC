#ifndef GAME_ITEM_POTION_HPP
#define GAME_ITEM_POTION_HPP

#include <string>
#include <vector>

template <class K, class V> class DefaultedMappedRegistry;
class Item;
class MobEffectInstance;
class ResourceLocation;
template <class K, class V, class H, class E> class SimpleRegistry;

class Potion
{
public:
    enum EType
    {
        NORMAL = 0,
        SPLASH = 1,
        LINGERING = 2,
        ARROW = 3
    };

    static Potion* byId(int id);
    static int getId(Potion* potion);
    static Potion* byName(const std::wstring& name);
    Potion(int nameId, int useDescriptionId, int splashNameId, int lingeringNameId, int arrowNameId,
           int arrowUseDescriptionId);
    Potion(MobEffectInstance* effect, int nameId, int useDescriptionId, int splashNameId, int lingeringNameId,
           int arrowNameId, int arrowUseDescriptionId);
    Potion(const std::vector<MobEffectInstance*>& effects, int nameId, int useDescriptionId, int splashNameId,
           int lingeringNameId, int arrowNameId, int arrowUseDescriptionId);
    std::wstring getName(EType type);
    std::vector<MobEffectInstance*>& getEffects();
    int getUseDescriptionId(EType type);
    static void addPotion(const std::wstring& name, Potion* potion);
    static void StaticCtor();
    static Item* getBasePotion(EType type);
    bool hasInstantEffects();

    static DefaultedMappedRegistry<ResourceLocation, Potion*>* potionRegistry;

private:
    static MobEffectInstance* effectWithId(MobEffectInstance* effect, int id);
    int mNameId;
    int mSplashNameId;
    int mLingeringNameId;
    int mArrowNameId;
    int mUseDescriptionId;
    int mArrowUseDescriptionId;
    std::vector<MobEffectInstance*> mEffects;
};

#endif
