#include "game/client/render/EnderParticle.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/TextureUVCoordinateSet.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Math.hpp"
#include "game/util/AABB.hpp"

EnderParticle::EnderParticle(Level* level, double x, double y, double z, double xa, double ya, double za)
    : Particle(level, x, y, z, xa, ya, za),
      originX(x),
      originY(y),
      originZ(z)
{
    xd = xa;
    yd = ya;
    zd = za;
    this->x = x;
    this->y = y;
    this->z = z;
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_Ender);
    rCol = (float)((colour >> 16) & 255) / 255.0f;
    gCol = (float)((colour >> 8) & 255) / 255.0f;
    bCol = (float)(colour & 255) / 255.0f;
    float colourScale = (float)(random.nextFloat() * 0.6f + 0.4f);
    rCol *= colourScale;
    gCol *= colourScale;
    bCol *= colourScale;
    scale = size = (float)(random.nextFloat() * 0.2f + 0.5f);
    lifetime = (int)(Math::random() * 10.0) + 40;
    hasPhysics = false;
    setMiscTex((int)(Math::random() * 8.0));
}
void EnderParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                           float ya, float za, float xa2, float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    progress = 1.0f - progress;
    progress *= progress;
    progress = 1.0f - progress;
    size = scale * progress;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}
int EnderParticle::getLightColor(float partialTicks)
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
float EnderParticle::getBrightness(float partialTicks)
{
    float brightness = Particle::getBrightness(partialTicks);
    float progress = (float)age / (float)lifetime;
    progress *= progress;
    progress *= progress;
    return brightness * (1.0f - progress) + progress;
}
void EnderParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    float progress = (float)age / (float)lifetime;
    float elapsed = progress;
    progress = -progress + progress * progress * 2.0f;
    progress = 1.0f - progress;
    x = xd * progress + originX;
    y = yd * progress + originY + (1.0f - elapsed);
    z = zd * progress + originZ;
    if (age++ >= lifetime)
        remove();
}
void EnderParticle::move(double xa, double ya, double za)
{
    setBoundingBox(getBoundingBox()->move(xa, ya, za));
    setLocationFromBoundingbox();
}
uint64_t EnderParticle::GetType()
{
    return ParticleKind::ENDER;
}
Particle* EnderParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z,
                                                  double xa, double ya, double za, arrayWithLength<int> params)
{
    return new EnderParticle(level, x, y, z, xa, ya, za);
}
