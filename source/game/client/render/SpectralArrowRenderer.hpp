#ifndef GAME_CLIENT_RENDER_SPECTRALARROWRENDERER_HPP
#define GAME_CLIENT_RENDER_SPECTRALARROWRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/ArrowRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class ResourceLocation;

class SpectralArrowRenderer : public ArrowRenderer
{
public:
    SpectralArrowRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~SpectralArrowRenderer();
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
};

#endif
