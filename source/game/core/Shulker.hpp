#ifndef GAME_CORE_SHULKER_HPP
#define GAME_CORE_SHULKER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Golem.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/core/BodyControl.hpp"
#include "game/entity/mob/Enemy.hpp"

class AABB;
class AttributeModifier;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Direction;
class DyeColor;
class Entity;
class EntityDataAccessor_Base;
class EntityTooltipDataHolder;
class Level;
class MobGroupData;
class SoundEvent;
template <class T> class EntityDataAccessor;

class Shulker : public Golem, public Enemy
{
public:
    class ShulkerAttackGoal : public Goal
    {
    public:
        ShulkerAttackGoal(Shulker* shulker);
        virtual ~ShulkerAttackGoal();
        virtual bool canUse();
        virtual void start();
        virtual void stop();
        virtual void tick();
        virtual std::wstring getName() { return L"ShulkerAttack"; }

    private:
        Shulker* mShulker;
        int mAttackTime;
    };

    class ShulkerBodyControl : public BodyControl
    {
    public:
        ShulkerBodyControl(Shulker* shulker);
        virtual ~ShulkerBodyControl();
        virtual void clientTick();

    private:
        Shulker* mShulker;
    };

    class ShulkerDefenseAttackGoal : public NearestAttackableTargetGoal
    {
    public:
        ShulkerDefenseAttackGoal(Shulker* shulker);
        virtual bool canUse();
        virtual AABB* getTargetSearchArea(double distance);
        virtual std::wstring getName() { return L"ShulkerDefenseAttack"; }
    };

    class ShulkerNearestAttackGoal : public NearestAttackableTargetGoal
    {
    public:
        ShulkerNearestAttackGoal(Shulker* shulker);
        virtual bool canUse();
        virtual AABB* getTargetSearchArea(double distance);
        virtual std::wstring getName() { return L"ShulkerNearestAttack"; }

    private:
        Shulker* mShulker;
    };

    class ShulkerPeekGoal : public Goal
    {
    public:
        ShulkerPeekGoal(Shulker* shulker);
        virtual ~ShulkerPeekGoal();
        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual void start();
        virtual void stop();
        virtual void tick();
        virtual std::wstring getName() { return L"ShulkerPeak"; }

    private:
        Shulker* mShulker;
        int mPeekTime;
    };

    Shulker(Level* level);
    static Entity* create(Level* level);
    static void staticCtor();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    virtual void registerGoals();
    virtual bool makeStepSound();
    virtual SoundSource::ESoundSource getSoundSource();
    virtual const SoundEvent* getAmbientSound();
    bool isClosed();
    virtual void playAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual void defineSynchedData();
    virtual void registerAttributes();
    virtual BodyControl* createBodyControl();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void tick();
    virtual void move(MoverType type, double xa, double ya, double za, bool onGround);
    virtual void setPos(double x, double y, double z);
    virtual double getRidingHeight();
    virtual bool teleportSomewhere();
    virtual void aiStep();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual AABB* getCollideBox();
    virtual const Direction* getAttachFace();
    virtual BlockPos* getAttachPosition();
    virtual void setAttachPosition(BlockPos* position);
    virtual unsigned char getRawPeekAmount();
    virtual void setRawPeekAmount(int amount);
    virtual float getClientPeekAmount(float partialTick);
    virtual int getClientSideTeleportInterpolation();
    virtual BlockPos* getOldAttachPosition();
    virtual float getEyeHeight();
    virtual int getMaxHeadXRot();
    virtual int getMaxHeadYRot();
    virtual void push(boost::shared_ptr<Entity> entity);
    virtual float getPickRadius();
    bool hasValidInterpolationPositions();
    virtual ResourceLocation* getDefaultLootTable();
    const DyeColor* getColor();
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual unsigned long long GetType();
    void setColor(const DyeColor* color);

private:
    static const EntityDataAccessor<const Direction*>* DATA_ATTACH_FACE;
    static const EntityDataAccessor<BlockPos*>* DATA_ATTACH_POSITION;
    static const EntityDataAccessor<unsigned char>* DATA_PEEK;
    static const EntityDataAccessor<unsigned char>* DATA_COLOR;
    static const DyeColor* DEFAULT_COLOR;
    static AttributeModifier* COVERED_ARMOR_MODIFIER;
    float mOldPeekAmount;
    float mPeekAmount;
    BlockPos* mOldAttachPosition;
    int mClientSideTeleportInterpolation;
};

#endif
