#ifndef GAME_CORE_VINDICATION_ILLAGER_HPP
#define GAME_CORE_VINDICATION_ILLAGER_HPP

#include "game/core/AbstractIllager.hpp"
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/world/block/Predicate.hpp"

class CompoundTag;
class DataFixerUpper;
class DamageSource;
class DifficultyInstance;
class Level;
class MobGroupData;
class Entity;
class ResourceLocation;
class LivingEntity;
class SoundEvent;

class VindicationIllager : public AbstractIllager
{
public:
    explicit VindicationIllager(Level* level);
    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual EMobType getMobType();
    ResourceLocation* getDefaultLootTable();
    ResourceLocation* getDefaultGearTable();
    bool isAgressive();
    void setAggressive(bool aggressive);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual int getArmPose();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                   const std::wstring& reason);
    virtual void customServerAiStep();
    virtual bool isAlliedTo(boost::shared_ptr<Entity> entity);
    void setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible);
    const SoundEvent* getAmbientSound();
    const SoundEvent* getDeathSound();
    const SoundEvent* getHurtSound(const DamageSource* source);
    unsigned long long GetType();

    class LIVING_ENTITY_SELECTOR : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        LIVING_ENTITY_SELECTOR() {}
        virtual ~LIVING_ENTITY_SELECTOR() {}
        bool apply(boost::shared_ptr<Entity> entity) const;
    };
    class VindicatorJohnnyAttackGoal : public NearestAttackableTargetGoal
    {
    public:
        explicit VindicatorJohnnyAttackGoal(VindicationIllager* mob);
        bool canUse();
    };

    bool mJohnny;
};

#endif
