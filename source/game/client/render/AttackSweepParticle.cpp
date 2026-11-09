#include "game/client/render/AttackSweepParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation PARTICLES_LOCATION(TN_MISC_EXPLOSION);

AttackSweepParticle::AttackSweepParticle(Textures* textures, Level* level, double x, double y, double z, double xa,
                                         double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0),
      textures(textures)
{
    quadSize = (float)(1.0 - xa * 0.5);
    float colour = random.nextFloat() * 0.6f + 0.4f;
    rCol = colour;
    gCol = colour;
    bCol = colour;
    lifetime = 4;
}

void AttackSweepParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                                 float xa, float ya, float za, float xa2, float za2)
{
    int frame = (int)(((float)age + partialTicks) * 3.0f / (float)lifetime);
    if (frame > 7)
        return;

    textures->bindTexture(&PARTICLES_LOCATION);

    float u0 = (float)(frame % 4) / 4.0f;
    float u1 = u0 + 0.24975f;
    float v0 = (float)(frame / 2) / 2.0f;
    float v1 = v0 + 0.4995f;
    float size = quadSize;
    float px = (float)((x - xo) * partialTicks + xo - Particle::xOff);
    float py = (float)((y - yo) * partialTicks + yo - Particle::yOff);
    float pz = (float)((z - zo) * partialTicks + zo - Particle::zOff);

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableLighting();
    Lighting::turnOff();
    buffer->begin(3);
    buffer->color(rCol, gCol, bCol);
    buffer->vertexUV(px - xa * size - xa2 * size, py - ya * size * 0.5f, pz - za * size - za2 * size, u1, v1);
    buffer->vertexUV(px - xa * size + xa2 * size, py + ya * size * 0.5f, pz - za * size + za2 * size, u1, v0);
    buffer->vertexUV(px + xa * size + xa2 * size, py + ya * size * 0.5f, pz + za * size + za2 * size, u0, v0);
    buffer->vertexUV(px + xa * size - xa2 * size, py - ya * size * 0.5f, pz + za * size - za2 * size, u0, v1);
    buffer->end();
    GlStateManager::enableLighting();
}

int AttackSweepParticle::getLightColor(float partialTicks)
{
    return 61680;
}

void AttackSweepParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    ++age;
    if (age == lifetime)
        remove();
}

int AttackSweepParticle::getParticleTexture()
{
    return 3;
}

uint64_t AttackSweepParticle::GetType()
{
    return ParticleKind::ATTACK_SWEEP;
}

Particle* AttackSweepParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                        double xa, double ya, double za, arrayWithLength<int> params)
{
    return new AttackSweepParticle(Minecraft::GetInstance()->mTextures, level, x, y, z, xa, ya, za);
}
