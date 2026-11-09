#include "game/core/Explosion.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ProtectionEnchantment.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

static unsigned int gExplosionCountTlsIndex = TlsAlloc();

void Explosion::_init(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius,
                      bool fire, bool breaksBlocks, bool playersOnly, bool noBlockDrops, bool silent,
                      float damageRadius, bool breaksBlocksUnderwater)
{
    mLevel = level;
    mSource = source;
    mX = x;
    mY = y;
    mZ = z;
    mRadius = radius;
    mFire = fire;
    mBreaksBlocks = breaksBlocks;
    mPlayersOnly = playersOnly;
    mNoBlockDrops = noBlockDrops;
    mSilent = silent;
    mDamageRadius = damageRadius;
    mBreaksBlocksUnderwater = breaksBlocksUnderwater;
}

Explosion::Explosion(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius,
                     float damageRadius)
{
    _init(level, source, x, y, z, radius, false, true, false, false, false, damageRadius, false);
}

Explosion::Explosion(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius,
                     std::vector<BlockPos>* toBlow, float damageRadius)
{
    _init(level, source, x, y, z, radius, false, true, false, false, false, damageRadius, false);

    mToBlow.insert(toBlow->begin(), toBlow->end());
}

Explosion::Explosion(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius,
                     bool fire, bool breaksBlocks, bool playersOnly, std::vector<BlockPos>* toBlow, float damageRadius)
{
    _init(level, source, x, y, z, radius, fire, breaksBlocks, playersOnly, false, false, damageRadius, false);

    mToBlow.insert(toBlow->begin(), toBlow->end());
}

Explosion::Explosion(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius,
                     bool fire, bool breaksBlocks, bool playersOnly, bool noBlockDrops, bool silent, float damageRadius,
                     bool breaksBlocksUnderwater)
{
    _init(level, source, x, y, z, radius, fire, breaksBlocks, playersOnly, noBlockDrops, silent, damageRadius,
          breaksBlocksUnderwater);
}

Explosion::~Explosion()
{
    for (HitPlayerMap::iterator hit = mHitPlayers.begin(); hit != mHitPlayers.end(); ++hit)
    {
        delete hit->second;
    }
}

void Explosion::explode()
{
    unsigned int nestedExplosions = (unsigned int)(uintptr_t)TlsGetValue(gExplosionCountTlsIndex);

    if (nestedExplosions > 50)
    {
        return;
    }

    nestedExplosions++;

    BlockPos pos(mX, mY, mZ);

    bool inWater = mLevel->getLiquidBlockState(pos)->getMaterial() == Material::WATER;

    if (mSource.get() != NULL && !mSource->mOnGround)
    {
        inWater = mLevel->containsLiquid(mSource->mBoundingBox->grow(0.0, 0.25, 0.0), Material::WATER);
    }

    if (mLevel->getBlockState(pos)->isSolid() && mSource.get() != NULL)
    {
        int startX = pos.x;
        int startY = pos.y;
        int startZ = pos.z;

        Vec3* push = Vec3::newTemp(mSource->mXd, mSource->mYd, mSource->mZd);

        float pushLength = (float)push->length();
        int pushSteps = (int)pushLength;

        if (pushLength > (float)pushSteps)
        {
            pushSteps++;
        }

        if (pushSteps > 5)
        {
            pushSteps = 5;
        }

        if (pushSteps > 0)
        {
            push = push->normalize();

            int step;

            for (step = 0; step < pushSteps; step++)
            {
                pos.subtract(Vec3i(push->x, push->y, push->z));

                mX = (double)pos.x;
                mY = (double)pos.y;
                mZ = (double)pos.z;

                if (!mLevel->getBlockState(pos)->isSolid())
                {
                    break;
                }
            }

            if (step == pushSteps)
            {
                pos = BlockPos(startX, startX, startZ);

                mX = (double)startX;
                mY = (double)startY;
                mZ = (double)startZ;
            }
            else
            {
                inWater = mLevel->getLiquidBlock(pos)->defaultBlockState()->getMaterial() == Material::WATER;
            }
        }
    }

    mBreaksBlocks = mBreaksBlocks && (!inWater || mBreaksBlocksUnderwater);

    int rayCount = 16;

    for (int rayX = 0; rayX < rayCount; rayX++)
    {
        for (int rayY = 0; rayY < rayCount; rayY++)
        {
            for (int rayZ = 0; rayZ < rayCount; rayZ++)
            {
                if (rayX != 0 && rayX != rayCount - 1 && rayY != 0 && rayY != rayCount - 1 && rayZ != 0
                    && rayZ != rayCount - 1)
                {
                    continue;
                }

                float extent = (float)rayCount - 1.0f;

                double directionX = (double)((float)rayX / extent * 2.0f - 1.0f);
                double directionY = (double)((float)rayY / extent * 2.0f - 1.0f);
                double directionZ = (double)((float)rayZ / extent * 2.0f - 1.0f);

                double directionLength = sqrt(directionX * directionX + directionY * directionY
                                              + directionZ * directionZ);

                directionX /= directionLength;
                directionY /= directionLength;
                directionZ /= directionLength;

                float power = mRadius * (mLevel->mRandom->nextFloat() * 0.6f + 0.7f);

                double rayPosX = mX;
                double rayPosY = mY;
                double rayPosZ = mZ;

                directionX *= 0.3f;
                directionY *= 0.3f;
                directionZ *= 0.3f;

                for (; power > 0.0f; power -= 0.3f * 0.75f)
                {
                    BlockPos rayBlockPos(rayPosX, rayPosY, rayPosZ);

                    if (!mBreaksBlocksUnderwater && mLevel->getExtraBlockState(pos)->getMaterial() == Material::WATER)
                    {
                        power = 0.0f;
                    }

                    Block* block = mLevel->getBlock(rayBlockPos);

                    if (block->mMaterial != Material::AIR
                        && (!mBreaksBlocksUnderwater || (block != Blocks::flowing_water && block != Blocks::water)))
                    {
                        float resistance;

                        if (mSource.get() != NULL)
                        {
                            resistance = mSource->getBlockExplosionResistance(this, mLevel, rayBlockPos);
                        }
                        else
                        {
                            resistance = block->getExplosionResistance(boost::shared_ptr<Entity>());
                        }

                        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureFairExplosions())
                        {
                            resistance = 5.0f;
                        }

                        power -= (resistance + 0.3f) * 0.3f;
                    }

                    if (power > 0.0f
                        && (mSource.get() == NULL || mSource->shouldBlockExplode(this, mLevel, rayBlockPos, power))
                        && mToBlow.find(rayBlockPos) == mToBlow.end())
                    {
                        mToBlow.insert(rayBlockPos);
                    }

                    rayPosX += directionX;
                    rayPosY += directionY;
                    rayPosZ += directionZ;
                }
            }
        }
    }

    if (mDamageRadius == -1.0f)
    {
        mDamageRadius = mRadius + mRadius;
    }

    double reach = (double)mDamageRadius;

    int minX = (int)(mX - reach - 1.0);
    if ((double)minX > mX - reach - 1.0)
    {
        minX--;
    }

    int maxX = (int)(mX + reach + 1.0);
    if ((double)maxX > mX + reach + 1.0)
    {
        maxX--;
    }

    int minY = (int)(mY - reach - 1.0);
    if ((double)minY > mY - reach - 1.0)
    {
        minY--;
    }

    int maxY = (int)(mY + reach + 1.0);
    if ((double)maxY > mY + reach + 1.0)
    {
        maxY--;
    }

    int minZ = (int)(mZ - reach - 1.0);
    if ((double)minZ > mZ - reach - 1.0)
    {
        minZ--;
    }

    int maxZ = (int)(mZ + reach + 1.0);
    if ((double)maxZ > mZ + reach + 1.0)
    {
        maxZ--;
    }

    std::vector<boost::shared_ptr<Entity> >* found = mLevel->getEntities(
        mSource, AABB::newTemp((double)minX, (double)minY, (double)minZ, (double)maxX, (double)maxY, (double)maxZ),
        NULL);

    std::vector<boost::shared_ptr<Entity> > entities(found->begin(), found->end());

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin(); it != entities.end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;

        if (mPlayersOnly && (entity->GetType() & EntityType::PLAYER) != EntityType::PLAYER)
        {
            continue;
        }

        bool inBlast = false;

        if (!mToBlow.empty())
        {
            for (ToBlowSet::const_iterator blown = mToBlow.begin(); blown != mToBlow.end(); ++blown)
            {
                const BlockPos& blownPos = *blown;

                if (entity->mBoundingBox->intersectsInner(
                        AABB::newTemp((double)blownPos.x, (double)blownPos.y, (double)blownPos.z,
                                      (double)blownPos.x + 1.0, (double)blownPos.y + 1.0, (double)blownPos.z + 1.0)))
                {
                    inBlast = true;
                    break;
                }
            }
        }

        double distance = entity->distanceTo(mX, mY, mZ);

        if (distance > (double)mDamageRadius)
        {
            continue;
        }

        double offsetX = entity->mX - mX;
        double offsetY = (entity->mY + (double)entity->getEyeHeight()) - mY;
        double offsetZ = entity->mZ - mZ;

        double offsetLength = sqrt(offsetX * offsetX + offsetY * offsetY + offsetZ * offsetZ);

        if (offsetLength == 0.0)
        {
            continue;
        }

        offsetX /= offsetLength;
        offsetY /= offsetLength;
        offsetZ /= offsetLength;

        double falloff = ((double)mDamageRadius - distance) * (double)(1.0f / mDamageRadius);

        if (inBlast)
        {
            DamageSource* damageSource = DamageSource::CreateExplosion(this);

            float damage = (float)((falloff * falloff + falloff) * 0.5 * 7.0 * (double)mDamageRadius + 1.0);

            if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
            {
                damage = 0.0f;
            }

            entity->hurt(damageSource, damage);
            delete damageSource;
        }

        double knockback = falloff;

        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
        {
            knockback = ProtectionEnchantment::getExplosionKnockbackAfterDampener(
                boost::static_pointer_cast<LivingEntity>(entity), falloff);
        }

        if (!inBlast)
        {
            knockback *= 0.25;
        }

        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades()
            && mSource->GetType() != EntityType::PRIMED_TNT)
        {
            knockback = -knockback;
        }

        offsetX *= knockback;
        offsetY *= knockback;
        offsetZ *= knockback;

        entity->mXd += offsetX;
        entity->mYd += offsetY;
        entity->mZd += offsetZ;

        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);

            if (!player->isSpectator() && (!player->isCreative() || !player->mAbilities.mFlying))
            {
                mHitPlayers.insert(std::make_pair(player, Vec3::newPermanent(offsetX, offsetY, offsetZ)));
            }
        }
    }

    nestedExplosions--;

    TlsSetValue(gExplosionCountTlsIndex, (void*)nestedExplosions);
}

void Explosion::finalizeExplosion(bool spawnParticles, std::vector<BlockPos>* toBlow)
{
    mLevel->playLocalSound(mX, mY, mZ, SoundEvents::GENERIC_EXPLODE, SoundSource::eSoundSource_Blocks, 4.0f,
                           ((mLevel->mRandom->nextFloat() - mLevel->mRandom->nextFloat()) * 0.2f + 1.0f) * 0.7f, false,
                           16.0f);

    if (spawnParticles)
    {
        if (mRadius >= 2.0f && mBreaksBlocks
            && !Minecraft::GetInstance()->GetMiniGame()->HasFeatureLessExplosionParticles())
        {
            mLevel->addParticle(PARTICLE_HUGE_EXPLOSION, mX, mY, mZ, 1.0, 0.0, 0.0, arrayWithLength<int>());
        }
        else
        {
            mLevel->addParticle(PARTICLE_LARGE_EXPLODE, mX, mY, mZ, 1.0, 0.0, 0.0, arrayWithLength<int>());
        }
    }

    std::vector<BlockPos>* affectedBlocks = toBlow;

    if (toBlow == NULL)
    {
        affectedBlocks = new std::vector<BlockPos>(mToBlow.begin(), mToBlow.end());
    }

    if (mBreaksBlocks)
    {
        PIXBeginNamedEvent(0.0f, "Finalizing explosion size %d", mToBlow.size());

        int particleDivisor = 50;

        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureLessExplosionParticles())
        {
            particleDivisor = 10;
        }

        unsigned int particleStride = (unsigned int)((int)affectedBlocks->size() / particleDivisor);

        if (particleStride == 0)
        {
            particleStride = 1;
        }

        unsigned int remaining = affectedBlocks->size();

        for (std::vector<BlockPos>::reverse_iterator it = affectedBlocks->rbegin(); it != affectedBlocks->rend(); ++it)
        {
            remaining--;

            BlockPos blockPos = *it;

            Block* block = mLevel->getBlock(blockPos);

            if (spawnParticles && remaining % particleStride == 0)
            {
                double particleX = (double)((float)blockPos.x + mLevel->mRandom->nextFloat());
                double particleY = (double)((float)blockPos.y + mLevel->mRandom->nextFloat());
                double particleZ = (double)((float)blockPos.z + mLevel->mRandom->nextFloat());

                double toParticleX = particleX - mX;
                double toParticleY = particleY - mY;
                double toParticleZ = particleZ - mZ;

                double toParticleLength = sqrt(toParticleX * toParticleX + toParticleY * toParticleY
                                               + toParticleZ * toParticleZ);

                toParticleX /= toParticleLength;
                toParticleY /= toParticleLength;
                toParticleZ /= toParticleLength;

                double speed = 0.5 / (toParticleLength / (double)mRadius + 0.1);
                speed *= (double)(mLevel->mRandom->nextFloat() * mLevel->mRandom->nextFloat() + 0.3f);
                toParticleX *= speed;
                toParticleY *= speed;
                toParticleZ *= speed;

                mLevel->addParticle(PARTICLE_EXPLODE, (particleX + mX) / 2.0, (particleY + mY) / 2.0,
                                    (particleZ + mZ) / 2.0, toParticleX, toParticleY, toParticleZ,
                                    arrayWithLength<int>());

                if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureLessExplosionParticles())
                {
                    mLevel->addParticle(PARTICLE_SMOKE, particleX, particleY, particleZ, toParticleX, toParticleY,
                                        toParticleZ, arrayWithLength<int>());
                }
            }

            if (block->mMaterial != Material::AIR)
            {
                if (block->dropFromExplosion(this))
                {
                    block->spawnResources(mLevel, blockPos, mLevel->getBlockState(blockPos), 1.0f / mRadius, 0);
                }

                mLevel->setBlock(blockPos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);

                block->wasExploded(mLevel, blockPos, this);
            }
        }
    }

    if (mFire)
    {
        for (std::vector<BlockPos>::reverse_iterator it = affectedBlocks->rbegin(); it != affectedBlocks->rend(); ++it)
        {
            BlockPos blockPos = *it;

            if (mLevel->getBlock(blockPos)->mMaterial == Material::AIR
                && mLevel->getBlockState(blockPos.below())->isSolid() && mRandom.nextInt(3) == 0)
            {
                mLevel->setBlockAndUpdate(blockPos, Blocks::fire->defaultBlockState());
            }
        }
    }

    PIXEndNamedEvent();

    if (toBlow == NULL)
    {
        delete affectedBlocks;
    }
}

Vec3* Explosion::getHitPlayerKnockback(boost::shared_ptr<Player> player)
{
    HitPlayerMap::iterator hit = mHitPlayers.find(player);

    if (hit == mHitPlayers.end())
    {
        return Vec3::newTemp(0.0, 0.0, 0.0);
    }

    return hit->second;
}

boost::shared_ptr<LivingEntity> Explosion::getSourceMob()
{
    if (mSource.get() == NULL)
    {
        return boost::shared_ptr<LivingEntity>();
    }

    if (eTYPE_DERIVED_FROM(EntityType::PRIMED_TNT, mSource->GetType()))
    {
        return boost::static_pointer_cast<PrimedTnt>(mSource)->getOwner();
    }

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, mSource->GetType()))
    {
        return boost::static_pointer_cast<LivingEntity>(mSource);
    }

    return boost::shared_ptr<LivingEntity>();
}

boost::shared_ptr<Entity> Explosion::getDirectSource()
{
    return mSource;
}

void Explosion::clearToBlow()
{
    mToBlow.clear();
}

Explosion::ToBlowSet* Explosion::getToBlow()
{
    return &mToBlow;
}
