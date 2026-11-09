#ifndef GAME_CORE_SPELLCASTERILLAGER_HPP
#define GAME_CORE_SPELLCASTERILLAGER_HPP

#include <string>

#include "game/core/AbstractIllager.hpp"
#include "game/entity/ai/Goal.hpp"

class CompoundTag;
class Level;
class SoundEvent;

class SpellcasterIllager : public AbstractIllager
{
public:
    class IllagerSpell
    {
    public:
        IllagerSpell(int id, double x, double y, double z);
        static IllagerSpell* byId(int id);

        int mId;
        double mX;
        double mY;
        double mZ;
    };

    class SpellcasterUseSpellGoal : public Goal
    {
    public:
        explicit SpellcasterUseSpellGoal(SpellcasterIllager* spellcaster);
        virtual ~SpellcasterUseSpellGoal() {}
        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual bool canInterrupt();
        virtual void start();
        virtual void stop();
        virtual void tick();
        virtual std::wstring getName() { return L"SpellcasterUseSpellGoal"; }
        virtual void performSpellCasting() = 0;
        virtual int getCastWarmupTime();
        virtual int getCastingTime() = 0;
        virtual int getCastingInterval() = 0;
        virtual const SoundEvent* getSpellPrepareSound() = 0;
        virtual IllagerSpell* getSpell() = 0;

    protected:
        SpellcasterIllager* mSpellcaster;
        int mSpellWarmup;
        int mNextAttackTickCount;
    };

    explicit SpellcasterIllager(Level* level);
    virtual ~SpellcasterIllager() {}

    static void staticCtor();
    virtual void defineSynchedData();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    bool isCastingSpell();
    virtual int getArmPose();
    virtual const SoundEvent* getCastingSoundEvent() = 0;
    void setIsCastingSpell(const IllagerSpell* spell);
    const IllagerSpell* getCurrentSpell();
    virtual void customServerAiStep();
    virtual void tick();
    int getSpellCastingTime() { return mSpellTicks; }

private:
    int mSpellTicks;
    const IllagerSpell* mCurrentSpell;
    static const EntityDataAccessor<unsigned char>* DATA_ID;
};

#endif
