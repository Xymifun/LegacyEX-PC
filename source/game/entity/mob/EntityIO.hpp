#ifndef GAME_ENTITY_MOB_ENTITYIO_HPP
#define GAME_ENTITY_MOB_ENTITYIO_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/core/ColourTable.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/MappedRegistry.hpp"

class CompoundTag;
class Level;

enum eMinecraftIconOverride
{
    eMinecraftIconOverride_NONE
};

class EntityIO
{
public:
    class SpawnableMobInfo
    {
    public:
        SpawnableMobInfo(int id, const ResourceLocation& location, eMinecraftColour colour,
                         eMinecraftColour secondaryColour, eMinecraftIconOverride icon);
        SpawnableMobInfo(const SpawnableMobInfo& other);

        int id;
        ResourceLocation location;
        eMinecraftColour colour;
        eMinecraftColour secondaryColour;
        eMinecraftIconOverride icon;
        int type;
        int flags;
    };

    typedef Entity* (*Creator)(Level*);
    typedef boost::unordered_map<ResourceLocation, SpawnableMobInfo*> SpawnInfo;
    typedef MappedRegistry<ResourceLocation, unsigned long long> EntityRegistry;
    static EntityRegistry sRegistry;
    static SpawnInfo sSpawnInfo;
    static ResourceLocation getKey(unsigned long long entityType);
    static boost::shared_ptr<Entity> newEntity(unsigned long long entityType, Level* level);
    static ResourceLocation getKey(const boost::shared_ptr<Entity>& entity);
    static std::wstring getTranslationName(boost::shared_ptr<Entity> entity);
    static unsigned long long byId(int id);
    static unsigned long long byString(const std::wstring& name);
    static boost::shared_ptr<Entity> newEntity(int id, Level* level);
    static boost::shared_ptr<Entity> newEntity(const ResourceLocation& location, Level* level);
    static boost::unordered_set<ResourceLocation>* getAllEntityTypes();
    static bool isValidEntityType(const ResourceLocation& location);
    static boost::shared_ptr<Entity> newEntity(CompoundTag* tag, Level* level);
    static bool isEntityOfType(boost::shared_ptr<Entity> entity, const ResourceLocation& location);
    static int getNameDisplayId(const std::wstring& name);
    static SpawnableMobInfo addSpawnInfo(int id, std::wstring name, eMinecraftColour colour,
                                         eMinecraftColour secondaryColour, eMinecraftIconOverride icon);
    static void Register(Creator creator, int id, const std::wstring& name, unsigned long long entityType,
                         const std::wstring& translationName);
    static void bootStrap();
    static std::wstring getTranslationName(const ResourceLocation& location);
    static int eTypeToIoid(unsigned long long entityType);
};

#endif
