#include "game/core/Abilities.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"

Abilities::Abilities()
    : mInvulnerable(false),
      mFlying(false),
      mMayfly(false),
      mInstabuild(false),
      mMayBuild(true),
      mFlyingSpeed(0.05f),
      mWalkingSpeed(0.1f)
{
}

bool Abilities::operator==(const Abilities& other) const
{
    return mInvulnerable == other.mInvulnerable && mFlying == other.mFlying && mMayfly == other.mMayfly
           && mInstabuild == other.mInstabuild && mMayBuild == other.mMayBuild && mFlyingSpeed == other.mFlyingSpeed
           && mWalkingSpeed == other.mWalkingSpeed;
}

void Abilities::addSaveData(CompoundTag* tag)
{
    CompoundTag* abilitiesTag = new CompoundTag();

    abilitiesTag->putBoolean(L"invulnerable", mInvulnerable);
    abilitiesTag->putBoolean(L"flying", mFlying);
    abilitiesTag->putBoolean(L"mayfly", mMayfly);
    abilitiesTag->putBoolean(L"instabuild", mInstabuild);
    abilitiesTag->putBoolean(L"mayBuild", mMayBuild);
    abilitiesTag->putFloat(L"flySpeed", mFlyingSpeed);
    abilitiesTag->putFloat(L"walkSpeed", mWalkingSpeed);

    tag->put(L"abilities", abilitiesTag);
}

void Abilities::loadSaveData(CompoundTag* tag)
{
    if (tag->contains(L"abilities", Tag::TAG_COMPOUND))
    {
        CompoundTag* abilitiesTag = tag->getCompound(L"abilities");

        mInvulnerable = abilitiesTag->getBoolean(L"invulnerable");
        mFlying = abilitiesTag->getBoolean(L"flying");
        mMayfly = abilitiesTag->getBoolean(L"mayfly");
        mInstabuild = abilitiesTag->getBoolean(L"instabuild");

        if (abilitiesTag->contains(L"flySpeed", Tag::TAG_ANY_NUMERIC))
        {
            mFlyingSpeed = abilitiesTag->getFloat(L"flySpeed");
            mWalkingSpeed = abilitiesTag->getFloat(L"walkSpeed");
        }

        if (abilitiesTag->contains(L"mayBuild", Tag::TAG_BYTE))
        {
            mMayBuild = abilitiesTag->getBoolean(L"mayBuild");
        }
    }
}

float Abilities::getFlyingSpeed()
{
    return mFlyingSpeed;
}

void Abilities::setFlyingSpeed(float speed)
{
    mFlyingSpeed = speed;
}

float Abilities::getWalkingSpeed()
{
    return mWalkingSpeed;
}

void Abilities::setWalkingSpeed(float speed)
{
    mWalkingSpeed = speed;
}

void Abilities::write(DataOutputStream* output)
{
    output->writeBoolean(mInvulnerable);
    output->writeBoolean(mFlying);
    output->writeBoolean(mMayfly);
    output->writeBoolean(mInstabuild);
    output->writeBoolean(mMayBuild);
    output->writeFloat(mFlyingSpeed);
    output->writeFloat(mWalkingSpeed);
}

void Abilities::read(DataInputStream* input)
{
    mInvulnerable = input->readBoolean();
    mFlying = input->readBoolean();
    mMayfly = input->readBoolean();
    mInstabuild = input->readBoolean();
    mMayBuild = input->readBoolean();
    mFlyingSpeed = input->readFloat();
    mWalkingSpeed = input->readFloat();
}
