#ifndef GAME_CLIENT_RENDER_FOOTSTEPPARTICLE_HPP
#define GAME_CLIENT_RENDER_FOOTSTEPPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class Level;
class Textures;

class FootstepParticle : public Particle
{
public:
    FootstepParticle(Textures* textures, Level* level, double x, double y, double z);
    virtual void tick();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual int getParticleTexture();
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    Textures* textures;
};

#endif
