#ifndef GAME_CLIENT_RENDER_FIREWORKSPARTICLES_FIREWORKSSTARTER_HPP
#define GAME_CLIENT_RENDER_FIREWORKSPARTICLES_FIREWORKSSTARTER_HPP

#include "game/client/render/FireworksParticles__FireworksSparkParticle.hpp"
#include "game/client/render/Particle.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/array2DWithLength.hpp"

class BufferBuilder;
class CompoundTag;
class ParticleEngine;
class Entity;
class ListTag;

class FireworksParticles::FireworksStarter : public Particle
{
public:
    FireworksStarter(Level* level, double x, double y, double z, double xa, double ya, double za,
                     ParticleEngine* particleEngine, CompoundTag* fireworks);
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    bool isFarAwayFromCamera();
    void createParticle(double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> colors,
                        arrayWithLength<int> fadeColors, bool trail, bool flicker);
    void createParticleBall(double scale, int radius, arrayWithLength<int> colors, arrayWithLength<int> fadeColors,
                            bool trail, bool flicker);
    void createParticleShape(double scale, array2DWithLength<double> shape, arrayWithLength<int> colors,
                             arrayWithLength<int> fadeColors, bool trail, bool flicker, bool keepShape);
    void createParticleBurst(arrayWithLength<int> colors, arrayWithLength<int> fadeColors, bool trail, bool flicker);
    virtual void tick();
    virtual int getParticleTexture();
    virtual uint64_t GetType();

private:
    int life;
    ParticleEngine* particleEngine;
    ListTag* explosions;
    bool twinkle;
};

#endif
