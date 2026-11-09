#include "game/client/render/FireworksParticles__FireworksStarter.hpp"
#include "game/world/Level.hpp"
#include "game/item/DyeItem.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/FireworksParticles__FireworksOverlayParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/FireworksParticles__FireworksSparkParticle.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/FireworksItem.hpp"
#include "game/util/Random.hpp"

FireworksParticles::FireworksStarter::FireworksStarter(Level* level, double x, double y, double z, double xa, double ya,
                                                       double za, ParticleEngine* particleEngine,
                                                       CompoundTag* fireworks)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0),
      life(0),
      twinkle(false)
{
    xd = xa;
    yd = ya;
    zd = za;
    this->particleEngine = particleEngine;
    lifetime = 8;
    if (fireworks != NULL)
    {
        explosions = fireworks->getList(FireworksItem::TAG_EXPLOSIONS, Tag::TAG_COMPOUND)->copy();
        if (explosions->isEmpty())
        {
            explosions = NULL;
        }
        else
        {
            lifetime = explosions->size() * 2 - 1;
            for (int i = 0; i < explosions->size(); ++i)
            {
                CompoundTag* explosion = explosions->getCompound(i);
                if (explosion->getBoolean(FireworksItem::TAG_E_FLICKER))
                {
                    twinkle = true;
                    lifetime += 15;
                    break;
                }
            }
        }
    }
    else
    {
        explosions = NULL;
    }
}

void FireworksParticles::FireworksStarter::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera,
                                                  float partialTicks, float xa, float ya, float za, float xa2,
                                                  float za2)
{
}

bool FireworksParticles::FireworksStarter::isFarAwayFromCamera()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->getCameraEntity())
    {
        if (minecraft->getCameraEntity()->distanceToSqr(x, y, z) < 256.0)
        {
            return false;
        }
    }
    return true;
}

void FireworksParticles::FireworksStarter::createParticle(double px, double py, double pz, double xa, double ya,
                                                          double za, arrayWithLength<int> colors,
                                                          arrayWithLength<int> fadeColors, bool trail, bool flicker)
{
    FireworksParticles::FireworksSparkParticle* particle = new FireworksParticles::FireworksSparkParticle(
        level, px, py, pz, xa, ya, za, particleEngine);
    particle->setTrail(trail);
    particle->setFlicker(flicker);
    particle->setColor(colors[random.nextInt(colors.length)]);
    if (fadeColors.length != 0)
    {
        particle->setFadeColor(fadeColors[random.nextInt(fadeColors.length)]);
    }
    particleEngine->add(particle);
}

void FireworksParticles::FireworksStarter::createParticleBall(double scale, int radius, arrayWithLength<int> colors,
                                                              arrayWithLength<int> fadeColors, bool trail, bool flicker)
{
    const double spread = 0.5;
    const double originX = x;
    const double originY = y;
    const double originZ = z;
    for (int yy = -radius; yy <= radius; ++yy)
    {
        for (int xx = -radius; xx <= radius; ++xx)
        {
            for (int zz = -radius; zz <= radius; ++zz)
            {
                double xa = xx + (random.nextDouble() - random.nextDouble()) * spread;
                double ya = yy + (random.nextDouble() - random.nextDouble()) * spread;
                double za = zz + (random.nextDouble() - random.nextDouble()) * spread;
                double velocity = std::sqrt(xa * xa + ya * ya + za * za) / scale + random.nextGaussian() * 0.05;
                createParticle(originX, originY, originZ, xa / velocity, ya / velocity, za / velocity, colors,
                               fadeColors, trail, flicker);
                if (yy != -radius && yy != radius && xx != -radius && xx != radius)
                    zz += radius * 2 - 1;
            }
        }
    }
}

void FireworksParticles::FireworksStarter::createParticleShape(double scale, array2DWithLength<double> shape,
                                                               arrayWithLength<int> colors,
                                                               arrayWithLength<int> fadeColors, bool trail,
                                                               bool flicker, bool keepShape)
{
    double firstX = shape.data[0]->data[0];
    double firstY = shape.data[0]->data[1];
    createParticle(x, y, z, firstX * scale, firstY * scale, 0.0, colors, fadeColors, trail, flicker);
    float initialAngle = random.nextFloat() * 3.1415927F;
    double rotationStep = keepShape ? 0.034 : 0.34;
    for (int plane = 0; plane < 3; ++plane)
    {
        double angle = (double)((float)plane * 3.1415927F) * rotationStep + initialAngle;
        double previousX = firstX;
        double previousY = firstY;
        for (unsigned int point = 1; point < shape.length; ++point)
        {
            double nextX = shape.data[point]->data[0];
            double nextY = shape.data[point]->data[1];
            for (double fraction = 0.25; fraction <= 1.0; fraction += 0.25)
            {
                double xd = (previousX + (nextX - previousX) * fraction) * scale;
                double yd = (previousY + (nextY - previousY) * fraction) * scale;
                double zd = xd * std::sin(angle);
                xd = xd * std::cos(angle);
                for (double side = -1.0; side <= 1.0; side += 2.0)
                {
                    createParticle(x, y, z, xd * side, yd, zd * side, colors, fadeColors, trail, flicker);
                }
            }
            previousX = nextX;
            previousY = nextY;
        }
    }
}

void FireworksParticles::FireworksStarter::createParticleBurst(arrayWithLength<int> colors,
                                                               arrayWithLength<int> fadeColors, bool trail,
                                                               bool flicker)
{
    double xVelocityBase = random.nextGaussian() * 0.05;
    double zVelocityBase = random.nextGaussian() * 0.05;
    for (int i = 0; i < 70; ++i)
    {
        double xVelocity = xd * 0.5 + random.nextGaussian() * 0.15 + xVelocityBase;
        double zVelocity = zd * 0.5 + random.nextGaussian() * 0.15 + zVelocityBase;
        double yVelocity = yd * 0.5 + random.nextDouble() * 0.5;
        createParticle(x, y, z, xVelocity, yVelocity, zVelocity, colors, fadeColors, trail, flicker);
    }
}

void FireworksParticles::FireworksStarter::tick()
{
    if (life == 0 && explosions != NULL)
    {
        const bool farAway = isFarAwayFromCamera();
        bool large = false;
        if (explosions->size() >= 3)
        {
            large = true;
        }
        else
        {
            for (int i = 0; i < explosions->size(); ++i)
            {
                if (explosions->getCompound(i)->getByte(FireworksItem::TAG_E_TYPE) == 1)
                {
                    large = true;
                    break;
                }
            }
        }
        const SoundEvent* blast;
        if (large)
            blast = farAway ? SoundEvents::FIREWORK_LARGE_BLAST_FAR : SoundEvents::FIREWORK_LARGE_BLAST;
        else
            blast = farAway ? SoundEvents::FIREWORK_BLAST_FAR : SoundEvents::FIREWORK_BLAST;
        level->playLocalSound(x, y, z, blast, SoundSource::eSoundSource_Ambient, 20.0f,
                              0.95f + random.nextFloat() * 0.1f, true, 100.0f);
    }

    if (life % 2 == 0 && explosions != NULL && life / 2 < explosions->size())
    {
        CompoundTag* explosion = explosions->getCompound(life / 2);
        const int type = explosion->getByte(FireworksItem::TAG_E_TYPE);
        const bool trail = explosion->getBoolean(FireworksItem::TAG_E_TRAIL);
        const bool flicker = explosion->getBoolean(FireworksItem::TAG_E_FLICKER);
        arrayWithLength<int> colors = explosion->getIntArray(FireworksItem::TAG_E_COLORS);
        arrayWithLength<int> fadeColors = explosion->getIntArray(FireworksItem::TAG_E_FADECOLORS);
        if (colors.length == 0)
        {
            colors = arrayWithLength<int>(1, true);
            colors.data[0] = DyeItem::DYE_COLORS[0];
        }

        if (type == 1)
        {
            createParticleBall(0.5, 4, colors, fadeColors, trail, flicker);
        }
        else if (type == 2)
        {
            double starShape[6][2] = {{0.0, 1.0},
                                      {0.3455, 0.309},
                                      {0.9511, 0.309},
                                      {0.3795918367346939, -0.12653061224489795},
                                      {0.6122448979591837, -0.8040816326530612},
                                      {0.0, -0.35918367346938773}};
            array2DWithLength<double> shape(6, 2);
            for (unsigned int point = 0; point < shape.length; ++point)
            {
                arrayWithLength<double>* row = shape.data[point];
                unsigned int count = row->length;
                for (unsigned int axis = 0; axis < count; ++axis)
                    row->data[axis] = starShape[point][axis];
            }
            createParticleShape(0.5, shape, colors, fadeColors, trail, flicker, false);
            for (unsigned int point = 0; point < shape.length; ++point)
                delete[] shape.data[point]->data;
            delete[] shape.data;
        }
        else if (type == 3)
        {
            double creeperShape[12][2] = {{0.0, 0.2}, {0.2, 0.2}, {0.2, 0.6},  {0.6, 0.6},  {0.6, 0.2},  {0.2, 0.2},
                                          {0.2, 0.0}, {0.4, 0.0}, {0.4, -0.6}, {0.2, -0.6}, {0.2, -0.4}, {0.0, -0.4}};
            array2DWithLength<double> shape(12, 2);
            for (unsigned int point = 0; point < shape.length; ++point)
            {
                arrayWithLength<double>* row = shape.data[point];
                unsigned int count = row->length;
                for (unsigned int axis = 0; axis < count; ++axis)
                    row->data[axis] = creeperShape[point][axis];
            }
            createParticleShape(0.5, shape, colors, fadeColors, trail, flicker, true);
            for (unsigned int point = 0; point < shape.length; ++point)
                delete[] shape.data[point]->data;
            delete[] shape.data;
        }
        else if (type == 4)
        {
            createParticleBurst(colors, fadeColors, trail, flicker);
        }
        else
        {
            createParticleBall(0.25, 2, colors, fadeColors, trail, flicker);
        }

        const int colour = colors.data[0];
        FireworksOverlayParticle* overlay = new FireworksOverlayParticle(level, x, y, z);
        overlay->setColor((float)((colour >> 16) & 0xFF) / 255.0f, (float)((colour >> 8) & 0xFF) / 255.0f,
                          (float)(colour & 0xFF) / 255.0f);
        particleEngine->add(overlay);
    }

    ++life;
    if (life > lifetime)
    {
        if (twinkle)
        {
            const bool farAway = isFarAwayFromCamera();
            level->playLocalSound(x, y, z, farAway ? SoundEvents::FIREWORK_TWINKLE_FAR : SoundEvents::FIREWORK_TWINKLE,
                                  SoundSource::eSoundSource_Ambient, 20.0f, 0.9f + random.nextFloat() * 0.15f, true,
                                  100.0f);
        }
        remove();
    }
}

int FireworksParticles::FireworksStarter::getParticleTexture()
{
    return 0;
}

uint64_t FireworksParticles::FireworksStarter::GetType()
{
    return ParticleKind::FIREWORKS_STARTER;
}
