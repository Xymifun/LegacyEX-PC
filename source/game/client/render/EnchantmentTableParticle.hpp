#ifndef GAME_CLIENT_RENDER_ENCHANTMENTTABLEPARTICLE_HPP
#define GAME_CLIENT_RENDER_ENCHANTMENTTABLEPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class EnchantmentTableParticle : public Particle
{
public:
    EnchantmentTableParticle(Level* level, double x, double y, double z, double xa, double ya, double za);
    virtual int getLightColor(float partialTicks);
    virtual float getBrightness(float partialTicks);
    virtual void move(double xa, double ya, double za);
    virtual void tick();
    virtual uint64_t GetType();

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                         double ya, double za, arrayWithLength<int> params);
    };

private:
    float mOriginalSize;
    double mOriginX;
    double mOriginY;
    double mOriginZ;
};

#endif
