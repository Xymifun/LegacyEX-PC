#ifndef GAME_CLIENT_RENDER_PARROTRENDERER_HPP
#define GAME_CLIENT_RENDER_PARROTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class LivingEntity;
class Model;
class ModelDefinition;
class MultiplayerLocalPlayer;
class Parrot;
class Player;
class ResourceLocation;

extern ResourceLocation PARROT_LOCATIONS[5];

class ParrotRenderer : public MobRenderer
{
public:
    ParrotRenderer(EntityRenderDispatcher* dispatcher);

    const ResourceLocation* getSpectateTextureLocation(boost::shared_ptr<MultiplayerLocalPlayer> player);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    float getCustomBob(boost::shared_ptr<Parrot> parrot, float partialTicks);
    float getCustomBob(boost::shared_ptr<Player> player, float partialTicks);
    virtual Model* createModel(const ModelDefinition& definition);
    virtual float getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
};

#endif
