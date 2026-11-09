#include "game/client/render/NetherPortalParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/Particle.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/AABB.hpp"

NetherPortalParticle::NetherPortalParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    xd = xa;
    yd = ya;
    zd = za;
    originX = this->x = x;
    originY = this->y = y;
    originZ = this->z = z;

    float colourScale = random.nextFloat() * 0.6f + 0.4f;
    float particleScale = random.nextFloat() * 0.2f + 0.5f;
    Particle::size = particleScale;
    scale = particleScale;

    int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_NetherPortal);
    float red = (float)((colour >> 16) & 255) / 255.0f;
    float green = (float)((colour >> 8) & 255) / 255.0f;
    float blue = (float)(colour & 255) / 255.0f;
    rCol = red * colourScale;
    gCol = green * colourScale;
    bCol = blue * colourScale;
    lifetime = random.nextInt(10) + 40;
    setMiscTex(random.nextInt(8));
}

void NetherPortalParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                                  float xa, float ya, float za, float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    progress = 1.0f - progress;
    progress *= progress;
    progress = 1.0f - progress;
    Particle::size = scale * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

int NetherPortalParticle::getLightColor(float partialTicks)
{
    int light = Particle::getLightColor(partialTicks);
    float progress = (float)age / (float)lifetime;
    progress *= progress;
    progress *= progress;
    int sky = light & 255;
    int block = (light >> 16) & 255;
    block += (int)(progress * 15.0f * 16.0f);
    if (block > 240)
        block = 240;
    return sky | (block << 16);
}

float NetherPortalParticle::getBrightness(float partialTicks)
{
    float brightness = Particle::getBrightness(partialTicks);
    float progress = (float)age / (float)lifetime;
    progress *= progress;
    progress *= progress;
    return brightness * (1.0f - progress) + progress;
}

void NetherPortalParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    float progress = (float)age / (float)lifetime;
    float curve = -progress + progress * progress * 2.0f;
    curve = 1.0f - curve;
    x = originX + xd * curve;
    y = originY + yd * curve + (1.0f - progress);
    z = originZ + zd * curve;

    if (age++ >= lifetime)
        remove();
}

void NetherPortalParticle::move(double xa, double ya, double za)
{
    setBoundingBox(getBoundingBox()->move(xa, ya, za));
    setLocationFromBoundingbox();
}

uint64_t NetherPortalParticle::GetType()
{
    return ParticleKind::NETHER_PORTAL;
}

Particle* NetherPortalParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                         double z, double xa, double ya, double za,
                                                         arrayWithLength<int> params)
{
    return new NetherPortalParticle(level, x, y, z, xa, ya, za);
}
