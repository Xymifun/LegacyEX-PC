#ifndef GAME_CLIENT_RENDER_PARTICLEENGINE_HPP
#define GAME_CLIENT_RENDER_PARTICLEENGINE_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/client/render/ParticleProvider.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BlockPos;
class BlockState;
class Direction;
class Entity;
class Level;
class Particle;
class Textures;
class TrackingEmitter;

class ParticleEngine
{
public:
    class ParticleList
    {
    public:
        ParticleList();
        ~ParticleList();
        void ClearParticles();
        Particle* at(int index);
        int size();
        void push_back(Particle* particle);
        void RemoveParticle(unsigned int index);
        bool empty();

    private:
        Particle** particles;
        int particleCount;
    };

    ParticleEngine(Level* level, Textures* textures);
    ~ParticleEngine();
    void ToggleParticles();
    void markTranslucent(Particle* particle);
    void markOpaque(Particle* particle);
    void registerParticle(ePARTICLE_TYPE type, ParticleProvider* provider);
    void registerProviders();
    void createTrackingEmitter(boost::shared_ptr<Entity> entity, const ParticleType* type);
    void createTrackingEmitter(boost::shared_ptr<Entity> entity, const ParticleType* type, int count);
    void add(Particle* particle);
    Particle* createParticle(ePARTICLE_TYPE type, double x, double y, double z, double xa, double ya, double za,
                             arrayWithLength<int> parameters, Level* level);
    void tickParticle(Particle* particle);
    void tickParticleList(ParticleList* particles);
    void moveParticlesBetweenLists(ParticleList* first, ParticleList* second);
    void tickTextureType(int textureType);
    void tick();
    void render(boost::shared_ptr<Entity>& entity, float partialTick, int layer);
    void renderLit(boost::shared_ptr<Entity> entity, float partialTick, int layer);
    void destroy(const BlockPos& pos, const BlockState* state);
    void crack(const BlockPos& pos, const Direction* direction);
    void setLevel(Level* level);
    std::wstring countParticles();

private:
    Level* level;
    ParticleList particles[3][5][2];
    std::vector<TrackingEmitter*> trackingEmitters;
    Textures* textures;
    Random random;
    boost::unordered_map<ePARTICLE_TYPE, ParticleProvider*> providers;
    bool translucent;
    bool opaque;
    int particleCount;
};

#endif
