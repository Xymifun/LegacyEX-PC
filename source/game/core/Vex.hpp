#ifndef GAME_CORE_VEX_HPP
#define GAME_CORE_VEX_HPP

#include <boost/shared_ptr.hpp>
#include "game/core/EntityType.hpp"
#include <string>

#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/MoverType.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/TargetGoal.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/world/block/EntityDataAccessor.hpp"

class BlockPos;
class CompoundTag;
class DamageSource;
class DifficultyInstance;
class Level;
class MobGroupData;
class Entity;
class Mob;
class DataFixerUpper;
class SoundEvent;
class ResourceLocation;

class Vex : public Monster
{
public:
    static const unsigned int WIIU_INSTANCE_SIZE = 1864;
    explicit Vex(Level* level);
    virtual ~Vex();
    static Entity* create(Level* level);
    static void staticCtor();
    virtual void move(MoverType operation, double x, double y, double z, bool force);
    virtual void tick();
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    static void addDataWalkers(DataFixerUpper* fixer);
    void setLimitedLife(int life);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    boost::shared_ptr<Mob> getOwner();
    BlockPos* getBoundOrigin();
    void setBoundOrigin(BlockPos* origin);
    bool getVexFlag(int flag);
    void setVexFlag(int flag, bool value);
    bool isCharging();
    void setIsCharging(bool charging);
    void setOwner(const boost::shared_ptr<Mob>& owner);
    const SoundEvent* getAmbientSound();
    const SoundEvent* getDeathSound();
    const SoundEvent* getHurtSound(const DamageSource* source);
    ResourceLocation* getDefaultLootTable();
    ResourceLocation* getDefaultGearTable();
    int getLightColor();
    float getBrightness();
    MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                   const std::wstring& reason);
    void populateDefaultEquipmentSlots(const DifficultyInstance& difficulty);
    unsigned long long GetType();

    class VexChargeAttackGoal : public Goal
    {
    public:
        explicit VexChargeAttackGoal(Vex* owner);
        virtual ~VexChargeAttackGoal();
        bool canUse();
        bool canContinueToUse();
        void start();
        void stop();
        void tick();
        std::wstring getName();

    private:
        Vex* owner;
    };
    class VexCopyOwnerTargetGoal : public TargetGoal
    {
    public:
        explicit VexCopyOwnerTargetGoal(PathfinderMob* owner);
        bool canUse();
        void start();
        std::wstring getName();
    };
    class VexMoveControl : public MoveControl
    {
    public:
        explicit VexMoveControl(Vex* owner);
        virtual ~VexMoveControl();
        virtual void tick();
    };
    class VexRandomMoveGoal : public Goal
    {
    public:
        explicit VexRandomMoveGoal(Vex* owner);
        virtual ~VexRandomMoveGoal();
        bool canUse();
        bool canContinueToUse();
        void tick();
        std::wstring getName();

    private:
        Vex* owner;
    };

private:
    boost::weak_ptr<Mob> owner;
    BlockPos* boundOrigin;
    bool hasLimitedLife;
    int limitedLife;
};

#endif
