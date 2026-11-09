#include "game/core/ExperienceOrb.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Math.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

ExperienceOrb::ExperienceOrb(Level* level, double x, double y, double z, int value) : Entity(level, true)
{
    _init();
    setSize(0.5f, 0.5f);
    setPos(x, y, z);
    mYRot = (float)(Math::random() * 360.0);
    mXd = (double)((float)(Math::random() * 0.20000000298023224 - 0.10000000149011612) * 2.0f);
    mYd = (double)((float)(Math::random() * 0.2) * 2.0f);
    mZd = (double)((float)(Math::random() * 0.20000000298023224 - 0.10000000149011612) * 2.0f);
    mValue = value;
}

ExperienceOrb::ExperienceOrb(Level* level) : Entity(level, true)
{
    _init();
    setSize(0.25f, 0.25f);
}

int ExperienceOrb::getExperienceValue(int value)
{
    if (value >= 2477)
    {
        return 2477;
    }
    if (value >= 1237)
    {
        return 1237;
    }
    if (value >= 617)
    {
        return 617;
    }
    if (value >= 307)
    {
        return 307;
    }
    if (value >= 149)
    {
        return 149;
    }
    if (value >= 73)
    {
        return 73;
    }
    if (value >= 37)
    {
        return 37;
    }
    if (value >= 17)
    {
        return 17;
    }
    if (value >= 7)
    {
        return 7;
    }
    if (value >= 3)
    {
        return 3;
    }
    return 1;
}

void ExperienceOrb::_init()
{
    fjDerivedCtorCalls();
    mTickCount = 0;
    mAge = 0;
    mPickupDelay = 0;
    mHealth = 5;
    mValue = 0;
    mFollowingPlayer = boost::shared_ptr<Player>();
    mFollowingTime = 0;
}

bool ExperienceOrb::makeStepSound()
{
    return false;
}

void ExperienceOrb::defineSynchedData()
{
}

int ExperienceOrb::getLightColor()
{
    float brightness = Mth::clamp(0.5f, 0.0f, 1.0f);
    int color = Entity::getLightColor();
    int lower = color & 255;
    int upper = (color >> 16) & 255;
    lower += (int)(brightness * 15.0f * 16.0f);
    if (lower > 240)
    {
        lower = 240;
    }
    return lower | (upper << 16);
}

void ExperienceOrb::tick()
{
    Entity::tick();
    if (mPickupDelay > 0)
    {
        --mPickupDelay;
    }

    mXo = mX;
    mYo = mY;
    mZo = mZ;

    if (!isNoGravity())
    {
        mYd -= 0.029999999329447746;
    }

    if (mLevel->getBlockState(BlockPos(shared_from_this()))->getBlock()->mMaterial == Material::LAVA)
    {
        mYd = 0.20000000298023224;
        mXd = (double)((mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f);
        mZd = (double)((mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f);
        playSound(SoundEvents::GENERIC_BURN, 0.4f, mRandom->nextFloat() * 0.4f + 2.0f);
    }

    checkInBlock(mX, (mBoundingBox->mMinY + mBoundingBox->mMaxY) * 0.5, mZ);

    double maxDist = 8.0;
    if (mFollowingTime < mTickCount - 20 + getId() % 100)
    {
        if (!mFollowingPlayer || mFollowingPlayer->distanceToSqr(shared_from_this()) > maxDist * maxDist)
        {
            mFollowingPlayer = mLevel->getNearestPlayer(shared_from_this(), maxDist, -1.0);
        }
        mFollowingTime = mTickCount;
    }

    if (mFollowingPlayer && mFollowingPlayer->isSpectator())
    {
        mFollowingPlayer = boost::shared_ptr<Player>();
    }

    if (mFollowingPlayer)
    {
        double xd = (mFollowingPlayer->mX - mX) / maxDist;
        double yd = (mFollowingPlayer->mY + (double)mFollowingPlayer->getEyeHeight() * 0.5 - mY) / maxDist;
        double zd = (mFollowingPlayer->mZ - mZ) / maxDist;
        double distance = std::sqrt(xd * xd + yd * yd + zd * zd);
        double strength = 1.0 - distance;

        if (strength > 0.0)
        {
            strength *= strength;
            mXd += xd / distance * strength * 0.1;
            mYd += yd / distance * strength * 0.1;
            mZd += zd / distance * strength * 0.1;
        }
    }

    move(eMoverType_Self, mXd, mYd, mZd, false);

    float friction = 0.98f;
    if (mOnGround)
    {
        friction = mLevel->getBlockState(BlockPos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY) - 1, Mth::floor(mZ)))
                       ->getBlock()
                       ->mFriction
                   * 0.98f;
    }

    mXd *= friction;
    mYd *= 0.9800000190734863;
    mZd *= friction;

    if (mOnGround)
    {
        mYd *= -0.8999999761581421;
    }

    ++mTickCount;
    if (++mAge >= 6000)
    {
        remove();
    }
}

bool ExperienceOrb::updateInWaterState()
{
    return mLevel->checkAndHandleWater(mBoundingBox, Material::WATER, shared_from_this());
}

void ExperienceOrb::burn(int damage)
{
    hurt(DamageSource::IN_FIRE, (float)damage);
}

bool ExperienceOrb::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
    {
        return false;
    }
    markHurt();
    if (!mLevel->mIsClientSide)
    {
        mHealth -= (int)damage;
        if (mHealth <= 0)
        {
            remove();
        }
    }
    return false;
}

void ExperienceOrb::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putShort(std::wstring(L"Health"), (short)mHealth);
    tag->putShort(std::wstring(L"Age"), (short)mAge);
    tag->putShort(std::wstring(L"Value"), (short)mValue);
}

void ExperienceOrb::readAdditionalSaveData(CompoundTag* tag)
{
    mHealth = tag->getShort(std::wstring(L"Health")) & 255;
    mAge = tag->getShort(std::wstring(L"Age"));
    mValue = tag->getShort(std::wstring(L"Value"));
}

int ExperienceOrb::xpToDurability(int value)
{
    return value << 1;
}

int ExperienceOrb::durabilityToXp(int value)
{
    return value / 2;
}

void ExperienceOrb::playerTouch(boost::shared_ptr<Player> player)
{
    if (!mLevel->mIsClientSide && mPickupDelay == 0 && player->mTakeXpDelay == 0)
    {
        player->mTakeXpDelay = 2;
        player->take(shared_from_this(), 1);

        boost::shared_ptr<ItemInstance> item = EnchantmentHelper::getRandomItemWith(Enchantments::MENDING, player);
        if (!item->isEmpty() && item->isDamaged())
        {
            int repaired = std::min(xpToDurability(mValue), item->getDamageValue());
            mValue -= durabilityToXp(repaired);
            item->setAuxValue(item->getDamageValue() - repaired);
        }

        if (mValue > 0)
        {
            player->increaseXp(mValue);
        }
        remove();
    }
}

int ExperienceOrb::getValue()
{
    return mValue;
}

unsigned char ExperienceOrb::getIcon()
{
    if (mValue >= 2477)
    {
        return 10;
    }
    if (mValue >= 1237)
    {
        return 9;
    }
    if (mValue >= 617)
    {
        return 8;
    }
    if (mValue >= 307)
    {
        return 7;
    }
    if (mValue >= 149)
    {
        return 6;
    }
    if (mValue >= 73)
    {
        return 5;
    }
    if (mValue >= 37)
    {
        return 4;
    }
    if (mValue >= 17)
    {
        return 3;
    }
    if (mValue >= 7)
    {
        return 2;
    }
    return (unsigned char)(mValue >= 3);
}

bool ExperienceOrb::isAttackable()
{
    return false;
}

bool ExperienceOrb::shouldRender(double distanceSqr)
{
    if (distanceSqr < 4.0)
    {
        return false;
    }
    return Entity::shouldRender(distanceSqr);
}

Entity* ExperienceOrb::create(Level* level)
{
    return new ExperienceOrb(level);
}

unsigned long long ExperienceOrb::GetType()
{
    return EntityType::EXPERIENCE_ORB;
}
