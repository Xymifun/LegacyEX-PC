#ifndef GAME_ENTITY_MOB_TAMABLEANIMAL_HPP
#define GAME_ENTITY_MOB_TAMABLEANIMAL_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/OwnableEntity.hpp"
#include "game/core/InteractionHand.hpp"

class CompoundTag;
class DamageSource;
class Entity;
class Level;
class Player;
class SitGoal;
class Team;
class Vec3;
template <class T> class EntityDataAccessor;

class TamableAnimal : public Animal, public OwnableEntity
{
public:
    TamableAnimal(Level* level);
    virtual ~TamableAnimal();
    static void staticCtor();
    virtual void fjDerivedCtorCalls();
    virtual void defineSynchedData();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual void spawnTamingParticles(bool success);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual bool isTame();
    virtual void setTame(bool tame);
    virtual void reassessTameGoals();
    virtual bool removeWhenFarAway();
    virtual bool isSitting();
    virtual void setSitting(bool sitting);
    virtual std::wstring getOwnerUUID();
    virtual void setOwnerUUID(const std::wstring& uuid);
    virtual void tame(boost::shared_ptr<Player> player);
    virtual boost::shared_ptr<Entity> getOwner();
    virtual bool isOwnedBy(boost::shared_ptr<LivingEntity> entity);
    virtual SitGoal* getSitGoal();
    virtual bool wantsToAttack(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<LivingEntity> owner);
    virtual Team* getTeam();
    virtual bool isAlliedTo(boost::shared_ptr<Entity> entity);
    virtual void die(const DamageSource* source);
    virtual InteractionResult::EInteractionResult interactAt(const boost::shared_ptr<Player>& player, Vec3* pos,
                                                             InteractionHand::EInteractionHand hand);

protected:
    SitGoal* mSitGoal;
    static const EntityDataAccessor<unsigned char>* DATA_FLAGS;
    static const EntityDataAccessor<std::wstring>* DATA_OWNER_UUID;
};

#endif
