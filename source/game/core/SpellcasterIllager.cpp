#include "game/core/SpellcasterIllager.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/Level.hpp"

const EntityDataAccessor<unsigned char>* SpellcasterIllager::DATA_ID = NULL;

SpellcasterIllager::IllagerSpell* gSpell0 = new SpellcasterIllager::IllagerSpell(0, 0.0, 0.0, 0.0);
SpellcasterIllager::IllagerSpell* gSpell1 = new SpellcasterIllager::IllagerSpell(1, 0.7, 0.7, 0.8);
SpellcasterIllager::IllagerSpell* gSpell2 = new SpellcasterIllager::IllagerSpell(2, 0.4, 0.3, 0.35);
SpellcasterIllager::IllagerSpell* gSpell3 = new SpellcasterIllager::IllagerSpell(3, 0.7, 0.5, 0.2);
SpellcasterIllager::IllagerSpell* gSpell4 = new SpellcasterIllager::IllagerSpell(4, 0.3, 0.3, 0.8);
SpellcasterIllager::IllagerSpell* gSpell5 = new SpellcasterIllager::IllagerSpell(5, 0.1, 0.1, 0.2);

SpellcasterIllager::SpellcasterIllager(Level* level) : AbstractIllager(level), mSpellTicks(0), mCurrentSpell(gSpell0)
{
}

void SpellcasterIllager::staticCtor()
{
    DATA_ID = SynchedEntityData::defineId<unsigned char>(EntityType::SPELLCASTER_ILLAGER, EntityType::ABSTRACT_ILLAGER,
                                                         EntityDataSerializers::BYTE);
}

void SpellcasterIllager::defineSynchedData()
{
    AbstractIllager::defineSynchedData();
    mEntityData->createDateItem(DATA_ID, (unsigned char)0);
}

void SpellcasterIllager::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    mSpellTicks = tag->getInt(L"SpellTicks");
}

void SpellcasterIllager::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"SpellTicks", mSpellTicks);
}

bool SpellcasterIllager::isCastingSpell()
{
    if (mLevel->mIsClientSide)
    {
        return mEntityData->get(DATA_ID) != 0;
    }
    return mSpellTicks > 0;
}

int SpellcasterIllager::getArmPose()
{
    return isCastingSpell() ? ILLAGER_ARM_POSE_SPELLCASTING : ILLAGER_ARM_POSE_CROSSED;
}

void SpellcasterIllager::setIsCastingSpell(const IllagerSpell* spell)
{
    mCurrentSpell = spell;
    mEntityData->set(DATA_ID, (unsigned char)spell->mId, true);
}

const SpellcasterIllager::IllagerSpell* SpellcasterIllager::getCurrentSpell()
{
    if (!mLevel->mIsClientSide)
    {
        return mCurrentSpell;
    }
    int id = mEntityData->get(DATA_ID);
    return IllagerSpell::byId(id);
}

void SpellcasterIllager::customServerAiStep()
{
    Mob::customServerAiStep();
}

void SpellcasterIllager::tick()
{
    Monster::tick();
    if (mLevel->mIsClientSide && isCastingSpell())
    {
        const IllagerSpell* spell = getCurrentSpell();
        double red = spell->mX;
        double green = spell->mY;
        double blue = spell->mZ;
        float angle = mYBodyRot * Mth::DEG_TO_RAD + Mth::cos((float)mTickCount * 0.6662f) * 0.25f;
        float cosAngle = Mth::cos(angle);
        float sinAngle = Mth::sin(angle);
        mLevel->addParticle(PARTICLE_MOB_SPELL, mX + cosAngle * 0.6, mY + 1.8, mZ + sinAngle * 0.6, red, green, blue,
                            arrayWithLength<int>());
        mLevel->addParticle(PARTICLE_MOB_SPELL, mX - cosAngle * 0.6, mY + 1.8, mZ - sinAngle * 0.6, red, green, blue,
                            arrayWithLength<int>());
    }
}

SpellcasterIllager::IllagerSpell::IllagerSpell(int id, double x, double y, double z) : mId(id), mX(x), mY(y), mZ(z)
{
}

SpellcasterIllager::IllagerSpell* SpellcasterIllager::IllagerSpell::byId(int id)
{
    switch (id)
    {
    case 1:
        return gSpell1;
    case 2:
        return gSpell2;
    case 3:
        return gSpell3;
    case 4:
        return gSpell4;
    case 5:
        return gSpell5;
    default:
        return gSpell0;
    }
}

SpellcasterIllager::SpellcasterUseSpellGoal::SpellcasterUseSpellGoal(SpellcasterIllager* spellcaster)
    : Goal(),
      mSpellcaster(spellcaster),
      mSpellWarmup(0),
      mNextAttackTickCount(0)
{
    setRequiredControlFlags(3);
}

bool SpellcasterIllager::SpellcasterUseSpellGoal::canUse()
{
    if (mSpellcaster->getTarget() == NULL)
        return false;
    if (mSpellcaster->isCastingSpell())
        return false;
    if (mSpellcaster->mTickCount < mNextAttackTickCount)
        return false;
    return true;
}

bool SpellcasterIllager::SpellcasterUseSpellGoal::canContinueToUse()
{
    return mSpellcaster->getTarget() != NULL && mSpellcaster->isCastingSpell();
}

bool SpellcasterIllager::SpellcasterUseSpellGoal::canInterrupt()
{
    return false;
}

void SpellcasterIllager::SpellcasterUseSpellGoal::start()
{
    mSpellWarmup = getCastWarmupTime();
    int castTicks = getCastWarmupTime() + getCastingTime();
    mSpellcaster->mSpellTicks = castTicks;
    mNextAttackTickCount = mSpellcaster->mTickCount + getCastingInterval();
    mSpellcaster->mNavigation->stop();
}

void SpellcasterIllager::SpellcasterUseSpellGoal::stop()
{
    Goal::stop();
    mSpellcaster->mSpellTicks = 0;
    mSpellcaster->setIsCastingSpell(gSpell0);
}

void SpellcasterIllager::SpellcasterUseSpellGoal::tick()
{
    if (mSpellcaster->getTarget())
    {
        mSpellcaster->getLookControl()->setLookAt(mSpellcaster->getTarget(), (float)mSpellcaster->getMaxHeadYRot(),
                                                  (float)mSpellcaster->getMaxHeadXRot());
    }

    if (mSpellWarmup > 0)
    {
        mSpellWarmup--;
        if (mSpellWarmup == 0)
        {
            mSpellcaster->setIsCastingSpell(getSpell());
            const SoundEvent* sound = getSpellPrepareSound();
            if (sound != NULL)
            {
                mSpellcaster->playSound(sound, 1.0f, 1.0f);
            }
        }
    }

    if (mSpellcaster->mSpellTicks > 0)
    {
        mSpellcaster->mSpellTicks--;
        if (mSpellcaster->mSpellTicks == 0)
        {
            performSpellCasting();
            mSpellcaster->playSound(mSpellcaster->getCastingSoundEvent(), 1.0f, 1.0f);
        }
    }
}

int SpellcasterIllager::SpellcasterUseSpellGoal::getCastWarmupTime()
{
    return 20;
}
