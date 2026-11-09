#ifndef GAME_ENTITY_AI_BREEDGOAL_HPP
#define GAME_ENTITY_AI_BREEDGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>
#include <typeinfo>
#include "game/entity/ai/Goal.hpp"

class Animal;
class Entity;
class Level;

class BreedGoal : public Goal
{
public:
    BreedGoal(Animal* animal, double speedModifier);
    BreedGoal(Animal* animal, double speedModifier, const std::type_info& partnerType);

    boost::shared_ptr<Animal> getFreePartner();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void tick();
    virtual void breed();
    virtual void setLevel(Level* level);
    virtual std::wstring getName() { return L"Breed"; }

protected:
    Animal* mAnimal;
    boost::weak_ptr<Animal> mPartner;
    Level* mLevel;
    const std::type_info* mPartnerType;
    int mTicks;
    double mSpeedModifier;
};

#endif
