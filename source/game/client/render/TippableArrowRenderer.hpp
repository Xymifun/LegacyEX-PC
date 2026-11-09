#ifndef GAME_CLIENT_RENDER_TIPPABLEARROWRENDERER_HPP
#define GAME_CLIENT_RENDER_TIPPABLEARROWRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/ArrowRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ResourceLocation;

class TippableArrowRenderer : public ArrowRenderer
{
public:
    TippableArrowRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~TippableArrowRenderer();
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
