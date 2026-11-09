#ifndef GAME_CORE_EVOCATIONILLAGER_HPP
#define GAME_CORE_EVOCATIONILLAGER_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "game/core/SpellcasterIllager.hpp"
#include "game/world/block/Predicate.hpp"

class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Entity;
class Level;
class Sheep;
class SoundEvent;

class EvocationIllager : public SpellcasterIllager
{
public:
    class EvokerAttackSpellGoal : public SpellcasterIllager::SpellcasterUseSpellGoal
    {
    public:
        explicit EvokerAttackSpellGoal(EvocationIllager* evoker);
        virtual ~EvokerAttackSpellGoal();
        virtual std::wstring getName();
        virtual int getCastingTime();
        virtual int getCastingInterval();
        void createSpellEntity(double x, double z, double minY, double maxY, float yRot, int warmupDelay);
        virtual void performSpellCasting();
        virtual const SoundEvent* getSpellPrepareSound();
        virtual IllagerSpell* getSpell();
    };
    class EvokerSummonSpellGoal : public SpellcasterIllager::SpellcasterUseSpellGoal
    {
    public:
        explicit EvokerSummonSpellGoal(EvocationIllager* evoker);
        virtual ~EvokerSummonSpellGoal();
        virtual bool canUse();
        virtual std::wstring getName();
        virtual int getCastingTime();
        virtual int getCastingInterval();
        virtual void performSpellCasting();
        virtual const SoundEvent* getSpellPrepareSound();
        virtual IllagerSpell* getSpell();
    };
    class EvokerWololoSpellGoal : public SpellcasterIllager::SpellcasterUseSpellGoal
    {
    public:
        class SheepSelector : public Predicate<boost::shared_ptr<Entity> >
        {
        public:
            virtual bool apply(boost::shared_ptr<Entity> entity) const;
            virtual ~SheepSelector() {}
        };
        explicit EvokerWololoSpellGoal(EvocationIllager* evoker);
        virtual ~EvokerWololoSpellGoal();
        virtual void tick();
        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual void stop();
        virtual std::wstring getName();
        virtual void performSpellCasting();
        virtual int getCastWarmupTime();
        virtual int getCastingTime();
        virtual int getCastingInterval();
        virtual const SoundEvent* getSpellPrepareSound();
        virtual IllagerSpell* getSpell();
    };
    explicit EvocationIllager(Level* level);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual ResourceLocation* getDefaultLootTable();
    virtual void customServerAiStep();
    virtual void tick();
    virtual bool isAlliedTo(boost::shared_ptr<Entity> entity);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    void setWololoTarget(const boost::shared_ptr<Sheep>& target);
    boost::shared_ptr<Sheep> getWololoTarget();
    virtual const SoundEvent* getCastingSoundEvent();
    static Entity* create(Level* level);
    virtual unsigned long long GetType();

private:
    boost::weak_ptr<Sheep> mWololoTarget;
};

#endif
