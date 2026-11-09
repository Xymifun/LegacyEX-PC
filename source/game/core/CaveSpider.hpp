#ifndef GAME_CORE_CAVESPIDER_HPP
#define GAME_CORE_CAVESPIDER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Spider.hpp"

class DataFixerUpper;
class DifficultyInstance;
class Entity;
class Level;
class MobGroupData;

class CaveSpider : public Spider
{
public:
    CaveSpider(Level* level);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerAttributes();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    virtual float getEyeHeight();
    virtual ResourceLocation* getDefaultLootTable();
    virtual unsigned long long GetType();
};

#endif
