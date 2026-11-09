#include "game/core/Phantom.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/CircleAroundAnchorGoal.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/ai/GlideMoveControl.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/SwoopAttackGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/InsomniaComponent.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcDedicated.hpp"
#endif

Phantom::Phantom(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    if (mMoveControl != NULL)
        delete mMoveControl;
    mMoveControl = new GlideMoveControl(this, 0.1f, 0.2f);
    setSize(0.9f, 0.5f);
    setNoGravity(true);
}

Entity* Phantom::create(Level* level)
{
    return new Phantom(level);
}

void Phantom::tick()
{
    if (mLevel->mIsClientSide)
    {
        float flap = Mth::cos((float)(getId() * 3 + mTickCount) * 0.13f);
        float nextFlap = Mth::cos((float)(getId() * 3 + mTickCount + 1) * 0.13f);

        if (flap > 0.0f && nextFlap <= 0.0f)
        {
            mLevel->playLocalSound(mX, mY, mZ, SoundEvents::PHANTOM_FLAP, getSoundSource(),
                                   0.95f + mRandom->nextFloat() * 0.05f, 0.95f + mRandom->nextFloat() * 0.05f, false,
                                   16.0f);
        }

        float wingAngle = (flap + 1.0f) * 0.5f * -50.0f * Mth::DEG_TO_RAD;
        float xOffset = Mth::cos(mYRot * Mth::DEG_TO_RAD) * 1.3f * Mth::cos(wingAngle);
        float zOffset = Mth::sin(mYRot * Mth::DEG_TO_RAD) * 1.3f * Mth::cos(wingAngle);
        float yOffset = Mth::sin(wingAngle) + 0.3f;
        mLevel->addParticle(PARTICLE_TOWN_AURA, mX + xOffset, mY + yOffset, mZ + zOffset, 0.0, 0.0, 0.0,
                            arrayWithLength<int>());
        mLevel->addParticle(PARTICLE_TOWN_AURA, mX - xOffset, mY + yOffset, mZ - zOffset, 0.0, 0.0, 0.0,
                            arrayWithLength<int>());
    }

    Monster::tick();
}

bool Phantom::checkSpawnRules()
{
#ifdef MINECRAFT_PC
    if (!PcPhantomsEnabled())
        return false;
#endif
    unsigned char hasSkyLight = mLevel->mDimension->isHasSkyLight();
    Random* random = mLevel->mRandom;
    PlayerSelector selector(this);
    std::vector<boost::shared_ptr<Player> >* players = mLevel->getPlayers(EntityType::PLAYER, &selector);
    for (std::vector<boost::shared_ptr<Player> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        BlockPos playerPos(player);
        InsomniaComponent* insomnia = player->getInsomniaComponent();
        if (insomnia != NULL && hasSkyLight != 0 && mLevel->canSeeSky(playerPos))
        {
            int timer = insomnia->getInsomniaTimerTicks();
            int until = insomnia->getTicksUntilInsomnia();
            if (timer < 1)
                timer = 1;
            int roll = random->nextInt(timer);
            if (roll >= until)
            {
                int x = random->nextInt(21);
                int y = random->nextInt(15);
                int z = random->nextInt(21);
                BlockPos spawnPos = playerPos + BlockPos(x - 10, y + 20, z - 10);
                setPos((double)spawnPos.x, (double)spawnPos.y, (double)spawnPos.z);
                return Monster::checkSpawnRules();
            }
        }
    }
    delete players;
    return false;
}

void Phantom::aiStep()
{
    Monster::aiStep();
    if (!mLevel->mIsClientSide && mLevel->isDay())
    {
        float brightness = getBrightness();
        if (brightness > 0.5f && mRandom->nextFloat() * 30.0f < (brightness - 0.4f) * 2.0f)
        {
            if (mLevel->canSeeSky(BlockPos(this)))
                setSecondsOnFire(8);
        }
    }
}

EMobType Phantom::getMobType()
{
    return MOB_TYPE_UNDEAD;
}
bool Phantom::canFly()
{
    return true;
}

void Phantom::registerGoals()
{
    mGoalSelector.addGoal(1, new SwoopAttackGoal(this, 1.0f, Goal::FloatRange(10.0f, 20.0f)), true);
    mGoalSelector.addGoal(2,
                          new CircleAroundAnchorGoal(this, 1.0f, Goal::FloatRange(12.0f, 18.0f), 250,
                                                     Goal::FloatRange(20.0f, 40.0f), Goal::FloatRange(-4.0f, 5.0f), 350,
                                                     1.0f),
                          true);
    mTargetSelector.addGoal(1, new NearestAttackableTargetGoal(this, typeid(Player), false, false, false), true);
}

void Phantom::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.8f);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10.0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(64.0);
}

PathNavigation* Phantom::createNavigation(Level* level)
{
    FlyingPathNavigation* navigation = new FlyingPathNavigation(this, level);
    navigation->setCanOpenDoors(false);
    return navigation;
}

const SoundEvent* Phantom::getAmbientSound()
{
    return SoundEvents::PHANTOM_AMBIENT;
}
const SoundEvent* Phantom::getHurtSound(const DamageSource*)
{
    return SoundEvents::PHANTOM_HURT;
}
const SoundEvent* Phantom::getDeathSound()
{
    return SoundEvents::PHANTOM_DEATH;
}
ResourceLocation* Phantom::getDefaultLootTable()
{
    return &BuiltInLootTables::PHANTOM;
}
unsigned long long Phantom::GetType()
{
    return EntityType::PHANTOM;
}

bool Phantom::PlayerSelector::apply(boost::shared_ptr<Player> player) const
{
#ifdef MINECRAFT_PC
    if (player->mHiddenHost)
        return false;
#endif
    return mPhantom->distanceToSqr(player) < 10000.0;
}
