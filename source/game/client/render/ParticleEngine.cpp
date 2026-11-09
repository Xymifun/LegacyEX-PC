#include "game/client/render/ParticleEngine.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/Particle.hpp"
#include "game/client/render/TerrainParticle.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Camera.hpp"
#include "game/core/Mth.hpp"
#include "game/core/TrackingEmitter.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/Direction.hpp"

#include "game/client/render/ExplodeParticle.hpp"
#include "game/client/render/SpitParticle.hpp"
#include "game/client/render/BubbleParticle.hpp"
#include "game/client/render/BubblePopParticle.hpp"
#include "game/client/render/WaterCurrentDownParticle.hpp"
#include "game/client/render/SplashParticle.hpp"
#include "game/client/render/WakeParticle.hpp"
#include "game/client/render/WaterDropParticle.hpp"
#include "game/client/render/SuspendedParticle.hpp"
#include "game/client/render/SuspendedTownParticle.hpp"
#include "game/client/render/CritParticle.hpp"
#include "game/client/render/SmokeParticle.hpp"
#include "game/client/render/LargeSmokeParticle.hpp"
#include "game/client/render/SpellParticle.hpp"
#include "game/client/render/DripParticle.hpp"
#include "game/client/render/HeartParticle.hpp"
#include "game/client/render/NoteParticle.hpp"
#include "game/client/render/NetherPortalParticle.hpp"
#include "game/client/render/EnchantmentTableParticle.hpp"
#include "game/client/render/DragonBreathParticle.hpp"
#include "game/client/render/EnderParticle.hpp"
#include "game/client/render/FlameParticle.hpp"
#include "game/client/render/LavaParticle.hpp"
#include "game/client/render/FootstepParticle.hpp"
#include "game/client/render/PlayerCloudParticle.hpp"
#include "game/client/render/RedDustParticle.hpp"
#include "game/client/render/FallingDustParticle.hpp"
#include "game/client/render/BreakingItemParticle.hpp"
#include "game/client/render/SnowShovelParticle.hpp"
#include "game/client/render/BarrierParticle.hpp"
#include "game/client/render/TerrainDustParticle.hpp"
#include "game/client/render/HugeExplosionSeedParticle.hpp"
#include "game/client/render/HugeExplosionParticle.hpp"
#include "game/client/render/FireworksParticles__Provider.hpp"
#include "game/client/render/MobAppearanceParticle.hpp"
#include "game/client/render/EndRodParticle.hpp"
#include "game/client/render/AttackSweepParticle.hpp"
#include "game/client/render/TotemParticle.hpp"
#include "game/client/render/InkParticle.hpp"
#include "game/client/render/ConduitParticle.hpp"
#include "game/client/render/ChestRefillParticle.hpp"

namespace
{
ResourceLocation particleTexture(TN_PARTICLES);
}

template <typename T>
std::wstring _toString(T value);

ParticleEngine::ParticleList::ParticleList() : particles(new Particle*[16384]), particleCount(0)
{
}

ParticleEngine::ParticleList::~ParticleList()
{
    ClearParticles();
    delete[] particles;
}

void ParticleEngine::ParticleList::ClearParticles()
{
    for (int i = 0; i < particleCount; ++i)
    {
        if (particles[i] != NULL)
        {
            delete particles[i];
        }
    }
    particleCount = 0;
}

Particle* ParticleEngine::ParticleList::at(int index)
{
    return particles[index];
}

int ParticleEngine::ParticleList::size()
{
    return particleCount;
}

void ParticleEngine::ParticleList::push_back(Particle* particle)
{
    particles[particleCount++] = particle;
}

void ParticleEngine::ParticleList::RemoveParticle(unsigned int index)
{
    particles[index] = particles[--particleCount];
}

bool ParticleEngine::ParticleList::empty()
{
    return particleCount == 0;
}

ParticleEngine::ParticleEngine(Level* level, Textures* textures) : translucent(false), opaque(false)
{
    this->level = level;
    this->textures = textures;
    registerProviders();
    particleCount = 0;
}

void ParticleEngine::registerParticle(ePARTICLE_TYPE type, ParticleProvider* provider)
{
    providers[type] = provider;
}

void ParticleEngine::registerProviders()
{
    registerParticle(PARTICLE_EXPLODE->getId(), new ExplodeParticle::Provider());
    registerParticle(PARTICLE_SPIT->getId(), new SpitParticle::Provider());
    registerParticle(PARTICLE_BUBBLE->getId(), new BubbleParticle::Provider());
    registerParticle(PARTICLE_BUBBLE_POP->getId(), new BubblePopParticle::Provider());
    registerParticle(PARTICLE_CURRENT_DOWN->getId(), new WaterCurrentDownParticle::Provider());
    registerParticle(PARTICLE_SPLASH->getId(), new SplashParticle::Provider());
    registerParticle(PARTICLE_WAKE->getId(), new WakeParticle::Provider());
    registerParticle(PARTICLE_DROPLET->getId(), new WaterDropParticle::Provider());
    registerParticle(PARTICLE_SUSPENDED->getId(), new SuspendedParticle::Provider());
    registerParticle(PARTICLE_DEPTH_SUSPEND->getId(), new SuspendedTownParticle::Provider());
    registerParticle(PARTICLE_CRIT->getId(), new CritParticle::Provider());
    registerParticle(PARTICLE_MAGIC_CRIT->getId(), new CritParticle::MagicProvider());
    registerParticle(PARTICLE_SMOKE->getId(), new SmokeParticle::Provider());
    registerParticle(PARTICLE_LARGE_SMOKE->getId(), new LargeSmokeParticle::Provider());
    registerParticle(PARTICLE_SPELL->getId(), new SpellParticle::Provider());
    registerParticle(PARTICLE_INSTANT_SPELL->getId(), new SpellParticle::InstantProvider());
    registerParticle(PARTICLE_MOB_SPELL->getId(), new SpellParticle::MobProvider());
    registerParticle(PARTICLE_MOB_SPELL_AMBIENT->getId(), new SpellParticle::AmbientMobProvider());
    registerParticle(PARTICLE_WITCH_MAGIC->getId(), new SpellParticle::WitchProvider());
    registerParticle(PARTICLE_DRIP_WATER->getId(), new DripParticle::WaterProvider());
    registerParticle(PARTICLE_DRIP_LAVA->getId(), new DripParticle::LavaProvider());
    registerParticle(PARTICLE_ANGRY_VILLAGER->getId(), new HeartParticle::AngryVillagerProvider());
    registerParticle(PARTICLE_HAPPY_VILLAGER->getId(), new SuspendedTownParticle::HappyVillagerProvider());
    registerParticle(PARTICLE_TOWN_AURA->getId(), new SuspendedTownParticle::Provider());
    registerParticle(PARTICLE_NOTE->getId(), new NoteParticle::Provider());
    registerParticle(PARTICLE_PORTAL->getId(), new NetherPortalParticle::Provider());
    registerParticle(PARTICLE_END_PORTAL->getId(), new SmokeParticle::Provider());
    registerParticle(PARTICLE_ENCHANTMENT_TABLE->getId(), new EnchantmentTableParticle::Provider());
    registerParticle(PARTICLE_DRAGON_BREATH->getId(), new DragonBreathParticle::Provider());
    registerParticle(PARTICLE_ENDER->getId(), new EnderParticle::Provider());
    registerParticle(PARTICLE_FLAME->getId(), new FlameParticle::Provider());
    registerParticle(PARTICLE_LAVA->getId(), new LavaParticle::Provider());
    registerParticle(PARTICLE_FOOTSTEP->getId(), new FootstepParticle::Provider());
    registerParticle(PARTICLE_CLOUD->getId(), new PlayerCloudParticle::Provider());
    registerParticle(PARTICLE_RED_DUST->getId(), new RedDustParticle::Provider());
    registerParticle(PARTICLE_FALLING_DUST->getId(), new FallingDustParticle::Provider());
    registerParticle(PARTICLE_SNOWBALL_POOF->getId(), new BreakingItemParticle::SnowballProvider());
    registerParticle(PARTICLE_SNOW_SHOVEL->getId(), new SnowShovelParticle::Provider());
    registerParticle(PARTICLE_SLIME->getId(), new BreakingItemParticle::SlimeProvider());
    registerParticle(PARTICLE_HEART->getId(), new HeartParticle::Provider());
    registerParticle(PARTICLE_BARRIER->getId(), new BarrierParticle::Provider());
    registerParticle(PARTICLE_ICON_CRACK->getId(), new BreakingItemParticle::Provider());
    registerParticle(PARTICLE_BLOCK_CRACK->getId(), new TerrainParticle::Provider());
    registerParticle(PARTICLE_BLOCK_DUST->getId(), new TerrainDustParticle::Provider());
    registerParticle(PARTICLE_HUGE_EXPLOSION->getId(), new HugeExplosionSeedParticle::Provider());
    registerParticle(PARTICLE_LARGE_EXPLODE->getId(), new HugeExplosionParticle::Provider());
    registerParticle(PARTICLE_FIREWORKS_SPARK->getId(), new FireworksParticles::Provider());
    registerParticle(PARTICLE_MOB_APPEARANCE->getId(), new MobAppearanceParticle::Provider());
    registerParticle(PARTICLE_END_ROD->getId(), new EndRodParticle::Provider());
    registerParticle(PARTICLE_DAMAGE_INDICATOR->getId(), new CritParticle::DamageIndicatorProvider());
    registerParticle(PARTICLE_SWEEP_ATTACK->getId(), new AttackSweepParticle::Provider());
    registerParticle(PARTICLE_TOTEM->getId(), new TotemParticle::Provider());
    registerParticle(PARTICLE_INK->getId(), new InkParticle::Provider());
    registerParticle(PARTICLE_CONDUIT->getId(), new ConduitParticle::Provider());
    registerParticle(PARTICLE_CHEST_REFILL->getId(), new ChestRefillParticle::Provider());
    registerParticle(PARTICLE_WINNING_INSTANT_SPELL->getId(), new SpellParticle::WinningInstantProvider());
}

void ParticleEngine::createTrackingEmitter(boost::shared_ptr<Entity> entity, const ParticleType* type)
{
    trackingEmitters.push_back(new TrackingEmitter(level, entity, type, 3));
}

void ParticleEngine::createTrackingEmitter(boost::shared_ptr<Entity> entity, const ParticleType* type, int count)
{
    trackingEmitters.push_back(new TrackingEmitter(level, entity, type, count));
}

void ParticleEngine::add(Particle* particle)
{
    int texture = particle->getParticleTexture();
    int dimension = particle->getLevel()->mDimension->getType() == DimensionType::OVERWORLD
                        ? 0
                        : (particle->getLevel()->mDimension->getType() == DimensionType::NETHER ? 1 : 2);
    int limit;
    bool lit = particle->getAlpha() == 1.0f;
    unsigned long long type = particle->GetType();
    switch (type)
    {
    case ParticleKind::DRAGON_BREATH:
        limit = 1000;
        break;
    case ParticleKind::FIREWORKS_SPARK:
        if (particleCount + 1 > 2000)
        {
            delete particle;
            return;
        }
        ++particleCount;
        limit = 0x4000;
        break;
    default:
        limit = 0x4000;
        break;
    }
    if (particle->isSemiTransparent())
    {
        lit = false;
    }
    if (particle->GetType() == ParticleKind::DRAGON_BREATH)
    {
        if (particles[dimension][texture][lit].size() < limit)
        {
            particles[dimension][texture][lit].push_back(particle);
        }
        return;
    }
    else if (particles[dimension][texture][lit].size() >= limit)
    {
        Particle* old = particles[dimension][texture][lit].at(0);
        if (old->GetType() == ParticleKind::FIREWORKS_SPARK)
        {
            --particleCount;
        }
        delete old;
        particles[dimension][texture][lit].RemoveParticle(0);
    }
    particles[dimension][texture][lit].push_back(particle);
}

Particle* ParticleEngine::createParticle(ePARTICLE_TYPE type, double x, double y, double z, double xa, double ya,
                                         double za, arrayWithLength<int> parameters, Level* level)
{
    ParticleProvider* provider = providers[type];
    if (provider != NULL)
    {
        Particle* particle = provider->createParticle(type, level != NULL ? level : this->level, x, y, z, xa, ya, za,
                                                      parameters);
        if (particle != NULL)
        {
            add(particle);
            return particle;
        }
    }
    return NULL;
}

void ParticleEngine::tickParticle(Particle* particle)
{
    particle->tick();
}

void ParticleEngine::tickParticleList(ParticleList* list)
{
    translucent = true;
    if (!list->empty())
    {
        int remaining = list->size();
        for (int i = 0; i < list->size() && remaining-- > 0; ++i)
        {
            Particle* particle = list->at(i);
            tickParticle(particle);
            if (!particle->isAlive())
            {
                particle = list->at(i);
                if (particle->GetType() == ParticleKind::FIREWORKS_SPARK)
                {
                    --particleCount;
                }
                delete particle;
                list->RemoveParticle(i--);
            }
        }
    }
    translucent = false;
}

void ParticleEngine::moveParticlesBetweenLists(ParticleList* first, ParticleList* second)
{
    translucent = true;
    if (!first->empty())
    {
        for (int i = 0; i < first->size(); ++i)
        {
            Particle* particle = first->at(i);
            if (particle->getAlpha() == 1.0)
            {
                first->RemoveParticle(i);
                second->push_back(particle);
            }
        }
    }
    if (!second->empty())
    {
        for (int i = 0; i < second->size(); ++i)
        {
            Particle* particle = second->at(i);
            if (particle->getAlpha() < 1.0)
            {
                second->RemoveParticle(i);
                first->push_back(particle);
            }
        }
    }
    translucent = false;
}

void ParticleEngine::tickTextureType(int textureType)
{
    for (int dimension = 0; dimension < 3; ++dimension)
    {
        for (int list = 0; list < 2; ++list)
        {
            tickParticleList(&particles[dimension][textureType][list]);
        }
        moveParticlesBetweenLists(&particles[dimension][textureType][0], &particles[dimension][textureType][1]);
    }
}

void ParticleEngine::tick()
{
    for (int i = 0; i < 5; ++i)
    {
        tickTextureType(i);
    }
    if (!trackingEmitters.empty())
    {
        for (std::vector<TrackingEmitter*>::iterator it = trackingEmitters.begin(); it != trackingEmitters.end();)
        {
            TrackingEmitter* emitter = *it;
            emitter->tick();
            if (!emitter->isAlive())
            {
                it = trackingEmitters.erase(it);
                delete emitter;
            }
            else
            {
                ++it;
            }
        }
    }
}

void ParticleEngine::render(boost::shared_ptr<Entity>& entity, float partialTick, int layer)
{
    if (opaque)
    {
        return;
    }

    const float cosYaw = gCameraXOff;
    const float cosPitch = gCameraYOff;
    const float sinYaw = gCameraZOff;
    const float cameraYOff = gCameraYOff2;
    const float cameraXOff = gCameraXOff2;

    Particle::setXOffset(entity->mXOld + (entity->mX - entity->mXOld) * partialTick);
    Particle::setYOffset(entity->mYOld + (entity->mY - entity->mYOld) * partialTick);
    Particle::setZOffset(entity->mZOld + (entity->mZ - entity->mZOld) * partialTick);
    Particle::setCamerViewDirection(entity->getViewVector(partialTick));

    int dimension;
    if (level->mDimension->getType() == DimensionType::OVERWORLD)
    {
        dimension = 0;
    }
    else
    {
        dimension = 2;
        if (level->mDimension->getType() == DimensionType::NETHER)
        {
            dimension = 1;
        }
    }

    if (layer != 0)
    {
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::alphaFunc(4, 0.003921569f);
    }

    for (int textureType = 0; textureType < 5; ++textureType)
    {
        if (textureType == 3)
        {
            continue;
        }

        if (!particles[dimension][textureType][layer].empty())
        {
            switch (layer)
            {
            case 0:
                GlStateManager::depthMask(false);
                break;
            case 1:
                GlStateManager::depthMask(true);
                break;
            }

            MemSect(0x1f);
            if (textureType == 0 || textureType == 4)
            {
                textures->bindTexture(&particleTexture);
            }
            if (textureType == 1)
            {
                textures->bindTexture(&BLOCKS_LOCATION);
            }
            else if (textureType == 2)
            {
                textures->bindTexture(&ITEM_LOCATION);
            }
            MemSect(0);

            BufferBuilder* buffer = Tesselator::getInstance()->getBuilder();
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
            buffer->begin();
            for (int i = 0; i < particles[dimension][textureType][layer].size(); ++i)
            {
                if (buffer->hasMaxVertices())
                {
                    buffer->end();
                    buffer->begin();
                }
                particles[dimension][textureType][layer].at(i)->render(buffer, entity, partialTick, cosYaw, cosPitch,
                                                                       sinYaw, cameraYOff, cameraXOff);
            }
            buffer->end();
        }
    }

    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::alphaFunc(4, 0.1f);
}

void ParticleEngine::renderLit(boost::shared_ptr<Entity> entity, float partialTick, int layer)
{
    Particle::setXOffset(entity->mXOld + (entity->mX - entity->mXOld) * partialTick);
    Particle::setYOffset(entity->mYOld + (entity->mY - entity->mYOld) * partialTick);
    Particle::setZOffset(entity->mZOld + (entity->mZ - entity->mZOld) * partialTick);
    Particle::setCamerViewDirection(entity->getViewVector(partialTick));

    const float cosYaw = Mth::cos(entity->mYRot * Mth::DEG_TO_RAD);
    const float sinYaw = Mth::sin(entity->mYRot * Mth::DEG_TO_RAD);
    const float cameraYOff = -sinYaw * Mth::sin(entity->mXRot * Mth::DEG_TO_RAD);
    const float cameraXOff = cosYaw * Mth::sin(entity->mXRot * Mth::DEG_TO_RAD);
    const float cosPitch = Mth::cos(entity->mXRot * Mth::DEG_TO_RAD);

    int dimension;
    if (level->mDimension->getType() == DimensionType::OVERWORLD)
    {
        dimension = 0;
    }
    else
    {
        dimension = 2;
        if (level->mDimension->getType() == DimensionType::NETHER)
        {
            dimension = 1;
        }
    }

    if (layer != 0)
    {
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::alphaFunc(4, 0.003921569f);
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    ParticleList& list = particles[dimension][3][layer];
    if (!list.empty())
    {
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        for (int i = 0; i < list.size(); ++i)
        {
            Particle* particle = list.at(i);
            particle->render(builder, entity, partialTick, cosYaw, cosPitch, sinYaw, cameraYOff, cameraXOff);
        }
        GlStateManager::disableBlend();
        GlStateManager::depthMask(true);
        GlStateManager::alphaFunc(4, 0.1f);
    }
}

void ParticleEngine::destroy(const BlockPos& pos, const BlockState* state)
{
    Block* block = state->getBlock();
    if (block->getCraftingMaterial() != Material::AIR)
    {
        block = state->getBlock();
        state = block->fillVirtualBlockStateProperties(state, level, pos);
        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                for (int z = 0; z < 4; ++z)
                {
                    const double xOffset = (x + 0.5) * 0.25;
                    const double yOffset = (y + 0.5) * 0.25;
                    const double zOffset = (z + 0.5) * 0.25;
                    TerrainParticle* particle = new TerrainParticle(level, pos.x + xOffset, pos.y + yOffset,
                                                                    pos.z + zOffset, xOffset - 0.5, yOffset - 0.5,
                                                                    zOffset - 0.5, state);
                    add(particle->init(pos));
                }
            }
        }
    }
}

void ParticleEngine::crack(const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (state->getRenderShape() != BLOCK_RENDER_INVISIBLE)
    {
        int xt = pos.x;
        int yt = pos.y;
        int zt = pos.z;
        const AABB* shape = block->getShape(state, level, pos);
        double x = xt + random.nextDouble() * ((shape->mMaxX - shape->mMinX) - 0.2f) + 0.1f + shape->mMinX;
        double y = yt + random.nextDouble() * ((shape->mMaxY - shape->mMinY) - 0.2f) + 0.1f + shape->mMinY;
        double z = zt + random.nextDouble() * ((shape->mMaxZ - shape->mMinZ) - 0.2f) + 0.1f + shape->mMinZ;

        if (direction == Direction::DOWN)
        {
            y = yt + shape->mMinY - 0.1f;
        }
        if (direction == Direction::UP)
        {
            y = yt + shape->mMaxY + 0.1f;
        }
        if (direction == Direction::NORTH)
        {
            z = zt + shape->mMinZ - 0.1f;
        }
        if (direction == Direction::SOUTH)
        {
            z = zt + shape->mMaxZ + 0.1f;
        }
        if (direction == Direction::WEST)
        {
            x = xt + shape->mMinX - 0.1f;
        }
        if (direction == Direction::EAST)
        {
            x = xt + shape->mMaxX + 0.1f;
        }

        state = block->fillVirtualBlockStateProperties(state, level, pos);
        TerrainParticle* particle = new TerrainParticle(level, x, y, z, 0.0, 0.0, 0.0, state);
        add(particle->init(pos)->setPower(0.2f)->scale(0.6f));
    }
}

void ParticleEngine::setLevel(Level* newLevel)
{
    level = newLevel;
    if (level == NULL)
    {
        for (int dimension = 0; dimension < 3; ++dimension)
        {
            for (int textureType = 0; textureType < 5; ++textureType)
            {
                for (int list = 0; list < 2; ++list)
                {
                    particles[dimension][textureType][list].ClearParticles();
                }
            }
        }
        for (unsigned int i = 0; i < trackingEmitters.size(); ++i)
        {
            delete trackingEmitters[i];
        }
        trackingEmitters.clear();
    }
}

std::wstring ParticleEngine::countParticles()
{
    int dimension;
    if (level->mDimension->getType() == DimensionType::OVERWORLD)
    {
        dimension = 0;
    }
    else
    {
        dimension = 2;
        if (level->mDimension->getType() == DimensionType::NETHER)
        {
            dimension = 1;
        }
    }
    int count = 0;
    for (int textureType = 0; textureType < 5; ++textureType)
    {
        for (int list = 0; list < 2; ++list)
        {
            count += particles[dimension][textureType][list].size();
        }
    }
    return _toString(count);
}
