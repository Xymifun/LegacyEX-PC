#ifndef GAME_ENTITY_AI_NONTAMERANDOMTARGETGOAL_HPP
#define GAME_ENTITY_AI_NONTAMERANDOMTARGETGOAL_HPP

#include <string>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/entity/ai/NearestAttackableTargetGoal.hpp"

class Entity;
class TamableAnimal;
template <class T> class Predicate;

class NonTameRandomTargetGoal : public NearestAttackableTargetGoal
{
public:
    NonTameRandomTargetGoal(TamableAnimal* tamableAnimal, const std::type_info& targetType, bool mustSee,
                            const Predicate<boost::shared_ptr<Entity> >* selector, bool includeRemoved);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual std::wstring getName() { return L"NonTameRandomTarget"; }

private:
    TamableAnimal* mTamableAnimal;
};

#endif
