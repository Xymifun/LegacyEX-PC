#include "game/entity/ai/TurtleBreedGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/ExperienceOrb.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

TurtleBreedGoal::TurtleBreedGoal(Turtle* turtle, double speedModifier)
    : BreedGoal(turtle, speedModifier),
      mTurtle(turtle)
{
}

bool TurtleBreedGoal::canUse()
{
    return BreedGoal::canUse() && !mTurtle->hasEgg();
}

void TurtleBreedGoal::breed()
{
    boost::shared_ptr<ServerPlayer> player = mAnimal->getLoveCause();
    if (!player && mPartner.lock()->getLoveCause())
    {
        player = mPartner.lock()->getLoveCause();
    }

    if (player)
    {
        player->awardStat(GenericStats::breedEntity(mTurtle->GetType()),
                          GenericStats::param_breedEntity(mTurtle->GetType()));
    }

    mTurtle->setHasEgg(true);
    mAnimal->resetLove();
    mPartner.lock()->resetLove();

    Random* random = mAnimal->getRandom();
    if (mLevel->getGameRules()->getBoolean(GameRules::RULE_DOMOBLOOT))
    {
        mLevel->addEntity(boost::shared_ptr<ExperienceOrb>(
            new ExperienceOrb(mLevel, mAnimal->mX, mAnimal->mY, mAnimal->mZ, random->nextInt(7) + 1)));
    }
}
