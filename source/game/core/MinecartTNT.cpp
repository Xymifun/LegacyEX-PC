#include "game/core/MinecartTNT.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <string>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/world/generation/Player.hpp"


Entity* MinecartTNT::create(Level* level)
{
    return new MinecartTNT(level);
}

MinecartTNT::MinecartTNT(Level* level) : Minecart(level)
{
    _init();
}

MinecartTNT::MinecartTNT(Level* level, double x, double y, double z) : Minecart(level, x, y, z)
{
    _init();
}

void MinecartTNT::_init()
{
    fjDerivedCtorCalls();
    mFuse = -1;
}

void MinecartTNT::addDataWalkers(DataFixerUpper*)
{
}

const Minecart::Type* MinecartTNT::getType()
{
    return Minecart::Type::TNT;
}

const BlockState* MinecartTNT::getDefaultDisplayBlockState()
{
    return Blocks::tnt->defaultBlockState();
}

void MinecartTNT::tick()
{
    PIXBeginNamedEvent(0.0f, "Minecart tnt tick");
    Minecart::tick();

    if (mFuse >= 0)
    {
        if (mFuse > 0)
        {
            --mFuse;
            mLevel->addParticle(PARTICLE_SMOKE, mX, mY + 0.5, mZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
        }
        else
        {
            explode(mXd * mXd + mZd * mZd);
        }
    }

    if (mHorizontalCollision && mXd * mXd + mZd * mZd >= 0.01f)
    {
        explode(mXd * mXd + mZd * mZd);
    }
    PIXEndNamedEvent();
}

bool MinecartTNT::hurt(const DamageSource* source, float damage)
{
    boost::shared_ptr<Entity> entity = source->getDirectEntity();
    if (entity)
    {
        if (eTYPE_DERIVED_FROM(EntityType::ARROW, entity->GetType()))
        {
            boost::shared_ptr<Arrow> arrow = boost::dynamic_pointer_cast<Arrow>(entity);
            if (arrow->isOnFire())
            {
                explode(arrow->mXd * arrow->mXd + arrow->mYd * arrow->mYd + arrow->mZd * arrow->mZd);
            }
        }
    }
    return Minecart::hurt(source, damage);
}

void MinecartTNT::destroy(const DamageSource* source)
{
    double speedSqr = mXd * mXd + mZd * mZd;
    if (gApp.GetGameHostOption(eGameHostOption_TNT) != 0
        && (source->isFire() || source->isExplosion() || speedSqr >= 0.01f))
    {
        if (mFuse < 0)
        {
            primeFuse();
            mFuse = mRandom->nextInt(20) + mRandom->nextInt(20);
        }
        return;
    }

    Minecart::destroy(source);
    if (gApp.GetGameHostOption(eGameHostOption_TNT) == 0
        || (!source->isExplosion() && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS)))
    {
        spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::tnt, 1)), 0.0f);
    }
}

void MinecartTNT::explode(double power)
{
    if (!mLevel->mIsClientSide)
    {
        if (gApp.GetGameHostOption(eGameHostOption_TNT) != 0)
        {
            double radius = std::sqrt(power);
            if (radius > 5.0)
                radius = 5.0;
            mLevel->explode(shared_from_this(), mX, mY, mZ, (float)(mRandom->nextDouble() * 1.5 * radius + 4.0), false,
                            true, false, false, false, -1.0f);
            remove();
        }
        else
        {
            destroy(NULL);
        }
    }
}

void MinecartTNT::causeFallDamage(float fallDistance, float damageMultiplier)
{
    if (fallDistance >= 3.0f)
    {
        explode((float)(fallDistance / 10.0f) * (float)(fallDistance / 10.0f));
    }
    Entity::causeFallDamage(fallDistance, damageMultiplier);
}

void MinecartTNT::activateMinecart(int, int, int, bool powered)
{
    if (powered && mFuse < 0)
        primeFuse();
}

void MinecartTNT::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::MINECART_TNT_PRIME)
        primeFuse();
    else
        Entity::handleEntityEvent(event, data);
}

void MinecartTNT::primeFuse()
{
    if (gApp.GetGameHostOption(eGameHostOption_TNT) != 0)
    {
        mFuse = 80;
        if (!mLevel->mIsClientSide)
        {
            mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::MINECART_TNT_PRIME, 0);
            if (!isSilent())
                mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::TNT_PRIMED,
                                  SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);
        }
    }
}

int MinecartTNT::getFuse()
{
    return mFuse;
}

bool MinecartTNT::isPrimed()
{
    return mFuse >= 0;
}

float MinecartTNT::getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos)
{
    if (isPrimed() && (BaseRailBlock::isRail(level->getBlock(pos)) || BaseRailBlock::isRail(level, pos.above()))
        && !isUnderLiquid(Material::WATER))
    {
        return 0.0f;
    }
    return Entity::getBlockExplosionResistance(explosion, level, pos);
}

bool MinecartTNT::shouldBlockExplode(Explosion* explosion, Level* level, const BlockPos& pos, float radius)
{
    if (gApp.GetGameHostOption(eGameHostOption_TNT) == 0 || BaseRailBlock::isRail(level->getBlock(pos))
        || BaseRailBlock::isRail(level, pos.above()))
    {
        return false;
    }
    return Entity::shouldBlockExplode(explosion, level, pos, radius);
}

void MinecartTNT::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    if (tag->contains(std::wstring(L"TNTFuse"), Tag::TAG_ANY_NUMERIC))
        mFuse = tag->getInt(std::wstring(L"TNTFuse"));
}

void MinecartTNT::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"TNTFuse"), mFuse);
}

unsigned long long MinecartTNT::GetType()
{
    return EntityType::MINECART_TNT;
}
