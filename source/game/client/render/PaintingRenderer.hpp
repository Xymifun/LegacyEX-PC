#ifndef GAME_CLIENT_RENDER_PAINTINGRENDERER_HPP
#define GAME_CLIENT_RENDER_PAINTINGRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Painting;
class Random;
class ResourceLocation;

class PaintingRenderer : public EntityRenderer
{
public:
    PaintingRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~PaintingRenderer();
    void setBrightness(boost::shared_ptr<Painting> painting, float x, float z);
    void renderPainting(boost::shared_ptr<Painting> painting, int width, int height, int offsetX, int offsetY);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);

private:
    Random* mRandom;
};

#endif
