#ifndef GAME_CLIENT_RENDER_NOTEPARTICLE_HPP
#define GAME_CLIENT_RENDER_NOTEPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class NoteParticle : public Particle
{
public:
    NoteParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    NoteParticle(Level* level, double x, double y, double z, double xa, double ya, double za, float scale);
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual void tick();
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    void init(Level* level, double x, double y, double z, double xa, double ya, double za, float scale);

    float scale;
};

#endif
