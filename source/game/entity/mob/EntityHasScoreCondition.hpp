#ifndef GAME_ENTITY_MOB_ENTITYHASSCORECONDITION_HPP
#define GAME_ENTITY_MOB_ENTITYHASSCORECONDITION_HPP

#include "game/core/LootItemCondition.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/util/RandomValueBounds.hpp"

#include <string>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class Entity;
class Random;
class Scoreboard;

class EntityHasScoreCondition : public LootItemCondition
{
public:
    EntityHasScoreCondition(boost::unordered_map<std::wstring, RandomValueBounds> scores,
                            LootTableContext::EntityTarget entityTarget);

    bool hasScore(const boost::shared_ptr<Entity>& entity, Scoreboard* scoreboard, const std::wstring& scoreName,
                  RandomValueBounds bounds);
    virtual bool applies(Random* random, LootTableContext& context);

private:
    boost::unordered_map<std::wstring, RandomValueBounds> scores;
    LootTableContext::EntityTarget entityTarget;
};

#endif
