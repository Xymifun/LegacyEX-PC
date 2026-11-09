#ifndef GAME_CLIENT_RENDER_MINECARTSPAWNERRENDERER_HPP
#define GAME_CLIENT_RENDER_MINECARTSPAWNERRENDERER_HPP

#include "game/client/render/MinecartRenderer.hpp"

class EntityRenderDispatcher;

class MinecartSpawnerRenderer : public MinecartRenderer
{
public:
    MinecartSpawnerRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~MinecartSpawnerRenderer();
};

#endif
