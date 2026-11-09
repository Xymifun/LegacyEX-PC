#ifndef GAME_CORE_ILLUSIONILLAGER_HPP
#define GAME_CORE_ILLUSIONILLAGER_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/core/SpellcasterIllager.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataFixerUpper;
class AABB;
class DamageSource;
class DifficultyInstance;
class Entity;
class Level;
class MobGroupData;
class SoundEvent;
class Arrow;
class TippableArrow;

class IllusionIllager : public SpellcasterIllager, public RangedAttackMob
{
public:
    class IllusionistBlindnessSpellGoal : public SpellcasterIllager::SpellcasterUseSpellGoal
    {
    public:
        IllusionistBlindnessSpellGoal(IllusionIllager* illager);
        virtual ~IllusionistBlindnessSpellGoal();
        virtual bool canUse();
        virtual void start();
        virtual std::wstring getName() { return L"IllusionistBlindnessSpellGoal"; }
        virtual void performSpellCasting();
        virtual int getCastingTime();
        virtual int getCastingInterval();
        virtual const SoundEvent* getSpellPrepareSound();
        virtual IllagerSpell* getSpell();

    private:
        int mTargetId;
    };
    class IllusionistMirrorSpellGoal : public SpellcasterIllager::SpellcasterUseSpellGoal
    {
    public:
        IllusionistMirrorSpellGoal(IllusionIllager* illager);
        virtual ~IllusionistMirrorSpellGoal();
        virtual bool canUse();
        virtual std::wstring getName() { return L"IllusionistMirrorSpellGoal"; }
        virtual void performSpellCasting();
        virtual int getCastingTime();
        virtual int getCastingInterval();
        virtual const SoundEvent* getSpellPrepareSound();
        virtual IllagerSpell* getSpell();
    };
    explicit IllusionIllager(Level* level);
    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    virtual void defineSynchedData();
    virtual ResourceLocation* getDefaultLootTable();
    virtual AABB* getBoundingBoxForCulling();
    virtual void aiStep();
    virtual bool isAlliedTo(boost::shared_ptr<Entity> entity);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getCastingSoundEvent();
    virtual int getArmPose();
    virtual unsigned long long GetType();
    arrayWithLength<Vec3*> getIllusionOffsets(float partialTick);
    boost::shared_ptr<Arrow> getArrow(float distanceFactor);

private:
    int mIllusionLevel;
    Vec3* mDisguisedPosition;
    Vec3* mDisguisedPosition2;
    Vec3* mDisguisedPosition3;
    Vec3* mDisguisedPosition4;
    Vec3* mDisguisedPositionO;
    Vec3* mDisguisedPosition2O;
    Vec3* mDisguisedPosition3O;
    Vec3* mDisguisedPosition4O;
};

#endif
