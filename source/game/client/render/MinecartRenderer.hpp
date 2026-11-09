#ifndef GAME_CLIENT_RENDER_MINECARTRENDERER_HPP
#define GAME_CLIENT_RENDER_MINECARTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class BlockState;
class Entity;
class EntityRenderDispatcher;
class Minecart;
class Model;
class ModelDefinition;
class ResourceLocation;

class MinecartRenderer : public EntityRenderer
{
public:
    MinecartRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~MinecartRenderer() {}

    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void renderMinecartContents(boost::shared_ptr<Minecart> minecart, float partialTick,
                                        const BlockState* state);
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
