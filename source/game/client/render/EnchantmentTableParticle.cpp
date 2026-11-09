#include "game/client/render/EnchantmentTableParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Math.hpp"

EnchantmentTableParticle::EnchantmentTableParticle(Level* level, double x, double y, double z, double xa, double ya,
                                                   double za)
    : Particle(level, x, y, z, xa, ya, za),
      mOriginX(x),
      mOriginY(y),
      mOriginZ(z)
{
    xd = xa;
    yd = ya;
    zd = za;
    xo = x + xa;
    yo = y + ya;
    zo = z + za;
    this->x = x + xa;
    this->y = y + ya;
    this->z = z + za;
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Particle_EnchantmentTable);
    float red = (float)((colour >> 16) & 255) / 255.0f;
    float green = (float)((colour >> 8) & 255) / 255.0f;
    float blue = (float)(colour & 255) / 255.0f;
    float colourScale = random.nextFloat() * 0.6f + 0.4f;
    rCol = red * colourScale;
    gCol = green * colourScale;
    bCol = blue * colourScale;
    size = random.nextFloat() * 0.5f + 0.2f;
    mOriginalSize = size;
    lifetime = (int)(Math::random() * 10.0) + 30;
    setMiscTex((int)(Math::random() * 26.0 + 225.0));
}

int EnchantmentTableParticle::getLightColor(float partialTicks)
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

float EnchantmentTableParticle::getBrightness(float partialTicks)
{
    float brightness = Particle::getBrightness(partialTicks);
    float progress = (float)age / (float)lifetime;
    progress *= progress;
    progress *= progress;
    return brightness * (1.0f - progress) + progress;
}

void EnchantmentTableParticle::move(double xa, double ya, double za)
{
    setBoundingBox(getBoundingBox()->move(xa, ya, za));
    setLocationFromBoundingbox();
}

void EnchantmentTableParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    float progress = (float)age / (float)lifetime;
    progress = 1.0f - progress;
    float drop = 1.0f - progress;
    drop *= drop;
    drop *= drop;
    x = xd * progress + mOriginX;
    y = yd * progress + mOriginY - drop * 1.2f;
    z = zd * progress + mOriginZ;
    if (age++ >= lifetime)
        remove();
}

uint64_t EnchantmentTableParticle::GetType()
{
    return ParticleKind::ENCHANTMENT_TABLE;
}

Particle* EnchantmentTableParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y,
                                                             double z, double xa, double ya, double za,
                                                             arrayWithLength<int> params)
{
    return new EnchantmentTableParticle(level, x, y, z, xa, ya, za);
}
