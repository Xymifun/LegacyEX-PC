#include "game/core/Snowball.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/DamageSource.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

Snowball::Snowball(Level* level, boost::shared_ptr<LivingEntity> owner) : ThrowableProjectile(level, owner)
{
    _init();
}

Snowball::Snowball(Level* level) : ThrowableProjectile(level)
{
    _init();
}

Snowball::Snowball(Level* level, double x, double y, double z) : ThrowableProjectile(level, x, y, z)
{
    _init();
}

Entity* Snowball::create(Level* level)
{
    return new Snowball(level);
}

void Snowball::_init()
{
    fjDerivedCtorCalls();
}

bool Snowball::onHit(const HitResult* result)
{
    Projectile::onHit(result);

    if (result->mEntity)
    {
        if (!mLevel->mIsClientSide)
        {
            if (Minecraft::InMiniGame(eMiniGameId_Any, true) && mOwner
                && eTYPE_DERIVED_FROM(EntityType::PLAYER, mOwner->GetType()))
            {
                GameMode* gameMode = boost::static_pointer_cast<Player>(mOwner)->GetGameMode();
                gameMode->RecordSnowballHit();

                if (eTYPE_DERIVED_FROM(EntityType::PLAYER, result->mEntity->GetType()))
                {
                    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(result->mEntity);
                    if (player && player->GetGameMode() != NULL)
                    {
                        player->GetGameMode()->RecordHitBySnowball();
                    }
                }
            }
        }

        float damage = 0.0f;
        bool isBlaze = result->mEntity->GetType() == EntityType::BLAZE;
        if (isBlaze)
        {
            damage = 3.0f;
        }
        else if (eTYPE_DERIVED_FROM(EntityType::PLAYER, result->mEntity->GetType())
                 && Minecraft::InMiniGame(eMiniGameId_Battle, false))
        {
            damage = 0.1f;
        }

        DamageSource* source = DamageSource::CreateThrown(shared_from_this(), getOwner());
        result->mEntity->hurt(source, damage);
        delete source;
    }

    if (!mLevel->mIsClientSide)
    {
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::DEATH, 0);
        remove();
    }
    else
    {
        fjSetParticleSpawnPos(mX, mY, mZ);
    }
    return true;
}

void Snowball::addDataWalkers(DataFixerUpper* fixer)
{
    ThrowableProjectile::addDataWalkers(fixer, EntityType::SNOWBALL);
}

void Snowball::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::DEATH)
    {
        Vec3* position = fjGetParticleSpawnPos();
        for (int i = 0; i < 8; ++i)
        {
            mLevel->addParticle(PARTICLE_SNOWBALL_POOF, position->x, position->y, position->z, 0.0, 0.0, 0.0,
                                arrayWithLength<int>());
        }
        return;
    }
    Entity::handleEntityEvent(event, data);
}

unsigned long long Snowball::GetType()
{
    return EntityType::SNOWBALL;
}
