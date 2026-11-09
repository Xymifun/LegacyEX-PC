#ifndef GAME_CORE_SQUID_HPP
#define GAME_CORE_SQUID_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/entity/mob/WaterAnimal.hpp"

class Block;
class BlockPos;
class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class SoundEvent;
class Vec3;

class Squid : public WaterAnimal
{
public:
    class SquidRandomMovementGoal : public Goal
    {
    public:
        SquidRandomMovementGoal(Squid* squid);
        virtual ~SquidRandomMovementGoal();
        virtual bool canUse();
        virtual void tick();
        virtual std::wstring getName() { return L"SquidRandomMovement"; }

    private:
        Squid* mSquid;
    };

    Squid(Level* level);
    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    void _init();
    void registerGoals();
    void registerAttributes();
    virtual float getEyeHeight();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual bool makeStepSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual ResourceLocation* getDefaultLootTable();
    virtual void aiStep();
    virtual void travel(float xa, float ya, float za);
    virtual bool checkSpawnRules();
    Vec3* randomInkDir();
    void spawnInkParticles();
    virtual void handleEntityEvent(unsigned char event, int data);
    void setMovementVector(float x, float y, float z);
    bool hasMovementVector();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual unsigned long long GetType();

    float mXBodyRot;
    float mXBodyRotO;
    float mZBodyRot;
    float mZBodyRotO;
    float mTentacleMovement;
    float mOldTentacleMovement;
    float mTentacleAngle;
    float mOldTentacleAngle;

private:
    float mSpeed;
    float mTentacleSpeed;
    float mRotateSpeed;
    float mMovementX;
    float mMovementY;
    float mMovementZ;
};

#endif
