#ifndef GAME_CLIENT_RENDER_AREAEFFECTCLOUDRENDERER_HPP
#define GAME_CLIENT_RENDER_AREAEFFECTCLOUDRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ResourceLocation;

class AreaEffectCloudRenderer : public EntityRenderer
{
public:
    AreaEffectCloudRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~AreaEffectCloudRenderer();
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
