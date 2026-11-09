#ifndef GAME_CLIENT_RENDER_PARTICLEPROVIDER_HPP
#define GAME_CLIENT_RENDER_PARTICLEPROVIDER_HPP

#include "game/client/render/ParticleType.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Level;
class Particle;

class ParticleProvider
{
public:
    virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa,
                                     double ya, double za, arrayWithLength<int> params) = 0;
};

#endif
