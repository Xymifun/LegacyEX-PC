#include "game/client/render/PlayerCloudParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

PlayerCloudParticle::PlayerCloudParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    xd = xd * 0.1 + xa;
    yd = yd * 0.1 + ya;
    zd = zd * 0.1 + za;
    rCol = 1.0f - random.nextFloat() * 0.3f;
    gCol = rCol;
    bCol = rCol;
    size *= 0.75f;
    size *= 2.5f;
    baseSize = size;
    lifetime = (int)std::floor(8.0 / (random.nextDouble() * 0.8 + 0.3) * 2.5);
}

void PlayerCloudParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                                 float xa, float ya, float za, float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    size = baseSize * Mth::clamp(progress * 32.0f, 0.0f, 1.0f);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void PlayerCloudParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    if (age++ >= lifetime)
    {
        remove();
    }

    setMiscTex(7 - (age << 3) / lifetime);
    move(xd, yd, zd);

    xd *= 0.96f;
    yd *= 0.96f;
    zd *= 0.96f;

    boost::shared_ptr<Player> nearestPlayer = level->getNearestPlayer(x, y, z, 2.0, false, -1.0);
    if (nearestPlayer)
    {
        AABB* box = nearestPlayer->mBoundingBox;
        if (y > box->mMinY)
        {
            y += (box->mMinY - y) * 0.2;
            yd += (nearestPlayer->mYd - yd) * 0.2;
            setPos(x, y, z);
        }
    }

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}

uint64_t PlayerCloudParticle::GetType()
{
    return ParticleKind::PLAYER_CLOUD;
}

Particle* PlayerCloudParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                        double xa, double ya, double za, arrayWithLength<int> params)
{
    return new PlayerCloudParticle(level, x, y, z, xa, ya, za);
}
