#ifndef GAME_CORE_LOOTTABLECONTEXT_HPP
#define GAME_CORE_LOOTTABLECONTEXT_HPP

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/server/ServerLevel.hpp"

class DamageSource;
class Entity;
class LootTable;
class LootTables;
class Player;

class LootTableContext
{
public:
    enum EntityTarget
    {
        THIS = 0,
        DIRECT_KILLER = 1,
        KILLER = 2
    };

    class Builder
    {
    public:
        Builder(ServerLevel* level) { mLevel = level; }

        Builder* withLuck(float luck)
        {
            mLuck = luck;
            return this;
        }

        Builder* withLootedEntity(boost::shared_ptr<Entity> entity)
        {
            mThisEntity = entity;
            return this;
        }

        Builder* withPlayer(boost::shared_ptr<Player> player)
        {
            mKillerEntity = player;
            return this;
        }

        Builder* withDamageSource(const DamageSource* damageSource)
        {
            mDamageSource = damageSource;
            return this;
        }

        LootTableContext build()
        {
            return LootTableContext(mLuck, mLevel, mLevel->getLootTables(), mThisEntity.lock(), mKillerEntity.lock(),
                                    mDamageSource);
        }

        ServerLevel* mLevel;
        float mLuck;
        boost::weak_ptr<Entity> mThisEntity;
        boost::weak_ptr<Player> mKillerEntity;
        const DamageSource* mDamageSource;
    };

    LootTableContext(float luck, ServerLevel* level, LootTables* lootTables, boost::shared_ptr<Entity> thisEntity,
                     boost::shared_ptr<Player> killerEntity, const DamageSource* damageSource);

    bool addVisitedTable(LootTable* lootTable);
    void removeVisitedTable(LootTable* lootTable);
    boost::shared_ptr<Entity> getEntity(EntityTarget target);

    float mLuck;
    ServerLevel* mLevel;
    LootTables* mLootTables;
    boost::weak_ptr<Entity> mThisEntity;
    boost::weak_ptr<Player> mKillerEntity;
    const DamageSource* mDamageSource;
    std::list<LootTable*> mVisitedTables;
};

#endif
