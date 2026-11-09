#ifndef GAME_UTIL_RABBIT_HPP
#define GAME_UTIL_RABBIT_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include <typeinfo>

#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/core/JumpControl.hpp"
#include "game/world/block/MoveToBlockGoal.hpp"
#include "game/client/audio/SoundSource.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DataFixerUpper;
class DamageSource;
class DifficultyInstance;
class Entity;
class Item;
class Level;
class MobGroupData;
class SoundEvent;
class ItemInstance;

class Rabbit : public Animal
{
public:
    class EvilRabbitAttackGoal : public MeleeAttackGoal
    {
    public:
        EvilRabbitAttackGoal(Rabbit* rabbit);
        virtual double getAttackReachSqr(boost::shared_ptr<LivingEntity> target);
    };

    class RabbitAvoidEntityGoal : public AvoidEntityGoal
    {
    public:
        RabbitAvoidEntityGoal(Rabbit* rabbit, const std::type_info& avoidType, float maxDistance,
                              double walkSpeedModifier, double sprintSpeedModifier);
        virtual bool canUse();

    private:
        Rabbit* mRabbit;
    };

    class RabbitGroupData : public MobGroupData
    {
    public:
        RabbitGroupData(int type);
        virtual ~RabbitGroupData();
        int mRabbitType;
    };

    class RabbitJumpControl : public JumpControl
    {
    public:
        RabbitJumpControl(Rabbit* rabbit);
        virtual ~RabbitJumpControl();
        void setCanJump(bool canJump);
        unsigned char wantJump();
        unsigned char canJump();
        virtual void tick();

    private:
        Rabbit* mRabbit;
        bool mCanJump;
    };

    class RabbitMoveControl : public MoveControl
    {
    public:
        RabbitMoveControl(Rabbit* rabbit);
        virtual ~RabbitMoveControl();
        virtual void tick();
        virtual void setWantedPosition(double x, double y, double z, double speedModifier);

    private:
        Rabbit* mRabbit;
        double mSpeedModifier;
    };

    class RabbitPanicGoal : public PanicGoal
    {
    public:
        RabbitPanicGoal(Rabbit* rabbit, double speedModifier);
        virtual ~RabbitPanicGoal();
        virtual void tick();

    private:
        Rabbit* mRabbit;
    };

    class RaidGardenGoal : public MoveToBlockGoal
    {
    public:
        RaidGardenGoal(Rabbit* rabbit);
        virtual ~RaidGardenGoal();
        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual void tick();
        virtual bool isValidTarget(Level* level, const BlockPos& pos);

    private:
        Rabbit* mRabbit;
        bool mWantsToRaid;
        bool mCanRaid;
    };

    Rabbit(Level* level);
    static Entity* create(Level* level);
    static void staticCtor();
    void setSpeedModifier(double speedModifier);
    virtual void registerGoals();
    virtual float getJumpPower();
    virtual void jumpFromGround();
    float getJumpCompletion(float partialTick);
    const SoundEvent* getJumpSound();
    virtual void setJumping(bool jumping);
    void startJumping();
    virtual bool checkSpawnRules();
    virtual void defineSynchedData();
    void setLandingDelay();
    void disableJumpControl();
    void checkLandingDelay();
    int getRabbitType();
    void facePoint(double x, double z);
    void enableJumpControl();
    virtual void customServerAiStep();
    virtual void updateSprintingState();
    virtual void aiStep();
    virtual void registerAttributes();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setRabbitType(int type);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual SoundSource::ESoundSource getSoundSource();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual ResourceLocation* getDefaultLootTable();
    bool isTemptingItem(Item* item);
    int getRandomRabbitType();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> parent);
    virtual bool isFood(const not_null_ptr<ItemInstance>& itemInstance);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& entityData);
    void eatCarrot();
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual unsigned long long GetType();
    bool wantsMoreFood();

private:
    int mJumpTicks;
    int mJumpDuration;
    bool mWasOnGround;
    int mJumpDelayTicks;
    int mMoreCarrotTicks;
    boost::shared_ptr<void> mField0x794;
    static const int SPAWN_BLOCK_COUNT = 4;

    Block* mSpawnBlocks[SPAWN_BLOCK_COUNT];
};

#endif
