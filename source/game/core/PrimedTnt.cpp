#include "game/core/PrimedTnt.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <string>

#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/Math.hpp"
#include "game/world/Level.hpp"
#include "game/core/PIXNamedEvents.hpp"

const EntityDataAccessor<int>* PrimedTnt::DATA_FUSE_ID = NULL;

PrimedTnt::PrimedTnt(Level* level) : Entity(level, true), mOwner()
{
    _init();
}

PrimedTnt::PrimedTnt(Level* level, double x, double y, double z, boost::shared_ptr<LivingEntity> owner)
    : Entity(level, true),
      mOwner()
{
    _init();
    setPos(x, y, z);
    const float angle = (float)(Math::random() * 6.2831854820251465);
    mXd = -(std::sin(angle) * 0.02f);
    mYd = 0.2f;
    mZd = -(std::cos(angle) * 0.02f);
    setFuse(80);
    mXo = x;
    mYo = y;
    mZo = z;
    mOwner = boost::weak_ptr<LivingEntity>(owner);
}

PrimedTnt::~PrimedTnt()
{
}

Entity* PrimedTnt::create(Level* level)
{
    return new PrimedTnt(level);
}

void PrimedTnt::staticCtor()
{
    DATA_FUSE_ID = SynchedEntityData::defineId(EntityType::PRIMED_TNT, EntityType::ENTITY, EntityDataSerializers::INT);
}

void PrimedTnt::_init()
{
    fjDerivedCtorCalls();
    mLife = 80;
    mBlocksBuilding = true;
    mFireImmune = true;
    setSize(0.98f, 0.98f);
    mOwner = boost::weak_ptr<LivingEntity>();
}

void PrimedTnt::setFuse(int fuse)
{
    mEntityData->set(DATA_FUSE_ID, fuse, true);
    mLife = fuse;
}

void PrimedTnt::defineSynchedData()
{
    mEntityData->createDateItem(DATA_FUSE_ID, 80);
}

bool PrimedTnt::makeStepSound()
{
    return false;
}

bool PrimedTnt::isPickable()
{
    return !mRemoved;
}

void PrimedTnt::explode()
{
    const float radius = 4.0f;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && !Minecraft::GetInstance()->GetMiniGame()->canTntDestroyBlocks())
    {
        mLevel->explode(shared_from_this(), mX, mY + static_cast<double>(mBbHeight * 0.0625f), mZ, radius, false, false,
                        true, false, false, -1.0f);
    }
    else
    {
        mLevel->explode(shared_from_this(), mX, mY + static_cast<double>(mBbHeight * 0.0625f), mZ, radius, false, true,
                        false, false, false, -1.0f);
    }
}

void PrimedTnt::tick()
{
    PIXBeginNamedEvent(0.0f, "Primed tnt tick");
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    if (!isNoGravity())
    {
        mYd -= 0.04f;
    }
    move(eMoverType_Self, mXd, mYd, mZd, false);
    mXd *= 0.98f;
    mYd *= 0.98f;
    mZd *= 0.98f;
    if (mOnGround)
    {
        mXd *= 0.7f;
        mZd *= 0.7f;
        mYd *= -0.5;
    }
    --mLife;
    if (mLife <= 0)
    {
        remove();
        if (!mLevel->mIsClientSide)
        {
            explode();
        }
    }
    else
    {
        updateInWaterState();
        mLevel->addParticle(PARTICLE_SMOKE, mX, mY + 0.5, mZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    PIXEndNamedEvent();
}

int PrimedTnt::getLife()
{
    return mLife;
}

void PrimedTnt::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putShort(L"Fuse", static_cast<short>(getLife()));
}

void PrimedTnt::readAdditionalSaveData(CompoundTag* tag)
{
    setFuse(tag->getShort(L"Fuse"));
}

boost::shared_ptr<LivingEntity> PrimedTnt::getOwner()
{
    return mOwner.lock();
}

float PrimedTnt::getEyeHeight()
{
    return 0.0;
}

int PrimedTnt::getFuse()
{
    return mEntityData->get(DATA_FUSE_ID);
}

void PrimedTnt::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (DATA_FUSE_ID->getId() == accessor->getId())
    {
        mLife = getFuse();
    }
}

unsigned long long PrimedTnt::GetType()
{
    return EntityType::PRIMED_TNT;
}
