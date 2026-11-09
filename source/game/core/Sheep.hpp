#ifndef GAME_CORE_SHEEP_HPP
#define GAME_CORE_SHEEP_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/entity/mob/Animal.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Block;
class BlockPos;
class CompoundTag;
class CraftingContainer;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class DyeColor;
class EntityTooltipDataHolder;
class Goal;
class Level;
class MobGroupData;
class Random;
class AgableMob;
class SoundEvent;

class Sheep : public Animal
{
public:
    class SheepContainer : public AbstractContainerMenu
    {
    public:
        virtual bool stillValid(boost::shared_ptr<Player> player);
    };

    Sheep(Level* level);
    virtual ~Sheep();
    static Entity* create(Level* level);
    static arrayWithLength<const float> getColorArray(const DyeColor* color);
    static arrayWithLength<const float> createSheepColor(const DyeColor* color);
    static void staticCtor();
    virtual void registerGoals();
    virtual void customServerAiStep();
    virtual void aiStep();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual ResourceLocation* getDefaultLootTable();
    virtual void handleEntityEvent(unsigned char event, int data);
    float getHeadEatPositionScale(float partialTick);
    float getHeadEatAngleScale(float partialTick);
    bool isSheared();
    void setSheared(bool sheared);
    const DyeColor* getColor();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setColor(const DyeColor* color);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    static const DyeColor* getRandomSheepColor(Random* random);
    const DyeColor* getOffspringColor(boost::shared_ptr<Animal> first, boost::shared_ptr<Animal> second);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual void ate();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* group,
                                           const std::wstring& name);
    virtual float getEyeHeight();
    virtual unsigned long long GetType();

private:
    AbstractContainerMenu* mContainer;
    boost::shared_ptr<CraftingContainer> mCraftingContainer;
    int mEatAnimationTick;
    Goal* mEatBlockGoal;
};

#endif
